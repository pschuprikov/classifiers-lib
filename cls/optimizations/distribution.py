from cls.classifiers.simple import SimpleVMREntry
import p4t_native

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
    fill_from_native(cls_there, there)
    if len(there) == 0:
        cls_there.vmr.default_action = None

    return cls_here, cls_there


def ours(cls, capacities):
    result = []
    for c in capacities:
        here, cls = place_one(cls, c)

        if here is None:
            print(cls.vmr)
            return None

        print(sum(1 for x in here if x.action is not None),
              sum(1 for x in cls if x.action is not None))

        result.append(here)

    if len(cls) != 0:
        return None

    return [len(x) for x in result]
