function [m] = DAG_representation_get(CAM,file_name)

if exist('file_name','var')
    
    last_dot = find (file_name == '.',1,'last');
    raw_DAG_file_name = [file_name(1:last_dot-1) '_raw_DAG.' file_name(last_dot+1:end)];
end

if exist('file_name','var') && exist (raw_DAG_file_name,'file')
    temp = dlmread(raw_DAG_file_name);
    if (size(temp,2) == 1)
        t = temp;
        as = length(temp)/3;
        
        temp = zeros(3,as);
        temp (1,1:as) = t(1:as);
        temp (2,1:as) = t(as+1:2*as);
        temp (3,1:as) = t(2*as+1:3*as);
    end
    if (~isempty(temp))
        m.Is = temp (1,:);
        m.Js = temp (2,:);
        m.Nums = temp (3,:);
    else
        m.Is = [];
        m.Js = [];
        m.Nums = [];
        
    end
else
    
    N = size(CAM,1);
    
    Is = zeros(1,500000);
    Js = zeros(1,500000);
    Nums = zeros(1,500000);
    
    c = 0;
    
    for i=2:N
        %i
        for j=1:(i-1)
            if is_match(CAM(i,:),CAM(j,:))
                c = c + 1;
                Is(c) = i;
                Js(c) = j;
                Nums(c) = sum(CAM(i,:) == 2 &  CAM(j,:) < 2);
                if (Nums(c) == 0)
                    Nums(c) = -1;
                end
            end
        end
    end
    
    m.Is = Is(1:c);
    m.Js = Js(1:c);
    m.Nums = Nums(1:c);

    if exist('file_name','var')
        dlmwrite(raw_DAG_file_name,[m.Is ; m.Js ; m.Nums]);
    end
    
end
