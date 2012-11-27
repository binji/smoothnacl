/** \file
 * \brief Color Processing Operations
 *
 * See Copyright Notice in im_lib.h
 */

#include <im.h>
#include <im_util.h>
#include <im_color.h>
#include <im_colorhsi.h>
#include <im_palette.h>

#include "im_process_counter.h"
#include "im_process_pnt.h"

#include <stdlib.h>
#include <memory.h>


static void rgb2yrgb(imbyte* r, imbyte* g, imbyte* b, imbyte* y)
{
  int ri,gi,bi;

  *y = imColorRGB2Luma(*r, *g, *b);
  ri = (*r) - (*y) + 128;
  gi = (*g) - (*y) + 128;
  bi = (*b) - (*y) + 128;

  if (ri < 0) ri = 0;
  if (gi < 0) gi = 0;
  if (bi < 0) bi = 0;

  *r = (imbyte)ri;
  *g = (imbyte)gi;
  *b = (imbyte)bi;
}

void imProcessSplitYChroma(const imImage* src_image, imImage* y_image, imImage* chroma_image)
{
  imbyte 
    *red=(imbyte*)src_image->data[0],
    *green=(imbyte*)src_image->data[1],
    *blue=(imbyte*)src_image->data[2],
    *red2=(imbyte*)chroma_image->data[0],
    *green2=(imbyte*)chroma_image->data[1],
    *blue2=(imbyte*)chroma_image->data[2],
    *map1=(imbyte*)y_image->data[0];

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(src_image->count))
#endif
  for (int i = 0; i < src_image->count; i++)
  {
    imbyte R = red[i];
    imbyte G = green[i];
    imbyte B = blue[i];
    imbyte Y;

    rgb2yrgb(&R, &G, &B, &Y);

    map1[i] = Y;

    red2[i] = R;
    green2[i] = G;
    blue2[i] = B;
  }
}

static void DoSplitHSIFloat(float** data, float* hue, float* saturation, float* intensity, int count)
{
  float *red=data[0],
      *green=data[1],
       *blue=data[2];

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    imColorRGB2HSI(red[i], green[i], blue[i], &hue[i], &saturation[i], &intensity[i]);
  }
}

static void DoSplitHSIByte(imbyte** data, float* hue, float* saturation, float* intensity, int count)
{
  imbyte *red=data[0],
       *green=data[1],
        *blue=data[2];

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    imColorRGB2HSIbyte(red[i], green[i], blue[i], &hue[i], &saturation[i], &intensity[i]);
  }
}

void imProcessSplitHSI(const imImage* src_image, imImage* dst_image1, imImage* dst_image2, imImage* dst_image3)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    DoSplitHSIByte((imbyte**)src_image->data, (float*)dst_image1->data[0], (float*)dst_image2->data[0], (float*)dst_image3->data[0], src_image->count);
    break;                                                                                                                                    
  case IM_FLOAT:                                                                                                                               
    DoSplitHSIFloat((float**)src_image->data, (float*)dst_image1->data[0], (float*)dst_image2->data[0], (float*)dst_image3->data[0], src_image->count);
    break;                                                                                
  }

  imImageSetPalette(dst_image1, imPaletteHues(), 256);
}

static void DoMergeHSIFloat(float** data, float* hue, float* saturation, float* intensity, int count)
{
  float *red=data[0],
      *green=data[1],
       *blue=data[2];

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    imColorHSI2RGB(hue[i], saturation[i], intensity[i], &red[i], &green[i], &blue[i]);
  }
}

static void DoMergeHSIByte(imbyte** data, float* hue, float* saturation, float* intensity, int count)
{
  imbyte *red=data[0],
       *green=data[1],
        *blue=data[2];

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    imColorHSI2RGBbyte(hue[i], saturation[i], intensity[i], &red[i], &green[i], &blue[i]);
  }
}

void imProcessMergeHSI(const imImage* src_image1, const imImage* src_image2, const imImage* src_image3, imImage* dst_image)
{
  switch(dst_image->data_type)
  {
  case IM_BYTE:
    DoMergeHSIByte((imbyte**)dst_image->data, (float*)src_image1->data[0], (float*)src_image2->data[0], (float*)src_image3->data[0], dst_image->count);
    break;                                                                                                                                    
  case IM_FLOAT:                                                                                                                               
    DoMergeHSIFloat((float**)dst_image->data, (float*)src_image1->data[0], (float*)src_image2->data[0], (float*)src_image3->data[0], dst_image->count);
    break;                                                                                
  }
}

void imProcessSplitComponents(const imImage* src_image, imImage** dst_image)
{
  memcpy(dst_image[0]->data[0], src_image->data[0], src_image->plane_size);
  memcpy(dst_image[1]->data[0], src_image->data[1], src_image->plane_size);
  memcpy(dst_image[2]->data[0], src_image->data[2], src_image->plane_size);
  if (imColorModeDepth(src_image->color_space) == 4 || src_image->has_alpha) 
    memcpy(dst_image[3]->data[0], src_image->data[3], src_image->plane_size);
}

void imProcessMergeComponents(const imImage** src_image, imImage* dst_image)
{
  memcpy(dst_image->data[0], src_image[0]->data[0], dst_image->plane_size);
  memcpy(dst_image->data[1], src_image[1]->data[0], dst_image->plane_size);
  memcpy(dst_image->data[2], src_image[2]->data[0], dst_image->plane_size);
  if (imColorModeDepth(dst_image->color_space) == 4 || dst_image->has_alpha) 
    memcpy(dst_image->data[3], src_image[3]->data[0], dst_image->plane_size);
}

template <class T> 
static void DoNormalizeComp(T** src_data, float** dst_data, int count, int depth)
{
  T* src_pdata[IM_MAXDEPTH];
  float* dst_pdata[IM_MAXDEPTH];

  for(int dt = 0; dt < depth; dt++)
  {
    dst_pdata[dt] = dst_data[dt];
    src_pdata[dt] = src_data[dt];
  }

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    int d;

    float sum = 0;
    for(d = 0; d < depth; d++)
      sum += (float)*(src_pdata[d]);

    for(d = 0; d < depth; d++)
    {
      if (sum == 0)
        dst_pdata[d][i] = 0;
      else
        dst_pdata[d][i] = (float)*(src_pdata[d]) / sum;
    }
  }
}

void imProcessNormalizeComponents(const imImage* src_image, imImage* dst_image)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    DoNormalizeComp((imbyte**)src_image->data, (float**)dst_image->data, src_image->count, src_image->depth);
    break;                                                                                                                                    
  case IM_SHORT:                                                                                                                               
    DoNormalizeComp((short**)src_image->data,  (float**)dst_image->data, src_image->count, src_image->depth);
    break;                                                                                                                                    
  case IM_USHORT:                                                                                                                               
    DoNormalizeComp((imushort**)src_image->data,  (float**)dst_image->data, src_image->count, src_image->depth);
    break;                                                                                                                                    
  case IM_INT:                                                                                                                               
    DoNormalizeComp((int**)src_image->data,  (float**)dst_image->data, src_image->count, src_image->depth);
    break;                                                                                                                                    
  case IM_FLOAT:                                                                                                                               
    DoNormalizeComp((float**)src_image->data, (float**)dst_image->data, src_image->count, src_image->depth);
    break;                                                                                
  }
}

template <class T> 
static void DoReplaceColor(T *src_data, T *dst_data, int count, int depth, float* src_color, float* dst_color)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    int d, equal = 1;
    for (d = 0; d < depth; d++)
    {
      if (src_data[i+d*count] != (T)src_color[d])
      {
        equal = 0;
        break;
      }
    }

    for (d = 0; d < depth; d++)
    {
      if (equal)
        dst_data[i+d*count] = (T)dst_color[d];
      else
        dst_data[i+d*count] = src_data[i+d*count];
    }
  }
}

void imProcessReplaceColor(const imImage* src_image, imImage* dst_image, float* src_color, float* dst_color)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    DoReplaceColor((imbyte*)src_image->data[0],   (imbyte*)dst_image->data[0],   src_image->count, src_image->depth, src_color, dst_color);
    break;                                                                                         
  case IM_SHORT:                                                                                   
    DoReplaceColor((short*)src_image->data[0], (short*)dst_image->data[0], src_image->count, src_image->depth, src_color, dst_color);
    break;                                                                                         
  case IM_USHORT:                                                                                   
    DoReplaceColor((imushort*)src_image->data[0], (imushort*)dst_image->data[0], src_image->count, src_image->depth, src_color, dst_color);
    break;                                                                                         
  case IM_INT:                                                                                     
    DoReplaceColor((int*)src_image->data[0],      (int*)dst_image->data[0],      src_image->count, src_image->depth, src_color, dst_color);
    break;                                                                                         
  case IM_FLOAT:                                                                                   
    DoReplaceColor((float*)src_image->data[0],    (float*)dst_image->data[0],    src_image->count, src_image->depth, src_color, dst_color);
    break;                                                                                
  }
}

template <class ST, class DT> 
static void DoSetAlphaColor(ST *src_data, DT *dst_data, int count, int depth, float* src_color, float dst_alpha)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(count))
#endif
  for (int i = 0; i < count; i++)
  {
    int equal = 1;
    for (int d = 0; d < depth; d++)
    {
      if (src_data[d*count + i] != (ST)src_color[d])
      {
        equal = 0;
        break;
      }
    }

    if (equal)
      dst_data[i] = (DT)dst_alpha;
  }
}

void imProcessSetAlphaColor(const imImage* src_image, imImage* dst_image, float* src_color, float dst_alpha)
{
  int a = 0; // dst_image is a mask to be used as alpha
  if (dst_image->has_alpha)
    a = dst_image->depth; // Index of the alpha channel

  switch(src_image->data_type)
  {
  case IM_BYTE:
    switch(dst_image->data_type)
    {
    case IM_BYTE:
      DoSetAlphaColor((imbyte*)src_image->data[0],   (imbyte*)dst_image->data[a],   src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_SHORT:                                                                                   
      DoSetAlphaColor((imbyte*)src_image->data[0], (short*)dst_image->data[a], src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_USHORT:                                                                                   
      DoSetAlphaColor((imbyte*)src_image->data[0], (imushort*)dst_image->data[a], src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_INT:                                                                                    
      DoSetAlphaColor((imbyte*)src_image->data[0],      (int*)dst_image->data[a],      src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_FLOAT:                                                                                   
      DoSetAlphaColor((imbyte*)src_image->data[0],    (float*)dst_image->data[a],    src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                
    }
    break;                                                                                        
  case IM_SHORT:                                                                                   
    switch(dst_image->data_type)
    {
    case IM_BYTE:
      DoSetAlphaColor((short*)src_image->data[0],   (imbyte*)dst_image->data[a],   src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_SHORT:                                                                                   
      DoSetAlphaColor((short*)src_image->data[0], (short*)dst_image->data[a], src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_USHORT:                                                                                   
      DoSetAlphaColor((short*)src_image->data[0], (imushort*)dst_image->data[a], src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_INT:                                                                                    
      DoSetAlphaColor((short*)src_image->data[0],      (int*)dst_image->data[a],      src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_FLOAT:                                                                                   
      DoSetAlphaColor((short*)src_image->data[0],    (float*)dst_image->data[a],    src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                
    }
    break;                                                                                        
  case IM_USHORT:                                                                                   
    switch(dst_image->data_type)
    {
    case IM_BYTE:
      DoSetAlphaColor((imushort*)src_image->data[0],   (imbyte*)dst_image->data[a],   src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_SHORT:                                                                                   
      DoSetAlphaColor((imushort*)src_image->data[0], (short*)dst_image->data[a], src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_USHORT:                                                                                   
      DoSetAlphaColor((imushort*)src_image->data[0], (imushort*)dst_image->data[a], src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_INT:                                                                                    
      DoSetAlphaColor((imushort*)src_image->data[0],      (int*)dst_image->data[a],      src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_FLOAT:                                                                                   
      DoSetAlphaColor((imushort*)src_image->data[0],    (float*)dst_image->data[a],    src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                
    }
    break;                                                                                        
  case IM_INT:                                                                                    
    switch(dst_image->data_type)
    {
    case IM_BYTE:
      DoSetAlphaColor((int*)src_image->data[0],   (imbyte*)dst_image->data[a],   src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_SHORT:                                                                                   
      DoSetAlphaColor((int*)src_image->data[0], (short*)dst_image->data[a], src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_USHORT:                                                                                   
      DoSetAlphaColor((int*)src_image->data[0], (imushort*)dst_image->data[a], src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_INT:                                                                                    
      DoSetAlphaColor((int*)src_image->data[0],      (int*)dst_image->data[a],      src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_FLOAT:                                                                                   
      DoSetAlphaColor((int*)src_image->data[0],    (float*)dst_image->data[a],    src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                
    }
    break;                                                                                        
  case IM_FLOAT:                                                                                   
    switch(dst_image->data_type)
    {
    case IM_BYTE:
      DoSetAlphaColor((float*)src_image->data[0],   (imbyte*)dst_image->data[a],   src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_SHORT:                                                                                   
      DoSetAlphaColor((float*)src_image->data[0], (short*)dst_image->data[a], src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_USHORT:                                                                                   
      DoSetAlphaColor((float*)src_image->data[0], (imushort*)dst_image->data[a], src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_INT:                                                                                    
      DoSetAlphaColor((float*)src_image->data[0],      (int*)dst_image->data[a],      src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                        
    case IM_FLOAT:                                                                                   
      DoSetAlphaColor((float*)src_image->data[0],    (float*)dst_image->data[a],    src_image->count, src_image->depth, src_color, dst_alpha);
      break;                                                                                
    }
    break;                                                                                
  }
}

