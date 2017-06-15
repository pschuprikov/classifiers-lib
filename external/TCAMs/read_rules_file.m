function [TCAM] = read_rules_file (file_name)

last_dot = find (file_name == '.',1,'last');
exp_delemitied_file_name = [file_name(1:last_dot-1) '_exp_delimited.' file_name(last_dot+1:end)];

if exist (exp_delemitied_file_name,'file')
    TCAM = dlmread(exp_delemitied_file_name);
else
    
    
    %dlmwrite('..\TCAM_files\IPv6\acl1_output_exp_delimited.txt', TCAM);
    
    
    
    fid = fopen(file_name);
    
    num_orig = 0;
    
    
    TCAM = -ones(15000,256+16+16+2);
    num_rules = 0;
    tline = fgetl(fid);
    tline(ismember(tline,' ')) = [];
    while ischar(tline)
        num_orig = num_orig + 1
        a = find (tline == ':');
        
        % first and second IPs.
        ips = zeros(1,256);
        first_ip_s = tline(a(1)-4:a(7)+4);
        first_im_s = tline(a(7)+6:(a(7)+4+find(tline(a(7)+6:end) == 9,1,'first')));
        
        second_ip_s = tline(a(8)-4:a(14)+4);
        second_im_s = tline(a(14)+6:(a(14)+4+find(tline(a(14)+6:end) == 9,1,'first')));
        
        
        k = 0;
        for j=1:length(first_ip_s)
            if (first_ip_s(j)== ':')
                continue;
            end
            b = dec2bin(hex2dec(first_ip_s(j)),4);
            for kk = 1:4
                k = k + 1;
                ips(k) = str2double(b(kk));
            end
        end
        ips((str2double(first_im_s)+1):128) = 2;
        
        for j=1:length(second_ip_s)
            if (second_ip_s(j)== ':')
                continue;
            end
            b = dec2bin(hex2dec(second_ip_s(j)),4);
            for kk = 1:4
                k = k + 1;
                ips(k) = str2double(b(kk));
            end
        end
        ips((str2double(second_im_s)+1+128):256) = 2;
        
        
        
        %source port
        e = a(15) - 1;
        s = e - 1;
        while (tline(s)~=9)
            s = s - 1;
        end
        s = s + 1;
        sp_from = str2double(tline(s:e));
        
        s = a(15) + 1;
        e = s + 1;
        while (tline(e)~=9)
            e = e + 1;
        end
        e = e - 1;
        sp_to = str2double(tline(s:e));
        
        
        e = a(16) - 1;
        s = e - 1;
        while (tline(s)~=9)
            s = s - 1;
        end
        s = s + 1;
        dp_from = str2double(tline(s:e));
        
        s = a(16) + 1;
        e = s + 1;
        while (tline(e)~=9)
            e = e + 1;
        end
        e = e - 1;
        dp_to = str2double(tline(s:e));
        
        
        
        sp_rules = range_to_rules (sp_from,sp_to,16);
        dp_rules = range_to_rules (dp_from,dp_to,16);
        
        protocol = str2double(tline(e+1:e+2));
        
        
        for i=1:size(sp_rules,1)
            for j=1:size(dp_rules,1)
                num_rules = num_rules + 1
                TCAM(num_rules,1:256) = ips;
                TCAM(num_rules,256+1:256+16) = sp_rules(i,:);
                TCAM(num_rules,256+16+1:256+16+16) = dp_rules(j,:);
                TCAM(num_rules,256+16+16+1) = floor(protocol / 2);
                TCAM(num_rules,256+16+16+2) = mod(protocol,2);
            end
        end
        
        tline = fgetl(fid);
        tline(ismember(tline,' ')) = [];
    end
    
    TCAM = TCAM(1:num_rules,:);
    fclose(fid);
    
    dlmwrite(exp_delemitied_file_name,TCAM);
end

if all(TCAM(end,:) == 2)
    TCAM = TCAM(1:end-1,:);
end