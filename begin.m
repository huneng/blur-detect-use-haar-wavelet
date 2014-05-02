t0 = clock;
Threshold = 35;
MinZero = 0.05;
MaximumWidth = 256;
MaximumHeight = 256;

I = imread(img_name);
[height, width, c] = size(I);
height = height-mod(height, 16);
width = width-mod(width, 16);

dr = min(height, width);

top = (height-dr)/2;
left =(width-dr)/2;

I = I(top+1:top+dr, left+1:left+dr, 1:3);

I = rgb2gray(I);


[blur,extent] = IsBlurredInner(I);

if(blur < MinZero)
    'Image is blurring, The blur extent is '
    extent
else
    'Image is not blurring'
    extent
end;
my_clock = etime(clock, t0)