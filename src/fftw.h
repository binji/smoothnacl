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

#ifndef FFTW_H_
#define FFTW_H_

#include <fftw3.h>

#ifdef USE_FLOAT

#define fftw_cleanup_threads           fftwf_cleanup_threads
#define fftw_complex                   fftwf_complex
#define fftw_destroy_plan              fftwf_destroy_plan
#define fftw_execute                   fftwf_execute
#define fftw_free                      fftwf_free
#define fftw_import_wisdom_from_string fftwf_import_wisdom_from_string
#define fftw_init_threads              fftwf_init_threads
#define fftw_malloc                    fftwf_malloc
#define fftw_plan                      fftwf_plan
#define fftw_plan_dft_c2r_2d           fftwf_plan_dft_c2r_2d
#define fftw_plan_dft_r2c_2d           fftwf_plan_dft_r2c_2d
#define fftw_plan_with_nthreads        fftwf_plan_with_nthreads

#else

#define fftw_cleanup_threads           fftw_cleanup_threads
#define fftw_complex                   fftw_complex
#define fftw_destroy_plan              fftw_destroy_plan
#define fftw_execute                   fftw_execute
#define fftw_free                      fftw_free
#define fftw_import_wisdom_from_string fftw_import_wisdom_from_string
#define fftw_init_threads              fftw_init_threads
#define fftw_malloc                    fftw_malloc
#define fftw_plan                      fftw_plan
#define fftw_plan_dft_c2r_2d           fftw_plan_dft_c2r_2d
#define fftw_plan_dft_r2c_2d           fftw_plan_dft_r2c_2d
#define fftw_plan_with_nthreads        fftw_plan_with_nthreads

#endif

#endif  // FFTW_H_
