// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <ppapi/cpp/size.h>
#include "fft_allocation.h"
#include "gpu/wrap_gl.h"

namespace gpu {

typedef float float4[4];

enum TextureOptions {
  TEXTURE_NOFRAMEBUFFER,
  TEXTURE_FRAMEBUFFER,
};

enum TextureFormat {
  FORMAT_REAL,
  FORMAT_COMPLEX,
  FORMAT_4FLOAT,
};

class Texture { 
 public:
  Texture(GLsizei width, GLsizei height, TextureFormat format,
          TextureOptions options);
  ~Texture();

  void BindFramebuffer();
  void Load(const float4* buffer, size_t width, size_t height);
  void Load(const AlignedFloats& buffer);
  void Load(const AlignedReals& buffer);
  void Load(const AlignedComplexes& buffer);
  size_t width() const { return width_; }
  size_t height() const { return height_; }

  ID id() const { return id_; }

 private:
  ID id_;
  ID fb_id_;
  GLsizei width_;
  GLsizei height_;
  GLenum format_;
  TextureOptions options_;

  Texture(const Texture&);  // undefined
  Texture& operator =(const Texture&);  // undefined
};

}  // namespace gpu

#endif  // TEXTURE_H_
