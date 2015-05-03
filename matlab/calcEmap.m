function Emap = calcEmap(I)
[height, width] = size(I);

half_height = height/2;
half_width  = width/2;

Emap = zeros(half_height, half_width);

for y = 1:half_height
    for x = 1:half_width
        tmp = double(I(y, x+half_width))^2 ...
        +double(I(y+half_height, x))^2 ...
        +double(I(y+half_height, x+half_width))^2;
        Emap(y, x) = sqrt(tmp);
    end;
end;