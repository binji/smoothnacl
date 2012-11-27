/** \file
 * \brief Rank Convolution Operations
 *
 * See Copyright Notice in im_lib.h
 */


#include <im.h>
#include <im_util.h>
#include <im_math.h>

#include "im_process_counter.h"
#include "im_process_loc.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>


template <class T, class DT> 
static int DoConvolveRankFunc(T *map, DT* new_map, int width, int height, int kw, int kh, T (*func)(T* value, int count, int center), int counter)
{
  int tcount = IM_MAX_THREADS;
  T* value = new T[kw*kh*tcount];

  int kh2 = kh/2;
  int kw2 = kw/2;
  int kh1 = -kh2;
  int kw1 = -kw2;
  if (kh%2==0) kh2--;  // if not odd decrease 1
  if (kw%2==0) kw2--;

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
    int toffset = IM_THREAD_NUM*(kw*kh);

    for(int i = 0; i < width; i++)
    {
      int v = 0; 
      int c = 0;
    
      for(int y = kh1; y <= kh2; y++)
      {
        if ((j + y < 0) ||        // pass the bottom border
            (j + y >= height))    // pass the top border
          continue;

        int offset = (j + y) * width;

        for(int x = kw1; x <= kw2; x++)
        {
          if ((i + x < 0) ||      // pass the left border
              (i + x >= width))   // pass the right border
            continue;

          if (x == 0 && y == 0)
            c = v;

          value[toffset + v] = map[offset + (i + x)];
          v++;
        }
      }
      
      new_map[new_offset + i] = (DT)func(value + toffset, v, c);
    }    

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  delete[] value;
  return processing;
}

static int compare_imReal(const void *elem1, const void *elem2) 
{
  float* v1 = (float*)elem1;
  float* v2 = (float*)elem2;

  if (*v1 < *v2)
    return -1;

  if (*v1 > *v2)
    return 1;

  return 0;
}

static int compare_imInt(const void *elem1, const void *elem2) 
{
  int* v1 = (int*)elem1;
  int* v2 = (int*)elem2;

  if (*v1 < *v2)
    return -1;

  if (*v1 > *v2)
    return 1;

  return 0;
}

static int compare_imShort(const void *elem1, const void *elem2) 
{
  short* v1 = (short*)elem1;
  short* v2 = (short*)elem2;

  if (*v1 < *v2)
    return -1;

  if (*v1 > *v2)
    return 1;

  return 0;
}

static int compare_imUShort(const void *elem1, const void *elem2) 
{
  imushort* v1 = (imushort*)elem1;
  imushort* v2 = (imushort*)elem2;

  if (*v1 < *v2)
    return -1;

  if (*v1 > *v2)
    return 1;

  return 0;
}

static int compare_imByte(const void *elem1, const void *elem2) 
{
  imbyte* v1 = (imbyte*)elem1;
  imbyte* v2 = (imbyte*)elem2;

  if (*v1 < *v2)
    return -1;

  if (*v1 > *v2)
    return 1;

  return 0;
}

static imbyte median_op_byte(imbyte* value, int count, int center)
{
  (void)center;
  qsort(value, count, sizeof(imbyte), compare_imByte);
  return value[count/2];
}

static short median_op_short(short* value, int count, int center)
{
  (void)center;
  qsort(value, count, sizeof(short), compare_imShort);
  return value[count/2];
}

static imushort median_op_ushort(imushort* value, int count, int center)
{
  (void)center;
  qsort(value, count, sizeof(imushort), compare_imUShort);
  return value[count/2];
}

static int median_op_int(int* value, int count, int center)
{
  (void)center;
  qsort(value, count, sizeof(int), compare_imInt);
  return value[count/2];
}

static float median_op_real(float* value, int count, int center)
{
  (void)center;
  qsort(value, count, sizeof(float), compare_imReal);
  return value[count/2];
}

int imProcessMedianConvolve(const imImage* src_image, imImage* dst_image, int ks)
{
  int i, ret = 0;
  int counter;

  counter = imProcessCounterBegin("Median Filter");
  imCounterTotal(counter, src_image->depth*src_image->height, "Filtering...");

  for (i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      ret = DoConvolveRankFunc((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, median_op_byte, counter);
      break;                                                                                
    case IM_SHORT:                                                                           
      ret = DoConvolveRankFunc((short*)src_image->data[i], (short*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, median_op_short, counter);
      break;                                                                                
    case IM_USHORT:                                                                           
      ret = DoConvolveRankFunc((imushort*)src_image->data[i], (imushort*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, median_op_ushort, counter);
      break;                                                                                
    case IM_INT:                                                                           
      ret = DoConvolveRankFunc((int*)src_image->data[i], (int*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, median_op_int, counter);
      break;                                                                                
    case IM_FLOAT:                                                                           
      ret = DoConvolveRankFunc((float*)src_image->data[i], (float*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, median_op_real, counter);
      break;                                                                                
    }
    
    if (!ret) 
      break;
  }

  imProcessCounterEnd(counter);

  return ret;
}

static imbyte range_op_byte(imbyte* value, int count, int center)
{
  imbyte min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return max-min;
}

static short range_op_short(short* value, int count, int center)
{
  short min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return max-min;
}

static imushort range_op_ushort(imushort* value, int count, int center)
{
  imushort min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return max-min;
}

static int range_op_int(int* value, int count, int center)
{
  int min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return max-min;
}

static float range_op_real(float* value, int count, int center)
{
  float min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return max-min;
}

int imProcessRangeConvolve(const imImage* src_image, imImage* dst_image, int ks)
{
  int i, ret = 0;
  int counter;

  counter = imProcessCounterBegin("Range Filter");
  imCounterTotal(counter, src_image->depth*src_image->height, "Filtering...");

  for (i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      ret = DoConvolveRankFunc((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, range_op_byte, counter);
      break;                                                                                
    case IM_SHORT:                                                                           
      ret = DoConvolveRankFunc((short*)src_image->data[i], (short*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, range_op_short, counter);
      break;                                                                                
    case IM_USHORT:                                                                           
      ret = DoConvolveRankFunc((imushort*)src_image->data[i], (imushort*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, range_op_ushort, counter);
      break;                                                                                
    case IM_INT:                                                                           
      ret = DoConvolveRankFunc((int*)src_image->data[i], (int*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, range_op_int, counter);
      break;                                                                                
    case IM_FLOAT:                                                                           
      ret = DoConvolveRankFunc((float*)src_image->data[i], (float*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, range_op_real, counter);
      break;                                                                                
    }

    if (!ret) 
      break;
  }

  imProcessCounterEnd(counter);

  return ret;
}

/*
Local variable threshold by the method of Bernsen.

Description:	
    If the difference between the largest and the smallest
		pixel value within the 'dx'*'dy' window is greater than
		or equal to 'cmin' (local contrast threshold), the average
		of the two values is used as threshold.

		Pixels in homogenous areas (difference below 'cmin') 
    are assumed to be below the threshold.

Reference:	
    Bernsen, J: "Dynamic thresholding of grey-level images"
		Proc. of the 8th ICPR, Paris, Oct 1986, 1251-1255.

Author:         Oivind Due Trier

Copyright 1990, Blab, UiO
Image processing lab, Department of Informatics
University of Oslo
*/

static int thresAux = 0;

static imbyte contrast_thres_op_byte(imbyte* value, int count, int center)
{
  int c, t;
  imbyte v = value[center], min, max;

  imMinMax(value, count, min, max);

  c = max-min;

  if (c < thresAux) 
    return 0;
  else
  { 
    t = ((int)max + (int)min) / 2;

    if (v >= t)
      return 1;
    else
      return 0;
  }
}

static imushort contrast_thres_op_ushort(imushort* value, int count, int center)
{
  int c, t;
  imushort v = value[center], min, max;

  imMinMax(value, count, min, max);

  c = max-min;

  if (c < thresAux) 
    return 0;
  else
  { 
    t = ((int)max + (int)min) / 2;

    if (v >= t)
      return 1;
    else
      return 0;
  }
}

static short contrast_thres_op_short(short* value, int count, int center)
{
  int c, t;
  short v = value[center], min, max;

  imMinMax(value, count, min, max);

  c = max-min;

  if (c < thresAux) 
    return 0;
  else
  { 
    t = ((int)max + (int)min) / 2;

    if (v >= t)
      return 1;
    else
      return 0;
  }
}

static int contrast_thres_op_int(int* value, int count, int center)
{
  int c, t;
  int v = value[center], min, max;

  imMinMax(value, count, min, max);

  c = max-min;

  if (c < thresAux) 
    return 0;
  else
  { 
    t = ((int)max + (int)min) / 2;

    if (v >= t)
      return 1;
    else
      return 0;
  }
}

int imProcessRangeContrastThreshold(const imImage* src_image, imImage* dst_image, int ks, int min_range)
{
  int ret = 0;
  int counter = imProcessCounterBegin("Range Contrast Threshold");
  imCounterTotal(counter, src_image->depth*src_image->height, "Filtering...");

  thresAux = min_range;

  switch(src_image->data_type)
  {
  case IM_BYTE:
    ret = DoConvolveRankFunc((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, ks, ks, contrast_thres_op_byte, counter);
    break;                                                                                
  case IM_SHORT:                                                                           
    ret = DoConvolveRankFunc((short*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, ks, ks, contrast_thres_op_short, counter);
    break;                                                                                
  case IM_USHORT:                                                                           
    ret = DoConvolveRankFunc((imushort*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, ks, ks, contrast_thres_op_ushort, counter);
    break;                                                                                
  case IM_INT:                                                                           
    ret = DoConvolveRankFunc((int*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, ks, ks, contrast_thres_op_int, counter);
    break;                                                                                
  }

  imProcessCounterEnd(counter);

  return ret;
}

static imbyte max_thres_op_byte(imbyte* value, int count, int center)
{
  imbyte v = value[center], min, max;

  if (v < thresAux) 
    return 0;

  imMinMax(value, count, min, max);

  if (v < max)
    return 0;

  return 1;
}

static short max_thres_op_short(short* value, int count, int center)
{
  short v = value[center], min, max;

  if (v < thresAux) 
    return 0;

  imMinMax(value, count, min, max);

  if (v < max)
    return 0;

  return 1;
}

static imushort max_thres_op_ushort(imushort* value, int count, int center)
{
  imushort v = value[center], min, max;

  if (v < thresAux) 
    return 0;

  imMinMax(value, count, min, max);

  if (v < max)
    return 0;

  return 1;
}

static int max_thres_op_int(int* value, int count, int center)
{
  int v = value[center], min, max;

  if (v < thresAux) 
    return 0;

  imMinMax(value, count, min, max);

  if (v < max)
    return 0;

  return 1;
}

int imProcessLocalMaxThreshold(const imImage* src_image, imImage* dst_image, int ks, int min_thres)
{
  int ret = 0;
  int counter = imProcessCounterBegin("Local Max Threshold");
  imCounterTotal(counter, src_image->depth*src_image->height, "Filtering...");

  thresAux = min_thres;

  switch(src_image->data_type)
  {
  case IM_BYTE:
    ret = DoConvolveRankFunc((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, ks, ks, max_thres_op_byte, counter);
    break;                                                                                
  case IM_SHORT:                                                                           
    ret = DoConvolveRankFunc((short*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, ks, ks, max_thres_op_short, counter);
    break;                                                                                
  case IM_USHORT:                                                                           
    ret = DoConvolveRankFunc((imushort*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, ks, ks, max_thres_op_ushort, counter);
    break;                                                                                
  case IM_INT:                                                                           
    ret = DoConvolveRankFunc((int*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, ks, ks, max_thres_op_int, counter);
    break;                                                                                
  }

  imProcessCounterEnd(counter);

  return ret;
}

static imbyte rank_closest_op_byte(imbyte* value, int count, int center)
{
  imbyte v = value[center];
  imbyte min, max;

  imMinMax(value, count, min, max);

  if (v - min < max - v) 
    return min;
  else
    return max;
}

static imushort rank_closest_op_ushort(imushort* value, int count, int center)
{
  imushort v = value[center];
  imushort min, max;

  imMinMax(value, count, min, max);

  if (v - min < max - v) 
    return min;
  else
    return max;
}

static short rank_closest_op_short(short* value, int count, int center)
{
  short v = value[center];
  short min, max;

  imMinMax(value, count, min, max);

  if (v - min < max - v) 
    return min;
  else
    return max;
}

static int rank_closest_op_int(int* value, int count, int center)
{
  int v = value[center];
  int min, max;

  imMinMax(value, count, min, max);

  if (v - min < max - v) 
    return min;
  else
    return max;
}

static float rank_closest_op_real(float* value, int count, int center)
{
  float v = value[center];
  float min, max;

  imMinMax(value, count, min, max);

  if (v - min < max - v) 
    return min;
  else
    return max;
}


int imProcessRankClosestConvolve(const imImage* src_image, imImage* dst_image, int ks)
{
  int i, ret = 0;
  int counter;

  counter = imProcessCounterBegin("Rank Closest");
  imCounterTotal(counter, src_image->depth*src_image->height, "Filtering...");

  for (i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      ret = DoConvolveRankFunc((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_closest_op_byte, counter);
      break;                                                                                
    case IM_SHORT:                                                                           
      ret = DoConvolveRankFunc((short*)src_image->data[i], (short*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_closest_op_short, counter);
      break;                                                                                
    case IM_USHORT:                                                                           
      ret = DoConvolveRankFunc((imushort*)src_image->data[i], (imushort*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_closest_op_ushort, counter);
      break;                                                                                
    case IM_INT:                                                                           
      ret = DoConvolveRankFunc((int*)src_image->data[i], (int*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_closest_op_int, counter);
      break;                                                                                
    case IM_FLOAT:                                                                           
      ret = DoConvolveRankFunc((float*)src_image->data[i], (float*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_closest_op_real, counter);
      break;                                                                                
    }
    
    if (!ret) 
      break;
  }

  imProcessCounterEnd(counter);

  return ret;
}

static imbyte rank_max_op_byte(imbyte* value, int count, int center)
{
  imbyte min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return max;
}

static imushort rank_max_op_ushort(imushort* value, int count, int center)
{
  imushort min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return max;
}

static short rank_max_op_short(short* value, int count, int center)
{
  short min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return max;
}

static int rank_max_op_int(int* value, int count, int center)
{
  int min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return max;
}

static float rank_max_op_real(float* value, int count, int center)
{
  float min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return max;
}

int imProcessRankMaxConvolve(const imImage* src_image, imImage* dst_image, int ks)
{
  int i, ret = 0;
  int counter;

  counter = imProcessCounterBegin("Rank Max");
  imCounterTotal(counter, src_image->depth*src_image->height, "Filtering...");

  for (i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      ret = DoConvolveRankFunc((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_max_op_byte, counter);
      break;                                                                                
    case IM_SHORT:                                                                           
      ret = DoConvolveRankFunc((short*)src_image->data[i], (short*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_max_op_short, counter);
      break;                                                                                
    case IM_USHORT:                                                                           
      ret = DoConvolveRankFunc((imushort*)src_image->data[i], (imushort*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_max_op_ushort, counter);
      break;                                                                                
    case IM_INT:                                                                           
      ret = DoConvolveRankFunc((int*)src_image->data[i], (int*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_max_op_int, counter);
      break;                                                                                
    case IM_FLOAT:                                                                           
      ret = DoConvolveRankFunc((float*)src_image->data[i], (float*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_max_op_real, counter);
      break;                                                                                
    }
    
    if (!ret) 
      break;
  }

  imProcessCounterEnd(counter);

  return ret;
}

static imbyte rank_min_op_byte(imbyte* value, int count, int center)
{
  imbyte min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return min;
}

static imushort rank_min_op_ushort(imushort* value, int count, int center)
{
  imushort min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return min;
}

static short rank_min_op_short(short* value, int count, int center)
{
  short min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return min;
}

static int rank_min_op_int(int* value, int count, int center)
{
  int min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return min;
}

static float rank_min_op_real(float* value, int count, int center)
{
  float min, max;
  (void)center;
  imMinMax(value, count, min, max);
  return min;
}

int imProcessRankMinConvolve(const imImage* src_image, imImage* dst_image, int ks)
{
  int i, ret = 0;
  int counter;

  counter = imProcessCounterBegin("Rank Min");
  imCounterTotal(counter, src_image->depth*src_image->height, "Filtering...");

  for (i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      ret = DoConvolveRankFunc((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_min_op_byte, counter);
      break;                                                                                
    case IM_SHORT:                                                                           
      ret = DoConvolveRankFunc((short*)src_image->data[i], (short*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_min_op_short, counter);
      break;                                                                                
    case IM_USHORT:                                                                           
      ret = DoConvolveRankFunc((imushort*)src_image->data[i], (imushort*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_min_op_ushort, counter);
      break;                                                                                
    case IM_INT:                                                                           
      ret = DoConvolveRankFunc((int*)src_image->data[i], (int*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_min_op_int, counter);
      break;                                                                                
    case IM_FLOAT:                                                                           
      ret = DoConvolveRankFunc((float*)src_image->data[i], (float*)dst_image->data[i], 
                               src_image->width, src_image->height, ks, ks, rank_min_op_real, counter);
      break;                                                                                
    }
    
    if (!ret) 
      break;
  }

  imProcessCounterEnd(counter);

  return ret;
}
