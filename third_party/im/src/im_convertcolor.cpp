/** \file
 * \brief Image Conversion
 *
 * See Copyright Notice in im_lib.h
 */

#include "im.h"
#include "im_util.h"
#include "im_complex.h"
#include "im_image.h"
#include "im_convert.h"
#include "im_color.h"
#ifdef IM_PROCESS
#include "process/im_process_counter.h"
#include "im_process_pnt.h"
#else
#include "im_counter.h"
#endif

#include <stdlib.h>
#include <assert.h>
#include <memory.h>

#ifndef IM_PROCESS
#define IM_INT_PROCESSING     int processing = IM_ERR_NONE;
#define IM_BEGIN_PROCESSING   
#define IM_COUNT_PROCESSING   if (!imCounterInc(counter)) { processing = IM_ERR_COUNTER; break; }
#define IM_END_PROCESSING
#endif


/* IMPORTANT: leave template functions not "static" 
   because of some weird compiler bizarre errors. 
   Report on AIX C++.
*/
#ifdef AIX
#define IM_STATIC 
#else
#define IM_STATIC static
#endif


static void iConvertSetTranspMap(imbyte *src_map, imbyte *dst_alpha, int count, imbyte *transp_map, int transp_count)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for(int i = 0; i < count; i++)
  {
    if (src_map[i] < transp_count)
      dst_alpha[i] = transp_map[src_map[i]];
    else
      dst_alpha[i] = 255;  /* opaque */
  }
}

static void iConvertSetTranspIndex(imbyte *src_map, imbyte *dst_alpha, int count, imbyte index)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for(int i = 0; i < count; i++)
  {
    if (src_map[i] == index)
      dst_alpha[i] = 0;    /* full transparent */
    else
      dst_alpha[i] = 255;  /* opaque */
  }
}

static void iConvertSetTranspColor(imbyte **dst_data, int count, imbyte r, imbyte g, imbyte b)
{
  imbyte *pr = dst_data[0];
  imbyte *pg = dst_data[1];
  imbyte *pb = dst_data[2];
  imbyte *pa = dst_data[3];

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for(int i = 0; i < count; i++)
  {
    if (pr[i] == r &&
        pg[i] == g &&
        pb[i] == b)
      pa[i] = 0;    /* transparent */
    else
      pa[i] = 255;  /* opaque */
  }
}

// convert bin2gray and gray2bin
static void iConvertBinary(imbyte* map, int count, imbyte value)
{
  imbyte thres = (value == 255)? 1: 128;

  // if gray2bin, check for invalid gray that already is binary
  if (value != 255)
  {
    imbyte min, max;
    imMinMax(map, count, min, max);

    if (max == 1)
      thres = 1;
    else
      thres = max / 2;
  }

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    if (map[i] >= thres)
      map[i] = value;
    else
      map[i] = 0;
  }
}

static void iConvertMap2Gray(const imbyte* src_map, imbyte* dst_map, int count, const long* palette, const int palette_count)
{
  imbyte r, g, b;
  imbyte remap[256];

  for (int c = 0; c < palette_count; c++)
  {
    imColorDecode(&r, &g, &b, palette[c]);
    remap[c] = imColorRGB2Luma(r, g, b);
  }

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    dst_map[i] = remap[src_map[i]];
  }
}

static void iConvertMapToRGB(const imbyte* src_map, imbyte* red, imbyte* green, imbyte* blue, int count, const long* palette, const int palette_count)
{
  imbyte r[256], g[256], b[256];
  for (int c = 0; c < palette_count; c++)
    imColorDecode(&r[c], &g[c], &b[c], palette[c]);

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    int index = src_map[i];
    red[i] = r[index];
    green[i] = g[index];
    blue[i] = b[index];
  }
}

template <class T> 
IM_STATIC int iDoConvert2Gray(int count, int data_type, 
                    const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T type_max = (T)imColorMax(data_type);
  T type_min = (T)imColorMin(data_type);

  const T* src_map0 = src_data[0];
  const T* src_map1 = src_data[1];
  const T* src_map2 = src_data[2];
  const T* src_map3 = (src_color_space == IM_CMYK)? src_data[3]: 0;
  T* dst_map = dst_data[0];

  imCounterTotal(counter, count, "Converting To Gray...");

  IM_INT_PROCESSING;

  switch(src_color_space)
  {
  case IM_XYZ: 
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // scale to 0-1
      float c1 = imColorReconstruct(src_map1[i], type_min, type_max);  // use only Y component

      // do gamma correction then scale back to 0-type_max
      dst_map[i] = imColorQuantize(imColorTransfer2Nonlinear(c1), type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_CMYK: 
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      T r, g, b;
      // result is still 0-type_max
      imColorCMYK2RGB(src_map0[i], src_map1[i], src_map2[i], src_map3[i], r, g, b, type_max);
      dst_map[i] = imColorRGB2Luma(r, g, b);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_RGB:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      dst_map[i] = imColorRGB2Luma(src_map0[i], src_map1[i], src_map2[i]);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_LUV:
  case IM_LAB:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // to increase precision do intermediate conversions in float
      
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max); // scale to 0-1
      c0 = imColorLightness2Luminance(c0);             // do the conversion

      // do gamma correction then scale back to 0-type_max
      dst_map[i] = imColorQuantize(imColorTransfer2Nonlinear(c0), type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
IM_STATIC int iDoConvert2RGB(int count, int data_type, 
                   const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T zero;
  T type_max = (T)imColorMax(data_type);
  T type_min = (T)imColorMin(data_type);

  const T* src_map0 = src_data[0];
  const T* src_map1 = src_data[1];
  const T* src_map2 = src_data[2];
  const T* src_map3 = (src_color_space == IM_CMYK)? src_data[3]: 0;
  T* dst_map0 = dst_data[0];
  T* dst_map1 = dst_data[1];
  T* dst_map2 = dst_data[2];

  imCounterTotal(counter, count, "Converting To RGB...");

  IM_INT_PROCESSING;

  switch(src_color_space)
  {
  case IM_XYZ: 
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // to increase precision do intermediate conversions in float

      // scale to 0-1
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);
      float c1 = imColorReconstruct(src_map1[i], type_min, type_max);
      float c2 = imColorReconstruct(src_map2[i], type_min, type_max);

      // result is still 0-1
      imColorXYZ2RGB(c0, c1, c2, 
                     c0, c1, c2);

      // do gamma correction then scale back to 0-type_max
      dst_map0[i] = imColorQuantize(imColorTransfer2Nonlinear(c0), type_min, type_max);
      dst_map1[i] = imColorQuantize(imColorTransfer2Nonlinear(c1), type_min, type_max);
      dst_map2[i] = imColorQuantize(imColorTransfer2Nonlinear(c2), type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_YCBCR: 
    zero = (T)imColorZeroShift(data_type);
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
      imColorYCbCr2RGB(src_map0[i], src_map1[i], src_map2[i], 
                       dst_map0[i], dst_map1[i], dst_map2[i], zero, type_min, type_max);
    }
    break;
  case IM_CMYK: 
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // result is still 0-type_max
      imColorCMYK2RGB(src_map0[i], src_map1[i], src_map2[i], src_map3[i], 
                      dst_map0[i], dst_map1[i], dst_map2[i], type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_LUV:
  case IM_LAB:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // to increase precision do intermediate conversions in float

      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);
      float c1 = imColorReconstruct(src_map1[i], type_min, type_max) - 0.5f;
      float c2 = imColorReconstruct(src_map2[i], type_min, type_max) - 0.5f;

      if (src_color_space == IM_LUV)
        imColorLuv2XYZ(c0, c1, c2,  // conversion in-place
                       c0, c1, c2);
      else
        imColorLab2XYZ(c0, c1, c2,  // conversion in-place
                       c0, c1, c2);

      imColorXYZ2RGB(c0, c1, c2,    // conversion in-place
                     c0, c1, c2);

      // do gamma correction then scale back to 0-type_max
      dst_map0[i] = imColorQuantize(imColorTransfer2Nonlinear(c0), type_min, type_max);
      dst_map1[i] = imColorQuantize(imColorTransfer2Nonlinear(c1), type_min, type_max);
      dst_map2[i] = imColorQuantize(imColorTransfer2Nonlinear(c2), type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
IM_STATIC int iDoConvert2YCbCr(int count, int data_type, 
                     const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T zero;

  const T* src_map0 = src_data[0];
  const T* src_map1 = src_data[1];
  const T* src_map2 = src_data[2];
  T* dst_map0 = dst_data[0];
  T* dst_map1 = dst_data[1];
  T* dst_map2 = dst_data[2];

  imCounterTotal(counter, count, "Converting To YCbCr...");

  IM_INT_PROCESSING;

  switch(src_color_space)
  {
  case IM_RGB: 
    zero = (T)imColorZeroShift(data_type);
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      imColorRGB2YCbCr(src_map0[i], src_map1[i], src_map2[i], 
                       dst_map0[i], dst_map1[i], dst_map2[i], zero);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
IM_STATIC int iDoConvert2XYZ(int count, int data_type, 
                   const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T type_max = (T)imColorMax(data_type);
  T type_min = (T)imColorMin(data_type);

  const T* src_map0 = src_data[0];
  const T* src_map1 = (src_color_space == IM_GRAY)? 0: src_data[1];
  const T* src_map2 = (src_color_space == IM_GRAY)? 0: src_data[2];
  T* dst_map0 = dst_data[0];
  T* dst_map1 = dst_data[1];
  T* dst_map2 = dst_data[2];

  imCounterTotal(counter, count, "Converting To XYZ...");

  IM_INT_PROCESSING;

  switch(src_color_space)
  {
  case IM_GRAY: 
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // scale to 0-1
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);

      // then scale back to 0-type_max
      dst_map0[i] = imColorQuantize(c0*0.9505f, type_min, type_max);    // Compensate D65 white point
      dst_map1[i] = imColorQuantize(c0, type_min, type_max);
      dst_map2[i] = imColorQuantize(c0*1.0890f, type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_RGB: 
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // to increase precision do intermediate conversions in float

      // scale to 0-1
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);
      float c1 = imColorReconstruct(src_map1[i], type_min, type_max);
      float c2 = imColorReconstruct(src_map2[i], type_min, type_max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);
      c1 = imColorTransfer2Linear(c1);
      c2 = imColorTransfer2Linear(c2);

      // result is still 0-1
      imColorRGB2XYZ(c0, c1, c2, 
                     c0, c1, c2);

      // then scale back to 0-type_max
      dst_map0[i] = imColorQuantize(c0, type_min, type_max);
      dst_map1[i] = imColorQuantize(c1, type_min, type_max);
      dst_map2[i] = imColorQuantize(c2, type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_LUV:
  case IM_LAB:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // to increase precision do intermediate conversions in float
      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);
      float c1 = imColorReconstruct(src_map1[i], type_min, type_max) - 0.5f;
      float c2 = imColorReconstruct(src_map2[i], type_min, type_max) - 0.5f;

      if (src_color_space == IM_LUV)
        imColorLuv2XYZ(c0, c1, c2,  // conversion in-place
                       c0, c1, c2);
      else
        imColorLab2XYZ(c0, c1, c2,  // conversion in-place
                       c0, c1, c2);

      // scale back to 0-type_max
      dst_map0[i] = imColorQuantize(c0, type_min, type_max);
      dst_map1[i] = imColorQuantize(c1, type_min, type_max);
      dst_map2[i] = imColorQuantize(c2, type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
IM_STATIC int iDoConvert2Lab(int count, int data_type, 
                   const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T type_max = (T)imColorMax(data_type);
  T type_min = (T)imColorMin(data_type);

  const T* src_map0 = src_data[0];
  const T* src_map1 = (src_color_space == IM_GRAY)? 0: src_data[1];
  const T* src_map2 = (src_color_space == IM_GRAY)? 0: src_data[2];
  T* dst_map0 = dst_data[0];
  T* dst_map1 = dst_data[1];
  T* dst_map2 = dst_data[2];

  imCounterTotal(counter, count, "Converting To Lab...");

  IM_INT_PROCESSING;

  switch(src_color_space)
  {
  case IM_GRAY: 
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // scale to 0-1
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);

      // do conversion
      c0 = imColorLuminance2Lightness(c0);

      // then scale back to 0-type_max
      dst_map0[i] = imColorQuantize(c0, type_min, type_max);  // update only the L component

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_RGB: 
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // to increase precision do intermediate conversions in float

      // scale to 0-1
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);
      float c1 = imColorReconstruct(src_map1[i], type_min, type_max);
      float c2 = imColorReconstruct(src_map2[i], type_min, type_max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);
      c1 = imColorTransfer2Linear(c1);
      c2 = imColorTransfer2Linear(c2);

      imColorRGB2XYZ(c0, c1, c2,  // conversion in-place
                     c0, c1, c2);

      imColorXYZ2Lab(c0, c1, c2,  // conversion in-place
                     c0, c1, c2);

      // then scale back to 0-type_max
      dst_map0[i] = imColorQuantize(c0, type_min, type_max);
      dst_map1[i] = imColorQuantize(c1 + 0.5f, type_min, type_max);
      dst_map2[i] = imColorQuantize(c2 + 0.5f, type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_XYZ:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // to increase precision do intermediate conversions in float
      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);
      float c1 = imColorReconstruct(src_map1[i], type_min, type_max);
      float c2 = imColorReconstruct(src_map2[i], type_min, type_max);

      imColorXYZ2Lab(c0, c1, c2,  // conversion in-place
                     c0, c1, c2);

      // scale back to 0-type_max
      dst_map0[i] = imColorQuantize(c0, type_min, type_max);
      dst_map1[i] = imColorQuantize(c1 + 0.5f, type_min, type_max);
      dst_map2[i] = imColorQuantize(c2 + 0.5f, type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_LUV:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // to increase precision do intermediate conversions in float
      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);
      float c1 = imColorReconstruct(src_map1[i], type_min, type_max) - 0.5f;
      float c2 = imColorReconstruct(src_map2[i], type_min, type_max) - 0.5f;

      imColorLuv2XYZ(c0, c1, c2,  // conversion in-place
                     c0, c1, c2);
      imColorXYZ2Lab(c0, c1, c2,  // conversion in-place
                     c0, c1, c2);

      // scale back to 0-type_max
      dst_map0[i] = imColorQuantize(c0, type_min, type_max);
      dst_map1[i] = imColorQuantize(c1 + 0.5f, type_min, type_max);
      dst_map2[i] = imColorQuantize(c2 + 0.5f, type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
IM_STATIC int iDoConvert2Luv(int count, int data_type, 
                   const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T type_max = (T)imColorMax(data_type);
  T type_min = (T)imColorMin(data_type);

  const T* src_map0 = src_data[0];
  const T* src_map1 = (src_color_space == IM_GRAY)? 0: src_data[1];
  const T* src_map2 = (src_color_space == IM_GRAY)? 0: src_data[2];
  T* dst_map0 = dst_data[0];
  T* dst_map1 = dst_data[1];
  T* dst_map2 = dst_data[2];

  imCounterTotal(counter, count, "Converting To Luv...");

  IM_INT_PROCESSING;

  switch(src_color_space)
  {
  case IM_GRAY: 
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // scale to 0-1
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);

      // do conversion
      c0 = imColorLuminance2Lightness(c0);

      // then scale back to 0-type_max
      dst_map0[i] = imColorQuantize(c0, type_min, type_max);  // update only the L component

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_RGB: 
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // to increase precision do intermediate conversions in float

      // scale to 0-1
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);
      float c1 = imColorReconstruct(src_map1[i], type_min, type_max);
      float c2 = imColorReconstruct(src_map2[i], type_min, type_max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);
      c1 = imColorTransfer2Linear(c1);
      c2 = imColorTransfer2Linear(c2);

      imColorRGB2XYZ(c0, c1, c2,  // conversion in-place
                     c0, c1, c2);

      imColorXYZ2Luv(c0, c1, c2,  // conversion in-place
                     c0, c1, c2);

      // then scale back to 0-type_max
      dst_map0[i] = imColorQuantize(c0, type_min, type_max);
      dst_map1[i] = imColorQuantize(c1 + 0.5f, type_min, type_max);
      dst_map2[i] = imColorQuantize(c2 + 0.5f, type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_XYZ:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // to increase precision do intermediate conversions in float
      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);
      float c1 = imColorReconstruct(src_map1[i], type_min, type_max);
      float c2 = imColorReconstruct(src_map2[i], type_min, type_max);

      imColorXYZ2Luv(c0, c1, c2,  // conversion in-place
                     c0, c1, c2);

      // scale back to 0-type_max
      dst_map0[i] = imColorQuantize(c0, type_min, type_max);
      dst_map1[i] = imColorQuantize(c1 + 0.5f, type_min, type_max);
      dst_map2[i] = imColorQuantize(c2 + 0.5f, type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  case IM_LAB:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
    {
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_BEGIN_PROCESSING;

      // to increase precision do intermediate conversions in float
      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(src_map0[i], type_min, type_max);
      float c1 = imColorReconstruct(src_map1[i], type_min, type_max) - 0.5f;
      float c2 = imColorReconstruct(src_map2[i], type_min, type_max) - 0.5f;

      imColorLab2XYZ(c0, c1, c2,  // conversion in-place
                     c0, c1, c2);
      imColorXYZ2Luv(c0, c1, c2,  // conversion in-place
                     c0, c1, c2);

      // scale back to 0-type_max
      dst_map0[i] = imColorQuantize(c0, type_min, type_max);
      dst_map1[i] = imColorQuantize(c1 + 0.5f, type_min, type_max);
      dst_map2[i] = imColorQuantize(c2 + 0.5f, type_min, type_max);

      IM_COUNT_PROCESSING;
#ifdef _OPENMP
      #pragma omp flush (processing)
#endif
      IM_END_PROCESSING;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
IM_STATIC int iDoConvertColorSpace(int count, int data_type, 
                                 const T** src_data, int src_color_space, 
                                       T** dst_data, int dst_color_space)
{
  int ret = IM_ERR_DATA, 
      convert2rgb = 0;

  if ((dst_color_space == IM_XYZ ||
       dst_color_space == IM_LAB ||
       dst_color_space == IM_LUV) && 
      (src_color_space == IM_CMYK ||
       src_color_space == IM_YCBCR))
  {
    convert2rgb = 1;
  }    

  if (dst_color_space == IM_YCBCR && src_color_space != IM_RGB)
    convert2rgb = 1;

#ifdef IM_PROCESS
  int counter = imProcessCounterBegin("Convert Color Space");
#else
  int counter = imCounterBegin("Convert Color Space");
#endif

  if (convert2rgb)
  {
    ret = iDoConvert2RGB(count, data_type, src_data, src_color_space, dst_data, counter);     

    if (ret != IM_ERR_NONE) 
    {
#ifdef IM_PROCESS
      imProcessCounterEnd(counter);
#else
      imCounterEnd(counter);
#endif
      return ret;
    }

    src_data = (const T**)dst_data;
    src_color_space = IM_RGB;
  }

  switch(dst_color_space)
  {
  case IM_GRAY: 
    ret = iDoConvert2Gray(count, data_type, src_data, src_color_space, dst_data, counter);
    break;
  case IM_RGB: 
    ret = iDoConvert2RGB(count, data_type, src_data, src_color_space, dst_data, counter);
    break;
  case IM_YCBCR: 
    ret = iDoConvert2YCbCr(count, data_type, src_data, src_color_space, dst_data, counter); 
    break;
  case IM_XYZ: 
    ret = iDoConvert2XYZ(count, data_type, src_data, src_color_space, dst_data, counter);
    break;
  case IM_LAB: 
    ret = iDoConvert2Lab(count, data_type, src_data, src_color_space, dst_data, counter);
    break;
  case IM_LUV: 
    ret = iDoConvert2Luv(count, data_type, src_data, src_color_space, dst_data, counter);
    break;
  default:
    ret = IM_ERR_DATA;
    break;
  }

#ifdef IM_PROCESS
  imProcessCounterEnd(counter);
#else
  imCounterEnd(counter);
#endif

  return ret;
}

static int iConvertColorSpace(const imImage* src_image, imImage* dst_image)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    return iDoConvertColorSpace(src_image->count, src_image->data_type,
                         (const imbyte**)src_image->data, src_image->color_space, 
                               (imbyte**)dst_image->data, dst_image->color_space);
  case IM_SHORT:
    return iDoConvertColorSpace(src_image->count, src_image->data_type, 
                         (const short**)src_image->data, src_image->color_space, 
                               (short**)dst_image->data, dst_image->color_space);
  case IM_USHORT:
    return iDoConvertColorSpace(src_image->count, src_image->data_type, 
                         (const imushort**)src_image->data, src_image->color_space, 
                               (imushort**)dst_image->data, dst_image->color_space);
  case IM_INT:
    return iDoConvertColorSpace(src_image->count, src_image->data_type,
                         (const int**)src_image->data, src_image->color_space, 
                               (int**)dst_image->data, dst_image->color_space);
  case IM_FLOAT:
    return iDoConvertColorSpace(src_image->count, src_image->data_type, 
                         (const float**)src_image->data, src_image->color_space, 
                               (float**)dst_image->data, dst_image->color_space);
  case IM_CFLOAT:
    /* treat complex as two real values */
    return iDoConvertColorSpace(2*src_image->count, src_image->data_type,
                         (const float**)src_image->data, src_image->color_space, 
                               (float**)dst_image->data, dst_image->color_space);
  }

  return IM_ERR_DATA;
}

#ifdef IM_PROCESS
int imProcessConvertColorSpace(const imImage* src_image, imImage* dst_image)
#else
int imConvertColorSpace(const imImage* src_image, imImage* dst_image)
#endif
{
  int ret = IM_ERR_NONE;
  assert(src_image);
  assert(dst_image);

  if (!imImageMatchDataType(src_image, dst_image))
    return IM_ERR_DATA;

  if (src_image->color_space != dst_image->color_space)
  {
    switch(dst_image->color_space)
    {
    case IM_RGB:
      switch(src_image->color_space)
      {
      case IM_BINARY:
          memcpy(dst_image->data[0], src_image->data[0], dst_image->plane_size);
          iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 255);
          memcpy(dst_image->data[1], dst_image->data[0], dst_image->plane_size);
          memcpy(dst_image->data[2], dst_image->data[0], dst_image->plane_size);
        ret = IM_ERR_NONE;
        break;
      case IM_MAP:
        iConvertMapToRGB((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], (imbyte*)dst_image->data[1], (imbyte*)dst_image->data[2], dst_image->count, src_image->palette, src_image->palette_count);
        ret = IM_ERR_NONE;
        break;
      case IM_GRAY:
          memcpy(dst_image->data[0], src_image->data[0], dst_image->plane_size);
          memcpy(dst_image->data[1], src_image->data[0], dst_image->plane_size);
          memcpy(dst_image->data[2], src_image->data[0], dst_image->plane_size);
        ret = IM_ERR_NONE;
        break;
      default: 
        ret = iConvertColorSpace(src_image, dst_image);
        break;
      }
      break;
    case IM_GRAY:  
      switch(src_image->color_space)
      {
      case IM_BINARY:
        memcpy(dst_image->data[0], src_image->data[0], dst_image->size);
        iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 255);
        ret = IM_ERR_NONE;
        break;
      case IM_MAP:
        iConvertMap2Gray((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], dst_image->count, src_image->palette, src_image->palette_count);
        ret = IM_ERR_NONE;
        break;
      case IM_YCBCR: 
        memcpy(dst_image->data[0], src_image->data[0], dst_image->plane_size);
        ret = IM_ERR_NONE;
        break;
      default:
        ret = iConvertColorSpace(src_image, dst_image);
        break;
      }
      break;
    case IM_MAP:   
      switch(src_image->color_space)
      {
      case IM_BINARY: // no break, same procedure as gray
      case IM_GRAY:
        memcpy(dst_image->data[0], src_image->data[0], dst_image->size);
        dst_image->palette_count = src_image->palette_count;
        memcpy(dst_image->palette, src_image->palette, dst_image->palette_count*sizeof(long));
        ret = IM_ERR_NONE;
        break;
      case IM_RGB:
        dst_image->palette_count = 256;
        ret = imConvertRGB2Map(src_image->width, src_image->height, 
                               (imbyte*)src_image->data[0], (imbyte*)src_image->data[1], (imbyte*)src_image->data[2], 
                               (imbyte*)dst_image->data[0], dst_image->palette, &dst_image->palette_count);
        break;
      default:
        ret = IM_ERR_DATA;
        break;
      }
      break;
    case IM_BINARY:
      switch(src_image->color_space)
      {
      case IM_GRAY:
        memcpy(dst_image->data[0], src_image->data[0], dst_image->size);
        iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 1);
        ret = IM_ERR_NONE;
        break;
      case IM_MAP:           // convert to gray, then convert to binary
        iConvertMap2Gray((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], dst_image->count, src_image->palette, src_image->palette_count);
        iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 1);
        ret = IM_ERR_NONE;
        break;
      case IM_YCBCR:         // convert to gray, then convert to binary
        memcpy(dst_image->data[0], src_image->data[0], dst_image->plane_size);
        iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 1);
        ret = IM_ERR_NONE;
        break;
      default:               // convert to gray, then convert to binary
        dst_image->color_space = IM_GRAY;
        ret = iConvertColorSpace(src_image, dst_image);
        dst_image->color_space = IM_BINARY;
        if (ret == IM_ERR_NONE)
          iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 1);
        ret = IM_ERR_NONE;
        break;
      }
      break;
    case IM_YCBCR: 
      switch(src_image->color_space)
      {
      case IM_GRAY:
        memcpy(dst_image->data[0], src_image->data[0], dst_image->plane_size);
        ret = IM_ERR_NONE;
        break;
      default:
        ret = iConvertColorSpace(src_image, dst_image);
        break;
      }
      break;
    default: 
      ret = iConvertColorSpace(src_image, dst_image);
      break;
    }
  }

  if (src_image->has_alpha && dst_image->has_alpha)
    memcpy(dst_image->data[dst_image->depth], src_image->data[src_image->depth], src_image->plane_size);
  else if (dst_image->color_space == IM_RGB && dst_image->data_type == IM_BYTE && dst_image->has_alpha)
  {
    if (src_image->color_space == IM_RGB)
    {
      imbyte* transp_color = (imbyte*)imImageGetAttribute(src_image, "TransparencyColor", NULL, NULL);
      if (transp_color)
        iConvertSetTranspColor((imbyte**)dst_image->data, dst_image->count, *(transp_color+0), *(transp_color+1), *(transp_color+2));
      else
        memset(dst_image->data[3], 255, dst_image->count);
    }
    else
    {
      int transp_count;
      imbyte* transp_index = (imbyte*)imImageGetAttribute(src_image, "TransparencyIndex", NULL, NULL);
      imbyte* transp_map = (imbyte*)imImageGetAttribute(src_image, "TransparencyMap", NULL, &transp_count);
      if (transp_map)
        iConvertSetTranspMap((imbyte*)src_image->data[0], (imbyte*)dst_image->data[3], dst_image->count, transp_map, transp_count);
      else if (transp_index)
        iConvertSetTranspIndex((imbyte*)src_image->data[0], (imbyte*)dst_image->data[3], dst_image->count, *transp_index);
      else
        memset(dst_image->data[3], 255, dst_image->count);
    }
  }

  return ret;
}

