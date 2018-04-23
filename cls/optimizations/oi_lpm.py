from itertools import chain, product
from functools import reduce
from collections import namedtuple

# TODO avoid this import?
from cls.vmrs.simple import SimpleVMREntry
import p4t_native


def expand(vmr_entry, bits):
    """ Performs an expansion of set of bits in the given entry.

    Args:
        vmr_entry: VMR entry.
        bits: Bit indices where exact match is required

    Returns:
        The list of expanded entries
    """
    bits = set(bits)
    mask = []
    value_options = []
    for i, (v, m) in enumerate(zip(vmr_entry.value, vmr_entry.mask)):  # pylint: disable=invalid-name
        mask.append(m or (i in bits))
        value_options.append([v] if m or (i not in bits) else [True, False])
    return [SimpleVMREntry(tuple(value), mask, vmr_entry.action, vmr_entry.priority)
            for value in product(*value_options)]


def _chain2diffs(bitchain):
    """ Returns adjacent differences of a given bit indices chain."""
    diffs = []
    last = set()
    for support in (set(bits) for bits in bitchain):
        diffs.append(sorted(list(support - last)))
        last = support
    return diffs


def _chain2bits(bitchain, bitwidth):
    """ Returns the sequence of bit indices corresponding to a given chain."""
    return tuple(chain(*_chain2diffs(bitchain + [list(range(bitwidth))])))


def minimize_num_groups(classifier):
    """ Splits the classifier into the minimal possible number of LPM classifiers.

    Args:
        classifier: Original classifier.

    Returns:
        The list of LPM classifiers equivalent to the original.
    """
    partition, partition_indices = p4t_native.min_pmgr(classifier)

    subclassifiers = []
    for bitchain, indices in zip(partition, partition_indices):
        subclassifiers.append(classifier.subset(indices).reorder(_chain2bits(bitchain, classifier.bit_width), match_type='lpm'))

    return subclassifiers


def maximize_coverage_bounded(classifiers, max_num_groups):
    """ Maximize the number of rules covered by the limited number of LPM groups.

    Args:
        classifiers: The set of original classifiers.
        max_num_groups: The maximal number of groups.

    Returns:
        The pair of LPM subclassifiers list and the leftover subclassifiers.
    """
    n_partitions, n_partition_indices = p4t_native.min_bmgr(classifiers, max_num_groups)

    subclassifiers = []
    traditionals = []
    for classifier, (partition, partition_indices) in zip(classifiers, zip(n_partitions, n_partition_indices)):
        remaining_indices = set(range(len(classifier)))

        for bitchain, indices in zip(partition, partition_indices):
            subclassifiers.append(classifier.subset(indices).reorder(_chain2bits(bitchain, classifier.bit_width)))
            remaining_indices -= set(indices)

        traditionals.append(classifier.subset(remaining_indices))

    return subclassifiers, traditionals


LPMGroupInfo = namedtuple('LPMGroupInfo', 
    ['classifier', 'nexp_classifier', 'indices']
)

def minimize_oi_lpm(classifier, max_width, algo, max_num_groups, *,
                    max_expanded_bits=None, provide_non_expanded=False,
                    max_candidate_groups=None, max_oi_algo='top_down'):
    """ Minimizes the number of subclassifiers, which are both LPM and OI.

    Args:
        classifier: Initial classifier.
        max_width: Maximal allowed classification width in the resulting subclassifiers.
        algo: Algorithm to use (Possible values 'icnp_oi', 'incp_blockers', 'min_similarity')
        max_num_groups: Maximal allowed number of subclassifiers.
        max_expanded_bits: Maximal allowed number of expanded bits.
        provide_non_expanded: Whether non-expanded versions should be returned.
        max_candidate_groups: The total number of candidate groups
        max_oi_algo: The algorithm used for maximal OI (Possible values
            'top_down' and 'min_degree').

    Returns:
        Pair of subclassifiers list and classifier with leftover rules. If
        provide_non_expanded is True, then the third element in a tuple  is
        a list of non-expanded classifiers.
    """
    assert max_expanded_bits is not None or not provide_non_expanded
    assert (max_candidate_groups is None 
            or max_num_groups <= max_candidate_groups)

    if max_candidate_groups is None:
        max_candidate_groups = max_num_groups

    indices = list(range(len(classifier)))
    lpm_groups = []

    while len(indices) > 0 and len(lpm_groups) < max_candidate_groups:
        p4t_native.log(f"OI-LPM has started for group #{len(lpm_groups) + 1}")

        oi_bits, oi_indices = p4t_native.best_subgroup(
            classifier.subset(indices), max_width, False, algo, max_oi_algo
        )
        oi_classifier = classifier.subset(
            indices[i] for i in oi_indices
        ).reorder(oi_bits)

        if max_expanded_bits is None:
            [[bitchain]], [[lpm_indices]] = p4t_native.min_bmgr(
                [oi_classifier], 1
            )
            group = oi_classifier.subset(lpm_indices).reorder(
                _chain2bits(bitchain, oi_classifier.bit_width)
            )
            nexp_group = None
        else:
            bitchain, lpm_indices, expansions = p4t_native.min_bmgr1_w_expansions(
                oi_classifier, max_expanded_bits)

            group = oi_classifier.subset([])
            for i, exp in zip(lpm_indices, expansions):
                for entry in expand(oi_classifier[i], exp):
                    group.vmr.append(entry)

            if provide_non_expanded:
                nexp_group = oi_classifier.subset(lpm_indices).reorder(
                    _chain2bits(bitchain, oi_classifier.bit_width)
                )
            else:
                nexp_group = None

        current_indices = [indices[oi_indices[i]] for i in lpm_indices]
        lpm_groups.append(LPMGroupInfo(
            classifier=group, 
            nexp_classifier=nexp_group, 
            indices=current_indices
        ))

        indices = sorted(set(indices) - set(current_indices))

        p4t_native.log("OI decomposition has finished")
    
    lpm_groups.sort(key = lambda x: len(x.indices), reverse=True)
    selected_groups = lpm_groups[:max_num_groups]
    rest_indices = sorted(set(range(len(classifier))) - reduce(
        lambda x, y: x | y,
        (set(x.indices) for x in selected_groups),
        set()
    ))

    if provide_non_expanded:
        return (
            [x.classifier for x in selected_groups], 
            classifier.subset(rest_indices), 
            [x.nexp_classifier for x in selected_groups]
        )
    else:
        return (
            [x.classifier for x in selected_groups], 
            classifier.subset(rest_indices)
        )

OIGroupInfo = namedtuple('OIGroupInfo', ['classifier', 'indices'])

def decompose_oi(classifier, max_width, algo, max_num_groups, *, 
        only_exact=False, max_candidate_groups=None, max_oi_algo='top_down'):
    """ Decomposes given classifier into a set of order-independent subclassifiers.

    Args:
        classifier: Initial classifier.
        max_width: Maximal allowed classification width in the resulting subclassifiers.
        algo: Algorithm to use (Possible values 'icnp_oi', 'incp_blockers', 'min_similarity')
        only_exact: Whether only exact bits should be allowed (False by default).
        max_num_groups: Maximal allowed number of subclassifiers.
        max_candidate_groups: The total number of candidate groups.
        max_oi_algo: The algorithm used for maximal OI (Possible values
            'top_down' and 'min_degree').

    Returns:
        Pair of subclassifiers list and classifier with leftover rules.
    """

    assert (max_candidate_groups is None 
            or max_num_groups <= max_candidate_groups)
    if max_candidate_groups is None:
        max_candidate_groups = max_num_groups

    p4t_native.log(f"OI decomposition has started: exact={only_exact}")

    indices = list(range(len(classifier)))
    oi_groups = []
    while len(oi_groups) < max_candidate_groups and len(indices) > 0:

        oi_bits, oi_indices = p4t_native.best_subgroup(classifier.subset(indices), 
                max_width, only_exact, algo, max_oi_algo)

        current_indices = [indices[i] for i in oi_indices]
        oi_groups.append(OIGroupInfo(
            classifier=classifier.subset(oi_indices).reorder(oi_bits),
            indices=current_indices
        ))
        indices = sorted(set(indices) - set(current_indices))

    p4t_native.log("OI decomposition has completed")

    oi_groups.sort(key=lambda x: len(x.indices), reverse=True)
    selected_groups = oi_groups[:max_num_groups]
    rest_indices = sorted(set(range(len(classifier))) - reduce(
        lambda x, y: x | y,
        (set(x.indices) for x in selected_groups),
        set()
    ))

    return ([x.classifier for x in selected_groups],
            classifier.subset(rest_indices))


IncrementalBatchStats = namedtuple(
        'IncrementalBatchStats', 
        ('num_in_groups', 'num_traditional')
    )

def test_incremental(classifier, max_width, max_num_groups, max_traditional, *,
                     algo='icnp_blockers', max_oi_algo='min_degree',
                     max_candidate_groups=None):
    """ Test incremental updates for a classifier 

    Args:
        classifier: Test classifier.
        max_width: Maximal allowed classification width.
        max_num_groups: Maximal allowed number of groups.
        max_traditional: Maximal allowed size of traditional representation.
        algo: Algorithm to use (Possible values 'icnp_oi', 'incp_blockers', 'min_similarity')
        max_candidate_groups: The total number of candidate groups
        max_oi_algo: The algorithm used for maximal OI (Possible values
            'top_down' and 'min_degree').
    
    Returns:
        A list of Incremental Batch Stats
    """

    p4t_native.log("Running incremental updates...")

    incremental_stats = []
    num_added, lpm_groups, traditional = 0, [], classifier.subset([])
    while num_added < len(classifier):
        incremental, traditional = p4t_native.incremental_updates(
            classifier.subset(range(num_added, len(classifier))), 
            lpm_groups, traditional, max_traditional)
        incremental_stats.append(IncrementalBatchStats(incremental, traditional))
        num_added += incremental + traditional
        p4t_native.log(
            f"... incremental: {incremental}, traditional: {traditional}"
            f" total: {num_added}, ... {max_traditional}")

        if num_added < len(classifier):
            lpm_groups, traditional = minimize_oi_lpm(
                classifier.subset(range(num_added)), max_width, 
                algo, max_num_groups, max_oi_algo=max_oi_algo,
                max_candidate_groups=max_candidate_groups)

    return incremental_stats
