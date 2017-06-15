

clear all;

N = 30;
M = 12;
ks = 2:8;


c = 0;

gc = zeros(1,length(ks));
dc = zeros(1,length(ks));
bgp = zeros(1,length(ks));

up = 5;

counter = 0;

i = 0;

while (counter < 100)
    bits = unidrnd(3,[N M])-1;
    %bits(N,:) = 2*ones(1,M);
    
    counter
    if ~is_minimized(bits)
        continue;
    end
    
    counter = counter + 1
    
    i = 0;
    for k=ks
        i=i+1;
        
        
        [greedy_cost greedy_tree] = pivot_bit_greedy_alg(bits,k,1);
        
        [dag_cost dag_costs TCAMS original_rules] = DAG_alg(bits,k,'C:\metis-5.0.2\',10);
        
        if (k ==2 || k == 4 || k ==8)
            bgp_cost = max(bit_group_partitioning (bits,k));
        end
        
        if ~verify_DAG_alg(TCAMS,original_rules,bits,k)
            disp 'found an error !!!!'
            j
            return;
        end
        
        
        gc(i) = gc(i) + (N/(greedy_cost*k));
        dc(i) = dc(i) + (N/(dag_cost*k));
        bgp(i) = bgp(i) + (N/(bgp_cost*k));
        
        
        
        
    end
    
end
gc = gc / counter;
dc = dc / counter;
bgp = bgp / counter;

fig = figure(1);
plot (ks,gc,'k-',ks,dc,'k--',ks([1 3 7]),bgp([1 3 7]),'kx')
legend ('PBD','CBD','bit gropus',1);
xlabel('number of partitions (c)');
ylabel('quality');
axis ([2 8 0.3 1]);
scrsz = get(fig,'OuterPosition');
set(fig,'OuterPosition',[scrsz(1) scrsz(2) 460 260]);





