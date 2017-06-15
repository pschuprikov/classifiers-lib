

clear all;

k = 3;


TCAM = read_rules_file ('..\..\TCAM_files\IPv6\acl1_output.txt');
%num_rules = bit_group_partitioning(TCAM,k);

trees = pivot_bit_get_all_trees(k,k,1);

M = size(TCAM,2);
poses = npermutek(1:M,k-1);

[greedy_cost greedy_tree] = pivot_bit_greedy_alg(TCAM,k);

[optimal_cost optimal_tree] = pivot_bit_find_optimal_tree_k3(TCAM,trees,poses);



