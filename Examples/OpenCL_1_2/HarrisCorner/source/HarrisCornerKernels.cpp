#include "OglView.h"

#define OCL_PROGRAM_SOURCE(s) #s

using namespace Ocl;

const std::string OglView::sSource = OCL_PROGRAM_SOURCE(

kernel void gradient(read_only image2d_t inpImg, write_only image2d_t outImg)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

    float ix = -read_imagef(inpImg, sampler, (int2)(x, y)).x;
    float iy = read_imagef(inpImg, sampler, (int2)(x, y)).x;
    iy += (2.0f*read_imagef(inpImg, sampler, (int2)(x + 1, y)).x);
    ix += read_imagef(inpImg, sampler, (int2)(x + 2, y)).x;
    iy += read_imagef(inpImg, sampler, (int2)(x + 2, y)).x;

    ix -= (2.0f*read_imagef(inpImg, sampler, (int2)(x, y + 1)).x);
    ix += (2.0f*read_imagef(inpImg, sampler, (int2)(x + 2, y + 1)).x);

    ix -= read_imagef(inpImg, sampler, (int2)(x, y + 2)).x;
    iy -= read_imagef(inpImg, sampler, (int2)(x, y + 2)).x;
    iy -= (2.0f*read_imagef(inpImg, sampler, (int2)(x + 1, y + 2)).x);
    ix += read_imagef(inpImg, sampler, (int2)(x + 2, y + 2)).x;
    iy -= read_imagef(inpImg, sampler, (int2)(x + 2, y + 2)).x;
    write_imagef(outImg, (int2)(x, y), (float4)(ix, iy, 0.0f, 0.0f));
}

inline float gauss_smooth(local float sh_data1[BLK_SIZE_Y + 4][BLK_SIZE_X + 4], local float sh_data2[BLK_SIZE_Y + 4][BLK_SIZE_X + 4])
{
    const int m = get_local_id(0);
    const int n = get_local_id(1);
    float idata = ((2.0f / 159.0f)*sh_data1[n][m] * sh_data2[n][m]);
    idata += ((4.0f / 159.0f)*sh_data1[n][m + 1] * sh_data2[n][m + 1]);
    idata += ((5.0f / 159.0f)*sh_data1[n][m + 2] * sh_data2[n][m + 2]);
    idata += ((4.0f / 159.0f)*sh_data1[n][m + 3] * sh_data2[n][m + 3]);
    idata += ((2.0f / 159.0f)*sh_data1[n][m + 4] * sh_data2[n][m + 4]);
    idata += ((4.0f / 159.0f)*sh_data1[n + 1][m] * sh_data2[n + 1][m]);
    idata += ((9.0f / 159.0f)*sh_data1[n + 1][m + 1] * sh_data2[n + 1][m + 1]);
    idata += ((12.0f / 159.0f)*sh_data1[n + 1][m + 2] * sh_data2[n + 1][m + 2]);
    idata += ((9.0f / 159.0f)*sh_data1[n + 1][m + 3] * sh_data2[n + 1][m + 3]);
    idata += ((4.0f / 159.0f)*sh_data1[n + 1][m + 4] * sh_data2[n + 1][m + 4]);
    idata += ((5.0f / 159.0f)*sh_data1[n + 2][m] * sh_data2[n + 2][m]);
    idata += ((12.0f / 159.0f)*sh_data1[n + 2][m + 1] * sh_data2[n + 2][m + 1]);
    idata += ((15.0f / 159.0f)*sh_data1[n + 2][m + 2] * sh_data2[n + 2][m + 2]);
    idata += ((12.0f / 159.0f)*sh_data1[n + 2][m + 3] * sh_data2[n + 2][m + 3]);
    idata += ((5.0f / 159.0f)*sh_data1[n + 2][m + 4] * sh_data2[n + 2][m + 4]);
    idata += ((4.0f / 159.0f)*sh_data1[n + 3][m] * sh_data2[n + 3][m]);
    idata += ((9.0f / 159.0f)*sh_data1[n + 3][m + 1] * sh_data2[n + 3][m + 1]);
    idata += ((12.0f / 159.0f)*sh_data1[n + 3][m + 2] * sh_data2[n + 3][m + 2]);
    idata += ((9.0f / 159.0f)*sh_data1[n + 3][m + 3] * sh_data2[n + 3][m + 3]);
    idata += ((4.0f / 159.0f)*sh_data1[n + 3][m + 4] * sh_data2[n + 3][m + 4]);
    idata += ((2.0f / 159.0f)*sh_data1[n + 4][m] * sh_data2[n + 4][m]);
    idata += ((4.0f / 159.0f)*sh_data1[n + 4][m + 1] * sh_data2[n + 4][m + 1]);
    idata += ((5.0f / 159.0f)*sh_data1[n + 4][m + 2] * sh_data2[n + 4][m + 2]);
    idata += ((4.0f / 159.0f)*sh_data1[n + 4][m + 3] * sh_data2[n + 4][m + 3]);
    idata += ((2.0f / 159.0f)*sh_data1[n + 4][m + 4] * sh_data2[n + 4][m + 4]);
    return idata;
}

kernel void eigen(read_only image2d_t inpImg, write_only image2d_t outImg)
{
    local float sh_img_data_x[BLK_SIZE_Y + 4][BLK_SIZE_X + 4];
    local float sh_img_data_y[BLK_SIZE_Y + 4][BLK_SIZE_X + 4];

    {
        const int x = (get_group_id(0)*get_local_size(0)) - 2;
        const int y = (get_group_id(1)*get_local_size(1)) - 2;
        const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

        for (int j = 0; j < 2; j++)
        {
            int n = get_local_id(1) + (j*get_local_size(1));
            if (n < (BLK_SIZE_Y + 4))
            {
                for (int i = 0; i < 2; i++)
                {
                    int m = get_local_id(0) + (i*get_local_size(0));
                    if (m < (BLK_SIZE_X + 4))
                    {
                        int2 coord = (int2)(x + m, y + n);
                        float2 idata = read_imagef(inpImg, sampler, coord).xy;
                        sh_img_data_x[n][m] = idata.x;
                        sh_img_data_y[n][m] = idata.y;
                        coord.x += get_local_size(0);
                    }
                }
            }
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    float ix2 = gauss_smooth(sh_img_data_x, sh_img_data_x);
    float iy2 = gauss_smooth(sh_img_data_y, sh_img_data_y);
    float ixiy = gauss_smooth(sh_img_data_x, sh_img_data_y);

    float detA = (ix2*iy2) - (ixiy*ixiy);
    float traceA = (ix2 + iy2);
    float mValue = detA - (0.04*traceA*traceA);
    write_imagef(outImg, (int2)(get_global_id(0), get_global_id(1)), mValue);
}

kernel void nms(read_only image2d_t inpImg, write_only image2d_t outImg, float limit)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

    float data = read_imagef(inpImg, sampler, (int2)(x, y)).x;
    if (read_imagef(inpImg, sampler, (int2)(x - 1, y)).x >= data)
    {
        data = 0.0f;
        goto exit;
    }

    if (read_imagef(inpImg, sampler, (int2)(x - 1, y - 1)).x >= data)
    {
        data = 0.0f;
        goto exit;
    }

    if (read_imagef(inpImg, sampler, (int2)(x - 1, y + 1)).x >= data)
    {
        data = 0.0f;
        goto exit;
    }

    if (read_imagef(inpImg, sampler, (int2)(x, y - 1)).x >= data)
    {
        data = 0.0f;
        goto exit;
    }

    if (read_imagef(inpImg, sampler, (int2)(x, y + 1)).x >= data)
    {
        data = 0.0f;
        goto exit;
    }

    if (read_imagef(inpImg, sampler, (int2)(x + 1, y - 1)).x >= data)
    {
        data = 0.0f;
        goto exit;
    }

    if (read_imagef(inpImg, sampler, (int2)(x + 1, y)).x >= data)
    {
        data = 0.0f;
        goto exit;
    }

    if (read_imagef(inpImg, sampler, (int2)(x + 1, y + 1)).x >= data)
    {
        data = 0.0f;
        goto exit;
    }

    if (data >= limit)
    {
        data = 1.0f;
    }
exit:
    write_imagef(outImg, (int2)(x, y), data);
}

inline void crossCoords(int i, int2 pos, int w, int h, global float2* coord)
{
    coord[i] = (float2)((float)(pos.x - 4) / (float)w, (float)pos.y / (float)h);
    coord[i + 1] = (float2)((float)(pos.x + 4) / (float)w, (float)pos.y / (float)h);
    coord[i + 2] = (float2)((float)pos.x / (float)w, (float)(pos.y - 4) / (float)h);
    coord[i + 3] = (float2)((float)pos.x / (float)w, (float)(pos.y + 4) / (float)h);
}

kernel void extractCoords(global int2 *p_pos_corner, int count, global float2* coord, int width, int height)
{
    const int i = get_global_id(0);
    if (i > count) return;
    int2 corner = p_pos_corner[i];
    int2 pos = (int2)(corner.x - width, height - corner.y);
    crossCoords(4 * i, pos, width, height, coord);
}

);
