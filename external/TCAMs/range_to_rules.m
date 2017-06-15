function rules = range_to_rules (from,to,bits)

tree = zeros(2,2^bits);

tree(1,1) = 0;
tree(2,1) = 2^bits - 1;
tree_rules = zeros(bits,2^bits);
tree_depth = zeros(1,2^bits);

to_check = zeros(1,1000);
to_check_from = 1;
to_check_to = 1;
to_check(1) = 1;


rules = zeros(1000,bits);
rules_counter = 0;

while (to_check_from <= to_check_to)
    
    node = to_check(to_check_from);
    to_check_from = to_check_from + 1;
    
    current_range_from = tree(1,node);
    current_range_to = tree(2,node);
    current_rule = tree_rules(:,node);
    current_depth = tree_depth(node);
    
    if (current_range_from >= from && current_range_to <= to)
        % add a rule to the TCAM.
        rules_counter = rules_counter + 1;
        rules ( rules_counter,:) = tree_rules(:,node)';
        rules ( rules_counter,current_depth+1:end) = 2;
        
    elseif ~(current_range_to < from || current_range_from > to)
        % go to the next two sons
        % left son
        
        left_node = 2*node;
        left_from = current_range_from;
        left_to = current_range_from + (current_range_to+1 - current_range_from)/2 - 1;
        left_depth = current_depth + 1;
        left_rule = current_rule;
        left_rule(left_depth) = 0;
        
        right_node = 2*node+1;
        right_from = current_range_from + (current_range_to+1 - current_range_from)/2;
        right_to = current_range_to;
        right_depth = current_depth + 1;
        right_rule = current_rule;
        right_rule(left_depth) = 1;
        
        
        tree(1,left_node) = left_from;
        tree(2,left_node) = left_to;
        tree_rules(:,left_node) = left_rule;
        tree_depth(left_node) = left_depth;
        
        
        tree(1,right_node) = right_from;
        tree(2,right_node) = right_to;
        tree_rules(:,right_node) = right_rule;
        tree_depth(right_node) = right_depth;
        
        to_check_to = to_check_to + 1;
        to_check(to_check_to) = left_node;
        to_check_to = to_check_to + 1;
        to_check(to_check_to) = right_node;
 
    end 
    
end

rules = rules (1:rules_counter,:);
