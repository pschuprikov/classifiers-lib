function [min_cost best_tree] = pivot_bit_find_optimal_tree(CAM,trees,poses,k)


    min_cost = size(CAM,1);
    
    for ii=1:size(trees,1)
        for jj = 1:size(poses,1)
            if (mod(jj,100)==0)
            jj
            end
            tree = trees(ii,:);
            tree(tree==1) = poses(jj,:);
            cost2 = pivot_bit_run_tree(CAM,tree,k);
            if (min_cost > cost2)
                min_cost = cost2
                best_tree = tree;
            end
        end
    end