import p4t_native
from cls.optimizations.compression import try_boolean_minimization


def split_shared_unshared(cls, is_shareable):
    """ Given a predicate for shareable actions compute shared and unshared parts.

    Args:
        cls: classifier
        is_shareable: shareable action predicate (function)

    Returns: pair of shared and unshared subclassifiers
    """
    shared = cls.subset(range(len(cls)))
    unshared = cls

    for i in range(len(cls)):
        if is_shareable(cls[i].action):
            unshared.vmr[i] = unshared[i]._replace(action=None)
        else:
            shared.vmr[i] = shared[i]._replace(action=None)
    shared = try_boolean_minimization(shared)
    unshared = try_boolean_minimization(unshared)
    return shared, unshared


def weight_action_obstruction(cls):
    """ Given a classifier, weight its actions according to obstruction

    Args:
        cls: classifier

    Returns:
        a dictionary mapping actions to their weights
    """
    return p4t_native.calc_obstruction_weights(cls)

