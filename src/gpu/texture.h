// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <GLES2/gl2.h>
#include <ppapi/cpp/size.h>
#include "fft_allocation.h"

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
  void Load(const AlignedReals& buffer);
  void Load(const AlignedComplexes& buffer);

  GLuint id() const { return id_; }

 private:
  GLuint id_;
  GLuint fb_id_;
  GLsizei width_;
  GLsizei height_;
  GLenum format_;
  TextureOptions options_;
};

}  // namespace gpu

#endif  // TEXTURE_H_
