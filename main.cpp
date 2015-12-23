#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FIXED_SIZE 512
#define EDGE_THRESH 35
#define MIN_ZERO 0.05

#define SHOW_IMAGE(x) {cv::imshow(#x, x); cv::waitKey();}


typedef struct {
    int width;
    int height;
    int stride;
    int *data;
}HuMat;


void create_humat(HuMat &mat, int width, int height)
{
    mat.width = width;
    mat.height = height;

    mat.stride = ((width + 3) >> 2) << 2;

    mat.data = (int*)malloc(sizeof(int) * mat.stride * mat.height);
}


void free_humat(HuMat &mat)
{
    free(mat.data);
    mat.data = NULL;
}


void show_humat(const char *winName, HuMat &src)
{
    int width = src.width;
    int height = src.height;

    cv::Mat res(height, width, CV_8UC1, cv::Scalar(0));

    int *ptrSrc = src.data;

    uchar *ptrRes = res.data;

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
            ptrRes[x] = abs(ptrSrc[x]);

        ptrSrc += src.stride;
        ptrRes += res.step;
    }

    cv::imshow(winName, res);
    cv::waitKey();
}


void haar_wavelet_transform(HuMat &img, HuMat &res)
{
    int height = img.height;
    int width = img.width;

    HuMat centerMat;

    create_humat(centerMat, width, height);

    int *ptrSrc = img.data;
    int *ptrRes = centerMat.data;

    int stride1 = img.stride;
    int stride2 = centerMat.stride;

    int len = width/2;

    assert(height % 2 == 0 && width % 2 == 0);

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x += 2)
        {
            int a = ptrSrc[x];
            int b = ptrSrc[x+1];

            int idx = x >> 1;

            ptrRes[idx] = (a+b)/2;
            ptrRes[idx + len] = (a-b)/2;
        }

        ptrSrc += stride1;
        ptrRes += stride2;
    }

    create_humat(res, width, height);

    len = height/2;

    ptrSrc = centerMat.data;
    ptrRes = res.data;

    stride1 = centerMat.stride;
    stride2 = res.stride;

    for(int y = 0; y < height; y += 2)
    {
        for(int x = 0; x < width; x++)
        {
            int a = ptrSrc[x];
            int b = ptrSrc[stride1 + x];

            ptrRes[x] = (a+b)/2;
            ptrRes[len * stride2 + x] = (a-b)/2;
        }

        ptrSrc += 2 * stride1;
        ptrRes += stride2;
    }

    free_humat(centerMat);
}


void calc_emap(HuMat &src, HuMat &res)
{
    int height = src.height;
    int width = src.width;

    int halfH = height >> 1;
    int halfW = width >> 1;

    int *ptrSrc, *ptrRes;

    create_humat(res, halfW, halfH);

    int stride1 = src.stride;
    int stride2 = res.stride;

    ptrSrc = src.data;
    ptrRes = res.data;

    for(int y = 0; y < halfH; y++)
    {
        for(int x = 0; x < halfW; x++)
        {
            int HL = ptrSrc[y * stride1 + x + halfW];
            int LH = ptrSrc[(y + halfH) * stride1 + x];
            int HH = ptrSrc[(y + halfH) * stride2 + x + halfW];

            ptrRes[x] = sqrt(HL * HL + LH * LH + HH * HH);
        }

        ptrRes += stride2;
    }
}


void calc_emax(HuMat &emap, HuMat &emax, int winSize)
{
    int width = emap.width;
    int height = emap.height;

    int mH = height / winSize;
    int mW = width / winSize;

    create_humat(emax, mW, mH);

    int* ptrEmap = emap.data;
    int* ptrEmax = emax.data;

    int stride1 = emap.stride;
    int stride2 = emax.stride;

    for(int y = 0; y < height; y++)
    {
        int y_ = y / winSize;
        int idx = y_ * stride2;

        for(int x = 0; x < width; x++)
        {
            int x_ = x / winSize;

            if(ptrEmap[x] > ptrEmax[idx + x_])
                ptrEmax[idx + x_] = ptrEmap[x];
        }

        ptrEmap += stride1;
    }
}


int blur_detect(HuMat &img, float *conf)
{
    HuMat haarRes[3];
    HuMat emap[3];
    HuMat emax[3];

    int width = img.width;
    int height = img.height;
    int stride = img.stride;

    int Nedge, Nda, Nrg, Nbrg;

    int *emax0, *emax1, *emax2;

    HuMat src = img;

    printf("BEGIN ...\n");

    assert(width % 8 == 0 && height % 8 == 0);

    for(int i = 0; i < 3; i++)
    {
        show_humat("src", src);
        haar_wavelet_transform(src, haarRes[i]);
        show_humat("haar res", haarRes[i]);

        calc_emap(haarRes[i], emap[i]);
        show_humat("edge map", emap[i]);

        calc_emax(emap[i], emax[i], (1 << (3-i)));
        show_humat("edge max", emax[i]);

        src.width = haarRes[i].width >> 1;
        src.height = haarRes[i].height >> 1;
        src.stride = haarRes[i].stride;
        src.data = haarRes[i].data;
    }

    for(int i = 0; i < 3; i++)
    {
        free_humat(haarRes[i]);
        free_humat(emap[i]);
    }

    Nedge = Nda = Nrg = Nbrg = 0;

    width  = emax[0].width;
    height = emax[0].height;
    stride = emax[0].stride;

    emax0 = emax[0].data;
    emax1 = emax[1].data;
    emax2 = emax[2].data;

    for(int l = 0; l < height; l++)
    {
        for(int k = 0; k < width; k++)
        {
            if(emax0[k] <= EDGE_THRESH &&
                emax1[k] <= EDGE_THRESH &&
                emax2[k] <= EDGE_THRESH)
                continue;

            Nedge ++;

            if(emax0[k] > emax1[k] && emax1[k] > emax2[k])
                Nda ++;

            else //if(emax0[k] < emax1[k] && emax1[k] < emax2[k])
            {
                Nrg++;
                if(emax0[k] < EDGE_THRESH)
                    Nbrg++;
            }
        }

        emax0 += stride;
        emax1 += stride;
        emax2 += stride;
    }


    float per = 1.0 * Nda / Nedge;

    *conf = 1.0 * Nbrg / Nrg;

    printf("%d %d %d %d\n", Nda, Nrg, Nbrg, Nedge);
    printf("%f %f\n", per, *conf);

    if(per < 0.01) return 1;

    return 0;
}


int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Usage: %s [image]\n", argv[0]);
        return 1;
    }

    cv::Mat img = cv::imread(argv[1], 0);
    float conf = 0;

    if(img.empty())
    {
        printf("Can't open image %s\n", argv[1]);
        return 1;
    }

    cv::resize(img, img, cv::Size(FIXED_SIZE, FIXED_SIZE));

    int rows = img.rows;
    int cols = img.cols;

    HuMat src;

    create_humat(src, cols, rows);

    uchar *ptrImg = img.data;
    int *ptrSrc = src.data;

    for(int y = 0; y < rows; y++)
    {
        for(int x = 0; x < cols; x++)
            ptrSrc[x] = ptrImg[x];

        ptrSrc += src.stride;
        ptrImg += img.step;
    }

    float confidence = 0;

    int ret = blur_detect(src, &confidence);

    return 0;
}



