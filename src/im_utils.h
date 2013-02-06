// Copyright 2013 Ben Smith. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
