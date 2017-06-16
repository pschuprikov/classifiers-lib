import p4t_native
from cls.classifiers.simple import SimpleVMREntry


# TODO: probably there should be a pair: to/from
def fill_from_native(cls, native_rules):
    """ Fills a classifier with rules in native representation.

    Arguments:
        cls: classifier
        native_rules: rules in native representation
    """
    for mask, value, action in native_rules:
        cls.vmr.append(SimpleVMREntry(mask, value, action, 0))


def set_number_of_threads(num_threads):
    """ Sets the number of threads to be used by an optimization engine."""
    p4t_native.set_num_threads(num_threads)


