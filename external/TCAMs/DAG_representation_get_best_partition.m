function [cost] = DAG_representation_get_best_partition(m,k,N)


cost = N;

for i=1:N
   c1 = i;
   
   
   
   sap_cost = 0;
   for ii=1:i
      for jj=(i+1):N 
          sap_cost = sap_cost + m(ii,jj);
      end
   end
   
   
   c2 = N-i + sap_cost;
 
   cost = min(cost,max(c1,c2));
    
end

