function [cost,tree,costs] = pivot_bit_greedy_alg(CAM,k,improved)




a = 2^floor(log2(k));

if ( k == a || ~exist('improved','var'))
    CAMS = cell(1,k);
    costs = zeros(1,k);
    tree_pos = zeros(1,k);
    tree = zeros(1,2^k-1);
    CAMS{1} = CAM;
    costs(1) = size(CAM,1);
    tree(1) = -1;
    tree_pos(1) = 1;


    for i=2:k
        [m ind] = max(costs(1:(i-1)));

        [best_i best] = pivot_bit_get_best_bit(CAMS{ind});
        sub0 = CAMS{ind}((CAMS{ind}(:,best_i) == 0 | CAMS{ind}(:,best_i) == 2),:);
        sub1 = CAMS{ind}((CAMS{ind}(:,best_i) == 1 | CAMS{ind}(:,best_i) == 2),:);
        sub0(:,best_i) = 0;
        sub1(:,best_i) = 1;

        CAMS{ind} = sub0;
        costs(ind) = size(sub0,1);
        CAMS{i} = sub1;
        costs(i) = size(sub1,1);

        last_tree_pos = tree_pos(ind);
        tree(last_tree_pos) = best_i;
        tree(2*last_tree_pos) = -1;
        tree(2*last_tree_pos+1) = -1;
        tree_pos(ind) = 2*last_tree_pos;
        tree_pos(i) = 2*last_tree_pos+1;

    end

    

else
    des_p = (k - a) / k;
    [best_i best son] = pivot_bit_get_best_bit(CAM,des_p);
    
    
    sub0 = CAM((CAM(:,best_i) == 0 | CAM(:,best_i) == 2),:);
    sub1 = CAM((CAM(:,best_i) == 1 | CAM(:,best_i) == 2),:);
        sub0(:,best_i) = 0;
        sub1(:,best_i) = 1;
        
    if son == 0
        [cost0,tree,costs0] = pivot_bit_greedy_alg(sub0,k - a,1);
        [cost1,tree,costs1] = pivot_bit_greedy_alg(sub1,a,1);
        costs = [costs0 costs1];
        
    else
        [cost0,tree,costs0] = pivot_bit_greedy_alg(sub0,a,1);
        [cost1,tree,costs1] = pivot_bit_greedy_alg(sub1,k - a,1);
        costs = [costs0 costs1];        
        
    end
    
end

cost = max(costs);