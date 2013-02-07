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

#include "message_handler.h"
#include <stdio.h>

std::vector<std::string> Split(const std::string& s, char delim) {
  const char kWhitespace[] = " \t\n";
  std::vector<std::string> components;
  size_t start = 0;
  size_t delim_pos;
  do {
    start = s.find_first_not_of(kWhitespace, start);
    if (start == std::string::npos)
      break;

    delim_pos = s.find(delim, start);
    size_t end = s.find_last_not_of(kWhitespace, delim_pos);
    if (end != delim_pos)
      end++;

    components.push_back(s.substr(start, end - start));
    start = delim_pos + 1;
  } while (delim_pos != std::string::npos);

  return components;
}

MessageHandler::MessageHandler() {
}

void MessageHandler::AddHandler(const std::string& name, MessageFunc func) {
  map_.insert(MessageMap::value_type(name, func));
}

void MessageHandler::HandleMessage(const std::string& message) {
  size_t colon = message.find(':');

  std::string function = message.substr(0, colon);
  std::vector<std::string> params;

  if (colon != std::string::npos) {
    params = Split(message.substr(colon + 1), ',');
  }

  MessageMap::iterator func_iter = map_.find(function);
  if (func_iter != map_.end()) {
    printf("Got message: %s\n", message.c_str());
    func_iter->second(params);
  } else {
    printf("Unknown message: %s\n", message.c_str());
  }
}
