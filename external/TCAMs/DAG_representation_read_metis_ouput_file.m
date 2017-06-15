function [partition_list] = DAG_representation_read_metis_ouput_file(path)

partition_list = dlmread(path);