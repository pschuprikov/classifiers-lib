from cls.classifiers.simple import SimpleVMREntry
from cls.optimizations.native_utils import fill_from_native
import p4t_native


def _bm_place_one(cls, capacity, use_resolution):
    """
    
    Args:
        cls: 
        capacity: 
        use_resolution: use resolution technique? (slower)

    Returns:

    """
    here, there = p4t_native.split(cls, capacity, use_resolution)

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


def boolean_minimization(cls, capacities, use_resolution=False):
    """ Runs an algorithm based using `nop` caps and  boolean minimization

    Arguments:
        cls: classifier
        capacities: switch capacities
        use_resolution: use resolution technique? (slower)

    Returns:
        The number of rules per node or None if method has failed

    """
    result = []
    for c in capacities:
        here, cls = _bm_place_one(cls, c, use_resolution)

        if here is None:
            return None

        result.append(here)

    if len(cls) != 0:
        return None

    return [len(x) for x in result]
