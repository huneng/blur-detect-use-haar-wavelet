function res = HaarWaveTran(I, direct)

if(direct == 'y')
    I = I';
end;

[h, w] = size(I);
res = zeros(h, w);

for j = 1:h
    k = 1;
    for i = 1:2:w-1
        res(j, k) = (I(j, i+1)+I(j, i))/2;
        res(j, k+w/2) = (I(j, i+1)-I(j, i));
        k = k+1;
    end;
end;

if(direct == 'y')
    res = res';
end;