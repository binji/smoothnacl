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

#include "post_buffer_task.h"
#include <algorithm>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/var_array_buffer.h>
#include <stdio.h>


namespace {

struct PostMessageData {
  PostMessageData() : instance(NULL), buffer(0) {}
  pp::Instance* instance;
  pp::VarArrayBuffer buffer;
};

}  // namespace

PostBufferTask::PostBufferTask(pp::Instance* instance, AlignedReals* buffer,
                               int request_id)
    : instance_(instance),
      buffer_(buffer),
      request_id_(request_id) {
}

void PostBufferTask::Run(WorkerThread*) {
  size_t size = buffer_->byte_size();
  pp::VarArrayBuffer array_buffer(sizeof(request_id_) + size);
  char* var_data = static_cast<char*>(array_buffer.Map());
  memcpy(var_data, &request_id_, sizeof(request_id_));
  memcpy(var_data + sizeof(request_id_), buffer_->data(), size);
  array_buffer.Unmap();

  PostMessageData* post_message_data = new PostMessageData;
  post_message_data->instance = instance_;
  post_message_data->buffer = array_buffer;

  pp::Module::Get()->core()->CallOnMainThread(
      0,
      pp::CompletionCallback(&PostBufferTask::MainThreadRun,
                             post_message_data));
}

void PostBufferTask::MainThreadRun(void* user_data, int32_t result) {
  PostMessageData* data = static_cast<PostMessageData*>(user_data);
  data->instance->PostMessage(data->buffer);
  delete data;
}
