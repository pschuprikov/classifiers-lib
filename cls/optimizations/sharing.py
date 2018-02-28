import p4t_native
from cls.optimizations.compression import try_boolean_minimization


def split_shared_unshared(cls, is_shareable, use_resolution=False):
    """ Given a predicate for shareable actions compute shared and unshared parts.

    Args:
        cls: classifier
        is_shareable: shareable action predicate (function)
        use_resolution: use resolution technique? (slower)

    Returns: pair of shared and unshared subclassifiers
    """
    shared = cls.subset(range(len(cls)))
    unshared = cls

    for i in range(len(cls)):
        if is_shareable(cls[i].action):
            unshared.vmr[i] = unshared[i]._replace(action=None)
        else:
            shared.vmr[i] = shared[i]._replace(action=None)
    shared = try_boolean_minimization(shared, use_resolution)
    unshared = try_boolean_minimization(unshared, use_resolution)
    return shared, unshared


def weight_action_obstruction(cls):
    """ Given a classifier, weight its actions according to obstruction

    Args:
        cls: classifier

    Returns:
        a dictionary mapping actions to their weights
    """
    p4t_native.log(f'running weight obstruction calculation for {len(cls)} rules')
    result = p4t_native.calc_obstruction_weights(cls)
    for e in cls:
        if e.action not in result:
            result[e.action] = 0
    return result

