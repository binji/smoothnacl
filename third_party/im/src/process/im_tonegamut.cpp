/** \file
 * \brief Tone Gamut Operations
 *
 * See Copyright Notice in im_lib.h
 */


#include <im.h>
#include <im_util.h>
#include <im_math.h>
#include <im_colorhsi.h>

#include "im_process_counter.h"
#include "im_process_pnt.h"
#include "im_process_ana.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>


template <class T>
static inline T line_op(const T& v, const T& min, const T& max, const float& a, const float& b)
{
  float r = v * a + b;
  if (r > (float)max) return max;
  if (r < (float)min) return min;
  return (T)r;
}

template <class T>
static inline T normal_op(const T& v, const T& min, const T& range)
{
  return (T)(float(v - min) / float(range));
}

template <class T>
static inline T zerostart_op(const T& v, const T& min)
{
  return (T)(v - min);
}

template <class T>
static inline float invert_op(const T& v, const T& min, const T& range)
{
  return 1.0f - float(v - min) / float(range);
}

template <class T>
static inline T solarize_op(const T& v, const T& level, const float& A, const float& B)
{
  if (v > level)
    return (T)(v * A + B);
  else
    return v;
}

template <class T>
static inline T slice_op(const T& v, const T& min, const T& max, const T& start, const T& end, int bin)
{
  if (v < start || v > end)
    return min;
  else
  {
    if (bin)
      return max;
    else
      return v;
  }
}

template <class T>
static inline T tonecrop_op(const T& v, const T& start, const T& end)
{
  if (v < start)
    return start;
  if (v > end)
    return end;
  else
    return v;
}

template <class T>
static inline T expand_op(const T& v, const T& min, const T& max, const T& start, const float& norm)
{
  float r = (v - start)*norm + min;
  if (r > (float)max) return max;
  if (r < (float)min) return min;
  return (T)r;
}

template <class T>
static inline float norm_pow_op(const T& v, const T& min, const T& range, const float& gamma)
{
  return (float)pow(float(v - min) / float(range), gamma);
}

template <class T>
static inline float norm_log_op(const T& v, const T& min, const T& range, const float& norm, const float& K)
{
  return (float)(log(K * float(v - min) / float(range) + 1) / norm);
}

template <class T>
static inline float norm_exp_op(const T& v, const T& min, const T& range, const float& norm, const float& K)
{
  return (float)((exp(K * float(v - min) / float(range)) - 1) / norm);
}

template <class T> 
static void DoNormalizedUnaryOp(T *map, T *new_map, int count, int op, float *args)
{
  int i;
  T min, max, range;

  if (op & IM_GAMUT_MINMAX)
  {
    min = (T)args[0];
    max = (T)args[1];
    args += 2;
  }
  else
    imMinMaxType(map, count, min, max);

  range = max-min;
  
  switch(op & 0x00FF)
  {
  case IM_GAMUT_NORMALIZE:
    {
      if (min >= 0 && max <= 1)  // Already normalized
      {
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
        for (i = 0; i < count; i++)
          new_map[i] = (T)map[i];
      }
      else
      {
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
        for (i = 0; i < count; i++)
          new_map[i] = normal_op(map[i], min, range);
      }
      break;
    }
  case IM_GAMUT_INVERT:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (T)(invert_op(map[i], min, range)*range + min);
    break;
  case IM_GAMUT_ZEROSTART:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (T)zerostart_op(map[i], min);
    break;
  case IM_GAMUT_SOLARIZE:
    {
      T level =  (T)(((100 - args[0]) * range) / 100.0f + min);
      float A = float(level - min) / float(level - max);
      float B = float(level * range) / float(max - level);
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
      for (i = 0; i < count; i++)
        new_map[i] = solarize_op(map[i], level, A, B);
      break;
    }
  case IM_GAMUT_POW:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (T)(norm_pow_op(map[i], min, range, args[0])*range + min);
    break;
  case IM_GAMUT_LOG:
    {
      float norm = float(log(args[0] + 1));
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
      for (i = 0; i < count; i++)
        new_map[i] = (T)(norm_log_op(map[i], min, range, norm, args[0])*range + min);
      break;
    }
  case IM_GAMUT_EXP:
    {
      float norm = float(exp(args[0]) - 1);
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
      for (i = 0; i < count; i++)
        new_map[i] = (T)(norm_exp_op(map[i], min, range, norm, args[0])*range + min);
      break;
    }
  case IM_GAMUT_SLICE:
    {
      if (args[0] > args[1]) { float tmp = args[1]; args[1] = args[0]; args[0] = tmp; }
      if (args[1] > max) args[1] = (float)max;
      if (args[0] < min) args[0] = (float)min;
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
      for (i = 0; i < count; i++)
        new_map[i] = slice_op(map[i], min, max, (T)args[0], (T)args[1], (int)args[2]);
      break;
    }
  case IM_GAMUT_CROP:
    {
      if (args[0] > args[1]) { float tmp = args[1]; args[1] = args[0]; args[0] = tmp; }
      if (args[1] > max) args[1] = (float)max;
      if (args[0] < min) args[0] = (float)min;
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
      for (i = 0; i < count; i++)
        new_map[i] = tonecrop_op(map[i], (T)args[0], (T)args[1]);
      break;
    }
  case IM_GAMUT_EXPAND:
    {
      if (args[0] > args[1]) { float tmp = args[1]; args[1] = args[0]; args[0] = tmp; }
      if (args[1] > max) args[1] = (float)max;
      if (args[0] < min) args[0] = (float)min;
      float norm = float(max - min)/(args[1] - args[0]);
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
      for (i = 0; i < count; i++)
        new_map[i] = expand_op(map[i], min, max, (T)args[0], norm);
      break;
    }
  case IM_GAMUT_BRIGHTCONT:
    {
      float bs = (args[0] * range) / 100.0f;
      float a = (float)tan((45+args[1]*0.449999)/57.2957795);
      float b = bs + (float)range*(1.0f - a)/2.0f;
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
      for (i = 0; i < count; i++)
        new_map[i] = line_op(map[i], min, max, a, b);
      break;
    }
  }
}

void imProcessToneGamut(const imImage* src_image, imImage* dst_image, int op, float *args)
{
  int count = src_image->count*src_image->depth;

  switch(src_image->data_type)
  {
  case IM_BYTE:
    DoNormalizedUnaryOp((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], count, op, args);
    break;                                                                                
  case IM_SHORT:                                                                           
    DoNormalizedUnaryOp((short*)src_image->data[0], (short*)dst_image->data[0], count, op, args);
    break;                                                                                
  case IM_USHORT:                                                                           
    DoNormalizedUnaryOp((imushort*)src_image->data[0], (imushort*)dst_image->data[0], count, op, args);
    break;                                                                                
  case IM_INT:                                                                           
    DoNormalizedUnaryOp((int*)src_image->data[0], (int*)dst_image->data[0], count, op, args);
    break;                                                                                
  case IM_FLOAT:                                                                           
    DoNormalizedUnaryOp((float*)src_image->data[0], (float*)dst_image->data[0], count, op, args);
    break;                                                                                
  }
}

template <class T> 
static void DoShiftHSI(T **map, T **new_map, int count, float h_shift, float s_shift, float i_shift)
{
  float min, max, range;
  T tmin, tmax;
  int tcount = count*3;

  imMinMaxType(map[0], tcount, tmin, tmax);

  min = (float)tmin;
  max = (float)tmax;

  range = max-min;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int j = 0; j < count; j++)
  {
    float h, s, i;
    float r, g, b;

    // Normalize to 0-1
    r = normal_op((float)map[0][j], min, range);
    g = normal_op((float)map[1][j], min, range);
    b = normal_op((float)map[2][j], min, range);

    imColorRGB2HSI(r, g, b, &h, &s, &i);

    h += h_shift; // in degrees
    s += s_shift;
    if (s < 0) s = 0;
    if (s > 1) s = 1;
    i += i_shift;
    if (i < 0) i = 0;
    if (i > 1) i = 1;

    imColorHSI2RGB(h, s, i, &r, &g, &b);

    // Expand to min-max
    new_map[0][j] = (T)(r*range + min);
    new_map[1][j] = (T)(g*range + min);
    new_map[2][j] = (T)(b*range + min);
  }
}

static void DoShiftHSIByte(imbyte **map, imbyte **new_map, int count, float h_shift, float s_shift, float i_shift)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int j = 0; j < count; j++)
  {
    float h, s, i;
    imbyte r, g, b;

    r = map[0][j];
    g = map[1][j];
    b = map[2][j];

    imColorRGB2HSIbyte(r, g, b, &h, &s, &i);

    h += h_shift; // in degrees
    s += s_shift;
    if (s < 0) s = 0;
    if (s > 1) s = 1;
    i += i_shift;
    if (i < 0) i = 0;
    if (i > 1) i = 1;

    imColorHSI2RGBbyte(h, s, i, &r, &g, &b);

    new_map[0][j] = r;
    new_map[1][j] = g;
    new_map[2][j] = b;
  }
}

void imProcessShiftHSI(const imImage* src_image, imImage* dst_image, float h_shift, float s_shift, float i_shift)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    DoShiftHSIByte((imbyte**)src_image->data, (imbyte**)dst_image->data, src_image->count, h_shift, s_shift, i_shift);
    break;                                                                                
  case IM_SHORT:                                                                           
    DoShiftHSI((short**)src_image->data, (short**)dst_image->data, src_image->count, h_shift, s_shift, i_shift);
    break;                                                                                
  case IM_USHORT:                                                                           
    DoShiftHSI((imushort**)src_image->data, (imushort**)dst_image->data, src_image->count, h_shift, s_shift, i_shift);
    break;                                                                                
  case IM_INT:                                                                           
    DoShiftHSI((int**)src_image->data, (int**)dst_image->data, src_image->count, h_shift, s_shift, i_shift);
    break;                                                                                
  case IM_FLOAT:                                                                           
    DoShiftHSI((float**)src_image->data, (float**)dst_image->data, src_image->count, h_shift, s_shift, i_shift);
    break;                                                                                
  }
}

float imProcessCalcAutoGamma(const imImage* image)
{
  float mean, min, max;
  imStats stats[4];
  imCalcImageStatistics(image, stats);
  mean = stats[0].mean;
  min = stats[0].min;
  max = stats[0].max;
  for (int i = 1; i < image->depth; i++)
  {
    if (stats[i].min < min)
      min = stats[i].min;
    if (stats[i].max > max)
      max = stats[i].max;

    mean += stats[i].mean;
  }

  mean /= (float)image->depth;

  return (float)(log((double)((mean-min)/(max-min)))/log(0.5));
}

void imProcessUnNormalize(const imImage* src_image, imImage* dst_image)
{
  int count = src_image->count*src_image->depth;

  float* map = (float*)src_image->data[0];
  imbyte* new_map = (imbyte*)dst_image->data[0];

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    if (map[i] > 1)
      new_map[i] = (imbyte)255;
    else if (map[i] < 0)
      new_map[i] = (imbyte)0;
    else
      new_map[i] = (imbyte)(map[i]*255);
  }
}

template <class T> 
static void DoDirectConv(T* map, imbyte* new_map, int count)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    if (map[i] > 255)
      new_map[i] = (imbyte)255;
    else if (map[i] < 0)
      new_map[i] = (imbyte)0;
    else
      new_map[i] = (imbyte)(map[i]);
  }
}

void imProcessDirectConv(const imImage* src_image, imImage* dst_image)
{
  int count = src_image->count*src_image->depth;

  switch(src_image->data_type)
  {
  case IM_SHORT:                                                                           
    DoDirectConv((short*)src_image->data[0], (imbyte*)dst_image->data[0], count);
    break;                                                                                
  case IM_USHORT:                                                                           
    DoDirectConv((imushort*)src_image->data[0], (imbyte*)dst_image->data[0], count);
    break;                                                                                
  case IM_INT:                                                                           
    DoDirectConv((int*)src_image->data[0], (imbyte*)dst_image->data[0], count);
    break;                                                                                
  case IM_FLOAT:                                                                           
    DoDirectConv((float*)src_image->data[0], (imbyte*)dst_image->data[0], count);
    break;                                                                                
  }
}

void imProcessNegative(const imImage* src_image, imImage* dst_image)
{
  if (src_image->color_space == IM_MAP)
  {
    unsigned char r, g, b;
    for (int i = 0; i < src_image->palette_count; i++)
    {
      imColorDecode(&r, &g, &b, src_image->palette[i]);
      r = ~r; g = ~g; b = ~b;
      dst_image->palette[i] = imColorEncode(r, g, b);
    }

    imImageCopyData(src_image, dst_image);
  }
  else if (src_image->color_space == IM_BINARY)
  {
    imbyte* map1 = (imbyte*)src_image->data[0];
    imbyte* map = (imbyte*)dst_image->data[0];
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(src_image->count))
#endif
    for (int i = 0; i < src_image->count; i++)
      map[i] = map1[i]? 0: 1;
  }
  else
    imProcessToneGamut(src_image, dst_image, IM_GAMUT_INVERT, NULL);
}
