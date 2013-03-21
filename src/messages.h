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

#ifndef MESSAGES_H_
#define MESSAGES_H_

#include <string>
#include <vector>

namespace pp {
class Fullscreen;
}  // namespace pp

class SimulationThread;

namespace msg {

typedef std::vector<std::string> ParamList;

void Clear(SimulationThread* thread, const ParamList& params);
void GetBuffer(SimulationThread* thread, const ParamList& params);
void SetBrush(const ParamList& params, double* out_radius, double* out_color);
void SetDrawOptions(SimulationThread* thread, const ParamList& params);
void SetFullscreen(const ParamList& params, pp::Fullscreen* fullscreen);
void SetKernel(SimulationThread* thread, const ParamList& params);
void SetPalette(SimulationThread* thread, const ParamList& params);
void SetRunOptions(SimulationThread* thread, const ParamList& params);
void SetSmoother(SimulationThread* thread, const ParamList& params);
void Splat(SimulationThread* thread, const ParamList& params);

}  // namespace msg

#endif  // MESSAGES_H_
