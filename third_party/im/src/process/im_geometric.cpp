/** \file
 * \brief Geometric Operations
 *
 * See Copyright Notice in im_lib.h
 */


#include <im.h>
#include <im_util.h>

#include "im_process_counter.h"
#include "im_process_loc.h"
#include "im_math_op.h"

#include <stdlib.h>
#include <memory.h>


static inline void imRect2Polar(float x, float y, float *radius, float *theta)
{
  *radius = sqrtf(x*x + y*y);
  *theta = atan2f(y, x);
}

static inline void imPolar2Rect(float radius, float theta, float *x, float *y)
{
  *x = radius * cosf(theta);
  *y = radius * sinf(theta);
}

static inline void swirl_invtransf(int x, int y, float *xl, float *yl, float k, float xc, float yc)
{
  float radius, theta;
  x -= (int)xc;
  y -= (int)yc;

  imRect2Polar((float)x, (float)y, &radius, &theta);

  theta += k * radius;

  imPolar2Rect(radius, theta, xl, yl);

  *xl += xc;
  *yl += yc;
}

template <class DT, class DTU> 
static int Swirl(int width, int height, DT *src_map, DT *dst_map, 
                         float k, int counter, DTU Dummy, int order)
{
  float xc = float(width/2.);
  float yc = float(height/2.);

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for (int y = 0; y < height; y++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int line_offset = y*width;

    for (int x = 0; x < width; x++)
    {
      float xl, yl;
      swirl_invtransf(x, y, &xl, &yl, k, xc, yc);
                   
      // if inside the original image broad area
      if (xl > 0.0 && yl > 0.0 && xl < width && yl < height)
      {
        if (order == 1)
          dst_map[line_offset+x] = imBilinearInterpolation(width, height, src_map, xl, yl);
        else if (order == 3)
          dst_map[line_offset+x] = imBicubicInterpolation(width, height, src_map, xl, yl, Dummy);
        else
          dst_map[line_offset+x] = imZeroOrderInterpolation(width, height, src_map, xl, yl);
      }
    }

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  return processing;
}

static inline void radial_invtransf(int x, int y, float *xl, float *yl, float k1, float xc, float yc)
{
  float aux;
  x -= (int)xc;
  y -= (int)yc;
  aux = 1.0f + k1*(x*x + y*y);
  *xl = x*aux + xc;
  *yl = y*aux + yc;
}

template <class DT, class DTU> 
static int Radial(int width, int height, DT *src_map, DT *dst_map, 
                         float k1, int counter, DTU Dummy, int order)
{
  float xc = float(width/2.);
  float yc = float(height/2.);
  int diag = (int)sqrt(float(width*width + height*height));

  k1 /= (diag * diag);
         
  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for (int y = 0; y < height; y++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int line_offset = y*width;

    for (int x = 0; x < width; x++)
    {
      float xl, yl;
      radial_invtransf(x, y, &xl, &yl, k1, xc, yc);
                   
      // if inside the original image broad area
      if (xl > 0.0 && yl > 0.0 && xl < width && yl < height)
      {
        if (order == 1)
          dst_map[line_offset+x] = imBilinearInterpolation(width, height, src_map, xl, yl);
        else if (order == 3)
          dst_map[line_offset+x] = imBicubicInterpolation(width, height, src_map, xl, yl, Dummy);
        else
          dst_map[line_offset+x] = imZeroOrderInterpolation(width, height, src_map, xl, yl);
      }
    }

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  return processing;
}

//*******************************************************************************************
//rotate_invtransf
//   shift the center to the origin of the destiny image
//   rotates centrered in the origin
//   shift the origin back to the center of the original image
//*******************************************************************************************

inline void rotate_invtransf(int x, int y, float *xl, float *yl, double cos0, double sin0, float dcx, float dcy, float scx, float scy)
{
  double xr = x+0.5 - dcx;
  double yr = y+0.5 - dcy;
  *xl = float(xr * cos0 - yr * sin0 + scx);
  *yl = float(xr * sin0 + yr * cos0 + scy);
}

template <class DT, class DTU> 
static int RotateCenter(int src_width, int src_height, DT *src_map, 
                        int dst_width, int dst_height, DT *dst_map, 
                        double cos0, double sin0, int counter, DTU Dummy, int order)
{
  float dcx = float(dst_width/2.);
  float dcy = float(dst_height/2.);
  float scx = float(src_width/2.);
  float scy = float(src_height/2.);

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(dst_height))
#endif
  for (int y = 0; y < dst_height; y++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int line_offset = y*dst_width;

    for (int x = 0; x < dst_width; x++)
    {
      float xl, yl;
      rotate_invtransf(x, y, &xl, &yl, cos0, sin0, dcx, dcy, scx, scy);
                   
      // if inside the original image broad area
      if (xl > 0.0 && yl > 0.0 && xl < src_width && yl < src_height)
      {
        if (order == 1)
          dst_map[line_offset+x] = imBilinearInterpolation(src_width, src_height, src_map, xl, yl);
        else if (order == 3)
          dst_map[line_offset+x] = imBicubicInterpolation(src_width, src_height, src_map, xl, yl, Dummy);
        else
          dst_map[line_offset+x] = imZeroOrderInterpolation(src_width, src_height, src_map, xl, yl);
      }
    }

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  return processing;
}

template <class DT, class DTU> 
static int Rotate(int src_width, int src_height, DT *src_map, 
                  int dst_width, int dst_height, DT *dst_map, 
                  double cos0, double sin0, int ref_x, int ref_y, int to_origin, 
                  int counter, DTU Dummy, int order)
{
  float sx = float(ref_x);
  float sy = float(ref_y);
  float dx = sx;
  float dy = sy;
  if (to_origin)
  {
    dx = 0;
    dy = 0;
  }

  IM_INT_PROCESSING;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(dst_height))
#endif
  for (int y = 0; y < dst_height; y++)
  {
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_BEGIN_PROCESSING;

    int line_offset = y*dst_width;

    for (int x = 0; x < dst_width; x++)
    {
      float xl, yl;
      rotate_invtransf(x, y, &xl, &yl, cos0, sin0, dx, dy, sx, sy);
                   
      // if inside the original image broad area
      if (xl > 0.0 && yl > 0.0 && xl < src_width && yl < src_height)
      {
        if (order == 1)
          dst_map[line_offset+x] = imBilinearInterpolation(src_width, src_height, src_map, xl, yl);
        else if (order == 3)
          dst_map[line_offset+x] = imBicubicInterpolation(src_width, src_height, src_map, xl, yl, Dummy);
        else
          dst_map[line_offset+x] = imZeroOrderInterpolation(src_width, src_height, src_map, xl, yl);
      }
    }

    IM_COUNT_PROCESSING;
#ifdef _OPENMP
#pragma omp flush (processing)
#endif
    IM_END_PROCESSING;
  }

  return processing;
}


/********************************************************************************/


template <class DT> 
static void Rotate90(int src_width, 
                   int src_height, 
                   DT *src_map, 
                   DT *dst_map, 
                   int dir)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(src_height))
#endif
  for(int y = 0; y < src_height; y++)
  {
    int yd, xd;

    // dir = clockwise (1) or counter clockwise (-1).
    // dst_width  = src_height
    // dst_height = src_width

    if (dir == 1)  
      xd = y;
    else
      xd = src_height-1 - y;

    int line_offset = y*src_width;

    for(int x = 0; x < src_width; x++)
    {
      if (dir == 1)
        yd = src_width-1 - x;
      else
        yd = x;

      dst_map[yd*src_height + xd] = src_map[line_offset + x];
    }        
  }
}

template <class DT> 
static void Rotate180(int width, 
                   int height, 
                   DT *src_map, 
                   DT *dst_map)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for(int y = 0; y < height; y++)
  {
    int yd = height-1 - y;

    int src_line_offset = y*width;
    int dst_line_offset = yd*width;

    for(int x = 0; x < width; x++)
    {
      int xd = width-1 - x;
      dst_map[dst_line_offset + xd] = src_map[src_line_offset + x];
    }        
  }
}

template <class DT> 
static void Mirror(int width, 
                   int height, 
                   DT *src_map, 
                   DT *dst_map)
{
  if (src_map == dst_map) // check of in-place operation
  {
    int half_width = width/2;

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
    for(int y = 0 ; y < height; y++)
    {
      int line_offset = y*width;

      for(int x = 0 ; x < half_width; x++)
      {
        int xd = width-1 - x;
        DT temp_value = src_map[line_offset + xd];
        src_map[line_offset + xd] = src_map[line_offset + x];
        src_map[line_offset + x] = temp_value;
        xd--;
      }        
    }
  }
  else
  {
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
    for(int y = 0 ; y < height; y++)
    {
      int line_offset = y*width;

      for(int x = 0 ; x < width; x++)
      {
        int xd = width-1 - x;
        dst_map[line_offset + xd] = src_map[line_offset + x];
      }        
    }
  }
}

template <class DT> 
static void Flip(int width, 
                   int height, 
                   DT *src_map, 
                   DT *dst_map)
{
  if (src_map == dst_map) // check of in-place operation
  {
    DT* temp_line = (DT*)malloc(width*sizeof(DT));
    int half_height = height/2;

    // Can NOT run in parallel
    for(int y = 0 ; y < half_height; y++)
    {
      int yd = height-1 - y;
      memcpy(temp_line, dst_map + yd*width, width*sizeof(DT));
      memcpy(dst_map + yd*width, src_map + y*width, width*sizeof(DT));
      memcpy(src_map + y*width, temp_line, width*sizeof(DT));
    }

    free(temp_line);
  }
  else
  {
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
    for(int y = 0 ; y < height; y++)
    {
      int yd = height-1 - y;
      memcpy(dst_map + yd*width, src_map + y*width,width * sizeof(DT));
    }
  }
}

template <class DT> 
static void InterlaceSplit(int width, 
                   int height, 
                   DT *src_map, 
                   DT *dst_map1,
                   DT *dst_map2)
{
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(height))
#endif
  for(int y = 0; y < height; y++)
  {
    int yd = y/2;
    if (y%2)
      memcpy(dst_map2 + yd*width, src_map + y*width, width*sizeof(DT));
    else
      memcpy(dst_map1 + yd*width, src_map + y*width, width*sizeof(DT));
  }
}


/********************************************************************************/


void imProcessRotate90(const imImage* src_image, imImage* dst_image, int dir)
{
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  for (int i = 0; i < src_depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      Rotate90(src_image->width, src_image->height, (imbyte*)src_image->data[i],  (imbyte*)dst_image->data[i], dir);
      break;
    case IM_SHORT:
      Rotate90(src_image->width, src_image->height, (short*)src_image->data[i],  (short*)dst_image->data[i], dir);
      break;
    case IM_USHORT:
      Rotate90(src_image->width, src_image->height, (imushort*)src_image->data[i],  (imushort*)dst_image->data[i], dir);
      break;
    case IM_INT:
      Rotate90(src_image->width, src_image->height, (int*)src_image->data[i],  (int*)dst_image->data[i], dir);
      break;
    case IM_FLOAT:
      Rotate90(src_image->width, src_image->height, (float*)src_image->data[i],  (float*)dst_image->data[i], dir);
      break;
    case IM_CFLOAT:
      Rotate90(src_image->width, src_image->height, (imcfloat*)src_image->data[i],  (imcfloat*)dst_image->data[i], dir);
      break;
    }
  }
}

void imProcessRotate180(const imImage* src_image, imImage* dst_image)
{
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  for (int i = 0; i < src_depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      Rotate180(src_image->width, src_image->height, (imbyte*)src_image->data[i],  (imbyte*)dst_image->data[i]);
      break;
    case IM_SHORT:
      Rotate180(src_image->width, src_image->height, (short*)src_image->data[i],  (short*)dst_image->data[i]);
      break;
    case IM_USHORT:
      Rotate180(src_image->width, src_image->height, (imushort*)src_image->data[i],  (imushort*)dst_image->data[i]);
      break;
    case IM_INT:
      Rotate180(src_image->width, src_image->height, (int*)src_image->data[i],  (int*)dst_image->data[i]);
      break;
    case IM_FLOAT:
      Rotate180(src_image->width, src_image->height, (float*)src_image->data[i],  (float*)dst_image->data[i]);
      break;
    case IM_CFLOAT:
      Rotate180(src_image->width, src_image->height, (imcfloat*)src_image->data[i],  (imcfloat*)dst_image->data[i]);
      break;
    }
  }
}

int imProcessRadial(const imImage* src_image, imImage* dst_image, float k1, int order)
{
  int ret = 0;

  int counter = imProcessCounterBegin("Radial Distort");
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  imCounterTotal(counter, src_depth*dst_image->height, "Processing...");  /* size of the destiny image */

  for (int i = 0; i < src_depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      ret = Radial(src_image->width, src_image->height, (imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], k1, counter, float(0), order);
      break;
    case IM_SHORT:
      ret = Radial(src_image->width, src_image->height, (short*)src_image->data[i], (short*)dst_image->data[i], k1, counter, float(0), order);
      break;
    case IM_USHORT:
      ret = Radial(src_image->width, src_image->height, (imushort*)src_image->data[i], (imushort*)dst_image->data[i], k1, counter, float(0), order);
      break;
    case IM_INT:
      ret = Radial(src_image->width, src_image->height, (int*)src_image->data[i], (int*)dst_image->data[i], k1, counter, float(0), order);
      break;
    case IM_FLOAT:
      ret = Radial(src_image->width, src_image->height, (float*)src_image->data[i], (float*)dst_image->data[i], k1, counter, float(0), order);
      break;
    case IM_CFLOAT:
      ret = Radial(src_image->width, src_image->height, (imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], k1, counter, imcfloat(0,0), order);
      break;
    }

    if (!ret)
      break;
  }

  imProcessCounterEnd(counter);

  return ret;
}

int imProcessSwirl(const imImage* src_image, imImage* dst_image, float k, int order)
{
  int ret = 0;

  int counter = imProcessCounterBegin("Swirl Distort");
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  imCounterTotal(counter, src_depth*dst_image->height, "Processing...");  /* size of the destiny image */

  for (int i = 0; i < src_depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      ret = Swirl(src_image->width, src_image->height, (imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], k, counter, float(0), order);
      break;
    case IM_SHORT:
      ret = Swirl(src_image->width, src_image->height, (short*)src_image->data[i], (short*)dst_image->data[i], k, counter, float(0), order);
      break;
    case IM_USHORT:
      ret = Swirl(src_image->width, src_image->height, (imushort*)src_image->data[i], (imushort*)dst_image->data[i], k, counter, float(0), order);
      break;
    case IM_INT:
      ret = Swirl(src_image->width, src_image->height, (int*)src_image->data[i], (int*)dst_image->data[i], k, counter, float(0), order);
      break;
    case IM_FLOAT:
      ret = Swirl(src_image->width, src_image->height, (float*)src_image->data[i], (float*)dst_image->data[i], k, counter, float(0), order);
      break;
    case IM_CFLOAT:
      ret = Swirl(src_image->width, src_image->height, (imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], k, counter, imcfloat(0,0), order);
      break;
    }

    if (!ret)
      break;
  }

  imProcessCounterEnd(counter);

  return ret;
}

//*******************************************************************************************
//rotate_transf
//   In this case shift to the origin, rotate, but do NOT shift back
//*******************************************************************************************

static void rotate_transf(float cx, float cy, int x, int y, float *xl, float *yl, double cos0, double sin0)
{
  double xr = x+0.5 - cx;
  double yr = y+0.5 - cy;
  *xl = float( xr*cos0 + yr*sin0);
  *yl = float(-xr*sin0 + yr*cos0);
}

void imProcessCalcRotateSize(int width, int height, int *new_width, int *new_height, double cos0, double sin0)
{
  float xl, yl, xmin, xmax, ymin, ymax;
  float wd2 = float(width)/2;
  float hd2 = float(height)/2;

  rotate_transf(wd2, hd2, 0, 0, &xl, &yl, cos0, sin0);
  xmin = xl; ymin = yl;
  xmax = xl; ymax = yl;

  rotate_transf(wd2, hd2, width-1, height-1, &xl, &yl, cos0, sin0);
  xmin = min_op(xmin, xl); ymin = min_op(ymin, yl);
  xmax = max_op(xmax, xl); ymax = max_op(ymax, yl);

  rotate_transf(wd2, hd2, 0, height-1, &xl, &yl, cos0, sin0);
  xmin = min_op(xmin, xl); ymin = min_op(ymin, yl);
  xmax = max_op(xmax, xl); ymax = max_op(ymax, yl);

  rotate_transf(wd2, hd2, width-1, 0, &xl, &yl, cos0, sin0);
  xmin = min_op(xmin, xl); ymin = min_op(ymin, yl);
  xmax = max_op(xmax, xl); ymax = max_op(ymax, yl);

  *new_width = (int)(xmax - xmin + 2.0);
  *new_height = (int)(ymax - ymin + 2.0);
}

int imProcessRotate(const imImage* src_image, imImage* dst_image, double cos0, double sin0, int order)
{
  int ret = 0;

  int counter = imProcessCounterBegin("Rotate");
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  imCounterTotal(counter, src_depth*dst_image->height, "Processing...");  /* size of the destiny image */

  if (src_image->color_space == IM_MAP)
  {
    ret = RotateCenter(src_image->width, src_image->height, (imbyte*)src_image->data[0],  dst_image->width, dst_image->height, (imbyte*)dst_image->data[0], cos0, sin0, counter, float(0), 0);
  }
  else
  {
    for (int i = 0; i < src_depth; i++)
    {
      switch(src_image->data_type)
      {
      case IM_BYTE:
        ret = RotateCenter(src_image->width, src_image->height, (imbyte*)src_image->data[i], dst_image->width, dst_image->height, (imbyte*)dst_image->data[i], cos0, sin0, counter, float(0), order);
        break;
      case IM_SHORT:
        ret = RotateCenter(src_image->width, src_image->height, (short*)src_image->data[i], dst_image->width, dst_image->height, (short*)dst_image->data[i], cos0, sin0, counter, float(0), order);
        break;
      case IM_USHORT:
        ret = RotateCenter(src_image->width, src_image->height, (imushort*)src_image->data[i], dst_image->width, dst_image->height, (imushort*)dst_image->data[i], cos0, sin0, counter, float(0), order);
        break;
      case IM_INT:
        ret = RotateCenter(src_image->width, src_image->height, (int*)src_image->data[i], dst_image->width, dst_image->height, (int*)dst_image->data[i], cos0, sin0, counter, float(0), order);
        break;
      case IM_FLOAT:
        ret = RotateCenter(src_image->width, src_image->height, (float*)src_image->data[i], dst_image->width, dst_image->height, (float*)dst_image->data[i], cos0, sin0, counter, float(0), order);
        break;
      case IM_CFLOAT:
        ret = RotateCenter(src_image->width, src_image->height, (imcfloat*)src_image->data[i], dst_image->width, dst_image->height, (imcfloat*)dst_image->data[i], cos0, sin0, counter, imcfloat(0,0), order);
        break;
      }

      if (!ret)
        break;
    }
   }

  imProcessCounterEnd(counter);

  return ret;
}

int imProcessRotateRef(const imImage* src_image, imImage* dst_image, double cos0, double sin0, int x, int y, int to_origin, int order)
{
  int ret = 0;

  int counter = imProcessCounterBegin("RotateRef");
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  imCounterTotal(counter, src_depth*dst_image->height, "Processing...");  /* size of the destiny image */

  if (src_image->color_space == IM_MAP)
  {
    ret = Rotate(src_image->width, src_image->height, (imbyte*)src_image->data[0],  dst_image->width, dst_image->height, (imbyte*)dst_image->data[0], cos0, sin0, x, y, to_origin, counter, float(0), 0);
  }
  else
  {
    for (int i = 0; i < src_depth; i++)
    {
      switch(src_image->data_type)
      {
      case IM_BYTE:
        ret = Rotate(src_image->width, src_image->height, (imbyte*)src_image->data[i], dst_image->width, dst_image->height, (imbyte*)dst_image->data[i], cos0, sin0, x, y, to_origin, counter, float(0), order);
        break;
      case IM_SHORT:
        ret = Rotate(src_image->width, src_image->height, (short*)src_image->data[i], dst_image->width, dst_image->height, (short*)dst_image->data[i], cos0, sin0, x, y, to_origin, counter, float(0), order);
        break;
      case IM_USHORT:
        ret = Rotate(src_image->width, src_image->height, (imushort*)src_image->data[i], dst_image->width, dst_image->height, (imushort*)dst_image->data[i], cos0, sin0, x, y, to_origin, counter, float(0), order);
        break;
      case IM_INT:
        ret = Rotate(src_image->width, src_image->height, (int*)src_image->data[i], dst_image->width, dst_image->height, (int*)dst_image->data[i], cos0, sin0, x, y, to_origin, counter, float(0), order);
        break;
      case IM_FLOAT:
        ret = Rotate(src_image->width, src_image->height, (float*)src_image->data[i], dst_image->width, dst_image->height, (float*)dst_image->data[i], cos0, sin0, x, y, to_origin, counter, float(0), order);
        break;
      case IM_CFLOAT:
        ret = Rotate(src_image->width, src_image->height, (imcfloat*)src_image->data[i], dst_image->width, dst_image->height, (imcfloat*)dst_image->data[i], cos0, sin0, x, y, to_origin, counter, imcfloat(0,0), order);
        break;
      }

      if (!ret)
        break;
    }
   }

  imProcessCounterEnd(counter);

  return ret;
}

void imProcessMirror(const imImage* src_image, imImage* dst_image)
{
  int i;
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;

  for (i = 0; i < src_depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      Mirror(src_image->width, src_image->height, (imbyte*)src_image->data[i],  (imbyte*)dst_image->data[i]);
      break;
    case IM_SHORT:
      Mirror(src_image->width, src_image->height, (short*)src_image->data[i],  (short*)dst_image->data[i]);
      break;
    case IM_USHORT:
      Mirror(src_image->width, src_image->height, (imushort*)src_image->data[i],  (imushort*)dst_image->data[i]);
      break;
    case IM_INT:
      Mirror(src_image->width, src_image->height, (int*)src_image->data[i],  (int*)dst_image->data[i]);
      break;
    case IM_FLOAT:
      Mirror(src_image->width, src_image->height, (float*)src_image->data[i],  (float*)dst_image->data[i]);
      break;
    case IM_CFLOAT:
      Mirror(src_image->width, src_image->height, (imcfloat*)src_image->data[i],  (imcfloat*)dst_image->data[i]);
      break;
    }
  }
}

void imProcessFlip(const imImage* src_image, imImage* dst_image)
{
  int i;
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;

  for (i = 0; i < src_depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      Flip(src_image->width, src_image->height, (imbyte*)src_image->data[i],  (imbyte*)dst_image->data[i]);
      break;
    case IM_SHORT:
      Flip(src_image->width, src_image->height, (short*)src_image->data[i],  (short*)dst_image->data[i]);
      break;
    case IM_USHORT:
      Flip(src_image->width, src_image->height, (imushort*)src_image->data[i],  (imushort*)dst_image->data[i]);
      break;
    case IM_INT:
      Flip(src_image->width, src_image->height, (int*)src_image->data[i],  (int*)dst_image->data[i]);
      break;
    case IM_FLOAT:
      Flip(src_image->width, src_image->height, (float*)src_image->data[i],  (float*)dst_image->data[i]);
      break;
    case IM_CFLOAT:
      Flip(src_image->width, src_image->height, (imcfloat*)src_image->data[i],  (imcfloat*)dst_image->data[i]);
      break;
    }
  }
}

void imProcessInterlaceSplit(const imImage* src_image, imImage* dst_image1, imImage* dst_image2)
{
  int i;
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;

  for (i = 0; i < src_depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      InterlaceSplit(src_image->width, src_image->height, (imbyte*)src_image->data[i],  (imbyte*)dst_image1->data[i], (imbyte*)dst_image2->data[i]);
      break;
    case IM_SHORT:
      InterlaceSplit(src_image->width, src_image->height, (short*)src_image->data[i],  (short*)dst_image1->data[i], (short*)dst_image2->data[i]);
      break;
    case IM_USHORT:
      InterlaceSplit(src_image->width, src_image->height, (imushort*)src_image->data[i],  (imushort*)dst_image1->data[i], (imushort*)dst_image2->data[i]);
      break;
    case IM_INT:
      InterlaceSplit(src_image->width, src_image->height, (int*)src_image->data[i],  (int*)dst_image1->data[i], (int*)dst_image2->data[i]);
      break;
    case IM_FLOAT:
      InterlaceSplit(src_image->width, src_image->height, (float*)src_image->data[i],  (float*)dst_image1->data[i], (float*)dst_image2->data[i]);
      break;
    case IM_CFLOAT:
      InterlaceSplit(src_image->width, src_image->height, (imcfloat*)src_image->data[i],  (imcfloat*)dst_image1->data[i], (imcfloat*)dst_image2->data[i]);
      break;
    }
  }
}
