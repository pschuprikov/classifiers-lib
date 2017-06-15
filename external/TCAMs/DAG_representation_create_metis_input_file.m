function [] = DAG_representation_create_metis_input_file(or_m,path)


num_edges = or_m.num_edges;
num_vertices = or_m.num_vertices;


temp = [[or_m.Is(1:num_edges) or_m.Js(1:num_edges)];[or_m.Js(1:num_edges) or_m.Is(1:num_edges)];[or_m.nums(1:num_edges) or_m.nums(1:num_edges)]]';

temp = sortrows(temp);

all_m.Is = temp(:,1);
all_m.Js = temp(:,2);
all_m.nums = temp(:,3);

num_all_edges = length(all_m.nums);


fileID = fopen(path,'w');
fprintf(fileID,'%d %d %s\n',num_vertices,num_edges,'001');


edge_pointer = 1;
for i=1:num_vertices

    while edge_pointer <= num_all_edges && all_m.Is(edge_pointer) == i
        
        if (all_m.nums(edge_pointer)~=-1)
        fprintf(fileID, '%d %d ', all_m.Js(edge_pointer) , all_m.nums(edge_pointer));
        end
        edge_pointer = edge_pointer + 1;

    end



    fprintf(fileID, ' \n');
end

fclose(fileID);