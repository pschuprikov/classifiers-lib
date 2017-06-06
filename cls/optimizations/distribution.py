from cls.classifiers.simple import SimpleVMREntry
import p4t_native

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
