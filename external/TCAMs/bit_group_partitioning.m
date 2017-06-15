function nums = bit_group_partitioning (TCAM, part)





t = ceil(log2(part));

poses = nchoosek(1:size(TCAM,2),t);

best = inf;


all = size(TCAM,1);
%
% s = sum(TCAM==2);
%
%
% b = zeros(1,t);



%
% for i =1:t
%     [temp b(i)] = min(s);
%     s(b(i)) = inf;
% end
bins = npermutek([0 1],t);
for j = 1:size(poses,1)
    
    b = poses(j,:);
    
    rules = zeros(1,part);
    
    for i =1:part
        bin = bins(i,:);
        flag = ones(all,1);
        for k=1:t
            flag = flag & (TCAM(:,b(k)) == 2 | TCAM(:,b(k)) == bin(k) );
        end
        rules(i) = sum(flag);
    end
    
    if (max(rules) < best)
        best = max(rules);
        bests = b;
        nums = rules;
    end
end
