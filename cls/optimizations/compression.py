import p4t_native
from cls.optimizations.native_utils import fill_from_native

def try_boolean_minimization(cls):
    result = cls.subset([]);
    fill_from_native(result, p4t_native.try_boolean_minimization(cls))
    return result
