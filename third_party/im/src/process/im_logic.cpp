/** \file
 * \brief Logical Arithmetic Operations
 *
 * See Copyright Notice in im_lib.h
 */


#include <im.h>
#include <im_util.h>

#include "im_process_counter.h"
#include "im_process_pnt.h"

#include <stdlib.h>
#include <memory.h>

template <class T> 
static void DoBitwiseOp(T *map1, T *map2, T *map, int count, int op)
{
  int i;

  switch(op)
  {
  case IM_BIT_AND:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      map[i] = map1[i] & map2[i];
    break;
  case IM_BIT_OR:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      map[i] = map1[i] | map2[i];
    break;
  case IM_BIT_XOR:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      map[i] = (T)~(map1[i] | map2[i]);
    break;
  }
}

void imProcessBitwiseOp(const imImage* src_image1, const imImage* src_image2, imImage* dst_image, int op)
{
  int count = src_image1->count*src_image1->depth;

  switch(src_image1->data_type)
  {
  case IM_BYTE:
    DoBitwiseOp((imbyte*)src_image1->data[0], (imbyte*)src_image2->data[0], (imbyte*)dst_image->data[0], count, op);
    break;                                                                                
  case IM_SHORT:
    DoBitwiseOp((short*)src_image1->data[0], (short*)src_image2->data[0], (short*)dst_image->data[0], count, op);
    break;                                                                                
  case IM_USHORT:
    DoBitwiseOp((imushort*)src_image1->data[0], (imushort*)src_image2->data[0], (imushort*)dst_image->data[0], count, op);
    break;                                                                                
  case IM_INT:                                                                           
    DoBitwiseOp((int*)src_image1->data[0], (int*)src_image2->data[0], (int*)dst_image->data[0], count, op);
    break;                                                                                
  }
}

template <class T> 
static void DoBitwiseNot(T *map1, T *map, int count)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
    map[i] = ~map1[i];
}

static void DoBitwiseNotBin(imbyte *map1, imbyte *map, int count)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
    map[i] = map1[i]? 0: 1;
}

void imProcessBitwiseNot(const imImage* src_image, imImage* dst_image)
{
  int count = src_image->count*src_image->depth;

  if (dst_image->color_space == IM_BINARY)
  {
    DoBitwiseNotBin((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], count);
    return;
  }

  switch(src_image->data_type)
  {
  case IM_BYTE:
    DoBitwiseNot((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], count);
    break;                                                                                
  case IM_SHORT:
    DoBitwiseNot((short*)src_image->data[0], (short*)dst_image->data[0], count);
    break;                                                                                
  case IM_USHORT:
    DoBitwiseNot((imushort*)src_image->data[0], (imushort*)dst_image->data[0], count);
    break;                                                                                
  case IM_INT:                                                                           
    DoBitwiseNot((int*)src_image->data[0], (int*)dst_image->data[0], count);
    break;                                                                                
  }
}

void imProcessBitMask(const imImage* src_image, imImage* dst_image, unsigned char mask, int op)
{
  imbyte* src_map = (imbyte*)src_image->data[0];
  imbyte* dst_map = (imbyte*)dst_image->data[0];
  int i;
  int count = dst_image->count * dst_image->depth;
  switch(op)
  {
  case IM_BIT_AND:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      dst_map[i] = src_map[i] & mask;
    break;
  case IM_BIT_OR:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      dst_map[i] = src_map[i] | mask;
    break;
  case IM_BIT_XOR:
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
    for (i = 0; i < count; i++)
      dst_map[i] = (imbyte)~(src_map[i] | mask);
    break;
  }

  if ((op == IM_BIT_XOR || op == IM_BIT_OR) && dst_image->color_space == IM_BINARY && mask > 1)
    dst_image->color_space = IM_GRAY;
}

void imProcessBitPlane(const imImage* src_image, imImage* dst_image, int plane, int reset)
{
  imbyte mask = imbyte(0x01 << plane);
  if (reset) mask = ~mask;
  imbyte* src_map = (imbyte*)src_image->data[0];
  imbyte* dst_map = (imbyte*)dst_image->data[0];
  int count = dst_image->count * dst_image->depth;
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    if (reset) 
      dst_map[i] = src_map[i] & mask;
    else
      dst_map[i] = (src_map[i] & mask)? 1: 0;
  }
}
