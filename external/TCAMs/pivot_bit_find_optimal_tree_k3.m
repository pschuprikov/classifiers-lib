function [min_cost best_tree] = pivot_bit_find_optimal_tree_k3(CAM,trees,poses)


    min_cost = size(CAM,1);
    
    for ii=1:size(trees,1)
        pointer = 0;
        while pointer <= size(poses,1)
            pointer = pointer + 1;
            if (mod(pointer,100)==0)
               pointer
            end
            tree = trees(ii,:);
            tree(tree==1) = poses(pointer,:);
            
            [cost2 costs] = pivot_bit_run_tree(CAM,tree,3);
            
            if (tree(2) == -1 && costs(1) >= min_cost || tree(3) == -1 && costs(2) >= min_cost)
               a_temp = poses(pointer,1);
               while (pointer <= size(poses,1) && poses(pointer,1) == a_temp)
                   pointer = pointer + 1;
               end
                
            end
            if (min_cost > cost2)
                min_cost = cost2
                best_tree = tree;
            end
        end
    end