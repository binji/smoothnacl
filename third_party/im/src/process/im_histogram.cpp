/** \file
 * \brief Histogram Based Operations
 *
 * See Copyright Notice in im_lib.h
 */


#include <im.h>
#include <im_util.h>
#include <im_math.h>

#include "im_process_counter.h"
#include "im_process_pnt.h"
#include "im_process_ana.h"

#include <stdlib.h>
#include <memory.h>


unsigned long* imHistogramNew(int data_type, int *hcount)
{
  *hcount = imHistogramCount(data_type);
  return (unsigned long*)calloc(*hcount, sizeof(unsigned long));
}

void imHistogramRelease(unsigned long* histo)
{
  free(histo);
}

int imHistogramShift(int data_type)
{
  if (data_type == IM_SHORT)
    return -32768;
  else
    return 0;
}

int imHistogramCount(int data_type)
{
  if (data_type == IM_USHORT || data_type == IM_SHORT)
    return 65536;
  else
    return 256;
}

template <class T>
static void DoExpandHistogram(T* src_map, T* dst_map, int size, int depth, int hcount, int low_level, int high_level)
{
  int i;

  T* re_map = new T [hcount];
  memset(re_map, 0, hcount*sizeof(T));

  int range = high_level-low_level+1;
  float factor = (float)hcount / (float)range;

  for (i = 0; i < hcount; i++)
  {             
    if (i <= low_level)
      re_map[i] = 0;
    else if (i >= high_level)
      re_map[i] = (T)(hcount-1);
    else
    {
      int value = imResampleInt(i - low_level, factor);
      re_map[i] = (T)IM_CROPMAX(value, hcount-1);
    }
  }

  int total_count = size*depth;
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(total_count))
#endif
  for (i = 0; i < total_count; i++)
    dst_map[i] = re_map[src_map[i]];

  delete [] re_map;
}

void imProcessExpandHistogram(const imImage* src_image, imImage* dst_image, float percent)
{
  int low_level, high_level;
  imCalcPercentMinMax(src_image, percent, 0, &low_level, &high_level);

  int hcount = imHistogramCount(src_image->data_type);

  if (src_image->data_type == IM_USHORT)
    DoExpandHistogram((imushort*)src_image->data[0], (imushort*)dst_image->data[0], src_image->count, src_image->depth, hcount, low_level, high_level);
  else if (src_image->data_type == IM_SHORT)
    DoExpandHistogram((short*)src_image->data[0], (short*)dst_image->data[0], src_image->count, src_image->depth, hcount, low_level, high_level);
  else
    DoExpandHistogram((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], src_image->count, src_image->depth, hcount, low_level, high_level);
}

template <class T>
static void DoEqualizeHistogram(T* src_map, T* dst_map, int size, int depth, int hcount, unsigned long* histo)
{
  int i;

  T* re_map = new T [hcount];
  memset(re_map, 0, hcount*sizeof(T));

  float factor = (float)hcount / (float)size;

  for (i = 0; i < hcount; i++)
  {             
    int value = imResampleInt(histo[i], factor); // from 0-size to 0-(hcount-1)
    re_map[i] = (T)IM_CROPMAX(value, hcount-1);
  }

  int total_count = size*depth;
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(total_count))
#endif
  for (i = 0; i < total_count; i++)
    dst_map[i] = re_map[src_map[i]];

  delete [] re_map;
}

void imProcessEqualizeHistogram(const imImage* src_image, imImage* dst_image)
{
  int hcount;
  unsigned long* histo = imHistogramNew(src_image->data_type, &hcount);

  imCalcHistogram(src_image, histo, 0, 1); // cumulative

  if (src_image->data_type == IM_USHORT)
    DoEqualizeHistogram((imushort*)src_image->data[0], (imushort*)dst_image->data[0], src_image->count, src_image->depth, hcount, histo);
  else if (src_image->data_type == IM_SHORT)
    DoEqualizeHistogram((short*)src_image->data[0], (short*)dst_image->data[0], src_image->count, src_image->depth, hcount, histo);
  else
    DoEqualizeHistogram((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], src_image->count, src_image->depth, hcount, histo);

  imHistogramRelease(histo);
}
