/** \file
 * \brief Unary Arithmetic Operations
 *
 * See Copyright Notice in im_lib.h
 */


#include <im.h>
#include <im_util.h>
#include <im_math.h>
#include <im_complex.h>

#include "im_process_counter.h"
#include "im_process_pnt.h"
#include "im_math_op.h"

#include <stdlib.h>
#include <memory.h>


// Fake complex operations for real types
static inline imbyte conj_op(const imbyte& v) {return v;}
static inline short conj_op(const short& v) {return v;}
static inline imushort conj_op(const imushort& v) {return v;}
static inline int conj_op(const int& v) {return v;}
static inline float conj_op(const float& v) {return v;}
static inline imbyte cpxnorm_op(const imbyte& v) {return v;}
static inline short cpxnorm_op(const short& v) {return v;}
static inline imushort cpxnorm_op(const imushort& v) {return v;}
static inline int cpxnorm_op(const int& v) {return v;}
static inline float cpxnorm_op(const float& v) {return v;}

static inline imcfloat conj_op(const imcfloat& v)
{
  imcfloat r;
  r.real = v.real;
  r.imag = -v.imag;
  return r;
}

static inline imcfloat cpxnorm_op(const imcfloat& v)
{
  imcfloat r;
  float rmag = cpxmag(v);
  if (rmag != 0.0f)
  {
    r.real = v.real/rmag;
    r.imag = v.imag/rmag;
  }
  else
  {
    r.real = 0.0f;
    r.imag = 0.0f;
  }
  return r;
}

template <class T>
inline T positives_op(const T& v)
{
  return v > 0? v: 0;
}

template <class T>
inline T negatives_op(const T& v)
{
  return v > 0? 0: v;
}

template <class T1, class T2> 
static void DoUnaryOp(T1 *map, T2 *new_map, int count, int op)
{
  int i;

  switch(op)
  {
  case IM_UN_ABS:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = abs_op((T2)map[i]);
    break;
  case IM_UN_INV:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = inv_op((T2)map[i]);
    break;
  case IM_UN_EQL:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (T2)map[i];
    break;
  case IM_UN_LESS:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = less_op((T2)map[i]);
    break;
  case IM_UN_SQR:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = sqr_op((T2)map[i]);
    break;
  case IM_UN_SQRT:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = sqrt_op((T2)map[i]);
    break;
  case IM_UN_LOG:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = log_op((T2)map[i]);
    break;
  case IM_UN_SIN:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = sin_op((T2)map[i]);
    break;
  case IM_UN_COS:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = cos_op((T2)map[i]);
    break;
  case IM_UN_EXP:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = exp_op((T2)map[i]);
    break;
  case IM_UN_CONJ:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = conj_op((T2)map[i]);
    break;
  case IM_UN_CPXNORM:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = cpxnorm_op((T2)map[i]);
    break;
  case IM_UN_POSITIVES:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = positives_op((T2)map[i]);
    break;
  case IM_UN_NEGATIVES:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = negatives_op((T2)map[i]);
    break;
  }
}

template <class T1> 
static void DoUnaryOpByte(T1 *map, imbyte *new_map, int count, int op)
{
  int i;

  switch(op)
  {
  case IM_UN_ABS:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (imbyte)crop_byte(abs_op((int)map[i]));
    break;
  case IM_UN_INV:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (imbyte)crop_byte(inv_op((int)map[i]));   /* will always be 0 */
    break;
  case IM_UN_EQL:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (imbyte)crop_byte((int)map[i]);
    break;
  case IM_UN_LESS:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (imbyte)crop_byte(less_op((int)map[i]));
    break;
  case IM_UN_SQR:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (imbyte)crop_byte(sqr_op((int)map[i]));
    break;
  case IM_UN_SQRT:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (imbyte)crop_byte(sqrt_op((int)map[i]));
    break;
  case IM_UN_LOG:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (imbyte)crop_byte(log_op((int)map[i]));
    break;
  case IM_UN_SIN:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (imbyte)crop_byte(sin_op((int)map[i]));
    break;
  case IM_UN_COS:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (imbyte)crop_byte(cos_op((int)map[i]));
    break;
  case IM_UN_EXP:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      new_map[i] = (imbyte)crop_byte(exp_op((int)map[i]));
    break;
  }
}

void imProcessUnArithmeticOp(const imImage* src_image, imImage* dst_image, int op)
{
  int total_count = src_image->count * src_image->depth;

  switch(src_image->data_type)
  {
  case IM_BYTE:
    if (dst_image->data_type == IM_FLOAT)
      DoUnaryOp((imbyte*)src_image->data[0], (float*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_INT)
      DoUnaryOp((imbyte*)src_image->data[0], (int*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_USHORT)
      DoUnaryOp((imbyte*)src_image->data[0], (imushort*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_SHORT)
      DoUnaryOp((imbyte*)src_image->data[0], (short*)dst_image->data[0], total_count, op);
    else
      DoUnaryOpByte((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], total_count, op);
    break;                                                                                
  case IM_SHORT:
    if (dst_image->data_type == IM_BYTE)
      DoUnaryOpByte((short*)src_image->data[0], (imbyte*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_USHORT)
      DoUnaryOp((short*)src_image->data[0], (imushort*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_INT)
      DoUnaryOp((short*)src_image->data[0], (int*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_FLOAT)
      DoUnaryOp((short*)src_image->data[0], (float*)dst_image->data[0], total_count, op);
    else
      DoUnaryOp((short*)src_image->data[0], (short*)dst_image->data[0], total_count, op);
    break;                                                                                
  case IM_USHORT:
    if (dst_image->data_type == IM_BYTE)
      DoUnaryOpByte((imushort*)src_image->data[0], (imbyte*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_SHORT)
      DoUnaryOp((imushort*)src_image->data[0], (short*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_INT)
      DoUnaryOp((imushort*)src_image->data[0], (int*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_FLOAT)
      DoUnaryOp((imushort*)src_image->data[0], (float*)dst_image->data[0], total_count, op);
    else
      DoUnaryOp((imushort*)src_image->data[0], (imushort*)dst_image->data[0], total_count, op);
    break;                                                                                
  case IM_INT:                                                                           
    if (dst_image->data_type == IM_BYTE)
      DoUnaryOpByte((int*)src_image->data[0], (imbyte*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_SHORT)
      DoUnaryOp((int*)src_image->data[0], (short*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_USHORT)
      DoUnaryOp((int*)src_image->data[0], (imushort*)dst_image->data[0], total_count, op);
    else if (dst_image->data_type == IM_FLOAT)
      DoUnaryOp((int*)src_image->data[0], (float*)dst_image->data[0], total_count, op);
    else
      DoUnaryOp((int*)src_image->data[0], (int*)dst_image->data[0], total_count, op);
    break;                                                                                
  case IM_FLOAT:                                                                           
    DoUnaryOp((float*)src_image->data[0], (float*)dst_image->data[0], total_count, op);
    break;                                                                                
  case IM_CFLOAT:            
    DoUnaryOp((imcfloat*)src_image->data[0], (imcfloat*)dst_image->data[0], total_count, op);
    break;
  }
}

void imProcessSplitComplex(const imImage* src_image, imImage* dst_image1, imImage* dst_image2, int polar)
{
  int total_count = src_image->count*src_image->depth;

  imcfloat* map = (imcfloat*)src_image->data[0];
  float* map1 = (float*)dst_image1->data[0];
  float* map2 = (float*)dst_image2->data[0];

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(total_count))
#endif
  for (int i = 0; i < total_count; i++)
  {
    if (polar)
    {
      map1[i] = cpxmag(map[i]);
      map2[i] = cpxphase(map[i]);
    }
    else
    {
      map1[i] = map[i].real;
      map2[i] = map[i].imag;
    }
  }
}
                  
void imProcessMergeComplex(const imImage* src_image1, const imImage* src_image2, imImage* dst_image, int polar)
{
  int total_count = src_image1->count*src_image1->depth;

  imcfloat* map = (imcfloat*)dst_image->data[0];
  float* map1 = (float*)src_image1->data[0];
  float* map2 = (float*)src_image2->data[0];

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(total_count))
#endif
  for (int i = 0; i < total_count; i++)
  {
    if (polar)
    {
      float phase = map2[i];
      if (phase > 180) phase -= 360;   
      phase /= 57.2957795f;

      map[i].real = (float)(map1[i] * cos(phase));
      map[i].imag = (float)(map1[i] * sin(phase));
    }
    else
    {
      map[i].real = map1[i];
      map[i].imag = map2[i];
    }
  }
}
