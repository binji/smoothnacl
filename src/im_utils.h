// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IM_UTILS_H_
#define IM_UTILS_H_

#include <stdlib.h>
#include <im.h>
#include <im_binfile.h>
#include <im_image.h>
#include <memory>

struct ImageDeleter {
  void operator ()(imImage* image) const;
};

struct FileDeleter {
  void operator ()(imFile* file) const;
};

struct MemoryFilenameDeleter {
  void operator ()(imBinMemoryFileName* filename) const;
};

typedef std::unique_ptr<imImage, ImageDeleter> ImagePtr;
typedef std::unique_ptr<imFile, FileDeleter> FilePtr;
typedef std::unique_ptr<imBinMemoryFileName, MemoryFilenameDeleter>
    MemoryFilenamePtr;


inline void ImageDeleter::operator ()(imImage* image) const {
  if (image)
    imImageDestroy(image);
}

inline void FileDeleter::operator ()(imFile* file) const {
  if (file)
    imFileClose(file);
}

inline void MemoryFilenameDeleter::operator ()(
    imBinMemoryFileName* filename) const {
  if (filename)
    free(filename->buffer);
  delete filename;
}

#endif  // IM_UTILS_H_
