clear all;


%file_name = '..\..\..\common\TCAM_files\IPv6\acl1_output.txt';
metis_path = 'C:\metis-5.0.2\';
%TCAM = read_rules_file (file_name);

file_name = '..\..\..\common\TCAM_files\Anat\f01.tcam';
TCAM = read_rules_file (file_name);
% file_name = '..\..\..\common\TCAM_files\Anat\f01.tcam';
% TCAM01 = read_rules_file (file_name);
% file_name = '..\..\..\common\TCAM_files\Anat\f02.tcam';
% TCAM02 = read_rules_file (file_name);
% file_name = '..\..\..\common\TCAM_files\Anat\f05.tcam';
% TCAM03 = read_rules_file (file_name);
% file_name = '..\..\..\common\TCAM_files\Anat\f07.tcam';
% TCAM04 = read_rules_file (file_name);
% file_name = '..\..\..\common\TCAM_files\Anat\f08.tcam';
% TCAM05 = read_rules_file (file_name);
% file_name = '..\..\..\common\TCAM_files\Anat\f12.tcam';
% TCAM06 = read_rules_file (file_name);
% file_name = '..\..\..\common\TCAM_files\Anat\f13.tcam';
% TCAM07 = read_rules_file (file_name);
% file_name = '..\..\..\common\TCAM_files\Anat\f18.tcam';
% TCAM08 = read_rules_file (file_name);
% file_name = '..\..\..\common\TCAM_files\Anat\f19.tcam';
% TCAM09 = read_rules_file (file_name);
% 
% %TCAM = [TCAM00 ;TCAM01 ;TCAM02 ;TCAM03 ;TCAM04 ;TCAM05 ;TCAM06 ;TCAM07 ;TCAM08 ;TCAM09] ;
% 
% TCAM = [TCAM00 ;TCAM08 ;TCAM05 ;TCAM03 ;TCAM04] ;




%num_rules = bit_group_partitioning(TCAM,k);


ks = 2:8;
gc = zeros(1,length(ks));
dc = zeros(1,length(ks));
bgp = zeros(1,length(ks));
i = 0;
for k = ks
    i = i+ 1;
%[greedy_cost greedy_tree greedy_costs] = pivot_bit_greedy_alg(TCAM,k);
[greedy_improved_cost(i) greedy_tree greedy_improved_costs] = pivot_bit_greedy_alg(TCAM,k,1);
[dag_cost(i) dag_costs TCAMS original_rules] = DAG_alg(TCAM,k,metis_path,20,file_name);
%[dag_cost(i) dag_costs TCAMS original_rules] = DAG_alg(TCAM,k,metis_path);
%if (k ==2 || k == 4 || k ==8)
%    bgp(i) = max(bit_group_partitioning (TCAM,k));
%end
%if ~verify_DAG_alg(TCAMS,original_rules,TCAM,k)
%    return;
%end

greedy_improved_cost(i) = size(TCAM,1) / (greedy_improved_cost(i) * k)
dag_cost(i) = size(TCAM,1) / (dag_cost(i) * k)
bgp(i) = size(TCAM,1) / (bgp(i) * k)
end



fig = figure(1);
plot (ks,greedy_improved_cost,'k-',ks,dag_cost,'k--',ks([1 3 7]),bgp([1 3 7]),'kx')
legend ('PBD','CBD','bit gropus',3);
xlabel('number of partitions (c)');
ylabel('quality');
axis ([2 8 0.2 1]);
scrsz = get(fig,'OuterPosition');
set(fig,'OuterPosition',[scrsz(1) scrsz(2) 460 260]);




%for acl1:
% dag_costs =
%         4565        4496        4800
% greedy_costs =
%         6855        3449        3444
% optimal_cost k = 3 and greedy  =
% 
%         4630
% 
% optimal_tree =
%    135    -1    10     0     0    -1    -1





%for acl5:
% dag_costs =
%         4565        4496        4800
% greedy_costs =
%         6855        3449        3444
