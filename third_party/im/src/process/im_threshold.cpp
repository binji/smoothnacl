/** \file
 * \brief Threshold Operations
 *
 * See Copyright Notice in im_lib.h
 */


#include <im.h>
#include <im_util.h>

#include "im_process_counter.h"
#include "im_process_pnt.h"
#include "im_process_ana.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>


template <class T> 
static void doThresholdSlice(T *src_map, imbyte *dst_map, int count, T start_level, T end_level)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    if (src_map[i] < start_level || src_map[i] > end_level)
      dst_map[i] = 0;
    else
      dst_map[i] = 1;
  }
}

void imProcessSliceThreshold(const imImage* src_image, imImage* dst_image, float start_level, float end_level)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    doThresholdSlice((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, (imbyte)start_level, (imbyte)end_level);
    break;                                                                                
  case IM_SHORT:                                                                           
    doThresholdSlice((short*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, (short)start_level, (short)end_level);
    break;                                                                                
  case IM_USHORT:                                                                           
    doThresholdSlice((imushort*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, (imushort)start_level, (imushort)end_level);
    break;                                                                                
  case IM_INT:                                                                           
    doThresholdSlice((int*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, (int)start_level, (int)end_level);
    break;                                                                                
  case IM_FLOAT:
    doThresholdSlice((float*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, (float)start_level, (float)end_level);
    break;                                                                                
  }
}

template <class T> 
static void doThresholdByDiff(T *src_map1, T *src_map2, imbyte *dst_map, int count)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    if (src_map1[i] <= src_map2[i])
      dst_map[i] = 0;
    else
      dst_map[i] = 1;
  }
}

void imProcessThresholdByDiff(const imImage* src_image1, const imImage* src_image2, imImage* dst_image)
{
  switch(src_image1->data_type)
  {
  case IM_BYTE:
    doThresholdByDiff((imbyte*)src_image1->data[0], (imbyte*)src_image2->data[0], (imbyte*)dst_image->data[0], src_image1->count);
    break;                                                                                
  case IM_SHORT:                                                                           
    doThresholdByDiff((short*)src_image1->data[0], (short*)src_image2->data[0], (imbyte*)dst_image->data[0], src_image1->count);
    break;                                                                                
  case IM_USHORT:                                                                           
    doThresholdByDiff((imushort*)src_image1->data[0], (imushort*)src_image2->data[0], (imbyte*)dst_image->data[0], src_image1->count);
    break;                                                                                
  case IM_INT:                                                                           
    doThresholdByDiff((int*)src_image1->data[0], (int*)src_image2->data[0], (imbyte*)dst_image->data[0], src_image1->count);
    break;                                                                                
  case IM_FLOAT:
    doThresholdByDiff((float*)src_image1->data[0], (float*)src_image2->data[0], (imbyte*)dst_image->data[0], src_image1->count);
    break;                                                                                
  }
}

template <class T> 
static void doThreshold(T *src_map, imbyte *dst_map, int count, T level, int value)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    if (src_map[i] <= level)
      dst_map[i] = 0;
    else
      dst_map[i] = (imbyte)value;
  }
}

void imProcessThreshold(const imImage* src_image, imImage* dst_image, float level, int value)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    doThreshold((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, (imbyte)level, value);
    break;                                                                                
  case IM_SHORT:                                                                           
    doThreshold((short*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, (short)level, value);
    break;                                                                                
  case IM_USHORT:                                                                           
    doThreshold((imushort*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, (imushort)level, value);
    break;                                                                                
  case IM_INT:                                                                           
    doThreshold((int*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, (int)level, value);
    break;                                                                                
  case IM_FLOAT:
    doThreshold((float*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, (float)level, value);
    break;                                                                                
  }
}

static int compare_int(const void *elem1, const void *elem2) 
{
  int* v1 = (int*)elem1;
  int* v2 = (int*)elem2;

  if (*v1 < *v2)
    return -1;

  if (*v1 > *v2)
    return 1;

  return 0;
}

static int thresUniErr(unsigned char* band, int width, int height)
{
  int x, y, i, bottom, top, ant2x2, maks1, maks2, maks4, t;
  int xsize, ysize, offset1, offset2;
  double a, b, c, phi;
  int g[4], tab1[256], tab2[256], tab4[256];

  memset(tab1, 0, sizeof(int)*256);
  memset(tab2, 0, sizeof(int)*256);
  memset(tab4, 0, sizeof(int)*256);

  xsize = width;
  ysize = height;

  if (xsize%2 != 0)
    xsize--;

  if (ysize%2 != 0)
    ysize--;
  
  /* examine all 2x2 neighborhoods */

  for (y=0; y<ysize; y+=2)
  {
    offset1 = y*width;
    offset2 = (y+1)*width;

    for (x=0; x<xsize; x+=2) 
    {
      g[0] = band[offset1 + x];
      g[1] = band[offset1 + x+1];
      g[2] = band[offset2 + x];
      g[3] = band[offset2 + x+1];

      /* Sorting */
      qsort(g, 4, sizeof(int), compare_int);

      /* Accumulating */
      tab1[g[0]] += 1; 
      tab1[g[1]] += 1; 
      tab1[g[2]] += 1; 
      tab1[g[3]] += 1; 

      tab2[g[0]] += 3;
      tab2[g[1]] += 2;
      tab2[g[2]] += 1;

      tab4[g[0]] += 1;
    }
  }

  /* Summing */
  for (i=254; i>=0; i--) 
  {
    tab1[i] += tab1[i+1];
    tab2[i] += tab2[i+1];
    tab4[i] += tab4[i+1];
  }
  
  /* Tables are ready, find threshold */
  bottom = 0; top = 255;
  ant2x2 = (xsize/2)*(ysize/2);
  maks1 = tab1[0]; /* = ant2x2 * 4; */
  maks2 = tab2[0]; /* = ant2x2 * 6; */
  maks4 = tab4[0]; /* = ant2x2;     */

  /* binary search */
  t = 0;
  while (bottom != top-1) 
  {
    t = (int) ((bottom+top)/2);

    /* Calculate probabilities */
    a = (double) tab1[t+1]/maks1;
    b = (double) tab2[t+1]/maks2;
    c = (double) tab4[t+1]/maks4;

    phi = sqrt((b*b - c) / (a*a - b));

    if (phi> 1)  
      bottom = t;
    else                        
      top = t;
  }
  
  return t;
}

int imProcessUniformErrThreshold(const imImage* src_image, imImage* dst_image)
{
  int level = thresUniErr((imbyte*)src_image->data[0], src_image->width, src_image->height);
  imProcessThreshold(src_image, dst_image, (float)level, 1);
  return level;
}

static void do_dither_error(imbyte* data1, imbyte* data2, int size, int t, int value)
{
  float scale = (float)(t/(255.0-t));

  int error = 0; /* always in [-127,127] */ 

  for (int i = 0; i < size; i++)
  {
    if ((int)(data1[i] + error) > t)
    {
      error -= (int)(((int)255 - (int)data1[i])*scale);
      data2[i] = (imbyte)value;
    }
    else
    {
      error += (int)data1[i];
      data2[i] = (imbyte)0;
    }
  }
}

void imProcessDifusionErrThreshold(const imImage* src_image, imImage* dst_image, int level)
{
  int value = src_image->depth > 1? 255: 1;
  for (int i = 0; i < src_image->depth; i++)
  {
    do_dither_error((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->count, level, value);
  }
}

int imProcessPercentThreshold(const imImage* src_image, imImage* dst_image, float percent)
{
  int hcount;
  unsigned long* histo = imHistogramNew(src_image->data_type, &hcount);

  unsigned long cut = (unsigned long)((src_image->count * percent)/100.);

  imCalcHistogram(src_image, histo, 0, 1); // cumulative

  int i;
  for (i = 0; i < hcount; i++)
  {
    if (histo[i] > cut)
      break;
  }

  int level = (i==0? 0: i==hcount? hcount-1: i-1);
  level += imHistogramShift(src_image->data_type);

  imProcessThreshold(src_image, dst_image, (float)level, 1);

  imHistogramRelease(histo);
  return level;
}

static int MaximizeDiscriminantFunction(double* p, int count)
{
  int k;

  double mi_max = 0;
  for (k=0; k<count; k++) 
    mi_max += k*p[k];

  int index = 0;
  double max = 0;
  double mi_k = 0;
  double w_k = 0;
  double value;
  for (k=0; k<count; k++) 
  {
    mi_k += k*p[k];
    w_k += p[k];

    if (w_k == 0 || w_k == 1)
      value = -1;
    else
      value = ((mi_max*w_k - mi_k)*(mi_max*w_k - mi_k))/(w_k*(1-w_k));

    if (value >= max) 
    {
      index = k;
      max = value;
    }
  }

  return index;
}

int imProcessOtsuThreshold(const imImage* src_image, imImage* dst_image)
{
  int hcount;
  unsigned long* histo = imHistogramNew(src_image->data_type, &hcount);

  imCalcHistogram(src_image, histo, 0, 0);

  double totalPixels = src_image->count;
  double* p = new double [hcount];

  for (int i=0; i<hcount; i++) 
    p[i] = (double)histo[i]/totalPixels;

  int level = MaximizeDiscriminantFunction(p, hcount);
  level += imHistogramShift(src_image->data_type);

  imProcessThreshold(src_image, dst_image, (float)level, 1);

  imHistogramRelease(histo);
  delete [] p;

  return level;
}

float imProcessMinMaxThreshold(const imImage* src_image, imImage* dst_image)
{
  imStats stats;
  imCalcImageStatistics(src_image, &stats);
  float level = (stats.max - stats.min)/2.0f;
  imProcessThreshold(src_image, dst_image, level, 1);
  return level;
}

void imProcessHysteresisThresEstimate(const imImage* image, int *low_thres, int *high_thres)
{
  int hcount;
  unsigned long* histo = imHistogramNew(image->data_type, &hcount);

  imCalcHistogram(image, histo, 0, 0);

  /* The high threshold should be > 80 or 90% of the pixels */
  unsigned long cut = (int)(0.1*image->count);

  int k = hcount-1;
  unsigned long count = histo[hcount-1];
  while (count < cut && k > 0)
  {
    k--;
    count += histo[k];
  }
  *high_thres = k;

  k=0;
  while (histo[k]==0 && k < hcount) 
    k++;

  *low_thres = (int)((*high_thres + k)/2.0) + k;

  *high_thres += imHistogramShift(image->data_type);
  *low_thres += imHistogramShift(image->data_type);

  imHistogramRelease(histo);
}

template <class T> 
static void doHysteresisThreshold(T *src_map, imbyte *dst_map, int width, int height, T low_thres, T high_thres)
{
  int count = width * height;
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    if (src_map[i] > high_thres)
      dst_map[i] = 1;
    else if (src_map[i] > low_thres)
      dst_map[i] = 2;          // mark for future replace
    else
      dst_map[i] = 0;
  }

  // now loop multiple times until there is no "2"s or no one was changed
  int changed = 1;
  while (changed) 
  {
    changed = 0;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
    for (int j=1; j<height-1; j++) 
    {
      for (int i=1; i<width-1; i++)
      {
        int offset = i+j*width;
        if (dst_map[offset] == 2)
        {
          // if there is an edge neighbor mark this as edge too
          if (dst_map[offset+1] == 1 || dst_map[offset-1] == 1 ||
              dst_map[offset+width] == 1 || dst_map[offset-width] == 1 ||
              dst_map[offset+width-1] == 1 || dst_map[offset+width+1] == 1 ||
              dst_map[offset-width-1] == 1 || dst_map[offset-width+1] == 1)
          {
            dst_map[offset] = 1;
            changed = 1;
          }
        }
      }
    }
  }

  // Clear the remaining "2"s
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    if (dst_map[i] == 2)
      dst_map[i] = 0;
  }
}

void imProcessHysteresisThreshold(const imImage* src_image, imImage* dst_image, int low_thres, int high_thres)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    doHysteresisThreshold((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, (imbyte)low_thres, (imbyte)high_thres);
    break;                                                                                
  case IM_SHORT:                                                                           
    doHysteresisThreshold((short*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, (short)low_thres, (short)high_thres);
    break;                                                                                
  case IM_USHORT:                                                                           
    doHysteresisThreshold((imushort*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, (imushort)low_thres, (imushort)high_thres);
    break;                                                                                
  case IM_INT:                                                                           
    doHysteresisThreshold((int*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, (int)low_thres, (int)high_thres);
    break;                                                                                
  case IM_FLOAT:
    doHysteresisThreshold((float*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->width, src_image->height, (float)low_thres, (float)high_thres);
    break;                                                                                
  }
}

void imProcessLocalMaxThresEstimate(const imImage* image, int *thres)
{
  int hcount;
  unsigned long* histo = imHistogramNew(image->data_type, &hcount);

  imCalcHistogram(image, histo, 0, 0);

  int high_count = 0;
  int index = hcount-1;
  while (high_count < 10 && index > 0)
  {
    if (histo[index] != 0)
      high_count++;

    index--;
  }

  *thres = index+1;
  *thres += imHistogramShift(image->data_type);

  imHistogramRelease(histo);
}

