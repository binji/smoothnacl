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
#ifdef IM_PROCESS
#include "process/im_process_counter.h"
#include "im_process_pnt.h"
#else
#include "im_counter.h"
#endif

#include <stdlib.h>
#include <assert.h>
#include <memory.h>


#ifdef IM_PROCESS
int imProcessConvertToBitmap(const imImage* src_image, imImage* dst_image, int cpx2real, float gamma, int abssolute, int cast_mode)
#else
int imConvertToBitmap(const imImage* src_image, imImage* dst_image, int cpx2real, float gamma, int abssolute, int cast_mode)
#endif
{
  assert(src_image);
  assert(dst_image);

  if (!imImageMatchSize(src_image, dst_image) || !imImageIsBitmap(dst_image))
    return IM_ERR_DATA;

#ifdef IM_PROCESS
  int counter = imProcessCounterBegin("Building Bitmap");
#else
  int counter = imCounterBegin("Building Bitmap");
#endif

  int ret;
  if (src_image->data_type == IM_BYTE)
  {
    // NO data type conversion, only color mode conversion
#ifdef IM_PROCESS
    ret = imProcessConvertColorSpace(src_image, dst_image);
#else
    ret = imConvertColorSpace(src_image, dst_image);
#endif
  }
  else
  {
    if (src_image->color_space == IM_RGB || 
        src_image->color_space == IM_GRAY)
    {
      // data type conversion, but NO color mode conversion
#ifdef IM_PROCESS
      ret = imProcessConvertDataType(src_image, dst_image, cpx2real, gamma, abssolute, cast_mode);
#else
      ret = imConvertDataType(src_image, dst_image, cpx2real, gamma, abssolute, cast_mode);
#endif
    }
    else
    {
      // data type conversion AND color mode conversion
      imImage* temp_image = imImageCreate(src_image->width, src_image->height, dst_image->color_space, src_image->data_type);
      if (!temp_image)
        ret = IM_ERR_MEM;
      else
      {
        // first convert color_mode in the bigger precision
#ifdef IM_PROCESS
        ret = imProcessConvertColorSpace(src_image, temp_image);
#else
        ret = imConvertColorSpace(src_image, temp_image);
#endif
        if (ret == IM_ERR_NONE)
        {
          // second just convert data type
#ifdef IM_PROCESS
          ret = imProcessConvertDataType(temp_image, dst_image, cpx2real, gamma, abssolute, cast_mode);
#else
          ret = imConvertDataType(temp_image, dst_image, cpx2real, gamma, abssolute, cast_mode);
#endif
        }
        imImageDestroy(temp_image);
      }
    }
  }

#ifdef IM_PROCESS
  imProcessCounterEnd(counter);
#else
  imCounterEnd(counter);
#endif

  return ret;
}

