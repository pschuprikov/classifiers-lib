function [bits o_m original_rules] = DAG_representation_reorganize2 (bits,m,metis_path,k,max_vertex_cost)

orig_num_rules = size(bits,1);

p_k = k;
num_edges = length(m.Nums);
num_vertices = size(bits,1);
o_bits = zeros(2*num_vertices,size(bits,2));
o_m.Is = zeros(1,num_edges*2);
o_m.Js = zeros(1,num_edges*2);
o_m.nums = zeros(1,num_edges*2);
original_rules = 1:num_vertices;

c = 0;


to_remove = zeros(1,1000);
to_remove_counter = 0;

for i=1:num_edges
    if m.Nums(i)~=-1
        c = c+ 1;
        o_m.Is(c) = m.Is(i);
        o_m.Js(c) = m.Js(i);
        o_m.nums(c) = m.Nums(i);
    else
        to_remove_counter = to_remove_counter  + 1;
        to_remove(to_remove_counter) = m.Is(i);
    end
end

o_m.Is = o_m.Is(1:c);
o_m.Js = o_m.Js(1:c);
o_m.nums = o_m.nums(1:c);
o_m.num_vertices = num_vertices;
o_m.num_edges = c;
num_edges = o_m.num_edges;



new_indexes = zeros(1,num_vertices);
cur_index = 0;
for i=1:num_vertices
    if sum(i == to_remove(1:to_remove_counter)) == 0
        cur_index = cur_index  + 1;
        new_indexes(i) = cur_index;
    end
end

original_rules = sort(setdiff(original_rules,to_remove(1:to_remove_counter)));

bits = bits(original_rules,:);

to_remove_edges = zeros(1,1000);
to_remove_edges_counter = 0;
for i=1:num_edges
    if any (o_m.Is(i) ==   to_remove(1:to_remove_counter)) || any (o_m.Js(i) ==   to_remove(1:to_remove_counter))
        to_remove_edges_counter = to_remove_edges_counter + 1;
        to_remove_edges(to_remove_edges_counter) = i;
    end
end

in_edges = sort(setdiff(1:num_edges,to_remove_edges(1:to_remove_edges_counter)));

o_m.Is = o_m.Is(in_edges);
o_m.Js = o_m.Js(in_edges);
o_m.nums = o_m.nums(in_edges);
o_m.num_vertices = length(original_rules);
o_m.num_edges = length(in_edges);
num_edges = o_m.num_edges;
num_vertices = o_m.num_vertices;
for i=1:o_m.num_edges
    o_m.Is(i) = new_indexes(o_m.Is(i));
    o_m.Js(i) = new_indexes(o_m.Js(i));
end

flag = 1;

m = o_m;
while (flag)

    DAG_representation_create_metis_input_file(o_m,['input_file.txt']);


    if (m.num_edges ~=0)

        o_original_rules = zeros(1,num_vertices*2);


        [er output] = system ([metis_path '/gpmetis ' 'input_file.txt ' num2str(p_k)])

        [partition_list] = DAG_representation_read_metis_ouput_file (['input_file.txt.part.' num2str(p_k)]);


        vertices_costs = zeros(1,m.num_vertices);

        for i=1:m.num_edges
            if (partition_list(m.Is(i)) ~= partition_list(m.Js(i)))
                vertices_costs(m.Is(i)) = vertices_costs(m.Is(i)) + m.nums(i);
            end
        end

        [problematic_w problematic_vertex] = max(vertices_costs);
        problematic_w_saved = problematic_w;
        
        x = hist(partition_list,0:p_k-1);

        [t1 t2] = max(x);
        [t3 t4] = min(x);
        
        most_load = t1/(num_vertices/p_k);
        
        
        %ff = most_load >= 1.02 && t1-t3 >= max(1,orig_num_rules * p_k / 10); 
        ff = most_load >= 1.02 && t1-t3 >= 30; 
        if  ff
            vertices_costs = zeros(1,m.num_vertices);
            for i=1:m.num_edges
                if (partition_list(m.Is(i)) == partition_list(m.Js(i)) && partition_list(m.Is(i)) == t2-1)
                    vertices_costs(m.Is(i)) = vertices_costs(m.Is(i)) + m.nums(i);
                end
            end
            [problematic_w problematic_vertex] = max(vertices_costs);
        end

        %if ((sum(vertices_costs) < 300 && max(vertices_costs) < 50 && ~ff) || (ff && problematic_w ==0))
        if ((max(vertices_costs) < max_vertex_cost && ~ff) || (ff && problematic_w ==0))
            if ((ff && problematic_w ==0) && problematic_w_saved > 200)
                vertices_costs = zeros(1,m.num_vertices);

                for i=1:m.num_edges
                    if (partition_list(m.Is(i)) ~= partition_list(m.Js(i)))
                        vertices_costs(m.Is(i)) = vertices_costs(m.Is(i)) + m.nums(i);
                    end
                end

                [problematic_w problematic_vertex] = max(vertices_costs);

            else

                flag = 0;
                break;
            end
        end

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


            if (i==problematic_vertex)

                num_bits2partition = max(1,num_bits2partition);

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

            else
                num_bits2partition = 0;
                best_bits = [];
                new_bits = [];

            end


            new_vertices_indexes(1,i) = c_vertices+1;
            new_vertices_indexes(2,i) = c_vertices+max(size(new_bits,1),1);

            for j=1:max(size(new_bits,1),1)

                saved_c_vertices = c_vertices;
                saved_c_edges = c_edges;

                c_vertices = c_vertices + 1;
                o_bits(c_vertices,:) = bits(i,:);
                o_original_rules(c_vertices) = original_rules(i);
                if (~isempty(best_bits))
                    o_bits(c_vertices,best_bits) = new_bits(j,:);
                end

                flag2 = 1;

                for k=first_edge_pointer:last_edge_pointer
                    if (flag2 == 0)
                        break;
                    end
                    for t=new_vertices_indexes(1,m.Js(k)):new_vertices_indexes(2,m.Js(k))

                        if all((o_bits(c_vertices,:) == 1) <= (o_bits(t,:) == 1 | o_bits(t,:) == 2)) ...
                                && all((o_bits(c_vertices,:) == 0) <= (o_bits(t,:) == 0 | o_bits(t,:) == 2)) ...
                                && all((o_bits(c_vertices,:) == 2) <= (o_bits(t,:) == 2 ))
                            c_vertices =  saved_c_vertices;
                            c_edges = saved_c_edges;
                            new_vertices_indexes(2,i) = new_vertices_indexes(2,i)-1;
                            flag2 = 0;
                            break;
                        end

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

        num_vertices = o_m.num_vertices;
        num_edges = o_m.num_edges;

        bits = o_bits(1:c_vertices,:);
        o_bits = zeros(2*num_vertices,size(bits,2));
        m = o_m;


        original_rules = o_original_rules;

    else

        n = o_m.num_vertices;
        i = k;
        dag_costs = zeros(1,k);
        while i > 0

            dag_costs(k-i+1) = ceil(n/i);

            n = n - dag_costs(k-i+1);
            i = i - 1;
        end
        dag_cost = max(dag_costs);

        flag = 0;
    end



end

original_rules = original_rules(1:o_m.num_vertices);







