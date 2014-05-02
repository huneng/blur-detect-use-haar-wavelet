function [blur, extent]= DetectBlur(I)
if(~exist('Threshold', 'var'))
    Threshold = 35;
end;
if(isempty(Threshold))
    Threshold = 35;
end;

Nedge = 0;
Nda = 0;
Nrg = 0;
Nbrg = 0;

[height, width, level] = size(I);


for k = 1: height
    for l = 1:width
        
        emax1 = I(k, l, 1);
        emax2 = I(k, l, 2);
        emax3 = I(k, l, 3);
        
        emax = max(max(emax1, emax2), emax3);
        if emax>Threshold
            Nedge = Nedge+1;
            if emax1 > emax2 && emax2 > emax3
                Nda = Nda+1;
            end;
            
            if emax1 < emax2 && emax2 <emax3
                Nrg = Nrg + 1;
                if emax1 < Threshold
                	Nbrg = Nbrg+1;
                end;
            end;
            if emax2 > emax1 && emax2 > emax3
                Nrg = Nrg+1;
                if emax1 < Threshold
                    Nbrg = Nbrg +1;
                end;
            end;
        end;
    end;
end;

if Nedge == 0
    per = 0;
else 
    per = double(Nda);
    per = per/double(Nedge);
end;

blur = per;
extent = double(Nbrg)/double(Nrg);
