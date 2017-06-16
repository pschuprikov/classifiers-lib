import subprocess
from os import path
from tempfile import TemporaryDirectory
from collections import namedtuple, defaultdict
from oct2py import Oct2Py
from bitstring import Bits

from cls.classifiers.simple import SimpleVMREntry
from cls.optimizations.native_utils import fill_from_native
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


def palette_cbd(cls, capacities):
    """ Runs Palette's cut-based algorithm.

    WARNING: the result is not always equivalent!

    Arguments:
        cls: classifier
        capacities: switch capacities

    Returns:
        The number of rules per node or None if capacities are not satisfied

    """
    octave = Oct2Py()
    _, _, [subclassifiers] = octave.feval(
        PALETTE_TEMPLATE.format('DAG_alg'),
        _to_octave_tcam(cls), len(capacities), METIS_DIR, 20, nout=3)

    if min(capacities) < max(len(sc) for sc in subclassifiers):
        return None

    return [len(sc) for sc in subclassifiers]


def palette_pbd(cls, capacities):
    """ Runs Palette's pivot-based algorithm

    Arguments:
        cls: classifier
        capacities: switch capacities

    Returns:
        The number of rules per node or None if capacities are not satisfied

    """
    octave = Oct2Py()
    _, _, [values] = octave.feval(
        PALETTE_TEMPLATE.format('pivot_bit_greedy_alg'),
        _to_octave_tcam(cls), len(capacities), 1, nout=3
    )

    if min(capacities) < max(values):
        return None
    return values


def _is_prefix(mask):
    return all(mask[i] or not mask[i + 1] for i in range(len(mask) - 1))


def _to_rect_id(value, mask):
    if not _is_prefix(mask):
        raise ValueError("Mask is not prefix!")

    base = 2 ** (len(mask) - sum(mask)) - 1
    offset = Bits(value[:sum(mask)]).uint if sum(mask) > 0 else 0

    return base + offset

OBSRule = namedtuple('OBSRule', ['src_range_id', 'dst_range_id', 'action'])


def _to_obs(entry):
    try:
        src_range_id = _to_rect_id(entry.value[:32], entry.mask[:32])
        dst_range_id = _to_rect_id(entry.value[32:64], entry.mask[32:64])
    except ValueError:
        print(entry, entry.mask[:32], entry.mask[32:64])
        raise
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
    """ Runs a rectangle-based algorithm from "One Big Switch" abstraction

    Args:
        cls: classifier
        capacities: nodes' capacities

    Returns:
        The number of rules per node or None if method has failed
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


def _bm_place_one(cls, capacity):
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
    fill_from_native(cls_there, there)
    if len(there) == 0:
        cls_there.vmr.default_action = None

    return cls_here, cls_there


def one_bit(cls, capacities):
    """ Runs trivial distribution with one bit of metadata

    Arguments:
        cls: classifier
        capacities: switch capacities

    Returns:
        The number of rules per node or None if method has failed

    """
    num_rules_left = len(cls)
    result = []
    for capacity in capacities:
        num_here = min(num_rules_left, capacity)
        result.append(num_here)
        num_rules_left -= num_here

    if num_rules_left > 0:
        return None
    return result


def boolean_minimization(cls, capacities):
    """ Runs an algorithm based using `nop` caps and  boolean minimization

    Arguments:
        cls: classifier
        capacities: switch capacities

    Returns:
        The number of rules per node or None if method has failed

    """
    result = []
    for c in capacities:
        here, cls = _bm_place_one(cls, c)

        if here is None:
            return None

        result.append(here)

    if len(cls) != 0:
        return None

    return [len(x) for x in result]
