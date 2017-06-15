function [o_bits o_m] = DAG_representation_reorganize (bits,m)


num_edges = length(m.Nums);
num_vertices = size(bits,1);
o_bits = zeros(2*num_vertices,size(bits,2));
o_m.Is = zeros(1,num_edges*2);
o_m.Js = zeros(1,num_edges*2);
o_m.nums = zeros(1,num_edges*2);


new_vertices_indexes = zeros(2,num_vertices);

c_vertices = 0;
c_edges = 0;

edge_pointer = 1;
for i=1:num_vertices

    first_edge_pointer = edge_pointer;
    while edge_pointer <= num_edges && m.Is(edge_pointer) == i
        edge_pointer = edge_pointer + 1;
    end
    last_edge_pointer = edge_pointer - 1;

    connectivity = last_edge_pointer - first_edge_pointer;

    num_bits2partition = ceil(log2(max(connectivity / 8,1)));
    
    num_bits2partition = min (num_bits2partition,3);
    
    f = find (bits(i,:) == 2);

    bit_connectivity_level = zeros(1,size(bits,2));
    for j=1:length(f)
        for k=first_edge_pointer:last_edge_pointer
            if bits(m.Js(k),f(j)) < 2
                bit_connectivity_level(f(j)) = bit_connectivity_level(f(j)) + 1;
            end
        end
    end


    [sortedValues,sortIndex] = sort(bit_connectivity_level,'descend');

    best_bits = sortIndex(1:num_bits2partition);



    new_bits = npermutek([0 1],num_bits2partition);


    new_vertices_indexes(1,i) = c_vertices+1;
    new_vertices_indexes(2,i) = c_vertices+max(size(new_bits,1),1);

    for j=1:max(size(new_bits,1),1)
        c_vertices = c_vertices + 1;
        o_bits(c_vertices,:) = bits(i,:);
        if (~isempty(best_bits))
            o_bits(c_vertices,best_bits) = new_bits(j,:);
        end


        for k=first_edge_pointer:last_edge_pointer
            for t=new_vertices_indexes(1,m.Js(k)):new_vertices_indexes(2,m.Js(k))
                s = sum(o_bits(c_vertices,:) == 2 & o_bits(t,:) < 2);
                if is_match(o_bits(c_vertices,:),o_bits(t,:)) && s > 0
                    c_edges =  c_edges + 1;
                    o_m.Is(c_edges) = c_vertices;
                    o_m.Js(c_edges) = t;
                    o_m.nums(c_edges) = s;
                end
            end
        end


    end




end

o_m.Is = o_m.Is(1:c_edges);
o_m.Js = o_m.Js(1:c_edges);
o_m.nums = o_m.nums(1:c_edges);

o_m.num_vertices = c_vertices;
o_m.num_edges = c_edges;


%
% for i=1:num_edges
%     if m.nums(i)~=-1
%         c = c+ 1;
%         o_m.Is(c) = m.Is(i);
%         o_m.Js(c) = m.Js(i);
%         o_m.nums(c) = m.nums(i);
%     end
% end
%
% o_m.Is = o_m.Is(1:c);
% o_m.Js = o_m.Js(1:c);
% o_m.nums = o_m.nums(1:c);
% o_m.num_vertices = size(bits,1);
% o_m.num_edges = c;

