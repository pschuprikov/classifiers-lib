function [x] = verify_DAG_alg(TCAMS,original_rules,bits,k)
x = 1;

for i=1:size(bits,1)
    str = bits(i,:);
    str(str==2) = unidrnd(2,1,sum(str==2))-1;
    
    hit_rule = -1;
    j = 0;
    while ( j < size(bits,1) && hit_rule == -1)
        j = j + 1;
        if is_match(bits(j,:),str)
            hit_rule = j;
        end
    end
    
    hit_rule_TCAMS = -ones(1,k);
    for t=1:k
        
        j = 0;
        while ( j < size(TCAMS{t},1) && hit_rule_TCAMS(t) == -1)
            j = j + 1;
            if is_match(TCAMS{t}(j,:),str)
                hit_rule_TCAMS(t) = original_rules{t}(j);
            end
        end    
        
    end
    
    if ~((hit_rule > 0 && sum(hit_rule_TCAMS == hit_rule)==1 && all(hit_rule_TCAMS==-1 | hit_rule_TCAMS==hit_rule)) || (hit_rule == -1 && all(hit_rule_TCAMS==-1))) 
        x = 0;
        break
    end
end

