#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <GLES2/gl2.h>
#include <ppapi/lib/gl/gles2/gl2ext_ppapi.h>

const double PI = 6.28318530718;

int NX, NY, NZ, BX, BY, BZ;

const int BMAX = 16;

const int AA = 0;
const int KR = 1;
const int KD = 2;
const int AN = 3;
const int AM = 4;
const int ARB = 5;		// number of real buffers

const int AF = 0;
const int KRF = 1;
const int KDF = 2;
const int ANF = 3;
const int AMF = 4;
const int FFT0 = 5;
const int FFT1 = 6;
const int AFB = 7;		// number of Fourier buffers

double kflr, kfld;

int SX, SY;


int ox, oy, qx, qy, qq;
double fx, fy, fz, bx, by;
double wi, wj, wx, wy, dw;

double ra, rr, rb, dt;
double b1, d1, b2, d2, sn, sm;
int mode, sigmode, sigtype, mixtype;
double colscheme, phase, dphase, visscheme;

GLuint shader_snm, shader_fft, shader_kernelmul, shader_draw;
GLuint shader_copybufferrc, shader_copybuffercr;
GLuint fb[AFB], tb[AFB];
GLuint fr[ARB], tr[ARB];
GLuint planx[BMAX][2], plany[BMAX][2];
GLenum ttd;		// texture target dimension

GLint loc_b1, loc_b2;
GLint loc_d1, loc_d2;
GLint loc_sn, loc_sm;
GLint loc_mode, loc_sigmode, loc_sigtype, loc_mixtype, loc_dt;
GLint loc_colscheme, loc_phase, loc_visscheme;
GLint loc_dim, loc_tang, loc_tangsc, loc_sc;


double RND (double x)
{
  return x * (double)rand()/((double)RAND_MAX + 1);
}


bool setShaders (char *dname, char *fname, GLuint &prog)
{
  char *vs = NULL, *fs = NULL, *gs = NULL;
  bool ret;
  GLuint v, f;

  v = glCreateShader (GL_VERTEX_SHADER);
  f = glCreateShader (GL_FRAGMENT_SHADER);

  FILE *fp;
  int count=0;
  char filename[128];

  printf("setting shader: %s\n", fname);

  sprintf (filename, "%s\\%s.vert", dname, fname);
  fp = fopen (filename, "rt");
  if (fp == NULL) { printf("couldn't open .vert\n");  return false; }
  fseek (fp, 0, SEEK_END);
  count = ftell (fp);
  rewind (fp);
  if (count > 0)
  {
    vs = (char *)malloc (sizeof(char)*(count+1));
    count = fread (vs, sizeof(char), count, fp);
    vs[count] = '\0';
  }
  fclose (fp);

  sprintf (filename, "%s\\%s.frag", dname, fname);
  fp = fopen (filename, "rt");
  if (fp == NULL) { printf("couldn't open .frag\n");  return false; }
  fseek (fp, 0, SEEK_END);
  count = ftell (fp);
  rewind (fp);
  if (count > 0)
  {
    fs = (char *)malloc (sizeof(char)*(count+1));
    count = fread (fs, sizeof(char), count, fp);
    fs[count] = '\0';
  }
  fclose (fp);

  char w4[256];
  int t, d, w;

  t = 0;
  while (fs[t]!='\0') t++;

  gs = (char*)calloc (t*2, sizeof(char));

  t = 0;
  d = 0;
  w4[0] = '\0';

  do
  {
    w = 0;
    while (! (fs[t]==' ' || fs[t]=='\n' || fs[t]=='\0'))
    {
      w4[w] = fs[t];
      w++; t++;
    }
    w4[w] = '\0';

    w = 0;
    while (w4[w]!='\0')
    {
      gs[d] = w4[w];
      d++; w++;
    }

    while (fs[t]==' ' || fs[t]=='\n' || fs[t]=='\0')
    {
      gs[d] = fs[t];
      if (fs[t]=='\0') break;
      d++; t++;
    }
  }
  while (fs[t]!='\0');

  /*FILE *file;
    sprintf (filename, "%s.test", fname);
    file = fopen (filename, "w");
    fwrite (gs, 1, d, file);
    fclose (file);*/

  const char *vv = vs;
  const char *ff = gs;

  glShaderSource (v, 1, &vv, NULL);
  glShaderSource (f, 1, &ff, NULL);

  free (vs); free (fs); free (gs);

  ret = false;

  glCompileShader (v);
  if (printShaderInfoLog (v))
  {
    printf("error in vertex shader!\n\n"); 
    ret |= true;
  }
  else
  {
    printf("vertex shader ok\n\n"); 
  }

  glCompileShader (f);
  if (printShaderInfoLog (f))
  {
    printf("error in fragment shader!\n\n"); 
    ret |= true;
  }
  else
  {
    printf("fragment shader ok\n\n"); 
  }

  prog = glCreateProgram ();
  glAttachShader (prog, v);
  glAttachShader (prog, f);

  glLinkProgram (prog);
  if (printProgramInfoLog (prog))
  {
    printf("shader program error!\n\n"); 
    ret |= true;
  }
  else
  {
    printf("shader program ok\n\n"); 
  }

  return ret;
}


void splat2D (float *buf)
{
  double mx, my, dx, dy, u, l;
  int ix, iy;

  mx = RND(NX);
  my = RND(NY);
  u = ra*(RND(0.5) + 0.5);

  for (iy=(int)(my-u-1); iy<=(int)(my+u+1); iy++)
    for (ix=(int)(mx-u-1); ix<=(int)(mx+u+1); ix++)
    {
      dx = mx-ix;
      dy = my-iy;
      l = sqrt(dx*dx+dy*dy);
      if (l<u)
      {
        int px = ix;
        int py = iy;
        while (px<  0) px+=NX;
        while (px>=NX) px-=NX;
        while (py<  0) py+=NY;
        while (py>=NY) py-=NY;
        if (px>=0 && px<NX && py>=0 && py<NY)
        {
          *(buf + NX*py + px) = 1.0;
        }
      }
    }
}


void inita (int a)
{
  float *buf = (float*)calloc(NX*NY, sizeof(float));

  double mx, my;

  mx = 2*ra; if (mx>NX) mx=NX;
  my = 2*ra; if (my>NY) my=NY;

  for (int t=0; t<=(int)(NX*NY/(mx*my)); t++)
  {
    splat2D (buf);
  }

  glBindTexture (GL_TEXTURE_2D, tr[a]);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_R32F, NX,NY, 0, GL_RED, GL_FLOAT, buf);

  free (buf);
}


bool create_buffers (void)
{
  unsigned int err;
  int t, s, eb;

  printf("create buffers 2D %d %d\n", NX, NY); 

  // Fourier (complex) buffers

  glGenTextures (AFB, &tb[0]);
  err = glGetError (); printf("GenTextures err %d\n", err);  if (err) return false;

  glGenFramebuffers (AFB, &fb[0]);
  err = glGetError (); printf("GenFramebuffers err %d\n", err);  if (err) return false;

  for (t=0; t<AFB; t++)
  {
    printf("complex buffer %d\n", t); 

    glBindTexture (ttd, tb[t]);
    err = glGetError (); printf("BindTexture err %d\n", err);  if (err) return false;

    glTexParameterf (ttd, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf (ttd, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf (ttd, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf (ttd, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D (GL_TEXTURE_2D, 0, GL_RG32F, NX/2+1,NY, 0, GL_RG, GL_FLOAT, NULL);
    err = glGetError (); printf("TexImage err %d\n", err);  if (err) return false;


    glBindFramebuffer (GL_FRAMEBUFFER, fb[t]);
    err = glGetError (); printf("BindFramebuffer err %d\n", err);  if (err) return false;

    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[t], 0);
    err = glGetError (); printf("FramebufferTexture err %d\n", err);  if (err) return false;

    err = glCheckFramebufferStatus (GL_FRAMEBUFFER);
    printf("FramebufferStatus 0x%x\n", err);  if (err!=0x8cd5) return false;
  }


  // real buffers

  glGenTextures (ARB, &tr[0]);
  err = glGetError (); printf("GenTextures err %d\n", err);  if (err) return false;

  glGenFramebuffers (ARB, &fr[0]);
  err = glGetError (); printf("GenFramebuffers err %d\n", err);  if (err) return false;

  for (t=0; t<ARB; t++)
  {
    printf("real buffer %d\n", t); 

    glBindTexture (ttd, tr[t]);
    err = glGetError (); printf("BindTexture err %d\n", err);  if (err) return false;

    glTexParameterf (ttd, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf (ttd, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf (ttd, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf (ttd, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D (GL_TEXTURE_2D, 0, GL_R32F, NX,NY, 0, GL_RED, GL_FLOAT, NULL);
    err = glGetError (); printf("TexImage err %d\n", err);  if (err) return false;


    glBindFramebuffer (GL_FRAMEBUFFER, fr[t]);
    err = glGetError (); printf("BindFramebuffer err %d\n", err);  if (err) return false;

    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr[t], 0);
    err = glGetError (); printf("FramebufferTexture err %d\n", err);  if (err) return false;

    err = glCheckFramebufferStatus (GL_FRAMEBUFFER);
    printf("FramebufferStatus 0x%x\n", err);  if (err!=0x8cd5) return false;
  }


  // FFT plan texture

  glGenTextures ((BX-1+2)*2, &planx[0][0]);
  for (s=0; s<=1; s++)
  {
    for (eb=0; eb<=BX-1+1; eb++)
    {
      glBindTexture (GL_TEXTURE_1D, planx[eb][s]);
      err = glGetError (); printf("BindTexture err %d\n", err);  if (err) return false;

      glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

      glTexImage1D (GL_TEXTURE_1D, 0, GL_RGBA32F, NX/2+1, 0, GL_RGBA, GL_FLOAT, NULL);
      err = glGetError (); printf("TexImage err %d\n", err);  if (err) return false;
    }
  }

  glGenTextures ((BY+1)*2, &plany[0][0]);
  for (s=0; s<=1; s++)
  {
    for (eb=1; eb<=BY; eb++)
    {
      glBindTexture (GL_TEXTURE_1D, plany[eb][s]);
      err = glGetError (); printf("BindTexture err %d\n", err);  if (err) return false;

      glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

      glTexImage1D (GL_TEXTURE_1D, 0, GL_RGBA32F, NY, 0, GL_RGBA, GL_FLOAT, NULL);
      err = glGetError (); printf("TexImage err %d\n", err);  if (err) return false;
    }
  }

  printf("all buffers ok\n"); 


  return true;
}


bool delete_buffers (void)
{
  unsigned int err;

  printf("delete buffers\n"); 

  glBindTexture (ttd, 0);
  err = glGetError (); printf("BindTexture 0 err %d\n", err);  if (err) return false;
  glDeleteTextures (AFB, &tb[0]);
  err = glGetError (); printf("DeleteTextures err %d\n", err);  if (err) return false;
  glDeleteTextures (ARB, &tr[0]);
  err = glGetError (); printf("DeleteTextures err %d\n", err);  if (err) return false;
  glDeleteTextures ((BX-1+2)*2, &planx[0][0]);
  err = glGetError (); printf("DeleteTextures err %d\n", err);  if (err) return false;
  glDeleteTextures ((BY+1)*2, &plany[0][0]);
  err = glGetError (); printf("DeleteTextures err %d\n", err);  if (err) return false;

  glBindFramebuffer (GL_FRAMEBUFFER, 0);
  err = glGetError (); printf("BindFramebuffer 0 err %d\n", err);  if (err) return false;
  glDeleteFramebuffers (AFB, &fb[0]);
  err = glGetError (); printf("DeleteFramebuffers err %d\n", err);  if (err) return false;
  glDeleteFramebuffers (ARB, &fr[0]);
  err = glGetError (); printf("DeleteFramebuffers err %d\n", err);  if (err) return false;

  return true;
}


void drawa (int a)
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (0, SX, 0, SY, -1, 1);
  glViewport (0, 0, SX, SY);

  glBindFramebuffer (GL_FRAMEBUFFER, 0);

  glClearColor (0.5, 0.5, 0.5, 1.0);
  glClear (GL_COLOR_BUFFER_BIT);

  glActiveTexture (GL_TEXTURE0);
  glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, tr[a]);

  glActiveTexture (GL_TEXTURE1);
  glDisable (GL_TEXTURE_2D);

  glUseProgram (shader_draw);
  glUniform1i (glGetUniformLocation (shader_draw, "tex0"), 0);
  glUniform1f (loc_colscheme, (float)colscheme);
  glUniform1f (loc_phase, (float)phase);

  glDisable (GL_DEPTH_TEST);

  glBegin (GL_QUADS);
  glMultiTexCoord2d (GL_TEXTURE0, fx   , fy   ); glVertex2i (ox   , oy   );
  glMultiTexCoord2d (GL_TEXTURE0, fx+bx, fy   ); glVertex2i (ox+qx, oy   );
  glMultiTexCoord2d (GL_TEXTURE0, fx+bx, fy+by); glVertex2i (ox+qx, oy+qy);
  glMultiTexCoord2d (GL_TEXTURE0, fx   , fy+by); glVertex2i (ox   , oy+qy);
  glEnd ();

  glUseProgram (0);
}


double func_hard (double x, double a)
{
  if (x>=a) return 1.0; else return 0.0;
}

double func_linear (double x, double a, double ea)
{
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else return (x-a)/ea + 0.5;
}

double func_hermite (double x, double a, double ea)
{
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else
  {
    double m = (x-(a-ea/2.0))/ea;
    return m*m*(3.0-2.0*m);
  }
}

double func_sin (double x, double a, double ea)
{
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else return sin(PI/2.0*(x-a)/ea)*0.5+0.5;
}

double func_smooth (double x, double a, double ea)
{
  return 1.0/(1.0+exp(-(x-a)*4.0/ea));
}

double func_kernel (double x, double a, double ea)
{
  //return func_hard   (x, a);
  return func_linear  (x, a, ea);
  //return func_hermite (x, a, ea);
  //return func_sin     (x, a, ea);
  //return func_smooth  (x, a, ea);
}


void makekernel (int kr, int kd)
{
  int ix, iy, iz, x, y, z;
  double l, n, m;
  float *ar, *ad;
  int Ra;
  double ri, bb;

  ad = (float*)calloc (NX*NY, sizeof(float));
  if (ad==0) { printf("ad failed\n");  return; }

  ar = (float*)calloc (NX*NY, sizeof(float));
  if (ar==0) { printf("ar failed\n");  return; }

  ri = ra/rr;
  bb = ra/rb;

  //Ra = (int)(ra+bb/2+1.0);
  Ra = (int)(ra*2);

  kflr = 0.0;
  kfld = 0.0;

  for (iz=0; iz<NZ; iz++)
  {
    memset (ad, 0, NX*NY*sizeof(float));
    memset (ar, 0, NX*NY*sizeof(float));
    z = 0;
    if (z>=-Ra && z<=Ra)
    {
      for (iy=0; iy<NY; iy++)
      {
        if (iy<NY/2) y = iy; else y = iy-NY;
        if (y>=-Ra && y<=Ra)
        {
          for (ix=0; ix<NX; ix++)
          {
            if (ix<NX/2) x = ix; else x = ix-NX;
            if (x>=-Ra && x<=Ra)
            {
              l = sqrt (x*x + y*y + z*z);
              m = 1-func_kernel (l, ri, bb);
              n = func_kernel (l, ri, bb) * (1-func_kernel (l, ra, bb));
              *(ad + iy*NX + ix) = (float)m;
              *(ar + iy*NX + ix) = (float)n;
              kflr += n;
              kfld += m;
            }	// if ix
          }	// for ix
        }	// if iy
      }	// for iy
    }	// if iz

    glBindTexture (GL_TEXTURE_2D, tr[kd]);
    glTexSubImage2D (GL_TEXTURE_2D, 0, 0,0, NX,NY, GL_RED, GL_FLOAT, ad);

    glBindTexture (GL_TEXTURE_2D, tr[kr]);
    glTexSubImage2D (GL_TEXTURE_2D, 0, 0,0, NX,NY, GL_RED, GL_FLOAT, ar);
  }	// for iz

  free (ar);
  free (ad);

  printf("ra=%f rr=%f rb=%f ri=%f bb=%f kflr=%f kfld=%f kflr/kfld=%f\n", ra, rr, rb, ri, bb, kflr, kfld, kflr/kfld); 
}


int bitreverse (int x, int b)
{
  int c, t;

  c = 0;
  for (t=0; t<b; t++)
  {
    c = (c<<1) | ((x>>t) & 1);
  }
  return c;
}


void fft_planx (void)
{
  float *p = (float*)calloc (4*(NX/2+1), sizeof(float));

  for (int s=0; s<=1; s++)
  {
    int si = s*2-1;
    for (int eb=0; eb<=BX-1+1; eb++)
    {

      for (int x=0; x<NX/2+1; x++)
      {

        if (si==1 && eb==0)
        {
          *(p + 4*x + 0) = (     x+0.5f)/(float)(NX/2+1);
          *(p + 4*x + 1) = (NX/2-x+0.5f)/(float)(NX/2+1);
          double w = si*PI*(x/(double)NX+0.25);
          *(p + 4*x + 2) = (float)cos(w);
          *(p + 4*x + 3) = (float)sin(w);
        }
        else if (si==-1 && eb==BX)
        {
          if (x==0 || x==NX/2)
          {
            *(p + 4*x + 0) = 0;
            *(p + 4*x + 1) = 0;
          }
          else
          {
            *(p + 4*x + 0) = (     x)/(float)(NX/2);
            *(p + 4*x + 1) = (NX/2-x)/(float)(NX/2);
          }
          double w = si*PI*(x/(double)NX+0.25);
          *(p + 4*x + 2) = (float)cos(w);
          *(p + 4*x + 3) = (float)sin(w);
        }
        else if (x<NX/2)
        {
          int l = 1<<eb;
          int j = x%l;
          double w = si*PI*j/l;
          if (j<l/2)
          {
            if (eb==1)
            {
              *(p + 4*x + 0) = bitreverse(x    ,BX-1)/(float)(NX/2);
              *(p + 4*x + 1) = bitreverse(x+l/2,BX-1)/(float)(NX/2);
            }
            else
            {
              *(p + 4*x + 0) = (x    )/(float)(NX/2);
              *(p + 4*x + 1) = (x+l/2)/(float)(NX/2);
            }
          }
          else
          {
            if (eb==1)
            {
              *(p + 4*x + 0) = bitreverse(x-l/2,BX-1)/(float)(NX/2);
              *(p + 4*x + 1) = bitreverse(x    ,BX-1)/(float)(NX/2);
            }
            else
            {
              *(p + 4*x + 0) = (x-l/2)/(float)(NX/2);
              *(p + 4*x + 1) = (x    )/(float)(NX/2);
            }
          }
          *(p + 4*x + 2) = (float)cos(w);
          *(p + 4*x + 3) = (float)sin(w);
        }
        else
        {
          *(p + 4*x + 0) = 0;
          *(p + 4*x + 1) = 0;
          *(p + 4*x + 2) = 0;
          *(p + 4*x + 3) = 0;
        }

      }

      glBindTexture (GL_TEXTURE_1D, planx[eb][s]);
      glTexSubImage1D (GL_TEXTURE_1D, 0, 0, NX/2+1, GL_RGBA, GL_FLOAT, p);
    }
  }

  free (p);
}


void fft_plany (void)
{
  float *p = (float*)calloc (4*NY, sizeof(float));

  for (int s=0; s<=1; s++)
  {
    int si = s*2-1;
    for (int eb=1; eb<=BY; eb++)
    {

      for (int x=0; x<NY; x++)
      {
        int l = 1<<eb;
        int j = x%l;
        double w = si*PI*j/l;
        if (j<l/2)
        {
          if (eb==1)
          {
            *(p + 4*x + 0) = bitreverse(x    ,BY)/(float)NY;
            *(p + 4*x + 1) = bitreverse(x+l/2,BY)/(float)NY;
          }
          else
          {
            *(p + 4*x + 0) = (x    )/(float)NY;
            *(p + 4*x + 1) = (x+l/2)/(float)NY;
          }
        }
        else
        {
          if (eb==1)
          {
            *(p + 4*x + 0) = bitreverse(x-l/2,BY)/(float)NY;
            *(p + 4*x + 1) = bitreverse(x    ,BY)/(float)NY;
          }
          else
          {
            *(p + 4*x + 0) = (x-l/2)/(float)NY;
            *(p + 4*x + 1) = (x    )/(float)NY;
          }
        }
        *(p + 4*x + 2) = (float)cos(w);
        *(p + 4*x + 3) = (float)sin(w);
      }

      glBindTexture (GL_TEXTURE_1D, plany[eb][s]);
      glTexSubImage1D (GL_TEXTURE_1D, 0, 0, NY, GL_RGBA, GL_FLOAT, p);
    }
  }

  free (p);
}


void copybufferrc (int vo, int na)
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (0, NX/2+1, 0, NY, -NZ, NZ);
  glViewport (0, 0, NX/2+1, NY);

  glBindFramebuffer (GL_FRAMEBUFFER, fb[na]);
  glUseProgram (shader_copybufferrc);

  glActiveTexture (GL_TEXTURE0);
  glBindTexture (ttd, tr[vo]);
  glUniform1i (glGetUniformLocation (shader_copybufferrc, "tex0"), 0);

  glActiveTexture (GL_TEXTURE1);
  glBindTexture (ttd, tr[vo]);
  glUniform1i (glGetUniformLocation (shader_copybufferrc, "tex1"), 1);

  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[na], 0);
  glBegin (GL_QUADS);
  glMultiTexCoord2d (GL_TEXTURE0, 0-1.0/NX, 0); glMultiTexCoord2d (GL_TEXTURE1, 0-0.0/NX, 0); glVertex2i (   0,  0);
  glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/NX, 0); glMultiTexCoord2d (GL_TEXTURE1, 1-0.0/NX, 0); glVertex2i (NX/2,  0);
  glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/NX, 1); glMultiTexCoord2d (GL_TEXTURE1, 1-0.0/NX, 1); glVertex2i (NX/2, NY);
  glMultiTexCoord2d (GL_TEXTURE0, 0-1.0/NX, 1); glMultiTexCoord2d (GL_TEXTURE1, 0-0.0/NX, 1); glVertex2i (   0, NY);
  glEnd ();

  glUseProgram (0);
}


void copybuffercr (int vo, int na)
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (0, NX, 0, NY, -NZ, NZ);
  glViewport (0, 0, NX, NY);

  glBindFramebuffer (GL_FRAMEBUFFER, fr[na]);
  glUseProgram (shader_copybuffercr);

  glActiveTexture (GL_TEXTURE0);
  glBindTexture (ttd, tb[vo]);
  glUniform1i (glGetUniformLocation (shader_copybuffercr, "tex0"), 0);

  glActiveTexture (GL_TEXTURE1);
  glBindTexture (ttd, tb[vo]);
  glUniform1i (glGetUniformLocation (shader_copybuffercr, "tex1"), 1);

  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr[na], 0);
  glBegin (GL_QUADS);
  glMultiTexCoord2d (GL_TEXTURE0,   0.0/(NX/2+1), 0); glMultiTexCoord2d (GL_TEXTURE1,  0, 0); glVertex2i ( 0,  0);
  glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/(NX/2+1), 0); glMultiTexCoord2d (GL_TEXTURE1, NX, 0); glVertex2i (NX,  0);
  glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/(NX/2+1), 1); glMultiTexCoord2d (GL_TEXTURE1, NX, 1); glVertex2i (NX, NY);
  glMultiTexCoord2d (GL_TEXTURE0,   0.0/(NX/2+1), 1); glMultiTexCoord2d (GL_TEXTURE1,  0, 1); glVertex2i ( 0, NY);
  glEnd ();

  glUseProgram (0);
}


void fft_stage (int dim, int eb, int si, int fftc, int ffto)
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (0, NX/2+1, 0, NY, -NZ, NZ);
  glViewport (0, 0, NX/2+1, NY);

  glBindFramebuffer (GL_FRAMEBUFFER, fb[ffto]);
  glUseProgram (shader_fft);
  glUniform1i (loc_dim, dim);

  int tang;
  double tangsc;
  if (dim==1 && si==1 && eb==0)
  {
    tang = 1;
    tangsc = 0.5*sqrt(2.0);
  }
  else if (dim==1 && si==-1 && eb==BX)
  {
    tang = 1;
    tangsc = 0.5/sqrt(2.0);
  }
  else
  {
    tang = 0;
    tangsc = 0.0;
  }
  glUniform1i (loc_tang, tang);
  glUniform1f (loc_tangsc, (float)tangsc);

  double gd;
  int gi;
  if (dim==2 || dim==3 || dim==1 && si==-1 && eb==BX)
  {
    gd = 1.0;
    gi = NX/2+1;
  }
  else
  {
    gd = 1.0-1.0/(NX/2+1);
    gi = NX/2;
  }

  glActiveTexture (GL_TEXTURE0);
  glBindTexture (ttd, tb[fftc]);
  glUniform1i (glGetUniformLocation (shader_fft, "tex0"), 0);

  glActiveTexture (GL_TEXTURE1);
  if (dim==1) glBindTexture (GL_TEXTURE_1D, planx[eb][(si+1)/2]);
  if (dim==2) glBindTexture (GL_TEXTURE_1D, plany[eb][(si+1)/2]);
  glUniform1i (glGetUniformLocation (shader_fft, "tex1"), 1);

  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[ffto], 0);
  glBegin (GL_QUADS);
  glMultiTexCoord2d (GL_TEXTURE0,  0, 0); glMultiTexCoord2d (GL_TEXTURE1,  0, 0); glVertex2i ( 0,  0);
  glMultiTexCoord2d (GL_TEXTURE0, gd, 0); glMultiTexCoord2d (GL_TEXTURE1, gd, 0); glVertex2i (gi,  0);
  glMultiTexCoord2d (GL_TEXTURE0, gd, 1); glMultiTexCoord2d (GL_TEXTURE1, gd, 1); glVertex2i (gi, NY);
  glMultiTexCoord2d (GL_TEXTURE0,  0, 1); glMultiTexCoord2d (GL_TEXTURE1,  0, 1); glVertex2i ( 0, NY);
  glEnd ();

  glUseProgram (0);
}


void fft (int vo, int na, int si)
{
  int t, s;
  int fftcur, fftoth;

  fftcur = FFT0;
  fftoth = FFT1;

  if (si==-1)
  {
    copybufferrc (vo, fftcur);

    for (t=1; t<=BX-1+1; t++)
    {
      fft_stage (1, t, si, fftcur, fftoth);
      s=fftcur; fftcur=fftoth; fftoth=s;
    }

    for (t=1; t<=BY; t++)
    {
      if (t==BY) fft_stage (2, t, si, fftcur, na);
      else fft_stage (2, t, si, fftcur, fftoth);
      s=fftcur; fftcur=fftoth; fftoth=s;
    }
  }
  else	// si==1
  {
    for (t=1; t<=BY; t++)
    {
      if (t==1) fft_stage (2, t, si, vo, fftoth);
      else fft_stage (2, t, si, fftcur, fftoth);
      s=fftcur; fftcur=fftoth; fftoth=s;
    }

    for (t=0; t<=BX-1; t++)
    {
      fft_stage (1, t, si, fftcur, fftoth);
      s=fftcur; fftcur=fftoth; fftoth=s;
    }

    copybuffercr (fftcur, na);
  }
}


void kernelmul (int vo, int ke, int na, double sc)
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (0, NX/2+1, 0, NY, -NZ, NZ);
  glViewport (0, 0, NX/2+1, NY);

  glBindFramebuffer (GL_FRAMEBUFFER, fb[na]);
  glUseProgram (shader_kernelmul);
  glUniform1f (loc_sc, (float)sc);

  glActiveTexture (GL_TEXTURE0);
  glBindTexture (ttd, tb[vo]);
  glUniform1i (glGetUniformLocation (shader_kernelmul, "tex0"), 0);

  glActiveTexture (GL_TEXTURE1);
  glBindTexture (ttd, tb[ke]);
  glUniform1i (glGetUniformLocation (shader_kernelmul, "tex1"), 1);

  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[na], 0);
  glBegin (GL_QUADS);
  glMultiTexCoord2d (GL_TEXTURE0, 0, 0); glMultiTexCoord2d (GL_TEXTURE1, 0, 0); glVertex2i (     0,  0);
  glMultiTexCoord2d (GL_TEXTURE0, 1, 0); glMultiTexCoord2d (GL_TEXTURE1, 1, 0); glVertex2i (NX/2+1,  0);
  glMultiTexCoord2d (GL_TEXTURE0, 1, 1); glMultiTexCoord2d (GL_TEXTURE1, 1, 1); glVertex2i (NX/2+1, NY);
  glMultiTexCoord2d (GL_TEXTURE0, 0, 1); glMultiTexCoord2d (GL_TEXTURE1, 0, 1); glVertex2i (     0, NY);
  glEnd ();

  glUseProgram (0);
}


void snm (int an, int am, int na)
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (0, NX, 0, NY, -NZ, NZ);
  glViewport (0, 0, NX, NY);

  glBindFramebuffer (GL_FRAMEBUFFER, fr[na]);
  glUseProgram (shader_snm);
  glUniform1f (loc_mode, (float)mode);
  glUniform1f (loc_dt, (float)dt);
  glUniform1f (loc_b1, (float)b1);
  glUniform1f (loc_b2, (float)b2);
  glUniform1f (loc_d1, (float)d1);
  glUniform1f (loc_d2, (float)d2);
  glUniform1f (loc_sigmode, (float)sigmode);
  glUniform1f (loc_sigtype, (float)sigtype);
  glUniform1f (loc_mixtype, (float)mixtype);
  glUniform1f (loc_sn, (float)sn);
  glUniform1f (loc_sm, (float)sm);

  glActiveTexture (GL_TEXTURE0);
  glBindTexture (ttd, tr[an]);
  glUniform1i (glGetUniformLocation (shader_snm, "tex0"), 0);

  glActiveTexture (GL_TEXTURE1);
  glBindTexture (ttd, tr[am]);
  glUniform1i (glGetUniformLocation (shader_snm, "tex1"), 1);

  glActiveTexture (GL_TEXTURE2);
  glBindTexture (ttd, tr[na]);
  glUniform1i (glGetUniformLocation (shader_snm, "tex2"), 2);

  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr[na], 0);
  glBegin (GL_QUADS);
  glMultiTexCoord2d (GL_TEXTURE0, 0, 0); glMultiTexCoord2d (GL_TEXTURE1, 0, 0); glMultiTexCoord2d (GL_TEXTURE2, 0, 0); glVertex2i ( 0,  0);
  glMultiTexCoord2d (GL_TEXTURE0, 1, 0); glMultiTexCoord2d (GL_TEXTURE1, 1, 0); glMultiTexCoord2d (GL_TEXTURE2, 1, 0); glVertex2i (NX,  0);
  glMultiTexCoord2d (GL_TEXTURE0, 1, 1); glMultiTexCoord2d (GL_TEXTURE1, 1, 1); glMultiTexCoord2d (GL_TEXTURE2, 1, 1); glVertex2i (NX, NY);
  glMultiTexCoord2d (GL_TEXTURE0, 0, 1); glMultiTexCoord2d (GL_TEXTURE1, 0, 1); glMultiTexCoord2d (GL_TEXTURE2, 0, 1); glVertex2i ( 0, NY);
  glEnd ();

  glUseProgram (0);
}


// initialize an and am with 0 to 1 gradient for drawing of snm (2D only)
//
void initan (int a)
{
  float *buf = (float*)calloc(NX*NY, sizeof(float));

  int x, y;

  for (y=0; y<NY; y++)
    for (x=0; x<NX; x++)
    {
      buf[y*NX+x] = (float)x/NX;
    }

  glBindTexture (GL_TEXTURE_2D, tr[a]);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_R32F, NX,NY, 0, GL_RED, GL_FLOAT, buf);

  free (buf);
}


void initam (int a)
{
  float *buf = (float*)calloc(NX*NY, sizeof(float));

  int x, y;

  for (y=0; y<NY; y++)
    for (x=0; x<NX; x++)
    {
      buf[y*NX+x] = (float)y/NY;
    }

  glBindTexture (GL_TEXTURE_2D, tr[a]);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_R32F, NX,NY, 0, GL_RED, GL_FLOAT, buf);

  free (buf);
}


void makesnm (int an, int am, int asnm)
{
  initan (an);
  initam (am);
  snm (an, am, asnm);
}
