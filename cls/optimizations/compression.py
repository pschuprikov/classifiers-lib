import p4t_native
from cls.optimizations.native_utils import fill_from_native


def try_boolean_minimization(cls):
    """ Applies boolean minimization to a given classifier

    Args:
        cls: classifier to minimize

    Returns: possibly minimized version of the given classifier
    """
    result = cls.subset([])
    fill_from_native(result, p4t_native.try_boolean_minimization(cls))
    return result
