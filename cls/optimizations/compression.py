import p4t_native
from cls.optimizations.native_utils import fill_from_native


def try_boolean_minimization(cls, use_resolution=False):
    """ Applies boolean minimization to a given classifier

    Args:
        cls: classifier to minimize
        use_resolution: use resolution technique (slower)

    Returns: possibly minimized version of the given classifier
    """
    p4t_native.log(f"Starting boolean minimization for {len(cls)} rules")
    result = cls.subset([])
    fill_from_native(result, 
                     p4t_native.try_boolean_minimization(cls, use_resolution))
    return result
