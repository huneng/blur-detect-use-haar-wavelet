function [conf, extent, blur] = IsBlurredInner(I)
if(~exist('Decomposition', 'var'))
    Decomposition = 3;
end;
if(isempty(Decomposition))
    Decomposition = 3;
end;

[h, w] = size(I);
Emax = zeros(h/16, w/16, 3);

    
for level = 1:Decomposition
    tmp = 2^level;
    win_size = 16/tmp;
    sh = h/tmp;
    sw = w/tmp;
    
    %哈尔小波转换
    I(1:sh*2, 1:sw*2) = HaarWaveTran(I(1:sh*2, 1:sw*2), 'x');
    I(1:sh*2, 1:sw*2) = HaarWaveTran(I(1:sh*2, 1:sw*2), 'y');
    
    %figure;imshow(I);
    %Emapi计算
    Emap = calcEmap(I(1:sh*2, 1:sw*2));
    
    %Emaxi计算
	Emax(1:h/16, 1:w/16, level) = CalcEmax(Emap, win_size, level);
    
end;


[conf, extent] = DetectBlur(Emax);



