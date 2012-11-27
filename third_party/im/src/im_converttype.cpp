/** \file
 * \brief Image Data Type Conversion
 *
 * See Copyright Notice in im_lib.h
 */

#include "im.h"
#include "im_util.h"
#include "im_complex.h"
#include "im_image.h"
#include "im_convert.h"
#include "im_color.h"
#include "im_attrib.h"
#ifdef IM_PROCESS
#include "process/im_process_counter.h"
#include "im_process_pnt.h"
#else
#include "im_counter.h"
#endif

#include <stdlib.h>
#include <stdio.h>
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

/* if gamma is applied then factor contains two conversions
   one for applying gamma,
   and other for conversion to dst_type_min-dst_type_max range.
   because gamma(0) = 0
     For EXP: gamma(x) = (e^(g*x))-1       
     For LOG: gamma(x) = log((g*x)+1)      
   because gamma(1) = 1
     gfactor = exp(g)-1
     gfactor = log(g+1)
*/

inline float iGammaFactor(float range, float gamma)
{
  if (gamma == 0)
    return range;
  else if (gamma < 0)
    return range/float(log((-gamma) + 1));
  else
    return range/float(exp(gamma) - 1);
}

inline float iGammaFunc(float factor, float min, float gamma, float value)
{
  // Here  0<value<1   (always)
  if (gamma != 0)
  {
    if (gamma < 0)
      value = log(value*(-gamma) + 1);
    else
      value = exp(value*gamma) - 1;
  }

  return factor*value + min;
}

template <class T> 
inline int iIsNegativeType(T tmp)
{
  tmp = (T)-1;
  if (tmp > 0)
    return 0;
  else
    return 1;
}

template <class T> 
inline int iDataType(T tmp)
{
  // Discover the data type from the template.
  int size_of = sizeof(T);
  int data_type = IM_BYTE;
  if (size_of == 4)
  {
    tmp = (T)0.1;
    if (tmp == 0)
      data_type = IM_INT;
    else
      data_type = IM_FLOAT;
  }
  else if (size_of == 2)
  {
    tmp = (T)-1;
    if (tmp > 0)
      data_type = IM_USHORT;
    else
      data_type = IM_SHORT;
  }
  return data_type;
}

template <class T> 
inline void iDataTypeIntMinMax(T& type_min, T& type_max, int abssolute)
{
  int data_type = iDataType(type_max);
  type_max = (T)imColorMax(data_type);
  type_min = (T)imColorMin(data_type);
  if (abssolute)
    type_min = 0;
}

template <class T> 
inline void iDataTypeRealMinMax(float& min, float& max, int abssolute, T tmp)
{
  // Used only when converting real<=>int
  max = 1.0f;
  min = 0;

  if (!abssolute)
  {
    if (iIsNegativeType(tmp))
    {
      min = -0.5f;
      max = +0.5f;
    }
  }
}


/**********************************************************************/


template <class SRCT, class DSTT> 
IM_STATIC int iPromoteIntDirect(int count, const SRCT *src_map, DSTT *dst_map)
{
  // small integer to big integer, no need for scale
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    dst_map[i] = (DSTT)(src_map[i]);
  }

  return IM_ERR_NONE;
}
  
template <class SRCT, class DSTT> 
IM_STATIC int iDemoteIntDirect(int count, const SRCT *src_map, DSTT *dst_map, int abssolute)
{
  // big integer to small integer, need to crop
  DSTT dst_type_min, dst_type_max;
  iDataTypeIntMinMax(dst_type_min, dst_type_max, abssolute);

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    SRCT value;

    if (abssolute)
      value = imAbs(src_map[i]);
    else
      value = src_map[i];

    if (value > dst_type_max)
      value = (SRCT)dst_type_max;

    if (value < dst_type_min)
      value = (SRCT)dst_type_min;

    dst_map[i] = (DSTT)(value);
  }

  return IM_ERR_NONE;
}

template <class SRCT, class DSTT> 
IM_STATIC int iPromoteInt(int count, const SRCT *src_map, DSTT *dst_map, int abssolute)
{
  // small integer to big integer, need to shift if necessary
  // also includes ushort <-> short conversion

  // If SRC can has negative values, but DST can't then must shift
  SRCT shift = 0;
  if (!abssolute)
  {
    if (iIsNegativeType(*src_map) && !iIsNegativeType(*dst_map))
    {
      SRCT type_min, type_max;
      iDataTypeIntMinMax(type_min, type_max, abssolute);
      shift = type_min;
    }
  }

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    SRCT value;

    if (abssolute)
      value = imAbs(src_map[i]);
    else
      value = src_map[i] - shift;

    dst_map[i] = (DSTT)(value);
  }

  return IM_ERR_NONE;
}

template <class SRCT, class DSTT> 
IM_STATIC int iDemoteInt(int count, const SRCT *src_map, DSTT *dst_map, int abssolute, int cast_mode, int counter, imAttribTable* attrib_table)
{
  // big integer to small integer, need to scale down
  SRCT min, max;
  DSTT dst_type_min, dst_type_max;

  if (cast_mode == IM_CAST_MINMAX)  // search for min-max
    imMinMaxType(src_map, count, min, max, abssolute);
  else  
  {
    // IM_CAST_FIXED - use data type limits for min-max
    iDataTypeIntMinMax(min, max, abssolute);

    if (cast_mode == IM_CAST_USER)  // get min,max from atributes
    {
      float* amin = (float*)attrib_table->Get("UserMin");
      if (amin) min = (SRCT)(*amin);
      float* amax = (float*)attrib_table->Get("UserMax");
      if (amax) max = (SRCT)(*amax);
    }
  }

  iDataTypeIntMinMax(dst_type_min, dst_type_max, abssolute);

  int direct = 0; // must scale SRC to fit DST
  if (min >= dst_type_min && max <= dst_type_max)
    direct = 1; // no need for conversion

  float factor = ((float)dst_type_max - (float)dst_type_min + 1.0f) / ((float)max - (float)min + 1.0f);

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
#ifdef _OPENMP
    #pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    SRCT value;
    if (abssolute)
      value = imAbs(src_map[i]);
    else
      value = src_map[i];

    if (value >= max)
      dst_map[i] = dst_type_max;
    else if (value <= min)
      dst_map[i] = dst_type_min;
    else
    {
      if (direct)
        dst_map[i] = (DSTT)value;
      else
        dst_map[i] = (DSTT)imResampleInt(value - min, factor) + dst_type_min;
    }

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
    #pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  return processing;
}


/**********************************************************************/


template <class SRCT> 
IM_STATIC int iPromoteReal(int count, const SRCT *src_map, float *dst_map, float gamma, int abssolute, int cast_mode, int counter, imAttribTable* attrib_table)
{
  // integer to real, always have to scale to 0:1 or -0.5:+0.5
  SRCT min, max;
  float dst_type_min, dst_type_max;

  if (cast_mode == IM_CAST_MINMAX)   // search for min-max
    imMinMaxType(src_map, count, min, max, abssolute);
  else  
  {
    // IM_CAST_FIXED - use data type limits for min-max
    iDataTypeIntMinMax(min, max, abssolute);

    if (cast_mode == IM_CAST_USER)  // get min,max from atributes
    {
      float* amin = (float*)attrib_table->Get("UserMin");
      if (amin) min = (SRCT)(*amin);
      float* amax = (float*)attrib_table->Get("UserMax");
      if (amax) max = (SRCT)(*amax);
    }
  }

  iDataTypeRealMinMax(dst_type_min, dst_type_max, abssolute, *src_map);

  float dst_type_range = 1.0f;
  float range = float(max - min + 1);

  gamma = -gamma; // gamma is inverted here, because we are promoting int2real
  float factor = iGammaFactor(dst_type_range, gamma);

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
#ifdef _OPENMP
    #pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    float fvalue;
    if (abssolute)
      fvalue = (imAbs(src_map[i]) - min + 0.5f)/range; 
    else
      fvalue = (src_map[i] - min + 0.5f)/range; 

    // Now 0 <= fvalue <= 1 (if min-max are correct)

    if (fvalue >= 1)
      dst_map[i] = dst_type_max;
    else if (fvalue <= 0)
      dst_map[i] = dst_type_min;
    else
      dst_map[i] = iGammaFunc(factor, dst_type_min, gamma, fvalue);

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
    #pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  return processing;
}

template <class DSTT> 
IM_STATIC int iDemoteReal(int count, const float *src_map, DSTT *dst_map, float gamma, int abssolute, int cast_mode, int counter, imAttribTable* attrib_table)
{
  // real to integer, always have to scale from 0:1 or -0.5:+0.5
  float min, max;
  DSTT dst_type_min, dst_type_max;

  if (cast_mode == IM_CAST_MINMAX)  // search for min-max
    imMinMaxType(src_map, count, min, max, abssolute);
  else  
  {
    // IM_CAST_FIXED - use data type limits for min-max
    iDataTypeRealMinMax(min, max, abssolute, *dst_map);

    if (cast_mode == IM_CAST_USER)  // get min,max from atributes
    {
      float* amin = (float*)attrib_table->Get("UserMin");
      if (amin) min = *amin;
      float* amax = (float*)attrib_table->Get("UserMax");
      if (amax) max = *amax;
    }
  }

  iDataTypeIntMinMax(dst_type_min, dst_type_max, abssolute);

  int dst_type_range = dst_type_max - dst_type_min + 1;
  float range = max - min;

  float factor = iGammaFactor((float)dst_type_range, gamma);

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
#ifdef _OPENMP
    #pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    float value;
    if (abssolute)
      value = ((float)imAbs(src_map[i]) - min)/range; 
    else
      value = (src_map[i] - min)/range; 

    // Now 0 <= value <= 1 (if min-max are correct)

    if (value >= 1)
      dst_map[i] = dst_type_max;
    else if (value <= 0)
      dst_map[i] = dst_type_min;
    else
    {
      value = iGammaFunc(factor, (float)dst_type_min, gamma, value);
      int ivalue = imRound(value);
      if (ivalue >= dst_type_max)
        dst_map[i] = dst_type_max;
      else if (ivalue <= dst_type_min)
        dst_map[i] = dst_type_min;
      else
        dst_map[i] = (DSTT)imRound(value - 0.5f);
    }

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
    #pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  return processing;
}


/**********************************************************************/


static int iDemoteCpxReal(int count, const imcfloat* src_map, float *dst_map, int cpx2real)
{
  float (*CpxCnv)(const imcfloat& cpx) = NULL;

  switch(cpx2real)
  {
  case IM_CPX_REAL:  CpxCnv = cpxreal; break;
  case IM_CPX_IMAG:  CpxCnv = cpximag; break;
  case IM_CPX_MAG:   CpxCnv = cpxmag; break;
  case IM_CPX_PHASE: CpxCnv = cpxphase; break;
  }

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    dst_map[i] = CpxCnv(src_map[i]);
  }

  return IM_ERR_NONE;
}
                                                                     
template <class DSTT> 
IM_STATIC int iDemoteCpxInt(int count, const imcfloat* src_map, DSTT *dst_map, int cpx2real, float gamma, int abssolute, int cast_mode, int counter, imAttribTable* attrib_table)
{
  float* real_map = (float*)malloc(count*sizeof(float));
  if (!real_map) return IM_ERR_MEM;

  // complex to real
  iDemoteCpxReal(count, src_map, real_map, cpx2real);

  // real to integer
  if (iDemoteReal(count, real_map, dst_map, gamma, abssolute, cast_mode, counter, attrib_table) != IM_ERR_NONE)
  {
    free(real_map);
    return IM_ERR_COUNTER;
  }

  free(real_map);
  return IM_ERR_NONE;
}

template <class SRCT> 
IM_STATIC int iPromoteCpxDirect(int count, const SRCT *src_map, imcfloat *dst_map)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    dst_map[i].real = (float)(src_map[i]);
  }

  return IM_ERR_NONE;
}

template <class SRCT> 
IM_STATIC int iPromoteCpx(int count, const SRCT* src_map, imcfloat *dst_map, float gamma, int abssolute, int cast_mode, int counter, imAttribTable* attrib_table)
{
  float* real_map = (float*)malloc(count*sizeof(float));
  if (!real_map) return IM_ERR_MEM;

  // integer to real
  if (iPromoteReal(count, src_map, real_map, gamma, abssolute, cast_mode, counter, attrib_table) != IM_ERR_NONE)
  {
    free(real_map);
    return IM_ERR_COUNTER;
  }

  // real to complex
  iPromoteCpxDirect(count, real_map, dst_map);

  free(real_map);
  return IM_ERR_NONE;
}


/**********************************************************************/


#ifdef IM_PROCESS
int imProcessConvertDataType(const imImage* src_image, imImage* dst_image, int cpx2real, float gamma, int abssolute, int cast_mode)
#else
int imConvertDataType(const imImage* src_image, imImage* dst_image, int cpx2real, float gamma, int abssolute, int cast_mode)
#endif
{
  assert(src_image);
  assert(dst_image);

  if (!imImageMatchColorSpace(src_image, dst_image))
    return IM_ERR_DATA;

  if (src_image->data_type == dst_image->data_type)
    return IM_ERR_DATA;

  int total_count = src_image->depth * src_image->count;
  int ret = IM_ERR_DATA;
#ifdef IM_PROCESS
  int counter = imProcessCounterBegin("Convert Data Type");
#else
  int counter = imCounterBegin("Convert Data Type");
#endif
  char msg[50];
  sprintf(msg, "Converting to %s...", imDataTypeName(dst_image->data_type));
  imCounterTotal(counter, total_count, msg);

  imAttribTable* attrib_table = (imAttribTable*)(src_image->attrib_table);

  switch(src_image->data_type)
  {
  case IM_BYTE:
    switch(dst_image->data_type)
    {
    case IM_SHORT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteIntDirect(total_count, (const imbyte*)src_image->data[0], (short*)dst_image->data[0]);
      else
        ret = iPromoteInt(total_count, (const imbyte*)src_image->data[0], (short*)dst_image->data[0], abssolute);
      break;
    case IM_USHORT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteIntDirect(total_count, (const imbyte*)src_image->data[0], (imushort*)dst_image->data[0]);
      else
        ret = iPromoteInt(total_count, (const imbyte*)src_image->data[0], (imushort*)dst_image->data[0], abssolute);
      break;
    case IM_INT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteIntDirect(total_count, (const imbyte*)src_image->data[0], (int*)dst_image->data[0]);
      else
        ret = iPromoteInt(total_count, (const imbyte*)src_image->data[0], (int*)dst_image->data[0], abssolute);
      break;
    case IM_FLOAT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteIntDirect(total_count, (const imbyte*)src_image->data[0], (float*)dst_image->data[0]);
      else
        ret = iPromoteReal(total_count, (const imbyte*)src_image->data[0], (float*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_CFLOAT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteCpxDirect(total_count, (const imbyte*)src_image->data[0], (imcfloat*)dst_image->data[0]);
      else
        ret = iPromoteCpx(total_count, (const imbyte*)src_image->data[0], (imcfloat*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    }
    break;
  case IM_SHORT:
    switch(dst_image->data_type)
    {
    case IM_BYTE:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iDemoteIntDirect(total_count, (const short*)src_image->data[0], (imbyte*)dst_image->data[0], abssolute);
      else
        ret = iDemoteInt(total_count, (const short*)src_image->data[0], (imbyte*)dst_image->data[0], abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_USHORT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iDemoteIntDirect(total_count, (const short*)src_image->data[0], (imushort*)dst_image->data[0], abssolute);
      else
        ret = iPromoteInt(total_count, (const short*)src_image->data[0], (imushort*)dst_image->data[0], abssolute);
      break;
    case IM_INT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteIntDirect(total_count, (const short*)src_image->data[0], (int*)dst_image->data[0]);
      else
        ret = iPromoteInt(total_count, (const short*)src_image->data[0], (int*)dst_image->data[0], abssolute);
      break;
    case IM_FLOAT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteIntDirect(total_count, (const short*)src_image->data[0], (float*)dst_image->data[0]);
      else
        ret = iPromoteReal(total_count, (const short*)src_image->data[0], (float*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_CFLOAT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteCpxDirect(total_count, (const short*)src_image->data[0], (imcfloat*)dst_image->data[0]);
      else
        ret = iPromoteCpx(total_count, (const short*)src_image->data[0], (imcfloat*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    }
    break;
  case IM_USHORT:
    switch(dst_image->data_type)
    {
    case IM_BYTE:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iDemoteIntDirect(total_count, (const imushort*)src_image->data[0], (imbyte*)dst_image->data[0], abssolute);
      else
        ret = iDemoteInt(total_count, (const imushort*)src_image->data[0], (imbyte*)dst_image->data[0], abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_SHORT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iDemoteIntDirect(total_count, (const imushort*)src_image->data[0], (short*)dst_image->data[0], abssolute);
      else
        ret = iPromoteInt(total_count, (const imushort*)src_image->data[0], (short*)dst_image->data[0], abssolute);
      break;
    case IM_INT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteIntDirect(total_count, (const imushort*)src_image->data[0], (int*)dst_image->data[0]);
      else
        ret = iPromoteInt(total_count, (const imushort*)src_image->data[0], (int*)dst_image->data[0], abssolute);
      break;
    case IM_FLOAT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteIntDirect(total_count, (const imushort*)src_image->data[0], (float*)dst_image->data[0]);
      else
        ret = iPromoteReal(total_count, (const imushort*)src_image->data[0], (float*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_CFLOAT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteCpxDirect(total_count, (const imushort*)src_image->data[0], (imcfloat*)dst_image->data[0]);
      else
        ret = iPromoteCpx(total_count, (const imushort*)src_image->data[0], (imcfloat*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    }
    break;
  case IM_INT:
    switch(dst_image->data_type)
    {
    case IM_BYTE:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iDemoteIntDirect(total_count, (const int*)src_image->data[0], (imbyte*)dst_image->data[0], abssolute);
      else
        ret = iDemoteInt(total_count, (const int*)src_image->data[0], (imbyte*)dst_image->data[0], abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_SHORT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iDemoteIntDirect(total_count, (const int*)src_image->data[0], (short*)dst_image->data[0], abssolute);
      else
        ret = iDemoteInt(total_count, (const int*)src_image->data[0], (short*)dst_image->data[0], abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_USHORT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iDemoteIntDirect(total_count, (const int*)src_image->data[0], (imushort*)dst_image->data[0], abssolute);
      else
        ret = iDemoteInt(total_count, (const int*)src_image->data[0], (imushort*)dst_image->data[0], abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_FLOAT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteIntDirect(total_count, (const int*)src_image->data[0], (float*)dst_image->data[0]);
      else
        ret = iPromoteReal(total_count, (const int*)src_image->data[0], (float*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_CFLOAT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iPromoteCpxDirect(total_count, (const int*)src_image->data[0], (imcfloat*)dst_image->data[0]);
      else
        ret = iPromoteCpx(total_count, (const int*)src_image->data[0], (imcfloat*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    }
    break;
  case IM_FLOAT:
    switch(dst_image->data_type)
    {
    case IM_BYTE:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iDemoteIntDirect(total_count, (const float*)src_image->data[0], (imbyte*)dst_image->data[0], abssolute);
      else
        ret = iDemoteReal(total_count, (const float*)src_image->data[0], (imbyte*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_SHORT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iDemoteIntDirect(total_count, (const float*)src_image->data[0], (short*)dst_image->data[0], abssolute);
      else
        ret = iDemoteReal(total_count, (const float*)src_image->data[0], (short*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_USHORT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iDemoteIntDirect(total_count, (const float*)src_image->data[0], (imushort*)dst_image->data[0], abssolute);
      else
        ret = iDemoteReal(total_count, (const float*)src_image->data[0], (imushort*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_INT:
      if (cast_mode == IM_CAST_DIRECT)
        ret = iDemoteIntDirect(total_count, (const float*)src_image->data[0], (int*)dst_image->data[0], abssolute);
      else
        ret = iDemoteReal(total_count, (const float*)src_image->data[0], (int*)dst_image->data[0], gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_CFLOAT:
      ret = iPromoteCpxDirect(total_count, (const float*)src_image->data[0], (imcfloat*)dst_image->data[0]);
      break;
    }
    break;
  case IM_CFLOAT:
    switch(dst_image->data_type)                                                                       
    {
    case IM_BYTE:
      ret = iDemoteCpxInt(total_count, (const imcfloat*)src_image->data[0], (imbyte*)dst_image->data[0], cpx2real, gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_SHORT:
      ret = iDemoteCpxInt(total_count, (const imcfloat*)src_image->data[0], (short*)dst_image->data[0], cpx2real, gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_USHORT:
      ret = iDemoteCpxInt(total_count, (const imcfloat*)src_image->data[0], (imushort*)dst_image->data[0], cpx2real, gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_INT:
      ret = iDemoteCpxInt(total_count, (const imcfloat*)src_image->data[0], (int*)dst_image->data[0], cpx2real, gamma, abssolute, cast_mode, counter, attrib_table);
      break;
    case IM_FLOAT:
      ret = iDemoteCpxReal(total_count, (const imcfloat*)src_image->data[0], (float*)dst_image->data[0], cpx2real);
      break;
    }
    break;
  }

#ifdef IM_PROCESS
  imProcessCounterEnd(counter);
#else
  imCounterEnd(counter);
#endif
  return ret;
}
