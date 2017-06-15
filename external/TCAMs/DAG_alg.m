function [dag_cost dag_costs TCAMS original_rules] = DAG_alg(bits,k,metis_path,max_vertex_cost,file_name)

if exist('file_name','var')
    m = DAG_representation_get(bits,file_name);
else
    m = DAG_representation_get(bits);
end

o_bits = bits;
[bits or_m original_rules] = DAG_representation_reorganize2 (bits,m,metis_path,k,max_vertex_cost);


%     if ~verify_DAG_alg({bits},{original_rules},o_bits,1)
%         return;
%         
%     end


DAG_representation_create_metis_input_file(or_m,['input_file.txt']);


if (or_m.num_edges ~=0)
    [er output] = system ([metis_path '/gpmetis ' 'input_file.txt ' num2str(k)]);

    [partition_list] = DAG_representation_read_metis_ouput_file (['input_file.txt.part.' num2str(k)]);

    [dag_cost dag_costs TCAMS original_rules]= DAG_representation_evaluate_partition(bits,or_m,partition_list,k,original_rules);

else
    
    TCAMS = cell(1,k);
    original_rules2 = original_rules;
    original_rules = cell(1,k);
    
    n = or_m.num_vertices;
    i = k;
    dag_costs = zeros(1,k);
    in = 0;
    
    while i > 0
        dag_costs(k-i+1) = ceil(n/i);
        TCAMS{k-i+1} = bits(in+1:in+dag_costs(k-i+1),:);
        original_rules{k-i+1} = original_rules2(in+1:in+dag_costs(k-i+1));
        in = in+dag_costs(k-i+1);
        n = n - dag_costs(k-i+1);
        i = i - 1;
    end
    dag_cost = max(dag_costs);
end


