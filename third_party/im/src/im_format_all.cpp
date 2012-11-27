/** \file
 * \brief Register all the internal File Format Classes
 *
 * See Copyright Notice in im_lib.h
 */


#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

#include "im.h"
#include "im_format.h"
#include "im_format_all.h"

void imFormatRegisterInternal(void)
{
  // IMPORTANT: RAW format is not registered.

  // The internal formats registration
  imFormatRegisterTIFF();
  imFormatRegisterJPEG();
  imFormatRegisterPNG();
  imFormatRegisterGIF();
  imFormatRegisterBMP();
  imFormatRegisterRAS();
  imFormatRegisterICO();
  imFormatRegisterPNM();
  imFormatRegisterKRN();
  imFormatRegisterLED();
  imFormatRegisterSGI();
  imFormatRegisterPCX();
  imFormatRegisterTGA();
}

