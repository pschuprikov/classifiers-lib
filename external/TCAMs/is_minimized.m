function [b] = is_minimized(bits)

pos = npermutek([0 1],size(bits,2));



mask0 = zeros(size(bits));
mask1 = zeros(size(bits));
rule_used = zeros(1,size(bits,1));

for i=1:size(pos,1)
    j = 1;
    while j<=size(bits,1) && ~is_match(pos(i,:),bits(j,:))
        j = j + 1;
    end
    if j<=size(bits,1)
        rule_used(j) = rule_used(j) + 1;
        mask0(j,pos(i,:)==0) = 1;
        mask1(j,pos(i,:)==1) = 1;
    end
end

if any(rule_used==0)
    b = 0;
else
    if all(all((mask0 & mask1 & (bits == 2)) == (bits==2)))
        b = 1;
    else
        b= 0;
    end


end