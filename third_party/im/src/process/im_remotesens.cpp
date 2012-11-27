/** \file
 * \brief Remote Sensing Operations
 *
 * See Copyright Notice in im_lib.h
 */


#include <im.h>
#include <im_util.h>
#include <im_math.h>

#include "im_process_counter.h"
#include "im_process_pnt.h"

#include <stdlib.h>
#include <memory.h>


template <class T> 
static void DoNormDiffRatio(T *map1, T *map2, float *new_map, int count)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    float num   = (float)(map1[i] - map2[i]);
    float denom = (float)(map1[i] + map2[i]);

    if (denom == 0) 
      new_map[i] = 0;
    else
      new_map[i] = num / denom;
  }
}

void imProcessNormDiffRatio(const imImage* image1, const imImage* image2, imImage* dst_image)
{
  int count = image1->count;

  switch(image1->data_type)
  {
  case IM_BYTE:
    DoNormDiffRatio((imbyte*)image1->data[0], (imbyte*)image2->data[0], (float*)dst_image->data[0], count);
    break;                                                                                
  case IM_SHORT:                                                                           
    DoNormDiffRatio((short*)image1->data[0], (short*)image2->data[0], (float*)dst_image->data[0], count);
    break;                                                                                
  case IM_USHORT:                                                                           
    DoNormDiffRatio((imushort*)image1->data[0], (imushort*)image2->data[0], (float*)dst_image->data[0], count);
    break;                                                                                
  case IM_INT:                                                                           
    DoNormDiffRatio((int*)image1->data[0], (int*)image2->data[0], (float*)dst_image->data[0], count);
    break;                                                                                
  case IM_FLOAT:                                                                           
    DoNormDiffRatio((float*)image1->data[0], (float*)image2->data[0], (float*)dst_image->data[0], count);
    break;                                                                                
  }
}

template <class T> 
static void DoAbnormalCorrection(T *map, T *new_map, int width, int height, imbyte* abnormal, int threshold_consecutive, int threshold_percent)
{
  // Mark candidates for abnormal pixels
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for (int y = 0; y < height; y++)
  {
    int line_offset = y*width;

    abnormal[line_offset] = 0;
    for (int x = 1; x < width-1; x++)
    {
      int offset = line_offset+x;
      if (map[offset] < map[offset-1] && map[offset] < map[offset+1])
        abnormal[offset] = 1;
      else
        abnormal[offset] = 0;
    }
    abnormal[line_offset+width-1] = 0;
  }

  threshold_percent = (height*threshold_percent)/100;

  // Select the abnormal pixels according to their distribution
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(width))
#endif
  for (int x = 1; x < width-1; x++)
  {
    int col_count = 0;

    for (int y = 0; y < height; y++)
    {
      int col_offset = y*width + x;
      if (abnormal[col_offset])
        col_count++;
    }

    if (col_count > threshold_percent)
    {
      int consecutive_count = 0, inside_range = 0;
      int range_start = 0, range_end;

      for (int y = 0; y < height; y++)
      {
        int col_offset = y*width + x;
        if (abnormal[col_offset])
        {
          if (inside_range)
          {
            consecutive_count++;
          }
          else
          {
            // range start
            range_start = y;
            inside_range = 1;
            consecutive_count = 1;
          }
        }
        else
        {
          if (inside_range)
          {
            // range end
            range_end = y;

            if (consecutive_count < threshold_consecutive)
            {
              // clear abnormal marks
              for (int i = range_start; i < range_end; i++)
                abnormal[i*width + x] = 0;
            }

            // restart
            inside_range = 0;
            consecutive_count = 0;
          }
        }
      }
    }
    else
    {
      // clear abnormal marks in the whole column
      for (int y = 0; y < height; y++)
        abnormal[y*width + x] = 0;
    }
  }

  // Correct the abnormal pixels 
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for (int y = 0; y < height; y++)
  {
    int line_offset = y*width;

    for (int x = 0; x < width; x++)
    {
      int offset = line_offset+x;

      if (abnormal[offset])
        new_map[offset] = (map[offset-1]+map[offset+1])/2;
      else
        new_map[offset] = map[offset];
    }
  }
}

void imProcessAbnormalHyperionCorrection(const imImage* src_image, imImage* dst_image, int threshold_consecutive, int threshold_percent, imImage* image_abnormal)
{
  imImage* abnormal = image_abnormal;
  if (!image_abnormal)
    abnormal = imImageCreateBased(src_image, 0, 0, IM_BINARY, IM_BYTE);

  switch(src_image->data_type)
  {
  case IM_BYTE:
    DoAbnormalCorrection((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], src_image->width, src_image->height, (imbyte*)abnormal->data[0], threshold_consecutive, threshold_percent);
    break;                                                                                
  case IM_SHORT:                                                                           
    DoAbnormalCorrection((short*)src_image->data[0], (short*)dst_image->data[0], src_image->width, src_image->height, (imbyte*)abnormal->data[0], threshold_consecutive, threshold_percent);
    break;                                                                                
  case IM_USHORT:                                                                           
    DoAbnormalCorrection((imushort*)src_image->data[0], (imushort*)dst_image->data[0], src_image->width, src_image->height, (imbyte*)abnormal->data[0], threshold_consecutive, threshold_percent);
    break;                                                                                
  case IM_INT:                                                                           
    DoAbnormalCorrection((int*)src_image->data[0], (int*)dst_image->data[0], src_image->width, src_image->height, (imbyte*)abnormal->data[0], threshold_consecutive, threshold_percent);
    break;                                                                                
  case IM_FLOAT:                                                                           
    DoAbnormalCorrection((float*)src_image->data[0], (float*)dst_image->data[0], src_image->width, src_image->height, (imbyte*)abnormal->data[0], threshold_consecutive, threshold_percent);
    break;                                                                                
  }

  if (!image_abnormal)
    imImageDestroy(abnormal);
}
