#include "smoothlife.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

const float mode = 0;
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

GLuint fb[AFB], tb[AFB];
GLuint fr[ARB], tr[ARB];

GLuint g_loc_mode;
GLuint g_loc_dt;
GLuint g_loc_b1;
GLuint g_loc_b2;
GLuint g_loc_d1;
GLuint g_loc_d2;
GLuint g_loc_sigmode;
GLuint g_loc_sigtype;
GLuint g_loc_mixtype;
GLuint g_loc_sn;
GLuint g_loc_sm;
GLuint g_loc_tex0;
GLuint g_loc_tex1;
GLuint g_loc_tex2;
GLuint g_loc_texcoord0;
GLuint g_loc_texcoord1;
GLuint g_loc_texcoord2;
GLuint g_loc_position;
GLuint g_loc_mat;

struct SnmVertex {
  GLfloat tex[6];
  GLfloat loc[3];
};
GLuint g_snm_vbo;

void InitializeVbo() {
  SnmVertex verts[4];
  verts[0].tex[0] = 0.f;
  verts[0].tex[1] = 0.f;
  verts[0].tex[2] = 0.f;
  verts[0].tex[3] = 0.f;
  verts[0].tex[4] = 0.f;
  verts[0].tex[5] = 0.f;
  verts[0].loc[0] = 0;
  verts[0].loc[1] = 0;
  verts[0].loc[2] = 0;

  verts[1].tex[0] = 1.f;
  verts[1].tex[1] = 0.f;
  verts[1].tex[2] = 1.f;
  verts[1].tex[3] = 0.f;
  verts[1].tex[4] = 1.f;
  verts[1].tex[5] = 0.f;
  verts[1].loc[0] = NX;
  verts[1].loc[1] = 0;
  verts[1].loc[2] = 0;

  verts[2].tex[0] = 0.f;
  verts[2].tex[1] = 1.f;
  verts[2].tex[2] = 0.f;
  verts[2].tex[3] = 1.f;
  verts[2].tex[4] = 0.f;
  verts[2].tex[5] = 1.f;
  verts[2].loc[0] = 0;
  verts[2].loc[1] = NY;
  verts[2].loc[2] = 0;

  verts[3].tex[0] = 1.f;
  verts[3].tex[1] = 1.f;
  verts[3].tex[2] = 1.f;
  verts[3].tex[3] = 1.f;
  verts[3].tex[4] = 1.f;
  verts[3].tex[5] = 1.f;
  verts[3].loc[0] = NX;
  verts[3].loc[1] = NY;
  verts[3].loc[2] = 0;

  glGenBuffers(1, &g_snm_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_snm_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts[0], GL_STATIC_DRAW);
}

void InitializeTextures() {
  glGenTextures(ARB, &tr[0]);
  //glGenFramebuffers(ARB, &fr[0]);

  for (int t=0; t<ARB; t++) {
    glBindTexture(GL_TEXTURE_2D, tr[t]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, NX, NY, 0, GL_LUMINANCE, GL_FLOAT, NULL);

    //glBindFramebuffer(GL_FRAMEBUFFER, fr[t]);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr[t], 0);
    //GLint err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    //if (err != GL_FRAMEBUFFER_COMPLETE) {
    //  printf("glCheckFramebufferStatus: %d\n", err);
    //}
  }

  //glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void GetSNMLocations(GLuint prog) {
  g_loc_mode = glGetUniformLocation(prog, "mode");
  g_loc_dt = glGetUniformLocation(prog, "dt");
  g_loc_b1 = glGetUniformLocation(prog, "b1");
  g_loc_b2 = glGetUniformLocation(prog, "b2");
  g_loc_d1 = glGetUniformLocation(prog, "d1");
  g_loc_d2 = glGetUniformLocation(prog, "d2");
  g_loc_sigmode = glGetUniformLocation(prog, "sigmode");
  g_loc_sigtype = glGetUniformLocation(prog, "sigtype");
  g_loc_mixtype = glGetUniformLocation(prog, "mixtype");
  g_loc_sn = glGetUniformLocation(prog, "sn");
  g_loc_sm = glGetUniformLocation(prog, "sm");
  g_loc_tex0 = glGetUniformLocation(prog, "tex0");
  g_loc_tex1 = glGetUniformLocation(prog, "tex1");
  g_loc_tex2 = glGetUniformLocation(prog, "tex2");
  g_loc_texcoord0 = glGetAttribLocation(prog, "a_texcoord0");
  g_loc_texcoord1 = glGetAttribLocation(prog, "a_texcoord1");
  g_loc_texcoord2 = glGetAttribLocation(prog, "a_texcoord2");
  g_loc_position = glGetAttribLocation(prog, "a_position");
  g_loc_mat = glGetUniformLocation(prog, "u_mat");
}

void snm(GLuint prog, int an, int am, int na) {
  GLfloat mat[16];

//  glBindFramebuffer(GL_FRAMEBUFFER, fr[na]);
  glUseProgram(prog);
  identity_matrix(&mat[0]);
  glhOrtho(&mat[0], 0, NX, 0, NY, -NZ, NZ);
//  glViewport(0, 0, NX, NY);
  glUniformMatrix4fv(g_loc_mat, 1, GL_FALSE, &mat[0]);
  glUniform1f(g_loc_mode, (float)mode);
  glUniform1f(g_loc_dt, (float)dt);
  glUniform1f(g_loc_b1, (float)b1);
  glUniform1f(g_loc_b2, (float)b2);
  glUniform1f(g_loc_d1, (float)d1);
  glUniform1f(g_loc_d2, (float)d2);
  glUniform1f(g_loc_sigmode, (float)sigmode);
  glUniform1f(g_loc_sigtype, (float)sigtype);
  glUniform1f(g_loc_mixtype, (float)mixtype);
  glUniform1f(g_loc_sn, (float)sn);
  glUniform1f(g_loc_sm, (float)sm);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tr[an]);
  glUniform1i(g_loc_tex0, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tr[am]);
  glUniform1i(g_loc_tex1, 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, tr[na]);
  glUniform1i(g_loc_tex2, 2);

//  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr[na], 0);

  glBindBuffer(GL_ARRAY_BUFFER, g_snm_vbo);
  glVertexAttribPointer(g_loc_texcoord0, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, tex));
  glVertexAttribPointer(g_loc_texcoord1, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)(offsetof(SnmVertex, tex) + sizeof(float) * 2));
  glVertexAttribPointer(g_loc_texcoord2, 2, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)(offsetof(SnmVertex, tex) + sizeof(float) * 4));
  glVertexAttribPointer(g_loc_position, 3, GL_FLOAT, GL_FALSE, sizeof(SnmVertex), (void*)offsetof(SnmVertex, loc));
  glEnableVertexAttribArray(g_loc_texcoord0);
  glEnableVertexAttribArray(g_loc_texcoord1);
  glEnableVertexAttribArray(g_loc_texcoord2);
  glEnableVertexAttribArray(g_loc_position);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glUseProgram(0);
}


// initialize an and am with 0 to 1 gradient for drawing of snm (2D only)
//
void initan(int a) {
  float *buf = (float*)calloc(NX*NY, sizeof(float));

  int x, y;

  for (y=0; y<NY; y++) {
    for (x=0; x<NX; x++) {
      buf[y*NX+x] = (float)x/NX;
    }
  }

  glBindTexture(GL_TEXTURE_2D, tr[a]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, NX, NY, 0, GL_LUMINANCE, GL_FLOAT, buf);

  free(buf);
}


void initam(int a) {
  float *buf = (float*)calloc(NX*NY, sizeof(float));

  int x, y;

  for (y=0; y<NY; y++) {
    for (x=0; x<NX; x++) {
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
