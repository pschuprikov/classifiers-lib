function [cost,costs,TCAMS,original_rules] = DAG_representation_evaluate_partition(bits,or_m,partition_list,k,original_rule)

num_vertices = or_m.num_vertices;
num_edges = or_m.num_edges;

partition_costs = zeros(1,k);



new_orig_rules = zeros(size(bits,1)*2,1);
partition_place = zeros(size(bits,1)*2,1);
TCAM_exp = zeros(size(bits,1)*2,size(bits,2));
next_to_copy = 1;


new_vertices_indexes = zeros(2,num_vertices);





TCAMS = cell(1,k);
original_rules = cell(1,k);

edge_pointer = 1;
num_vertices
for i=1:num_vertices
    if mod(i,1000) == 1
        i
    end
    %i
    local_bits = bits(i,:);
    local_bits_valid = 1;
    
    while edge_pointer <= num_edges && or_m.Is(edge_pointer) == i
        up_vertex = or_m.Js(edge_pointer);
        
        if ( partition_list(i)~=partition_list(up_vertex))
            new_local_bits = zeros(100000,size(bits,2));
            new_local_bits_valid = 0;
            %             new_local_bits = zeros(10000,size(bits,2));
            %             new_local_bits_valid = 0;
            %for jj=new_vertices_indexes(1,up_vertex) : new_vertices_indexes(2,up_vertex)
            for ii=1:local_bits_valid
                f = find ( local_bits(ii,:) == 2 & bits(up_vertex,:) ~=2);

                if all((local_bits(ii,:) == 1) <= (bits(up_vertex,:) == 1 | bits(up_vertex,:) == 2)) ...
                                && all((local_bits(ii,:) == 0) <= (bits(up_vertex,:) == 0 | bits(up_vertex,:) == 2)) ...
                                && all((local_bits(ii,:) == 2) <= (bits(up_vertex,:) == 2 ))
                    continue;
                elseif ~is_match(local_bits(ii,:),bits(up_vertex,:)) || isempty(f)
                new_local_bits_valid = new_local_bits_valid+ 1;
                new_local_bits(new_local_bits_valid,:) = local_bits(ii,:);
                %     continue;
                else
                    for kk=1:length(f)
                        new_local_bits_valid = new_local_bits_valid+ 1;
                        %new_local_bits(new_local_bits_valid,:) = local_bits(ii,:);
                        new_local_bits(new_local_bits_valid,:) = local_bits(ii,:);
                        new_local_bits(new_local_bits_valid,f(kk)) = 1 - bits(up_vertex,f(kk)); %TCAM_exp(jj,f(kk));
                    end
                end
            end
            %end
           
            
                local_bits = new_local_bits(1:new_local_bits_valid,:);
                local_bits_valid = new_local_bits_valid;
        
                local_bits = unique(local_bits(1:local_bits_valid,:),'rows');
                local_bits_valid = size(local_bits,1);
                
                to_remove = false(1,new_local_bits_valid);
                for iii=1:new_local_bits_valid
                    for jjj=1:new_local_bits_valid
                       if iii==jjj
                           continue;
                       end
                       
                       if all((local_bits(iii,:) == 1) <= (local_bits(jjj,:) == 1 | local_bits(jjj,:) == 2)) ...
                                && all((local_bits(iii,:) == 0) <= (local_bits(jjj,:) == 0 | local_bits(jjj,:) == 2)) ...
                                && all((local_bits(iii,:) == 2) <= (local_bits(jjj,:) == 2 ))
                            to_remove(iii)= true;
                       end                        
                    end
                end
                local_bits = local_bits(~to_remove,:);
                local_bits_valid = size(local_bits,1);
        end
        
        
        edge_pointer = edge_pointer + 1;
        
    end
    

    
    
    
    new_vertices_indexes(1,i) = next_to_copy;
    new_vertices_indexes(2,i) = next_to_copy+local_bits_valid-1;
    
    
    TCAM_exp(next_to_copy:next_to_copy+local_bits_valid-1,:) = local_bits;
    new_orig_rules(next_to_copy:next_to_copy+local_bits_valid-1,1) = original_rule(i);
    partition_place(next_to_copy:next_to_copy+local_bits_valid-1,1) = partition_list(i);
    
    next_to_copy = next_to_copy + local_bits_valid;
    
    partition_costs(partition_list(i) + 1) = partition_costs(partition_list(i) + 1) + local_bits_valid;
    
end



TCAMS_sizes = zeros(1,k);
for i=1:k
    TCAMS{i} = zeros(size(bits,1)*2,size(bits,2));
    original_rules{i} = zeros(size(bits,1)*2,1);
end

for i=1:num_vertices
    for kk=new_vertices_indexes(1,i) : new_vertices_indexes(2,i)
        index =  partition_list(i)+1;
        TCAMS_sizes(index) = TCAMS_sizes(index) + 1;
        TCAMS{index}(TCAMS_sizes(index),:) = TCAM_exp(kk,:);
        original_rules{index}(TCAMS_sizes(index)) = new_orig_rules(kk);
    end
end

for i=1:k
    TCAMS{i} = TCAMS{i}(1:TCAMS_sizes(i),:);
    original_rules{i} = original_rules{i}(1:TCAMS_sizes(i),1);
    
    
end







cost = max(partition_costs);
costs = partition_costs;
