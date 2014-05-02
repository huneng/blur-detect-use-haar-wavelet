function Emax = CalcEmax(I, win_size, scale)

[h, w] = size(I);
scale = 16/(2^scale);
Emax = zeros(h/scale, w/scale);

k = 1;
for i = 1:win_size: h-win_size+1
    l = 1;
	for j = 1:win_size:w-win_size+1
        win_M = I(i:i+win_size-1, j:j+win_size-1);
        Emax(k, l) = max(max(win_M));
        l = l+1;
	end;
    k = k+1;
end;
    