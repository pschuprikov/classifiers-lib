function [cost costs]= pivot_bit_run_tree(CAM,tree,k)

CAMS = cell(1,k);
costs = zeros(1,k);
tree_pos = zeros(1,k);

CAMS{1} = CAM;
costs(1) = size(CAM,1);
tree_pos(1) = 1;
c = 1;

for i=1:length(tree)
    if (tree(i) <= 0)
        continue;
    end
    
   ind = find (tree_pos(1:c) == i);
    
   sub0 = CAMS{ind}((CAMS{ind}(:,tree(i)) == 0 | CAMS{ind}(:,tree(i)) == 2),:);
   sub1 = CAMS{ind}((CAMS{ind}(:,tree(i)) == 1 | CAMS{ind}(:,tree(i)) == 2),:);
   sub0(:,tree(i)) = 0;
   sub1(:,tree(i)) = 1;
   
   CAMS{ind} = sub0;
   costs(ind) = size(sub0,1);
   c = c + 1;
   
   CAMS{c} = sub1;
   costs(c) = size(sub1,1);
   
   last_tree_pos = tree_pos(ind);
   tree_pos(ind) = 2*last_tree_pos;
   tree_pos(c) = 2*last_tree_pos+1;
   
end

cost = max(costs);

