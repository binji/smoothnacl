/** \file
 * \brief Convolution Operations
 *
 * See Copyright Notice in im_lib.h
 */


#include <im.h>
#include <im_util.h>
#include <im_complex.h>
#include <im_math_op.h>
#include <im_image.h>
#include <im_kernel.h>

#include "im_process_counter.h"
#include "im_process_loc.h"
#include "im_process_pnt.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>


template <class T> 
static T iKernelTotal(T* map, int w, int h)
{
  T total = 0;
  int count = w*h;
  for(int i = 0; i < count; i++) 
    total += map[i];
  if (total == 0)
    total = 1;
  return total;
}

template <class T> 
static T iKernelTotalH(T* map, int w, int h)
{
  T total = 0;
  for(int i = 0; i < h; i++) 
    total += map[i*w];  // Only for the first column
  if (total == 0)
    total = 1;
  return total;
}

template <class T> 
static T iKernelTotalW(T* map, int w)
{
  T total = 0;
  for(int i = 0; i < w; i++) 
    total += map[i];  // Only for the first line
  if (total == 0)
    total = 1;
  return total;
}

/* Rotating Kernels
3x3
  6 7 8   7 8 5
  3 4 5   6 4 2
  0 1 2   3 0 1

5x5
  20 21 22 23 24   22 23 24 19 14
  15 16 17 18 19   21 17 18 13  9
  10 11 12 13 14   20 16 12  8  4
   5  6  7  8  9   15 11  6  7  3
   0  1  2  3  4   10  5  0  1  2

7x7
  42 43 44 45 46 47 48     45 46 47 48 41 34 27
  35 36 37 38 39 40 41     44 38 39 40 33 26 20
  28 29 30 31 32 33 34     43 37 31 32 25 19 13
  21 22 23 24 25 26 27     42 36 30 24 18 12  6
  14 15 16 17 18 19 20     35 29 23 16 17 11  5
   7  8  9 10 11 12 13     28 22 15  8  9 10  4
   0  1  2  3  4  5  6     21 14  7  0  1  2  3

 TO DO: a generic odd rotation function...
*/

template <class KT> 
static void iKernelRotate(KT* kernel_map, int kernel_size)
{
  KT temp;

  switch (kernel_size)
  {
  case 3:
    {
      temp = kernel_map[0];
      kernel_map[0] = kernel_map[3];
      kernel_map[3] = kernel_map[6];
      kernel_map[6] = kernel_map[7];
      kernel_map[7] = kernel_map[8];
      kernel_map[8] = kernel_map[5];
      kernel_map[5] = kernel_map[2];
      kernel_map[2] = kernel_map[1];
      kernel_map[1] = temp;
    }
    break;
  case 5:
    {
      temp = kernel_map[0];
      kernel_map[0] = kernel_map[10];
      kernel_map[10] = kernel_map[20];
      kernel_map[20] = kernel_map[22];
      kernel_map[22] = kernel_map[24];
      kernel_map[24] = kernel_map[14];
      kernel_map[14] = kernel_map[4];
      kernel_map[4] = kernel_map[2];
      kernel_map[2] = temp;

      temp = kernel_map[5];
      kernel_map[5] = kernel_map[15];
      kernel_map[15] = kernel_map[21];
      kernel_map[21] = kernel_map[23];
      kernel_map[23] = kernel_map[19];
      kernel_map[19] = kernel_map[9];
      kernel_map[9] = kernel_map[3];
      kernel_map[3] = kernel_map[1];
      kernel_map[1] = temp;

      temp = kernel_map[6];
      kernel_map[6] = kernel_map[11];
      kernel_map[11] = kernel_map[16];
      kernel_map[16] = kernel_map[17];
      kernel_map[17] = kernel_map[18];
      kernel_map[18] = kernel_map[13];
      kernel_map[13] = kernel_map[8];
      kernel_map[8] = kernel_map[7];
      kernel_map[7] = temp;
    }
    break;
  case 7:
    {
      temp = kernel_map[2];
      kernel_map[2] = kernel_map[7];
      kernel_map[7] = kernel_map[28];
      kernel_map[28] = kernel_map[43];
      kernel_map[43] = kernel_map[46];
      kernel_map[46] = kernel_map[41];
      kernel_map[41] = kernel_map[20];
      kernel_map[20] = kernel_map[5];
      kernel_map[5] = temp;

      temp = kernel_map[1];
      kernel_map[1] = kernel_map[14];
      kernel_map[14] = kernel_map[35];
      kernel_map[35] = kernel_map[44];
      kernel_map[44] = kernel_map[47];
      kernel_map[47] = kernel_map[34];
      kernel_map[34] = kernel_map[13];
      kernel_map[13] = kernel_map[4];
      kernel_map[4] = temp;

      temp = kernel_map[0];
      kernel_map[0] = kernel_map[21];
      kernel_map[21] = kernel_map[42];
      kernel_map[42] = kernel_map[45];
      kernel_map[45] = kernel_map[48];
      kernel_map[48] = kernel_map[27];
      kernel_map[27] = kernel_map[6];
      kernel_map[6] = kernel_map[3];
      kernel_map[3] = temp;

      temp = kernel_map[9];
      kernel_map[9] = kernel_map[15];
      kernel_map[15] = kernel_map[29];
      kernel_map[29] = kernel_map[37];
      kernel_map[37] = kernel_map[39];
      kernel_map[39] = kernel_map[33];
      kernel_map[33] = kernel_map[19];
      kernel_map[19] = kernel_map[11];
      kernel_map[11] = temp;

      temp = kernel_map[8];
      kernel_map[8] = kernel_map[22];
      kernel_map[22] = kernel_map[36];
      kernel_map[36] = kernel_map[38];
      kernel_map[38] = kernel_map[40];
      kernel_map[40] = kernel_map[26];
      kernel_map[26] = kernel_map[12];
      kernel_map[12] = kernel_map[10];
      kernel_map[10] = temp;

      temp = kernel_map[16];
      kernel_map[16] = kernel_map[23];
      kernel_map[23] = kernel_map[30];
      kernel_map[30] = kernel_map[31];
      kernel_map[31] = kernel_map[32];
      kernel_map[32] = kernel_map[25];
      kernel_map[25] = kernel_map[18];
      kernel_map[18] = kernel_map[17];
      kernel_map[17] = temp;
    }
    break;
  }
}

void imProcessRotateKernel(imImage* kernel)
{
  if (kernel->data_type == IM_INT)
    iKernelRotate((int*)kernel->data[0], kernel->width);
  else
    iKernelRotate((float*)kernel->data[0], kernel->width);
}

template <class T, class KT, class CT> 
static int DoCompassConvolve(T* map, T* new_map, int width, int height, KT* orig_kernel_map, int kernel_size, int counter, CT)
{
  KT total, *kernel_line;

  // duplicate the kernel data so we can rotate it
  int ksize = kernel_size*kernel_size*sizeof(KT);
  KT* kernel_map = (KT*)malloc(ksize);
  memcpy(kernel_map, orig_kernel_map, ksize);

  int ks2 = kernel_size/2;

  total = iKernelTotal(kernel_map, kernel_size, kernel_size);

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for(int j = 0; j < height; j++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int new_offset = j * width;

    for(int i = 0; i < width; i++)
    {
      CT max_value = 0;

      for(int k = 0; k < 8; k++) // Rotate 8 times
      {
        CT value = 0;
      
        for(int y = -ks2; y <= ks2; y++)
        {
          int offset;

          kernel_line = kernel_map + (y+ks2)*kernel_size;

          if (j + y < 0)             // pass the bottom border
            offset = -(y + j + 1) * width;
          else if (j + y >= height)  // pass the top border
            offset = (2*height - 1 - (j + y)) * width;
          else
            offset = (j + y) * width;

          for(int x = -ks2; x <= ks2; x++)
          {
            if (i + x < 0)            // pass the left border
              value += kernel_line[x+ks2] * map[offset - (i + x + 1)];
            else if (i + x >= width)  // pass the right border
              value += kernel_line[x+ks2] * map[offset + 2*width - 1 - (i + x)];
            else if (offset != -1)
              value += kernel_line[x+ks2] * map[offset + (i + x)];
          }
        }

        if (abs_op(value) > max_value)
          max_value = abs_op(value);

        iKernelRotate(kernel_map, kernel_size);
      }  

      max_value /= total;

      int size_of = sizeof(imbyte);
      if (sizeof(T) == size_of)
        new_map[new_offset + i] = (T)IM_BYTECROP(max_value);
      else
        new_map[new_offset + i] = (T)max_value;
    }    

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  free(kernel_map);
  return processing;
}

int imProcessCompassConvolve(const imImage* src_image, imImage* dst_image, imImage *kernel)
{
  int ret = 0;

  int counter = imProcessCounterBegin("Compass Convolution");
  const char* msg = (const char*)imImageGetAttribute(kernel, "Description", NULL, NULL);
  if (!msg) msg = "Filtering...";
  imCounterTotal(counter, src_image->depth*src_image->height, msg);

  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      if (kernel->data_type == IM_INT)
        ret = DoCompassConvolve((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, counter, (int)0);
      else
        ret = DoCompassConvolve((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, counter, (float)0);
      break;                                                                                
    case IM_SHORT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoCompassConvolve((short*)src_image->data[i], (short*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, counter, (int)0);
      else
        ret = DoCompassConvolve((short*)src_image->data[i], (short*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, counter, (float)0);
      break;                                                                                
    case IM_USHORT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoCompassConvolve((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, counter, (int)0);
      else
        ret = DoCompassConvolve((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, counter, (float)0);
      break;                                                                                
    case IM_INT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoCompassConvolve((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, counter, (int)0);
      else
        ret = DoCompassConvolve((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, counter, (float)0);
      break;                                                                                
    case IM_FLOAT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoCompassConvolve((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, counter, (float)0);
      else
        ret = DoCompassConvolve((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, counter, (float)0);
      break;                                                                                
    }
    
    if (!ret) 
      break;
  }

  imProcessCounterEnd(counter);

  return ret;
}

template <class T, class KT, class CT> 
static int DoConvolveDual(T* map, T* new_map, int width, int height, KT* kernel_map1, KT* kernel_map2, int kernel_width, int kernel_height, int counter, CT)
{
  KT total1, total2, *kernel_line;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  total1 = iKernelTotal(kernel_map1, kernel_width, kernel_height);
  total2 = iKernelTotal(kernel_map2, kernel_width, kernel_height);

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for(int j = 0; j < height; j++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int new_offset = j * width;

    for(int i = 0; i < width; i++)
    {
      CT value1 = 0;
      CT value2 = 0;
    
      for(int y = -kh2; y <= kh2; y++)
      {
        int offset, x;

        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        kernel_line = kernel_map1 + (y+kh2)*kernel_width;
        for(x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value1 += kernel_line[x+kw2] * map[offset - (i + x + 1)];
          else if (i + x >= width)  // pass the right border
            value1 += kernel_line[x+kw2] * map[offset + 2*width - 1 - (i + x)];
          else if (offset != -1)
            value1 += kernel_line[x+kw2] * map[offset + (i + x)];
        }

        kernel_line = kernel_map2 + (y+kh2)*kernel_width;
        for(x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value2 += kernel_line[x+kw2] * map[offset - (i + x + 1)];
          else if (i + x >= width)  // pass the right border
            value2 += kernel_line[x+kw2] * map[offset + 2*width - 1 - (i + x)];
          else if (offset != -1)
            value2 += kernel_line[x+kw2] * map[offset + (i + x)];
        }
      }
      
      value1 /= total1;
      value2 /= total2;

      CT value = (CT)sqrt((double)(value1*value1 + value2*value2));

      int size_of = sizeof(imbyte);
      if (sizeof(T) == size_of)
        new_map[new_offset + i] = (T)IM_BYTECROP(value);
      else
        new_map[new_offset + i] = (T)value;
    }    

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  return processing;
}

template <class KT> 
static int DoConvolveDualCpx(imcfloat* map, imcfloat* new_map, int width, int height, KT* kernel_map1, KT* kernel_map2, int kernel_width, int kernel_height, int counter)
{
  KT total1, total2, *kernel_line;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  total1 = iKernelTotal(kernel_map1, kernel_width, kernel_height);
  total2 = iKernelTotal(kernel_map2, kernel_width, kernel_height);

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for(int j = 0; j < height; j++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int new_offset = j * width;

    for(int i = 0; i < width; i++)
    {
      imcfloat value1 = 0;
      imcfloat value2 = 0;
    
      for(int y = -kh2; y <= kh2; y++)
      {
        int offset, x;

        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        kernel_line = kernel_map1 + (y+kh2)*kernel_width;
        for(x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value1 += map[offset - (i + x + 1)] * (float)kernel_line[x+kw2];
          else if (i + x >= width)  // pass the right border
            value1 += map[offset + 2*width - 1 - (i + x)] * (float)kernel_line[x+kw2];
          else if (offset != -1)
            value1 += map[offset + (i + x)] * (float)kernel_line[x+kw2];
        }

        kernel_line = kernel_map2 + (y+kh2)*kernel_width;
        for(x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value2 += map[offset - (i + x + 1)] * (float)kernel_line[x+kw2];
          else if (i + x >= width)  // pass the right border
            value2 += map[offset + 2*width - 1 - (i + x)] * (float)kernel_line[x+kw2];
          else if (offset != -1)
            value2 += map[offset + (i + x)] * (float)kernel_line[x+kw2];
        }
      }
      
      value1 /= (float)total1;
      value2 /= (float)total2;

      new_map[new_offset + i] = sqrt(value1*value1 + value2*value2);
    }    

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  return processing;
}

int imProcessConvolveDual(const imImage* src_image, imImage* dst_image, const imImage *kernel1, const imImage *kernel2)
{
  int counter = imProcessCounterBegin("Convolution");
  const char* msg = (const char*)imImageGetAttribute(kernel1, "Description", NULL, NULL);
  if (!msg) msg = "Filtering...";
  imCounterTotal(counter, src_image->depth*src_image->height, msg);

  int ret = 0;

  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      if (kernel1->data_type == IM_INT)
        ret = DoConvolveDual((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel1->data[0], (int*)kernel2->data[0], kernel1->width, kernel1->height, counter, (int)0);
      else
        ret = DoConvolveDual((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel1->data[0], (float*)kernel2->data[0], kernel1->width, kernel1->height, counter, (float)0);
      break;                                                                                
    case IM_SHORT:                                                                           
      if (kernel1->data_type == IM_INT)
        ret = DoConvolveDual((short*)src_image->data[i], (short*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel1->data[0], (int*)kernel2->data[0], kernel1->width, kernel1->height, counter, (int)0);
      else
        ret = DoConvolveDual((short*)src_image->data[i], (short*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel1->data[0], (float*)kernel2->data[0], kernel1->width, kernel1->height, counter, (float)0);
      break;                                                                                
    case IM_USHORT:                                                                           
      if (kernel1->data_type == IM_INT)
        ret = DoConvolveDual((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel1->data[0], (int*)kernel2->data[0], kernel1->width, kernel1->height, counter, (int)0);
      else
        ret = DoConvolveDual((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel1->data[0], (float*)kernel2->data[0], kernel1->width, kernel1->height, counter, (float)0);
      break;                                                                                
    case IM_INT:                                                                           
      if (kernel1->data_type == IM_INT)
        ret = DoConvolveDual((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel1->data[0], (int*)kernel2->data[0], kernel1->width, kernel1->height, counter, (int)0);
      else
        ret = DoConvolveDual((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel1->data[0], (float*)kernel2->data[0], kernel1->width, kernel1->height, counter, (float)0);
      break;                                                                                
    case IM_FLOAT:                                                                           
      if (kernel1->data_type == IM_INT)
        ret = DoConvolveDual((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel1->data[0], (int*)kernel2->data[0], kernel1->width, kernel1->height, counter, (float)0);
      else
        ret = DoConvolveDual((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel1->data[0], (float*)kernel2->data[0], kernel1->width, kernel1->height, counter, (float)0);
      break;                                                                                
    case IM_CFLOAT:            
      if (kernel1->data_type == IM_INT)
        ret = DoConvolveDualCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel1->data[0], (int*)kernel2->data[0], kernel1->width, kernel1->height, counter);
      else
        ret = DoConvolveDualCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel1->data[0], (float*)kernel2->data[0], kernel1->width, kernel1->height, counter);
      break;
    }
    
    if (!ret) 
      break;
  }

  imProcessCounterEnd(counter);

  return ret;
}

template <class T, class KT, class CT> 
static int DoConvolve(T* map, T* new_map, int width, int height, KT* kernel_map, int kernel_width, int kernel_height, int counter, CT)
{
  KT total, *kernel_line;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  total = iKernelTotal(kernel_map, kernel_width, kernel_height);

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for(int j = 0; j < height; j++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int new_offset = j * width;

    for(int i = 0; i < width; i++)
    {
      CT value = 0;
    
      for(int y = -kh2; y <= kh2; y++)
      {
        int offset;

        kernel_line = kernel_map + (y+kh2)*kernel_width;

        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        for(int x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value += kernel_line[x+kw2] * map[offset - (i + x + 1)];
          else if (i + x >= width)  // pass the right border
            value += kernel_line[x+kw2] * map[offset + 2*width - 1 - (i + x)];
          else if (offset != -1)
            value += kernel_line[x+kw2] * map[offset + (i + x)];
        }
      }
      
      value /= total;

      int size_of = sizeof(imbyte);
      if (sizeof(T) == size_of)
        new_map[new_offset + i] = (T)IM_BYTECROP(value);
      else
        new_map[new_offset + i] = (T)value;
    }    

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  return processing;
}

template <class KT> 
static int DoConvolveCpx(imcfloat* map, imcfloat* new_map, int width, int height, KT* kernel_map, int kernel_width, int kernel_height, int counter)
{
  KT total, *kernel_line;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  total = iKernelTotal(kernel_map, kernel_width, kernel_height);

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for(int j = 0; j < height; j++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int new_offset = j * width;

    for(int i = 0; i < width; i++)
    {
      imcfloat value = 0;
    
      for(int y = -kh2; y <= kh2; y++)
      {
        int offset;

        kernel_line = kernel_map + (y+kh2)*kernel_width;

        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        for(int x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value += map[offset - (i + x + 1)] * (float)kernel_line[x+kw2];
          else if (i + x >= width)  // pass the right border
            value += map[offset + 2*width - 1 - (i + x)] * (float)kernel_line[x+kw2];
          else if (offset != -1)
            value += map[offset + (i + x)] * (float)kernel_line[x+kw2];
        }
      }
      
      value /= (float)total;

      new_map[new_offset + i] = value;
    }    

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  return processing;
}

static int DoConvolveStep(const imImage* src_image, imImage* dst_image, const imImage *kernel, int counter)
{
  int ret = 0;

  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      if (kernel->data_type == IM_INT)
        ret = DoConvolve((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolve((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_SHORT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolve((short*)src_image->data[i], (short*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolve((short*)src_image->data[i], (short*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_USHORT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolve((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolve((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_INT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolve((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolve((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_FLOAT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolve((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      else
        ret = DoConvolve((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_CFLOAT:            
      if (kernel->data_type == IM_INT)
        ret = DoConvolveCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter);
      else
        ret = DoConvolveCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter);
      break;
    }
    
    if (!ret) 
      break;
  }

  return ret;
}

int imProcessConvolve(const imImage* src_image, imImage* dst_image, const imImage *kernel)
{
  int counter = imProcessCounterBegin("Convolution");
  const char* msg = (const char*)imImageGetAttribute(kernel, "Description", NULL, NULL);
  if (!msg) msg = "Filtering...";
  imCounterTotal(counter, src_image->depth*src_image->height, msg);

  int ret = DoConvolveStep(src_image, dst_image, kernel, counter);

  imProcessCounterEnd(counter);

  return ret;
}

int imProcessConvolveRep(const imImage* src_image, imImage* dst_image, const imImage *kernel, int ntimes)
{
  imImage *AuxImage = imImageClone(dst_image);
  if (!AuxImage)
    return 0;

  int counter = imProcessCounterBegin("Repeated Convolution");
  const char* msg = (const char*)imImageGetAttribute(kernel, "Description", NULL, NULL);
  if (!msg) msg = "Filtering...";
  imCounterTotal(counter, src_image->depth*src_image->height*ntimes, msg);

  const imImage *image1 = src_image;
  imImage *image2 = dst_image;

  for (int i = 0; i < ntimes; i++)
  {
    if (!DoConvolveStep(image1, image2, kernel, counter))
    {
      imProcessCounterEnd(counter);
      imImageDestroy(AuxImage);
      return 0;
    }
    
    image1 = image2;

    if (image1 == dst_image)
      image2 = AuxImage;
    else
      image2 = dst_image;
  }

  // The result is in image1, if in the Aux swap the data
  if (image1 == AuxImage)
  {
    void** temp = (void**)dst_image->data;
    dst_image->data = AuxImage->data;
    AuxImage->data = (void**)temp;
  }

  imProcessCounterEnd(counter);
  imImageDestroy(AuxImage);

  return 1;
}

template <class T, class KT, class CT> 
static int DoConvolveSep(T* map, T* new_map, int width, int height, KT* kernel_map, int kernel_width, int kernel_height, int counter, CT)
{
  KT totalH, totalW, *kernel_line;
  T* aux_line;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  // use only the first line and the first column of the kernel

  totalH = iKernelTotalH(kernel_map, kernel_width, kernel_height);
  totalW = iKernelTotalW(kernel_map, kernel_width);

  aux_line = (T*)malloc(width*sizeof(T));

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for(int j = 0; j < height; j++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int new_offset = j * width;

    for(int i = 0; i < width; i++)
    {
      CT value = 0;

      // first pass, only for columns
    
      for(int y = -kh2; y <= kh2; y++)
      {
        int offset;

        kernel_line = kernel_map + (y+kh2)*kernel_width;  // Use only the first column

        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        if (offset != -1)
          value += kernel_line[0] * map[offset + i];
      }
      
      value /= totalH;

      int size_of = sizeof(imbyte);
      if (sizeof(T) == size_of)
        new_map[new_offset + i] = (T)IM_BYTECROP(value);
      else
        new_map[new_offset + i] = (T)value;
    }    

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  if (!processing)
  {
    free(aux_line);
    return 0;
  }

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for(int j = 0; j < height; j++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int offset = j * width;
    int new_offset = offset;

    for(int i = 0; i < width; i++)
    {
      CT value = 0;

      // second pass, only for lines, but has to use an auxiliar buffer
    
      kernel_line = kernel_map;  // Use only the first line

      for(int x = -kw2; x <= kw2; x++)
      {
        if (i + x < 0)            // pass the left border
          value += kernel_line[x+kw2] * new_map[offset - (i + x + 1)];
        else if (i + x >= width)  // pass the right border
          value += kernel_line[x+kw2] * new_map[offset + 2*width - 1 - (i + x)];
        else
          value += kernel_line[x+kw2] * new_map[offset + (i + x)];
      }
      
      value /= totalW;

      int size_of = sizeof(imbyte);
      if (sizeof(T) == size_of)
        aux_line[i] = (T)IM_BYTECROP(value);
      else
        aux_line[i] = (T)value;
    }    

    memcpy(new_map + new_offset, aux_line, width*sizeof(T));

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  free(aux_line);
  return processing;
}


template <class KT> 
static int DoConvolveSepCpx(imcfloat* map, imcfloat* new_map, int width, int height, KT* kernel_map, int kernel_width, int kernel_height, int counter)
{
  KT totalH, totalW, *kernel_line;
  imcfloat* aux_line;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  // use only the first line and the first column of the kernel

  totalH = iKernelTotalH(kernel_map, kernel_width, kernel_height);
  totalW = iKernelTotalW(kernel_map, kernel_width);

  aux_line = (imcfloat*)malloc(width*sizeof(imcfloat));

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for(int j = 0; j < height; j++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int new_offset = j * width;

    for(int i = 0; i < width; i++)
    {
      imcfloat value = 0;

      // first pass, only for columns
    
      for(int y = -kh2; y <= kh2; y++)
      {
        int offset;

        kernel_line = kernel_map + (y+kh2)*kernel_width;

        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        if (offset != -1)
          value += map[offset + i] * (float)kernel_line[0];
      }
      
      value /= (float)totalH;

      new_map[new_offset + i] = value;
    }    

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  if (!processing)
  {
    free(aux_line);
    return 0;
  }

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for(int j = 0; j < height; j++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int offset = j * width;
    int new_offset = offset;

    for(int i = 0; i < width; i++)
    {
      int x;
      imcfloat value = 0;

      // second pass, only for lines, but has to use an auxiliar buffer
    
      kernel_line = kernel_map;
    
      for(x = -kw2; x <= kw2; x++)
      {
        if (i + x < 0)            // pass the left border
          value += new_map[offset - (i + x + 1)] * (float)kernel_line[x+kw2];
        else if (i + x >= width)  // pass the right border
          value += new_map[offset + 2*width - 1 - (i + x)] * (float)kernel_line[x+kw2];
        else if (offset != -1)
          value += new_map[offset + (i + x)] * (float)kernel_line[x+kw2];
      }
      
      value /= (float)totalW;

      aux_line[i] = value;
    }    

    memcpy(new_map + new_offset, aux_line, width*sizeof(imcfloat));

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  free(aux_line);
  return processing;
}

int imProcessConvolveSep(const imImage* src_image, imImage* dst_image, const imImage *kernel)
{
  int counter = imProcessCounterBegin("Separable Convolution");
  const char* msg = (const char*)imImageGetAttribute(kernel, "Description", NULL, NULL);
  if (!msg) msg = "Filtering...";
  imCounterTotal(counter, 2*src_image->depth*src_image->height, msg);

  int ret = 0;

  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      if (kernel->data_type == IM_INT)
        ret = DoConvolveSep((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolveSep((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_SHORT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolveSep((short*)src_image->data[i], (short*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolveSep((short*)src_image->data[i], (short*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_USHORT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolveSep((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolveSep((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_INT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolveSep((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolveSep((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_FLOAT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolveSep((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      else
        ret = DoConvolveSep((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_CFLOAT:            
      if (kernel->data_type == IM_INT)
        ret = DoConvolveSepCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter);
      else
        ret = DoConvolveSepCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter);
      break;
    }
    
    if (!ret) 
      break;
  }

  imProcessCounterEnd(counter);

  return ret;
}

/*
Description:	
    Can be used to find zero crossing of second derivative,
		laplace. Can also be used to determine any other kind
		of crossing. Pixels below or equal to 't' are set if the pixel
		to the right or below is above 't', pixels above 't' are
		set if the pixel to the right or below is below or equal to
		't'. Pixels that are "set" are set to the maximum absolute
		difference of the two neighbours, to indicate the strength
		of the edge.

		| IF (crossing t)
		|   out(x,y) = MAX(ABS(in(x,y)-in(x+1,y)), ABS(in(x,y)-in(x,y+1)))
		| ELSE
		|   out(x,y) = 0

Author:		Tor Lønnestad, BLAB, Ifi, UiO

Copyright 1991, Blab, UiO
Image processing lab, Department of Informatics
University of Oslo
*/
template <class T> 
static void do_crossing(T* iband, T* oband, int width, int height, T t)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for (int y=0; y < height-1; y++)
  {
    int offset00 = y*width;
    int offset10 = (y+1)*width;
    int offset01 = offset00 + 1;

    for (int x=0; x < width-1; x++)
    {
      T v = 0;

      if (iband[offset00] <= t)
      {
        if (iband[offset10] > t) 
          v = iband[offset10]-iband[offset00];

	      if (iband[offset01] > t) 
        {
          T diff = iband[offset01]-iband[offset00];
          if (diff > v) v = diff;
        }
      }
      else
      {
	      if (iband[offset10] <= t) 
          v = iband[offset00]-iband[offset10];

	      if (iband[offset01] <= t) 
        {
          T diff = iband[offset00]-iband[offset01];
          if (diff > v) v = diff;
        }
      }

      oband[offset00] = v;

      offset00++;
      offset10++;
      offset01++;
    }

    /* last pixel on line */
    offset00++;
    offset10++;

    T v = 0;

    if (iband[offset00] <= t)
    {
      if (iband[offset10] > t)
        v = iband[offset10]-iband[offset00];
    }
    else
    {
      if (iband[offset10] <= t)
        v = iband[offset00]-iband[offset10];
    }

    oband[offset00] = v;
  }

  /* last line */
  int offset00 = (height-1)*width;
  int offset01 = offset00 + 1;

  for (int x=0; x < width-1; x++)
  {
    T v = 0;

    if (iband[offset00] <= t)
    {
      if (iband[offset01] > t)
        v = iband[offset01]-iband[offset00];
    }
    else
    {
      if (iband[offset01] <= t)
        v = iband[offset00]-iband[offset01];
    }

    oband[offset00] = v;

    offset00++;
    offset01++;
  }

  offset00++;

  /* last pixel */
  oband[offset00] = 0;
}

void imProcessZeroCrossing(const imImage* src_image, imImage* dst_image)
{
  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_INT:                                                                           
      do_crossing((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, 0);
      break;                                                                                
    case IM_FLOAT:                                                                           
      do_crossing((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, 0.0f);
      break;                                                                                
    }
  }
}

int imProcessBarlettConvolve(const imImage* src_image, imImage* dst_image, int kernel_size)
{
  imImage* kernel = imImageCreate(kernel_size, kernel_size, IM_GRAY, IM_INT);
  if (!kernel)
    return 0;

  imImageSetAttribute(kernel, "Description", IM_BYTE, -1, (void*)"Barlett");

  /* fill only the first line and the first column */
  int* kernel_data = (int*)kernel->data[0];
  int half = kernel_size / 2;
  for (int i = 0; i < kernel_size; i++)
  {
    if (i <= half)
      kernel_data[i] = i+1;
    else
      kernel_data[i] = kernel_size-i;
  }
  for (int j = 0; j < kernel_size; j++)
  {
    if (j <= half)
      kernel_data[j*kernel_size] = j+1;
    else
      kernel_data[j*kernel_size] = kernel_size-j;
  }

  int ret = imProcessConvolveSep(src_image, dst_image, kernel);

  imImageDestroy(kernel);

  return ret;
}

int imProcessSobelConvolve(const imImage* src_image, imImage* dst_image)
{
	int ret = 0;

  imImage* kernel1 = imKernelSobel();
  imImage* kernel2 = imImageCreate(3, 3, IM_GRAY, IM_INT);
  imProcessRotate90(kernel1, kernel2, 1);

  ret = imProcessConvolveDual(src_image, dst_image, kernel1, kernel2);

  imImageDestroy(kernel1);
  imImageDestroy(kernel2);

  return ret;
}

int imProcessPrewittConvolve(const imImage* src_image, imImage* dst_image)
{
	int ret = 0;

  imImage* kernel1 = imKernelPrewitt();
  imImage* kernel2 = imImageClone(kernel1);
  imProcessRotate90(kernel1, kernel2, 1);

  ret = imProcessConvolveDual(src_image, dst_image, kernel1, kernel2);

  imImageDestroy(kernel1);
  imImageDestroy(kernel2);

  return ret;
}

int imProcessSplineEdgeConvolve(const imImage* src_image, imImage* dst_image)
{
	int ret = 0;

  imImage* tmp_image = imImageClone(src_image);
  if (!tmp_image) return 0;

  imImage* kernel1 = imImageCreate(5, 5, IM_GRAY, IM_INT);
  imImageSetAttribute(kernel1, "Description", IM_BYTE, -1, (void*)"SplineEdge");

  int* kernel_data = (int*)kernel1->data[0];
  kernel_data[10] = -1;
  kernel_data[11] = 8;
  kernel_data[12] = 0;
  kernel_data[13] = -8;
  kernel_data[14] = 1;

  imImage* kernel2 = imImageClone(kernel1);
  imProcessRotate90(kernel1, kernel2, 1);

  imImage* kernel3 = imImageClone(kernel1);
  imProcessRotateKernel(kernel3);

  imImage* kernel4 = imImageClone(kernel1);
  imProcessRotate90(kernel3, kernel4, 1);

  ret = imProcessConvolveDual(src_image, tmp_image, kernel1, kernel2);
  ret = imProcessConvolveDual(src_image, dst_image, kernel3, kernel4);

  imProcessArithmeticConstOp(tmp_image, (float)sqrt(2.0), tmp_image, IM_BIN_MUL);
  imProcessArithmeticOp(tmp_image, dst_image, dst_image, IM_BIN_ADD);

  imImageDestroy(kernel1);
  imImageDestroy(kernel2);
  imImageDestroy(kernel3);
  imImageDestroy(kernel4);
  imImageDestroy(tmp_image);

  return ret;
}

int imGaussianStdDev2KernelSize(float stddev)
{
  if (stddev < 0)
    return (int)-stddev;
  else
  {
	  int width = (int)(3.35*stddev + 0.3333);
    return 2*width + 1;
  }
}

float imGaussianKernelSize2StdDev(int kernel_size)
{
  int width = (kernel_size - 1)/2;
	return (width - 0.3333f)/3.35f;
}

int imProcessGaussianConvolve(const imImage* src_image, imImage* dst_image, float stddev)
{
  int kernel_size = imGaussianStdDev2KernelSize(stddev);

  imImage* kernel = imImageCreate(kernel_size, kernel_size, IM_GRAY, IM_FLOAT);
  if (!kernel)
    return 0;

  imImageSetAttribute(kernel, "Description", IM_BYTE, -1, (void*)"Gaussian");
  imProcessRenderGaussian(kernel, stddev);

  int ret = imProcessConvolveSep(src_image, dst_image, kernel);

  imImageDestroy(kernel);

  return ret;
}

int imProcessLapOfGaussianConvolve(const imImage* src_image, imImage* dst_image, float stddev)
{
  int kernel_size = imGaussianStdDev2KernelSize(stddev);

  imImage* kernel = imImageCreate(kernel_size, kernel_size, IM_GRAY, IM_FLOAT);
  if (!kernel)
    return 0;

  imImageSetAttribute(kernel, "Description", IM_BYTE, -1, (void*)"Laplacian Of Gaussian");
  imProcessRenderLapOfGaussian(kernel, stddev);

  int ret;
  if (src_image->data_type == IM_BYTE ||  // Unsigned types
      src_image->data_type == IM_USHORT)
  {
    imImage* aux_image = imImageClone(dst_image);
    if (!aux_image)
    {
      imImageDestroy(kernel);
      return 0;
    }

    imProcessUnArithmeticOp(src_image, aux_image, IM_UN_EQL);  // Convert to IM_INT
    ret = imProcessConvolve(aux_image, dst_image, kernel);
    imImageDestroy(aux_image);
  }
  else
    ret = imProcessConvolve(src_image, dst_image, kernel);

  imImageDestroy(kernel);

  return ret;
}

int imProcessDiffOfGaussianConvolve(const imImage* src_image, imImage* dst_image, float stddev1, float stddev2)
{
  imImage* aux_image1 = imImageClone(src_image);
  imImage* aux_image2 = imImageClone(src_image);
  if (!aux_image1 || !aux_image2)
  {
    if (aux_image1) imImageDestroy(aux_image1);
    return 0;
  }

  int kernel_size1 = imGaussianStdDev2KernelSize(stddev1);
  int kernel_size2 = imGaussianStdDev2KernelSize(stddev2);
  int size = kernel_size1;
  if (kernel_size1 < kernel_size2) size = kernel_size2;

  imImage* kernel1 = imImageCreate(size, size, IM_GRAY, IM_FLOAT);
  imImage* kernel2 = imImageCreate(size, size, IM_GRAY, IM_FLOAT);
  if (!kernel1 || !kernel2)
  {
    if (kernel1) imImageDestroy(kernel1);
    if (kernel2) imImageDestroy(kernel2);
    imImageDestroy(aux_image1);
    imImageDestroy(aux_image2);
    return 0;
  }

  imImageSetAttribute(kernel1, "Description", IM_BYTE, -1, (void*)"Gaussian1");
  imImageSetAttribute(kernel2, "Description", IM_BYTE, -1, (void*)"Gaussian2");

  imProcessRenderGaussian(kernel1, stddev1);
  imProcessRenderGaussian(kernel2, stddev2);

  if (!imProcessConvolve(src_image, aux_image1, kernel1) ||
      !imProcessConvolve(src_image, aux_image2, kernel2))
  {
    imImageDestroy(kernel1);
    imImageDestroy(kernel2);
    imImageDestroy(aux_image1);
    imImageDestroy(aux_image2);
    return 0;
  }

  imProcessArithmeticOp(aux_image1, aux_image2, dst_image, IM_BIN_SUB);

  imImageDestroy(kernel1);
  imImageDestroy(kernel2);
  imImageDestroy(aux_image1);
  imImageDestroy(aux_image2);

  return 1;
}

int imProcessMeanConvolve(const imImage* src_image, imImage* dst_image, int ks)
{
  int counter = imProcessCounterBegin("Mean Convolve");
  imCounterTotal(counter, src_image->depth*src_image->height, "Filtering...");

  imImage* kernel = imImageCreate(ks, ks, IM_GRAY, IM_INT);

  int* kernel_data = (int*)kernel->data[0];

  int ks2 = ks/2;
  for(int ky = 0; ky < ks; ky++)
  {
    int ky2 = ky-ks2;
    ky2 = ky2*ky2;
    for(int kx = 0; kx < ks; kx++) 
    {
      int kx2 = kx-ks2;
      kx2 = kx2*kx2;
      int radius = imRound(sqrt(double(kx2 + ky2)));
      if (radius <= ks2)
        kernel_data[ky*ks + kx] = 1;
    }
  }

  int ret = DoConvolveStep(src_image, dst_image, kernel, counter);

  imImageDestroy(kernel);
  imProcessCounterEnd(counter);

  return ret;
}

template <class T1, class T2> 
static void DoSharpOp(T1 *src_map, T1 *dst_map, int count, float amount, T2 threshold, int gauss)
{
  T1 min, max;

  // min,max values used only for cropping
  imMinMaxType(src_map, count, min, max);

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    T2 diff;
    
    if (gauss)
      diff = 20*(src_map[i] - dst_map[i]);  /* dst_map contains a gaussian filter of the source image, must compensate for small edge values */
    else
      diff = dst_map[i];  /* dst_map contains a laplacian filter of the source image */

    if (threshold && abs_op(2*diff) < threshold)
      diff = 0;

    T2 value = (T2)(src_map[i] + amount*diff);
    if (value < min) // keep current min,max range
      value = min;
    else if (value > max)
      value = max;

    dst_map[i] = (T1)value;
  }
}

static void doSharp(const imImage* src_image, imImage* dst_image, float amount, float threshold, int gauss)
{
  int count = src_image->count;

  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      DoSharpOp((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], count, amount, (int)threshold, gauss);
      break;
    case IM_SHORT:
      DoSharpOp((short*)src_image->data[i], (short*)dst_image->data[i], count, amount, (int)threshold, gauss);
      break;
    case IM_USHORT:
      DoSharpOp((imushort*)src_image->data[i], (imushort*)dst_image->data[i], count, amount, (int)threshold, gauss);
      break;
    case IM_INT:
      DoSharpOp((int*)src_image->data[i], (int*)dst_image->data[i], count, amount, (int)threshold, gauss);
      break;
    case IM_FLOAT:
      DoSharpOp((float*)src_image->data[i], (float*)dst_image->data[i], count, amount, (float)threshold, gauss);
      break;
    }
  }
}

int imProcessUnsharp(const imImage* src_image, imImage* dst_image, float stddev, float amount, float threshold)
{
  int kernel_size = imGaussianStdDev2KernelSize(stddev);

  imImage* kernel = imImageCreate(kernel_size, kernel_size, IM_GRAY, IM_FLOAT);
  if (!kernel)
    return 0;

  imImageSetAttribute(kernel, "Description", IM_BYTE, -1, (void*)"Unsharp");
  imProcessRenderGaussian(kernel, stddev);

  int ret = imProcessConvolveSep(src_image, dst_image, kernel);
  doSharp(src_image, dst_image, amount, threshold, 1);

  imImageDestroy(kernel);

  return ret;
}

int imProcessSharp(const imImage* src_image, imImage* dst_image, float amount, float threshold)
{
  imImage* kernel = imKernelLaplacian8();
  if (!kernel)
    return 0;

  int ret = imProcessConvolve(src_image, dst_image, kernel);
  doSharp(src_image, dst_image, amount, threshold, 0);

  imImageDestroy(kernel);

  return ret;
}

static int iProcessCheckKernelType(const imImage* kernel)
{
  if (kernel->data_type == IM_INT)
  {
    int* kernel_data = (int*)kernel->data[0];
    for (int i = 0; i < kernel->count; i++)
    {
      if (kernel_data[i] < 0)   /* if there are negative values, assume kernel is an edge detector */
        return 0;
    }
  }
  else if (kernel->data_type == IM_FLOAT)
  {
    float* kernel_data = (float*)kernel->data[0];
    for (int i = 0; i < kernel->count; i++)
    {
      if (kernel_data[i] < 0)   /* if there are negative values, assume kernel is an edge detector */
        return 0;
    }
  }
  return 1;  /* default kernel is a smooth filter */
}

int imProcessSharpKernel(const imImage* src_image, const imImage* kernel, imImage* dst_image, float amount, float threshold)
{
  int ret = imProcessConvolve(src_image, dst_image, kernel);
  doSharp(src_image, dst_image, amount, threshold, iProcessCheckKernelType(kernel));
  return ret;
}

