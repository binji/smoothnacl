/** \file
 * \brief Additional Image Quantization Operations
 *
 * See Copyright Notice in im_lib.h
 */


#include <im.h>
#include <im_util.h>
#include <im_palette.h>
#include <im_math.h>

#include "im_process_counter.h"
#include "im_process_pnt.h"

#include <stdlib.h>
#include <memory.h>


void imProcessQuantizeRGBUniform(const imImage* src_image, imImage* dst_image, int dither)
{
  imbyte *dst_map=(imbyte*)dst_image->data[0], 
         *red_map=(imbyte*)src_image->data[0],
         *green_map=(imbyte*)src_image->data[1],
         *blue_map=(imbyte*)src_image->data[2];

  imImageSetPalette(dst_image, imPaletteUniform(), 256);

#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINHEIGHT(src_image->height))
#endif
  for (int y = 0; y < src_image->height; y++)
  {
    int line_offset = y*src_image->width;
    for (int x = 0; x < src_image->width; x++)
    {                         
      int offset = line_offset+x;
      if (dither)
        dst_map[offset] = (imbyte)imPaletteUniformIndexHalftoned(imColorEncode(red_map[offset], green_map[offset], blue_map[offset]), x, y);
      else
        dst_map[offset] = (imbyte)imPaletteUniformIndex(imColorEncode(red_map[offset], green_map[offset], blue_map[offset]));
    }
  }
}

void imProcessQuantizeGrayUniform(const imImage* src_image, imImage* dst_image, int grays)
{
  int i;

  imbyte *dst_map=(imbyte*)dst_image->data[0], 
         *src_map=(imbyte*)src_image->data[0];

  imbyte re_map[256];
  memset(re_map, 0, 256);

  float factor = (float)grays/256.0f;
  float factor256 = 256.0f/(float)grays;

  for (i = 0; i < 256; i++)  // for all src values
  {             
    int value = imResampleInt(i, factor);     // from 0-255 to 0-(grays-1)  => this will discart information
    value = imResampleInt(value, factor256);  // from 0-(grays-1) back to 0-255
    re_map[i] = (imbyte)IM_BYTECROP(value);
  }

  int total_count = src_image->count*src_image->depth;
#ifdef _OPENMP
#pragma omp parallel for if (IM_OMP_MINCOUNT(total_count))
#endif
  for (i = 0; i < total_count; i++)
    dst_map[i] = re_map[src_map[i]];
}
