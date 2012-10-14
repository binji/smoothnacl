#ifndef SMOOTHLIFE_H_
#define SMOOTHLIFE_H_

#include <GLES2/gl2.h>

static const int AA = 0;
static const int KR = 1;
static const int KD = 2;
static const int AN = 3;
static const int AM = 4;
static const int ARB = 5;  // number of real buffers

static const int AF = 0;
static const int KRF = 1;
static const int KDF = 2;
static const int ANF = 3;
static const int AMF = 4;
static const int FFT0 = 5;
static const int FFT1 = 6;
static const int AFB = 7;  // number of Fourier buffers

static const int NX = 512;
static const int NY = 512;
static const int NZ = 1;


void InitializeVbo();
void InitializeTextures();
GLuint MakeProgram(const char* vertex_shader, const char* frag_shader);
void GetSNMLocations(GLuint prog);
void makesnm (GLuint prog, int an, int am, int asnm);

#endif  // SMOOTHLIFE_H_