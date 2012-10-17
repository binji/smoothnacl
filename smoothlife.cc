#include "smoothlife.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix.h"

const double PI = 6.28318530718;
#if 0
const float mode = 1;
const float ra = 12.f;
const float rr = 3.f;
const float rb = 12.f;
const float dt = 0.100f;
const float b1 = 0.278f;
const float b2 = 0.365f;
const float d1 = 0.267f;
const float d2 = 0.445f;
const float sigmode = 4;
const float sigtype = 4;
const float mixtype = 4;
const float sn = 0.028f;
const float sm = 0.147f;
#else
//2 1   31.8  3.0  31.8  0.157   0.092  0.098  0.256  0.607   4 4 4   0.015  0.340    // 2d alien stuff
const float mode = 3;
const float ra = 31.8;
const float rr = 3.f;
const float rb = 31.8;
const float dt = 0.157;
const float b1 = 0.092;
const float b2 = 0.098;
const float d1 = 0.256;
const float d2 = 0.607;
const float sigmode = 4;
const float sigtype = 4;
const float mixtype = 4;
const float sn = 0.015;
const float sm = 0.340;
#endif
const float colscheme = 5;
const float phase = 0;
const int BMAX = 16;

GLuint fb[AFB], tb[AFB];
GLuint fr[ARB], tr[ARB];
GLuint planx[BMAX][2], plany[BMAX][2];

double kflr = 0.f;
double kfld = 0.f;

struct SnmVertex {
  GLfloat tex[6];
  GLfloat loc[3];
};
GLuint g_quad_vbo;
GLuint g_copybufferrc_vbo;
GLuint g_copybuffercr_vbo;
GLuint g_fftstage_vbo;
GLuint g_fftstage2_vbo;

void InitializeVbo() {
  SnmVertex verts[4];
  memset(verts, 0, sizeof(verts));
  verts[1].tex[0] = 1.f;
  verts[1].tex[2] = 1.f;
  verts[1].tex[4] = 1.f;
  verts[1].loc[0] = 1;
  verts[2].tex[1] = 1.f;
  verts[2].tex[3] = 1.f;
  verts[2].tex[5] = 1.f;
  verts[2].loc[1] = 1;
  verts[3].tex[0] = 1.f; verts[3].tex[1] = 1.f;
  verts[3].tex[2] = 1.f; verts[3].tex[3] = 1.f;
  verts[3].tex[4] = 1.f; verts[3].tex[5] = 1.f;
  verts[3].loc[0] = 1; verts[3].loc[1] = 1;

  glGenBuffers(1, &g_quad_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_quad_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts[0], GL_STATIC_DRAW);

  memset(verts, 0, sizeof(verts));
  verts[0].tex[0] = 0-1.f/NX;
  verts[1].tex[0] = 1-1.f/NX;
  verts[1].tex[2] = 1;
  verts[1].loc[0] = NX/2;
  verts[2].tex[0] = 0-1.f/NX; verts[2].tex[1] = 1.f;
  verts[2].tex[3] = 1.f;
  verts[2].loc[1] = NY;
  verts[3].tex[0] = 1-1.f/NX; verts[3].tex[1] = 1.f;
  verts[3].tex[2] = 1.f; verts[3].tex[3] = 1.f;
  verts[3].loc[0] = NX/2; verts[3].loc[1] = NY;

  glGenBuffers(1, &g_copybufferrc_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_copybufferrc_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts[0], GL_STATIC_DRAW);

  memset(verts, 0, sizeof(verts));
  verts[1].tex[0] = 1-1.f/(NX/2+1);
  verts[1].tex[2] = NX;
  verts[1].loc[0] = NX;
  verts[2].tex[1] = 1.f;
  verts[2].tex[3] = 1.f;
  verts[2].loc[1] = NY;
  verts[3].tex[0] = 1-1.f/(NX/2+1); verts[3].tex[1] = 1.f;
  verts[3].tex[2] = NX; verts[3].tex[3] = 1.f;
  verts[3].loc[0] = NX; verts[3].loc[1] = NY;

  glGenBuffers(1, &g_copybuffercr_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_copybuffercr_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts[0], GL_STATIC_DRAW);

  memset(verts, 0, sizeof(verts));
  verts[1].tex[0] = 1.f;
  verts[1].tex[2] = 1.f;
  verts[1].loc[0] = NX/2+1;
  verts[2].tex[1] = 1.f;
  verts[2].tex[3] = 1.f;
  verts[2].loc[1] = NY;
  verts[3].tex[0] = 1.f; verts[3].tex[1] = 1.f;
  verts[3].tex[2] = 1.f; verts[3].tex[3] = 1.f;
  verts[3].loc[0] = NX/2+1; verts[3].loc[1] = NY;

  glGenBuffers(1, &g_fftstage_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_fftstage_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts[0], GL_STATIC_DRAW);

  memset(verts, 0, sizeof(verts));
#if 1
  verts[1].tex[0] = 1-1.f/(NX/2+1);
  verts[1].tex[2] = 1-1.f/(NX/2+1);
  verts[1].loc[0] = NX/2;
  verts[2].tex[1] = 1.f;
  verts[2].tex[3] = 1.f;
  verts[2].loc[1] = NY;
  verts[3].tex[0] = 1-1.f/(NX/2+1); verts[3].tex[1] = 1.f;
  verts[3].tex[2] = 1-1.f/(NX/2+1); verts[3].tex[3] = 1.f;
  verts[3].loc[0] = NX/2; verts[3].loc[1] = NY;
#else
  verts[1].tex[0] = 1;
  verts[1].tex[2] = 1;
  verts[1].loc[0] = NX/2;
  verts[2].tex[1] = 1.f;
  verts[2].tex[3] = 1.f;
  verts[2].loc[1] = NY;
  verts[3].tex[0] = 1; verts[3].tex[1] = 1.f;
  verts[3].tex[2] = 1; verts[3].tex[3] = 1.f;
  verts[3].loc[0] = NX/2; verts[3].loc[1] = NY;
#endif

  glGenBuffers(1, &g_fftstage2_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_fftstage2_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts[0], GL_STATIC_DRAW);
}

void InitializeTextures() {
  glGenTextures(AFB, &tb[0]);
  glGenFramebuffers(AFB, &fb[0]);

  for (int t=0; t<AFB; t++) {
    glBindTexture(GL_TEXTURE_2D, tb[t]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, NX/2+1, NY, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, fb[t]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[t], 0);
    GLint err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
      printf("glCheckFramebufferStatus: %d\n", err);
    }
  }

  glGenTextures(ARB, &tr[0]);
  glGenFramebuffers(ARB, &fr[0]);

  for (int t=0; t<ARB; t++) {
    glBindTexture(GL_TEXTURE_2D, tr[t]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, NX, NY, 0, GL_LUMINANCE, GL_FLOAT, NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, fr[t]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr[t], 0);
    GLint err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
      printf("glCheckFramebufferStatus: %d\n", err);
    }
  }

  glGenTextures((BX-1+2)*2, &planx[0][0]);
  for (int s=0; s<=1; s++) {
    for (int eb=0; eb<=BX-1+1; eb++) {
      glBindTexture(GL_TEXTURE_2D, planx[eb][s]);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, NX/2+1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
    }
  }

  glGenTextures((BY+1)*2, &plany[0][0]);
  for (int s=0; s<=1; s++) {
    for (int eb=1; eb<=BY; eb++) {
      glBindTexture(GL_TEXTURE_2D, plany[eb][s]);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, NY, 1, 0, GL_RGBA, GL_FLOAT, NULL);
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint CompileShader(GLenum type, const char *data) {
  const char *shaderStrings[1];
  shaderStrings[0] = data;

  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, shaderStrings, NULL);
  glCompileShader(shader);

  char buffer[4096];
  GLsizei length;
  glGetShaderInfoLog(shader, 4096, &length, &buffer[0]);
  buffer[length] = 0;
  printf("shaderLog: %s\n", buffer);
  return shader;
}

GLuint MakeProgram(const char* vertex_shader, const char* frag_shader) {
  GLuint vert = CompileShader(GL_VERTEX_SHADER, vertex_shader);
  GLuint frag = CompileShader(GL_FRAGMENT_SHADER, frag_shader);
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  glLinkProgram(prog);

  char buffer[4096];
  GLsizei length;
  glGetProgramInfoLog(prog, 4096, &length, &buffer[0]);
  buffer[length] = 0;
  printf("progLog: %s\n", buffer);

  return prog;
}

void snm(GLuint prog, int an, int am, int na) {
  GLfloat mat[16];

  glBindFramebuffer(GL_FRAMEBUFFER, fr[na]);
  glUseProgram(prog);
  identity_matrix(&mat[0]);
  glhOrtho(&mat[0], 0, 1, 0, 1, -NZ, NZ);
  glViewport(0, 0, NX, NY);
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_mat"), 1, GL_FALSE, &mat[0]);
  glUniform1f(glGetUniformLocation(prog, "mode"), (float)mode);
  glUniform1f(glGetUniformLocation(prog, "dt"), (float)dt);
  glUniform1f(glGetUniformLocation(prog, "b1"), (float)b1);
  glUniform1f(glGetUniformLocation(prog, "b2"), (float)b2);
  glUniform1f(glGetUniformLocation(prog, "d1"), (float)d1);
  glUniform1f(glGetUniformLocation(prog, "d2"), (float)d2);
  glUniform1f(glGetUniformLocation(prog, "sigmode"), (float)sigmode);
  glUniform1f(glGetUniformLocation(prog, "sigtype"), (float)sigtype);
  glUniform1f(glGetUniformLocation(prog, "mixtype"), (float)mixtype);
  glUniform1f(glGetUniformLocation(prog, "sn"), (float)sn);
  glUniform1f(glGetUniformLocation(prog, "sm"), (float)sm);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tr[an]);
  glUniform1i(glGetUniformLocation(prog, "tex0"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tr[am]);
  glUniform1i(glGetUniformLocation(prog, "tex1"), 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, tr[na]);
  glUniform1i(glGetUniformLocation(prog, "tex2"), 2);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr[na], 0);

  GLuint loc_texcoord0 = glGetAttribLocation(prog, "a_texcoord0");
  GLuint loc_texcoord1 = glGetAttribLocation(prog, "a_texcoord1");
  GLuint loc_texcoord2 = glGetAttribLocation(prog, "a_texcoord2");
  GLuint loc_position = glGetAttribLocation(prog, "a_position");
  glBindBuffer(GL_ARRAY_BUFFER, g_quad_vbo);
  glVertexAttribPointer(loc_texcoord0, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, tex));
  glVertexAttribPointer(loc_texcoord1, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)(offsetof(SnmVertex, tex) + sizeof(float) * 2));
  glVertexAttribPointer(loc_texcoord2, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)(offsetof(SnmVertex, tex) + sizeof(float) * 4));
  glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, loc));
  glEnableVertexAttribArray(loc_texcoord0);
  glEnableVertexAttribArray(loc_texcoord1);
  glEnableVertexAttribArray(loc_texcoord2);
  glEnableVertexAttribArray(loc_position);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glUseProgram(0);
}


// initialize an and am with 0 to 1 gradient for drawing of snm (2D only)
//
void initan(int a) {
  float *buf = (float*)calloc(NX*NY, sizeof(float));
  for (int y=0; y<NY; y++) {
    for (int x=0; x<NX; x++) {
      buf[y*NX+x] = (float)x/NX;
    }
  }

  glBindTexture(GL_TEXTURE_2D, tr[a]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, NX, NY, 0, GL_LUMINANCE, GL_FLOAT, buf);

  free(buf);
}


void initam(int a) {
  float *buf = (float*)calloc(NX*NY, sizeof(float));

  for (int y=0; y<NY; y++) {
    for (int x=0; x<NX; x++) {
      buf[y*NX+x] = (float)y/NY;
    }
  }

  glBindTexture(GL_TEXTURE_2D, tr[a]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, NX, NY, 0, GL_LUMINANCE, GL_FLOAT, buf);

  free(buf);
}

void makesnm(GLuint prog, int an, int am, int asnm) {
  initan(an);
  initam(am);
  snm(prog, an, am, asnm);
}

void drawa(GLuint prog, int a) {
  GLfloat mat[16];
  glUseProgram(prog);
  identity_matrix(&mat[0]);
  glhOrtho(&mat[0], 0, 1, 0, 1, -1, 1);
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_mat"), 1, GL_FALSE, &mat[0]);
  glViewport(0, 0, SX, SY);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tr[a]);

  glUniform1i(glGetUniformLocation(prog, "tex0"), 0);
  glUniform1f(glGetUniformLocation(prog, "colscheme"), (float)colscheme);
  glUniform1f(glGetUniformLocation(prog, "phase"), (float)phase);

  GLuint loc_texcoord0 = glGetAttribLocation(prog, "a_texcoord0");
  GLuint loc_position = glGetAttribLocation(prog, "a_position");
  glBindBuffer(GL_ARRAY_BUFFER, g_quad_vbo);
  glVertexAttribPointer(loc_texcoord0, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, tex));
  glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, loc));
  glEnableVertexAttribArray(loc_texcoord0);
  glEnableVertexAttribArray(loc_position);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glUseProgram(0);
}

static int bitreverse(int x, int b) {
  int c, t;

  c = 0;
  for (t=0; t<b; t++) {
    c = (c<<1) | ((x>>t) & 1);
  }
  return c;
}

void fft_planx() {
  float *p = (float*)calloc (4*(NX/2+1), sizeof(float));

  for (int s=0; s<=1; s++) {
    int si = s*2-1;
    for (int eb=0; eb<=BX-1+1; eb++) {
      for (int x=0; x<NX/2+1; x++) {
        if (si==1 && eb==0) {
          *(p + 4*x + 0) = (     x+0.5f)/(float)(NX/2+1);
          *(p + 4*x + 1) = (NX/2-x+0.5f)/(float)(NX/2+1);
          double w = si*PI*(x/(double)NX+0.25);
          *(p + 4*x + 2) = (float)cos(w);
          *(p + 4*x + 3) = (float)sin(w);
        }
        else if (si==-1 && eb==BX) {
          if (x==0 || x==NX/2) {
            *(p + 4*x + 0) = 0;
            *(p + 4*x + 1) = 0;
          } else {
            *(p + 4*x + 0) = (     x)/(float)(NX/2);
            *(p + 4*x + 1) = (NX/2-x)/(float)(NX/2);
          }
          double w = si*PI*(x/(double)NX+0.25);
          *(p + 4*x + 2) = (float)cos(w);
          *(p + 4*x + 3) = (float)sin(w);
        }
        else if (x<NX/2) {
          int l = 1<<eb;
          int j = x%l;
          double w = si*PI*j/l;
          if (j<l/2) {
            if (eb==1) {
              *(p + 4*x + 0) = bitreverse(x    ,BX-1)/(float)(NX/2);
              *(p + 4*x + 1) = bitreverse(x+l/2,BX-1)/(float)(NX/2);
            } else {
              *(p + 4*x + 0) = (x    )/(float)(NX/2);
              *(p + 4*x + 1) = (x+l/2)/(float)(NX/2);
            }
          } else {
            if (eb==1) {
              *(p + 4*x + 0) = bitreverse(x-l/2,BX-1)/(float)(NX/2);
              *(p + 4*x + 1) = bitreverse(x    ,BX-1)/(float)(NX/2);
            } else {
              *(p + 4*x + 0) = (x-l/2)/(float)(NX/2);
              *(p + 4*x + 1) = (x    )/(float)(NX/2);
            }
          }
          *(p + 4*x + 2) = (float)cos(w);
          *(p + 4*x + 3) = (float)sin(w);
        } else {
          *(p + 4*x + 0) = 0;
          *(p + 4*x + 1) = 0;
          *(p + 4*x + 2) = 0;
          *(p + 4*x + 3) = 0;
        }
      }

      glBindTexture(GL_TEXTURE_2D, planx[eb][s]);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, NX/2+1, 1, GL_RGBA, GL_FLOAT, p);
    }
  }

  free (p);
}


void fft_plany() {
  float *p = (float*)calloc (4*NY, sizeof(float));

  for (int s=0; s<=1; s++) {
    int si = s*2-1;
    for (int eb=1; eb<=BY; eb++) {
      for (int x=0; x<NY; x++) {
        int l = 1<<eb;
        int j = x%l;
        double w = si*PI*j/l;
        if (j<l/2) {
          if (eb==1)
          {
            *(p + 4*x + 0) = bitreverse(x    ,BY)/(float)NY;
            *(p + 4*x + 1) = bitreverse(x+l/2,BY)/(float)NY;
          } else {
            *(p + 4*x + 0) = (x    )/(float)NY;
            *(p + 4*x + 1) = (x+l/2)/(float)NY;
          }
        } else {
          if (eb==1) {
            *(p + 4*x + 0) = bitreverse(x-l/2,BY)/(float)NY;
            *(p + 4*x + 1) = bitreverse(x    ,BY)/(float)NY;
          } else {
            *(p + 4*x + 0) = (x-l/2)/(float)NY;
            *(p + 4*x + 1) = (x    )/(float)NY;
          }
        }
        *(p + 4*x + 2) = (float)cos(w);
        *(p + 4*x + 3) = (float)sin(w);
      }

      glBindTexture(GL_TEXTURE_2D, plany[eb][s]);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, NY, 1, GL_RGBA, GL_FLOAT, p);
    }
  }

  free (p);
}

double func_hard (double x, double a) {
  if (x>=a) return 1.0; else return 0.0;
}

double func_linear (double x, double a, double ea) {
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else return (x-a)/ea + 0.5;
}

double func_hermite (double x, double a, double ea) {
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else
  {
    double m = (x-(a-ea/2.0))/ea;
    return m*m*(3.0-2.0*m);
  }
}

double func_sin (double x, double a, double ea) {
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else return sin(PI/2.0*(x-a)/ea)*0.5+0.5;
}

double func_smooth (double x, double a, double ea) {
  return 1.0/(1.0+exp(-(x-a)*4.0/ea));
}

double func_kernel (double x, double a, double ea) {
  //return func_hard   (x, a);
  return func_linear  (x, a, ea);
  //return func_hermite (x, a, ea);
  //return func_sin     (x, a, ea);
  //return func_smooth  (x, a, ea);
}

void makekernel (int kr, int kd) {
  int ix, iy, iz, x, y, z;
  double l, n, m;
  float *ar, *ad;
  int Ra;
  double ri, bb;

  ad = (float*)calloc (NX*NY, sizeof(float));
  ar = (float*)calloc (NX*NY, sizeof(float));

  ri = ra/rr;
  bb = ra/rb;

  //Ra = (int)(ra+bb/2+1.0);
  Ra = (int)(ra*2);

  kflr = 0.0;
  kfld = 0.0;

  for (iz=0; iz<NZ; iz++) {
    memset (ad, 0, NX*NY*sizeof(float));
    memset (ar, 0, NX*NY*sizeof(float));
    z = 0;
    if (z>=-Ra && z<=Ra) {
      for (iy=0; iy<NY; iy++) {
        if (iy<NY/2) y = iy; else y = iy-NY;
        if (y>=-Ra && y<=Ra) {
          for (ix=0; ix<NX; ix++) {
            if (ix<NX/2)
              x = ix;
            else
              x = ix-NX;
            if (x>=-Ra && x<=Ra) {
              l = sqrt (x*x + y*y + z*z);
              m = 1-func_kernel (l, ri, bb);
              n = func_kernel (l, ri, bb) * (1-func_kernel (l, ra, bb));
              *(ad + iy*NX + ix) = (float)m;
              *(ar + iy*NX + ix) = (float)n;
              kflr += n;
              kfld += m;
            }
          }
        }
      }
    }

    glBindTexture(GL_TEXTURE_2D, tr[kd]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, NX, NY, GL_LUMINANCE, GL_FLOAT, ad);

    glBindTexture(GL_TEXTURE_2D, tr[kr]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, NX, NY, GL_LUMINANCE, GL_FLOAT, ar);
  }

  free(ar);
  free(ad);
}

void copybufferrc(GLuint prog, int vo, int na) {
  GLfloat mat[16];
  glUseProgram(prog);
  identity_matrix(&mat[0]);
  glhOrtho(&mat[0], 0, NX/2+1, 0, NY, -1, 1);
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_mat"), 1, GL_FALSE, &mat[0]);
  glViewport(0, 0, NX/2+1, NY);

  glBindFramebuffer(GL_FRAMEBUFFER, fb[na]);
  glUseProgram(prog);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tr[vo]);
  glUniform1i(glGetUniformLocation(prog, "tex0"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tr[vo]);
  glUniform1i(glGetUniformLocation(prog, "tex1"), 1);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[na], 0);

  GLuint loc_texcoord0 = glGetAttribLocation(prog, "a_texcoord0");
  GLuint loc_texcoord1 = glGetAttribLocation(prog, "a_texcoord1");
  GLuint loc_position = glGetAttribLocation(prog, "a_position");
  glBindBuffer(GL_ARRAY_BUFFER, g_copybufferrc_vbo);
  glVertexAttribPointer(loc_texcoord0, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, tex));
  glVertexAttribPointer(loc_texcoord1, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)(offsetof(SnmVertex, tex) + sizeof(float) * 2));
  glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, loc));
  glEnableVertexAttribArray(loc_texcoord0);
  glEnableVertexAttribArray(loc_texcoord1);
  glEnableVertexAttribArray(loc_position);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glUseProgram(0);
}

void copybuffercr(GLuint prog, int vo, int na) {
  GLfloat mat[16];
  glUseProgram(prog);
  identity_matrix(&mat[0]);
  glhOrtho(&mat[0], 0, NX, 0, NY, -1, 1);
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_mat"), 1, GL_FALSE, &mat[0]);
  glViewport(0, 0, NX, NY);

  glBindFramebuffer(GL_FRAMEBUFFER, fr[na]);
  glUseProgram(prog);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tb[vo]);
  glUniform1i(glGetUniformLocation(prog, "tex0"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tb[vo]);
  glUniform1i(glGetUniformLocation(prog, "tex1"), 1);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr[na], 0);

  GLuint loc_texcoord0 = glGetAttribLocation(prog, "a_texcoord0");
  GLuint loc_texcoord1 = glGetAttribLocation(prog, "a_texcoord1");
  GLuint loc_position = glGetAttribLocation(prog, "a_position");
  glBindBuffer(GL_ARRAY_BUFFER, g_copybuffercr_vbo);
  glVertexAttribPointer(loc_texcoord0, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, tex));
  glVertexAttribPointer(loc_texcoord1, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)(offsetof(SnmVertex, tex) + sizeof(float) * 2));
  glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, loc));
  glEnableVertexAttribArray(loc_texcoord0);
  glEnableVertexAttribArray(loc_texcoord1);
  glEnableVertexAttribArray(loc_position);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glUseProgram(0);
}

void fft_stage(GLuint prog, int dim, int eb, int si, int fftc, int ffto) {
  GLfloat mat[16];
  glUseProgram(prog);
  identity_matrix(&mat[0]);
  glhOrtho(&mat[0], 0, NX/2+1, 0, NY, -1, 1);
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_mat"), 1, GL_FALSE, &mat[0]);
  glViewport(0, 0, NX/2+1, NY);

  glBindFramebuffer(GL_FRAMEBUFFER, fb[ffto]);
  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "dim"), dim);

  int tang = 0;
  double tangsc = 0.0;
  if (dim==1 && si==1 && eb==0) {
    tang = 1;
    tangsc = 0.5*sqrt(2.0);
  } else if (dim==1 && si==-1 && eb==BX) {
    tang = 1;
    tangsc = 0.5/sqrt(2.0);
  } else {
    tang = 0;
    tangsc = 0.0;
  }
  glUniform1i(glGetUniformLocation(prog, "tang"), tang);
  glUniform1f(glGetUniformLocation(prog, "tangsc"), (float)tangsc);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tb[fftc]);
  glUniform1i(glGetUniformLocation(prog, "tex0"), 0);

  glActiveTexture(GL_TEXTURE1);
  if (dim==1) glBindTexture(GL_TEXTURE_2D, planx[eb][(si+1)/2]);
  if (dim==2) glBindTexture(GL_TEXTURE_2D, plany[eb][(si+1)/2]);
  glUniform1i(glGetUniformLocation(prog, "tex1"), 1);

  GLuint vbo;
  if (dim==2 || dim==3 || (dim==1 && si==-1 && eb==BX)) {
    vbo = g_fftstage_vbo;
  } else {
    vbo = g_fftstage2_vbo;
  }

  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[ffto], 0);
  GLuint loc_texcoord0 = glGetAttribLocation(prog, "a_texcoord0");
  GLuint loc_texcoord1 = glGetAttribLocation(prog, "a_texcoord1");
  GLuint loc_position = glGetAttribLocation(prog, "a_position");
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(loc_texcoord0, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, tex));
  glVertexAttribPointer(loc_texcoord1, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)(offsetof(SnmVertex, tex) + sizeof(float) * 2));
  glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, loc));
  glEnableVertexAttribArray(loc_texcoord0);
  glEnableVertexAttribArray(loc_texcoord1);
  glEnableVertexAttribArray(loc_position);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glUseProgram(0);
}

void fft(GLuint rcprog, GLuint crprog, GLuint fftprog, int vo, int na, int si) {
  int t, s;
  int fftcur, fftoth;

  fftcur = FFT0;
  fftoth = FFT1;

  if (si==-1) {
    copybufferrc(rcprog, vo, fftcur);

    for (t=1; t<=BX-1+1; t++) {
      fft_stage(fftprog, 1, t, si, fftcur, fftoth);
      s=fftcur; fftcur=fftoth; fftoth=s;
    }

    for (t=1; t<=BY; t++) {
      if (t==BY) fft_stage(fftprog, 2, t, si, fftcur, na);
      else fft_stage(fftprog, 2, t, si, fftcur, fftoth);
      s=fftcur; fftcur=fftoth; fftoth=s;
    }
  } else {
    for (t=1; t<=BY; t++) {
      if (t==1) fft_stage(fftprog, 2, t, si, vo, fftoth);
      else fft_stage(fftprog, 2, t, si, fftcur, fftoth);
      s=fftcur; fftcur=fftoth; fftoth=s;
    }

    for (t=0; t<=BX-1; t++) {
      fft_stage(fftprog, 1, t, si, fftcur, fftoth);
      s=fftcur; fftcur=fftoth; fftoth=s;
    }

    copybuffercr(crprog, fftcur, na);
  }
}

void kernelmul(GLuint prog, int vo, int ke, int na, double sc) {
  GLfloat mat[16];
  glUseProgram(prog);
  identity_matrix(&mat[0]);
  glhOrtho(&mat[0], 0, NX/2+1, 0, NY, -1, 1);
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_mat"), 1, GL_FALSE, &mat[0]);
  glViewport(0, 0, NX/2+1, NY);

  glBindFramebuffer(GL_FRAMEBUFFER, fb[na]);
  glUseProgram(prog);
  glUniform1f(glGetUniformLocation(prog, "sc"), (float)sc);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tb[vo]);
  glUniform1i(glGetUniformLocation(prog, "tex0"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tb[ke]);
  glUniform1i(glGetUniformLocation(prog, "tex1"), 1);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[na], 0);
  GLuint loc_texcoord0 = glGetAttribLocation(prog, "a_texcoord0");
  GLuint loc_texcoord1 = glGetAttribLocation(prog, "a_texcoord1");
  GLuint loc_position = glGetAttribLocation(prog, "a_position");
  glBindBuffer(GL_ARRAY_BUFFER, g_fftstage_vbo);
  glVertexAttribPointer(loc_texcoord0, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, tex));
  glVertexAttribPointer(loc_texcoord1, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)(offsetof(SnmVertex, tex) + sizeof(float) * 2));
  glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, loc));
  glEnableVertexAttribArray(loc_texcoord0);
  glEnableVertexAttribArray(loc_texcoord1);
  glEnableVertexAttribArray(loc_position);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glUseProgram(0);
}

double RND(double x) {
  return x * (double)rand()/((double)RAND_MAX + 1);
}

void splat2D (float *buf) {
  double mx, my, dx, dy, u, l;
  int ix, iy;

  mx = RND(NX);
  my = RND(NY);
  u = ra*(RND(0.5) + 0.5);

  for (iy=(int)(my-u-1); iy<=(int)(my+u+1); iy++)
    for (ix=(int)(mx-u-1); ix<=(int)(mx+u+1); ix++) {
      dx = mx-ix;
      dy = my-iy;
      l = sqrt(dx*dx+dy*dy);
      if (l<u) {
        int px = ix;
        int py = iy;
        while (px<  0) px+=NX;
        while (px>=NX) px-=NX;
        while (py<  0) py+=NY;
        while (py>=NY) py-=NY;
        if (px>=0 && px<NX && py>=0 && py<NY) {
          *(buf + NX*py + px) = 1.0;
        }
      }
    }
}

void inita2D (int a) {
  float *buf = (float*)calloc(NX*NY, sizeof(float));

  double mx, my;
  mx = 2*ra; if (mx>NX) mx=NX;
  my = 2*ra; if (my>NY) my=NY;

  for (int t=0; t<=(int)(NX*NY/(mx*my)); t++) {
    splat2D(buf);
  }

  glBindTexture (GL_TEXTURE_2D, tr[a]);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE, NX, NY, 0, GL_LUMINANCE, GL_FLOAT, buf);

  free (buf);
}
