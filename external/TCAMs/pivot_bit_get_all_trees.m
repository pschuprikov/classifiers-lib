function trees = pivot_bit_get_all_trees(k,left_k,ind)
  if (left_k == 1)
        trees = zeros(1,2^k-1);
        trees(ind) = -1;
  else
     
      c = 0;
      trees = zeros(300,2^k-1);
      for i=1:(left_k-1)
        left_trees = pivot_bit_get_all_trees(k,i,ind*2);
        right_trees = pivot_bit_get_all_trees(k,left_k-i,ind*2+1);
        
        for ii=1:size(left_trees,1)
           for jj=1:size(right_trees,1)
               c= c+1;
               trees(c,ind) = 1;
               trees(c,:) = trees(c,:) + left_trees(ii,:) + right_trees(jj,:);
            
           end 
        end
        
      end
      trees = trees(1:c,:);
  end