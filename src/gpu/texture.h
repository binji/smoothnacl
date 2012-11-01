// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <GLES2/gl2.h>
#include "fft_allocation.h"

enum TextureOptions {
  TEXTURE_NOFRAMEBUFFER,
  TEXTURE_FRAMEBUFFER,
};

class Texture { 
 public:
  Texture(GLsizei width, GLsizei height, GLenum format, TextureOptions options);
  ~Texture();

  void BindFramebuffer();
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

#endif  // TEXTURE_H_
