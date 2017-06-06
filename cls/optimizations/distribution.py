import subprocess
from os import path
from tempfile import TemporaryDirectory
from collections import namedtuple, defaultdict
from oct2py import Oct2Py
from bitstring import Bits

from cls.classifiers.simple import SimpleVMREntry
import p4t_native

PALETTE_TEMPLATE = '/home/pschuprikov/test/TCAMs/{}.m'
METIS_DIR = '/home/pschuprikov/.nix-profile/bin'
OBS_EXEC = '/home/pschuprikov/repos/opt-big-switch/src/graph_algorithm/batch_DFS_MP'

def _to_octave_tcam(cls):
    octave_matrix = []
    for entry in cls:
        octave_matrix.append([
            v if m else 2 for v, m in zip(entry.value, entry.mask)
        ])
    return octave_matrix




def palette_cdb(cls, num_nodes):
    """ Runs Palette's cut-based algorithm.
    
    WARNING: the result is not always equivalent!
    
    Arguments:
        cls: classifier
        num_nodes: number of nodes to distribute on
        
    Returns: 
        The number of rules per node
        
    """
    octave = Oct2Py()
    _, _, [subclassifiers] = octave.feval(
        PALETTE_TEMPLATE.format('DAG_alg'),
        _to_octave_tcam(cls), num_nodes, METIS_DIR, 20, nout=3)
    return [len(sc) for sc in subclassifiers]


def palette_pdb(cls, num_nodes):
    """ Runs Palette's pivot-based algorithm
    
    Arguments: 
        cls: classifier
        num_nodes: number of nodes to distribute on
        
    Returns:
        The number of rules per node
        
    """
    octave = Oct2Py()
    _, _, [values] = octave.feval(
        PALETTE_TEMPLATE.format('pivot_bit_greedy_alg'),
        _to_octave_tcam(cls), num_nodes, 1, nout=3
    )
    return values


def _is_prefix(mask):
    return all(not mask[i] or mask[i + 1] for i in range(len(mask) - 1))


def _to_rect_id(value, mask):
    assert _is_prefix(mask)
    return 2 ** (len(mask) - sum(mask)) - 1 + Bits(value[:sum(mask)]).uint

OBSRule = namedtuple('OBSRule', ['src_range_id', 'dst_range_id', 'action'])


def _to_obs(entry):
    src_range_id = _to_rect_id(entry.value[:32], entry.mask[:32])
    dst_range_id = _to_rect_id(entry.value[32:64], entry.mask[32:64])
    return OBSRule(src_range_id, dst_range_id, entry.action)


def _to_obs_classifier(cls):
    obs_cls = defaultdict(int)
    for entry in cls:
        obs_cls[_to_obs(entry)] += 1
    return obs_cls


def _create_obs_policy_file(dir, obs_cls):
    filename = path.join(dir, 'policy.txt')
    with open(filename, 'w+') as policy_file:
        policy_file.write('1 _ {:d}\n'.format(len(obs_cls)))

        for rule, count in obs_cls.items():
            policy_file.write('{1} {2} 0 0 0 0 {0} {3}\n'.format(count, *rule))
    return filename


def _create_obs_alloc_file(dir, capacities):
    filename = path.join(dir, 'alloc.txt')
    with open(filename, 'w+') as alloc_file:
        alloc_file.write('1 {:d}\n'.format(len(capacities)))

        for c in capacities:
            alloc_file.write('{:d}\n'.format(c))
    return filename


def one_big_switch(cls, capacities):
    """
    
    Args:
        cls: classifier
        capacities: nodes' capacities

    Returns:
        True if capacities were satisfied
    """

    obs_classifier = _to_obs_classifier(cls)

    with TemporaryDirectory() as dir:
        policy_file = _create_obs_policy_file(dir, obs_classifier)
        alloc_file = _create_obs_alloc_file(dir, capacities)

        res = subprocess.run(
            [OBS_EXEC, alloc_file, policy_file],
            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
            universal_newlines=True)
        *lines, outcome, _ = res.stdout.split('\n')

        if outcome != 'OK':
            return None

        return [int(s) for s in lines]


def subsums(e1, e2):
    for (v1, m1), (v2, m2) in zip(zip(e1.value, e1.mask), zip(e2.value, e2.mask)):
        if m1 and (not m2 or v1 != v2):
            return False
    return True


def try_forward_subsumption(cls):
    active = set(range(len(cls)))

    for i, u in enumerate(cls):
        if i not in active:
            continue
        for j in range(i + 1, len(cls)):
            if subsums(u, cls[j]):
                active.remove(j)

    return cls.subset(sorted(active))


def intersects(e1, e2):
    for (v1, m1), (v2, m2) in zip(zip(e1.value, e1.mask), zip(e2.value, e2.mask)):
        if m1 and m2 and v1 != v2:
            return False
    return True


def check_no_intersection_with(entry, cls):
    for other_entry in cls:
        if intersects(entry, other_entry) and entry.action != other_entry.action:
            return False
    return True


def try_backward_subsumption(cls):
    active = set(range(len(cls)))

    for i, u in enumerate(cls):
        for j in range(i + 1, len(cls)):
            v = cls[j]
            if u.action == v.action and subsums(v, u) \
                    and check_no_intersection_with(u, cls[i + 1:j]):
                active.remove(i)
        if u.action == cls.default_action and check_no_intersection_with(u, cls[i + 1:]):
            active.remove(i)

    return cls.subset(sorted(active))


def find_resolution_bit(e1, e2):
    candidate = None
    for i, (v1, m1, v2, m2) in enumerate(zip(e1.value, e1.mask, e2.value, e2.mask)):
        if m1 != m2:
            return None
        if m1 and m2 and v1 != v2:
            if candidate is not None:
                return None
            candidate = i
    return candidate


def try_resolution(cls):
    active = {x: None for x in range(len(cls))}
    for i, u in enumerate(cls):
        if i not in active:
            continue
        for j in range(i + 1, len(cls)):
            v = cls[j]
            resolution_bit = find_resolution_bit(u, v)
            if v.action == u.action and resolution_bit is not None\
                    and check_no_intersection_with(u, cls[i + 1:j]):
                active[i] = resolution_bit
                del active[j]

    for i, res in active.items():
        if res is None:
            continue
        new_mask = cls[i].mask
        new_mask[res] = False
        cls.vmr[i] = cls[i]._replace(mask=new_mask)

    return cls.subset(active)


def boolean_minimization(cls):
    old_len = len(cls)
    while True:
        cls = try_forward_subsumption(cls)
        cls = try_backward_subsumption(cls)
        cls = try_resolution(cls)

        if len(cls) == old_len:
            break
        old_len = len(cls)
    return cls


def fill_from_native(cls, native_rules):
    for mask, value, action in native_rules:
        cls.vmr.append(SimpleVMREntry(mask, value, action, 0))


def place_one(cls, capacity):
    """
    
    Args:
        cls: 
        capacity: 

    Returns:

    """
    here, there = p4t_native.split(cls, capacity)

    if here is None:
        return None, cls

    cls_here = cls.subset([])
    fill_from_native(cls_here, here)

    cls_there = cls.subset([])
    if there is None:
        cls_there.vmr.default_action = None
    else:
        fill_from_native(cls_there, there)

    return cls_here, cls_there


def ours(cls, capacities):
    result = []
    for c in capacities:
        here, cls = place_one(cls, c)

        if here is None:
            return None

        result.append(here)

    if len(cls) != 0:
        return None

    for x in result:
        print("=======")
        print(x.vmr)
    return [len(x) for x in result]
