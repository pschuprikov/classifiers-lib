

clear all;

N = 7;
M = 5;
k = 4;

trees = pivot_bit_get_all_trees(k,k,1);
poses = npermutek(1:M,k-1);

for i=1:100000
    bits = unidrnd(3,[N M])-1;
    %bits(N,:) = 2*ones(1,M);
    
    if ~is_minimized(bits)
        continue;
    end
    
    [greedy_cost greedy_tree] = pivot_bit_greedy_alg(bits,k);

    [optimal_cost optimal_tree] = pivot_bit_find_optimal_tree(bits,trees,poses,k);
    
    if (greedy_cost~=optimal_cost && optimal_tree(1) < greedy_tree(1))
       a = 1  
    end
    
    
end
    
    