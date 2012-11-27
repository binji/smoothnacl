/** \file
 * \brief libTIFF I/O and error handlers.
 * I/O uses imBinFile instead of libTIFF original handlers.
 *
 * See Copyright Notice in im_lib.h
 */

#include "tiffiop.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <memory.h>

static tmsize_t iTIFFReadProc(thandle_t fd, void* buf, tmsize_t size)
{
  imBinFile* file_bin = (imBinFile*)fd;
  return imBinFileRead(file_bin, buf, (unsigned long)size, 1);
}

static tmsize_t iTIFFWriteProc(thandle_t fd, void* buf, tmsize_t size)
{
  imBinFile* file_bin = (imBinFile*)fd;
  return imBinFileWrite(file_bin, buf, (unsigned long)size, 1);
}

static toff_t iTIFFSeekProc(thandle_t fd, toff_t off, int whence)
{
  imBinFile* file_bin = (imBinFile*)fd;
  switch (whence)
  {
  case SEEK_SET:
    imBinFileSeekTo(file_bin, (unsigned long)off);
    break;
  case SEEK_CUR:
    imBinFileSeekOffset(file_bin, (unsigned long)off);
    break;
  case SEEK_END: 
    imBinFileSeekFrom(file_bin, (unsigned long)off);
    break;
  }

  return imBinFileTell(file_bin);
}

static int iTIFFCloseProc(thandle_t fd)
{
  imBinFile* file_bin = (imBinFile*)fd;
  imBinFileClose(file_bin);
  return 0;
}

static toff_t iTIFFSizeProc(thandle_t fd)
{
  imBinFile* file_bin = (imBinFile*)fd;
  return imBinFileSize(file_bin);
}

static int iTIFFMapProc(thandle_t fd, void** pbase, toff_t* psize)
{
  (void) fd; (void) pbase; (void) psize;
  return (0);
}

static void iTIFFUnmapProc(thandle_t fd, void* base, toff_t size)
{
  (void) fd; (void) base; (void) size;
}

TIFF* TIFFFdOpen(int fd, const char* name, const char* mode)
{
  TIFF* tif;

  tif = TIFFClientOpen(name, mode, (thandle_t) fd,  iTIFFReadProc, iTIFFWriteProc,
                                                    iTIFFSeekProc, iTIFFCloseProc, 
                                                    iTIFFSizeProc, iTIFFMapProc, 
                                                    iTIFFUnmapProc);
  if (tif)
    tif->tif_fd = fd;

  return (tif);
}

TIFF* TIFFOpen(const char* name, const char* mode)
{
  imBinFile* bin_file;
  TIFF* tiff;

  if (mode[0] == 'r')
    bin_file = imBinFileOpen(name);
  else
    bin_file = imBinFileNew(name);

  if (!bin_file)
    return NULL;
  
  tiff = TIFFClientOpen(name, mode, (thandle_t)bin_file,  iTIFFReadProc, iTIFFWriteProc,
                                                          iTIFFSeekProc, iTIFFCloseProc, 
                                                          iTIFFSizeProc, iTIFFMapProc, 
                                                          iTIFFUnmapProc);
  if (!tiff)
    imBinFileClose(bin_file);

  return tiff;
}

void* _TIFFmalloc(tmsize_t s)
{
  return (malloc((size_t) s));
}

void _TIFFfree(void* p)
{
  free(p);
}

void* _TIFFrealloc(void* p, tmsize_t s)
{
  return (realloc(p, (size_t) s));
}

void _TIFFmemset(void* p, int v, tmsize_t c)
{
  memset(p, v, (size_t) c);
}

void _TIFFmemcpy(void* d, const void* s, tmsize_t c)
{
  memcpy(d, s, (size_t) c);
}

int _TIFFmemcmp(const void* p1, const void* p2, tmsize_t c)
{
  return (memcmp(p1, p2, (size_t) c));
}

TIFFErrorHandler _TIFFwarningHandler = NULL;
TIFFErrorHandler _TIFFerrorHandler = NULL;
