function [best_i best son] = pivot_bit_get_best_bit(bits,desired_fraction)

if ~exist('desired_fraction','var')

    best = inf;
    best_second = inf;
    for i=1:size(bits,2)
        a1 = sum(bits(:,i) == 1) +  sum(bits(:,i) == 2);
        a2 = sum(bits(:,i) == 0) +  sum(bits(:,i) == 2);
        a = max(a1,a2);
        b = min(a1,a2);
        if (a<best)% || (a == best && b < best_second))
            best = a;
            best_second = b;
            best_i = i;
        end
    end
else
    best = inf;
    best_second = inf;
    son = 2;
    for i=1:size(bits,2)
        a1 = sum(bits(:,i) == 1) +  sum(bits(:,i) == 2);
        a2 = sum(bits(:,i) == 0) +  sum(bits(:,i) == 2);


        cost1 = abs(a1 / (a1 + a2) - desired_fraction);
        cost2 = abs(a2 / (a1 + a2) - desired_fraction);



        if (cost1<best)% || (a == best && b < best_second))
            best = cost1;

            best_i = i;
            son = 1;
        end

        if (cost2<best)% || (a == best && b < best_second))
            best = cost2;

            best_i = i;
            son = 0;
        end


    end



end