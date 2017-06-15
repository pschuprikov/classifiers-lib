clear all;



TCAM = read_rules_file('..\..\TCAM_files\IPv6\fw1_output.txt');
dlmwrite('..\TCAM_files\IPv6\fwl1_output_exp_delimited.txt', TCAM);

TCAM = read_rules_file('..\..\TCAM_files\IPv6\fw2_output.txt');
dlmwrite('..\TCAM_files\IPv6\fw2_output_exp_delimited.txt', TCAM);

TCAM = read_rules_file('..\..\TCAM_files\IPv6\fw3_output.txt');
dlmwrite('..\TCAM_files\IPv6\fw3_output_exp_delimited.txt', TCAM);

TCAM = read_rules_file('..\..\TCAM_files\IPv6\fw4_output.txt');
dlmwrite('..\TCAM_files\IPv6\fw4_output_exp_delimited.txt', TCAM);

TCAM = read_rules_file('..\..\TCAM_files\IPv6\fw5_output.txt');
dlmwrite('..\TCAM_files\IPv6\fw5_output_exp_delimited.txt', TCAM);
