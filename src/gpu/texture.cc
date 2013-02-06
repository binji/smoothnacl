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

#include "gpu/texture.h"
#include <algorithm>
#include <assert.h>

namespace gpu {
namespace {

GLenum GetGLFormat(TextureFormat format) {
  switch (format) {
    case FORMAT_REAL: return GL_LUMINANCE;
    case FORMAT_COMPLEX: return GL_RGBA;
    case FORMAT_4FLOAT: return GL_RGBA;
    default: assert(0);
  }
}

}  // namespace

Texture::Texture(GLsizei width, GLsizei height, TextureFormat format,
                 TextureOptions options)
    : width_(width),
      height_(height),
      format_(GetGLFormat(format)),
      options_(options) {
  glGenTextures(1, &id_);
  glBindTexture(GL_TEXTURE_2D, id_);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, format_, width, height, 0, format_, GL_FLOAT,
               NULL);

  if (options == TEXTURE_FRAMEBUFFER) {
    glGenFramebuffers(1, &fb_id_);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           id_, 0);
    /*
    GLint err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
      printf("glCheckFramebufferStatus: %d\n", err);
    }
    */
  }
}

Texture::~Texture() {
}

void Texture::BindFramebuffer() {
  assert(options_ == TEXTURE_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, fb_id_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         id_, 0);
}

void Texture::Load(const AlignedFloats& buffer) {
  assert(buffer.size().width() == width_);
  assert(buffer.size().height() == height_);
  assert(GL_LUMINANCE == format_);
  glBindTexture(GL_TEXTURE_2D, id_);
  glTexImage2D(GL_TEXTURE_2D, 0, format_, width_, height_, 0, format_,
               GL_FLOAT, &buffer[0]);
}

void Texture::Load(const AlignedReals& buffer) {
  assert(buffer.size().width() == width_);
  assert(buffer.size().height() == height_);
  assert(GL_LUMINANCE == format_);
  float* temp = new float [width_ * height_];
  std::fill(temp, temp + width_ * height_, 0);
  for (int y = 0; y < height_; ++y)
  for (int x = 0; x < width_; ++x) {
    temp[y * width_ + x] = static_cast<float>(buffer[y * width_ + x]);
  }
  glBindTexture(GL_TEXTURE_2D, id_);
  glTexImage2D(GL_TEXTURE_2D, 0, format_, width_, height_, 0, format_,
               GL_FLOAT, temp);
  delete [] temp;
}

void Texture::Load(const float4* buffer, size_t width, size_t height) {
  assert(width == width_);
  assert(height == height_);
  assert(GL_RGBA == format_);
  glBindTexture(GL_TEXTURE_2D, id_);
  glTexImage2D(GL_TEXTURE_2D, 0, format_, width_, height_, 0, format_,
               GL_FLOAT, &buffer[0]);
}

void Texture::Load(const AlignedComplexes& buffer) {
  assert(buffer.size().width()/2 + 1 == width_);
  assert(buffer.size().height() == height_);
  assert(GL_RGBA == format_);
  float* temp = new float [width_ * height_ * 4];
  std::fill(temp, temp + width_ * height_ * 4, 0);
  for (int y = 0; y < height_; ++y)
  for (int x = 0; x < width_; ++x) {
    float* pixel = &temp[y * (width_ * 4) + x];
    pixel[0] = buffer[y * width_ + x][0];
    pixel[1] = buffer[y * width_ + x][1];
  }
  glBindTexture(GL_TEXTURE_2D, id_);
  glTexImage2D(GL_TEXTURE_2D, 0, format_, width_, height_, 0, format_,
               GL_FLOAT, temp);
  delete [] temp;
}

}  // namespace gpu
