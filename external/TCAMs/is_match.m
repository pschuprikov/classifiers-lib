function b = is_match (str,rule)
b = 1;
for i=1:length(str)
    if (str(i) ~= rule(i) && rule(i) ~=2 && str(i)~=2)
        b = 0;
        break;
    end
    
end
