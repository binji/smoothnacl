/*
	SmoothLife


	esc			Quit

	x/y			color scheme
	X/Y			phase for color schemes 1 and 7
	C/V			visualization style 0-6 (3D only)
	v			show/hide timing information and values
	c			show kernels and snm (switch to mode 0 for correct display)
	p			pause
	b/n/space	fill buffer with random blobs
	m			save values (append at the end of config file)
	,			resize window to client area size 640x480 for video recording
	.			maximize to full screen / restore window
	-			save buffer as .bmp (2D only)
	crsr		scroll around (also pg up/down in 3D)
	f1/f2		in 3D rotate box

	q/a			increase/decrease b1 (with shift factor 10 faster)
	w/s			increase/decrease b2
	e/d			increase/decrease d1
	r/f			increase/decrease d2

	t/g			sigmode
	z/h			sigtype
	u/j			mixtype
	i/k			sn
	o/l			sm

	T/G			radius +/-
	Z/H			dt +/-
	5-9			new size NN=128,256,512,1024,2048 in 2D NN=32,64,128,256,512 in 3D
	0/1/2		mode 0/1/2 (discrete/smooth/smooth2 time stepping)

	lmb			in 2D move window in 3D rotate box
	wheel		zoom
*/

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <sys\timeb.h>
#include "glee.h"
#include <gl/glu.h>


const double PI = 6.28318530718;

int dims;

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

typedef unsigned char ubyte;

ubyte prgname[] = "SmoothLife";
FILE *logfile=0;
HWND hwnd;
HDC hdc;
int maximized;
int SX, SY;
_int64 freq, tim, tima;


int timing;
int oldx, oldy, oldw, oldh;
int windowfocus;
double savedispcnt;
int anz, pause;

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
GLuint planx[BMAX][2], plany[BMAX][2], planz[BMAX][2];
GLuint spfb, sptb;		// for save picture
GLenum ttd;		// texture target dimension

GLint loc_b1, loc_b2;
GLint loc_d1, loc_d2;
GLint loc_sn, loc_sm;
GLint loc_mode, loc_sigmode, loc_sigtype, loc_mixtype, loc_dt;
GLint loc_colscheme, loc_phase, loc_visscheme;
GLint loc_dim, loc_tang, loc_tangsc, loc_sc;

bool neu;


double RND (double x)
{
	return x * (double)rand()/((double)RAND_MAX + 1);
}


bool read_config (void)
{
	FILE *file;
	int t;
	double d;

	file = fopen ("SmoothLifeConfig.txt", "r");
	if (file==0) return false;

	fscanf (file, "%d", &t); dims = t;
	fscanf (file, "%d", &t); mode = t;

	fscanf (file, "%lf", &d); ra = d;
	fscanf (file, "%lf", &d); rr = d;
	fscanf (file, "%lf", &d); rb = d;
	fscanf (file, "%lf", &d); dt = d;

	fscanf (file, "%lf", &d); b1 = d;
	fscanf (file, "%lf", &d); b2 = d;
	fscanf (file, "%lf", &d); d1 = d;
	fscanf (file, "%lf", &d); d2 = d;

	fscanf (file, "%d", &t); sigmode = t;
	fscanf (file, "%d", &t); sigtype = t;
	fscanf (file, "%d", &t); mixtype = t;

	fscanf (file, "%lf", &d); sn = d;
	fscanf (file, "%lf", &d); sm = d;

	fclose (file);
	return true;
}


bool write_config (void)
{
	FILE *file;

	file = fopen ("SmoothLifeConfig.txt", "a");
	if (file==0) return false;

	fprintf (file, "%d ", (int)dims);
	fprintf (file, "%d   ", (int)mode);

	fprintf (file, "%.1f  ", ra);
	fprintf (file, "%.1f  ", rr);
	fprintf (file, "%.1f  ", rb);
	fprintf (file, "%.3f   ", dt);

	fprintf (file, "%.3f  ", b1);
	fprintf (file, "%.3f  ", b2);
	fprintf (file, "%.3f  ", d1);
	fprintf (file, "%.3f   ", d2);

	fprintf (file, "%d ", (int)sigmode);
	fprintf (file, "%d ", (int)sigtype);
	fprintf (file, "%d   ", (int)mixtype);

	fprintf (file, "%.3f  ", sn);
	fprintf (file, "%.3f    //\n", sm);

	fclose (file);
	return true;
}


bool create_render_buffer (void)
{
	unsigned int err;

	glGenTextures (1, &sptb);
	err = glGetError (); fprintf (logfile, "sp GenTextures err %d\n", err); fflush (logfile); if (err) return false;

	glBindTexture (GL_TEXTURE_2D, sptb);
	err = glGetError (); fprintf (logfile, "sp BindTexture err %d\n", err); fflush (logfile); if (err) return false;

    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB8, NX, NY, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	err = glGetError (); fprintf (logfile, "sp TexImage %d %d  err %d\n", NX, NY, err); fflush (logfile); if (err) return false;


	glGenFramebuffers (1, &spfb);
	err = glGetError (); fprintf (logfile, "sp GenFramebuffers err %d\n", err); fflush (logfile); if (err) return false;


	glBindFramebuffer (GL_FRAMEBUFFER, spfb);
	err = glGetError (); fprintf (logfile, "sp BindFramebuffer err %d\n", err); fflush (logfile); if (err) return false;

	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sptb, 0);
	err = glGetError (); fprintf (logfile, "sp FramebufferTexture2D err %d\n", err); fflush (logfile); if (err) return false;

	err = glCheckFramebufferStatus (GL_FRAMEBUFFER);
	fprintf (logfile, "sp FramebufferStatus 0x%x\n", err); fflush (logfile); if (err!=0x8cd5) return false;


	fprintf (logfile, "\n"); fflush (logfile);
	return true;
}


bool delete_render_buffer (void)
{
	unsigned int err;

	glBindTexture (GL_TEXTURE_2D, 0);
	err = glGetError (); fprintf (logfile, "sp BindTexture 0 err %d\n", err); fflush (logfile); if (err) return false;
	glDeleteTextures (1, &sptb);
	err = glGetError (); fprintf (logfile, "sp DeleteTextures err %d\n", err); fflush (logfile); if (err) return false;

	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	err = glGetError (); fprintf (logfile, "sp BindFramebuffer 0 err %d\n", err); fflush (logfile); if (err) return false;
	glDeleteFramebuffers (1, &spfb);
	err = glGetError (); fprintf (logfile, "sp DeleteFramebuffers err %d\n", err); fflush (logfile); if (err) return false;

	fprintf (logfile, "\n"); fflush (logfile);
	return true;
}


void drawa_render_buffer (int a)
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, NX, 0, NY, -1, 1);
	glViewport (0, 0, NX, NY);

	glBindFramebuffer (GL_FRAMEBUFFER, spfb);
	glUseProgram (shader_draw);
	glUniform1f (loc_colscheme, (float)colscheme);
	glUniform1f (loc_phase, (float)phase);

	glClearColor (0.5, 0.5, 0.5, 1.0);
	glClear (GL_COLOR_BUFFER_BIT);

	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, tr[a]);
	glUniform1i (glGetUniformLocation (shader_draw, "tex0"), 0);

	glBegin (GL_QUADS);
	glMultiTexCoord2d (GL_TEXTURE0, fx   , fy   ); glVertex2i ( 0,  0);
	glMultiTexCoord2d (GL_TEXTURE0, fx+bx, fy   ); glVertex2i (NX,  0);
	glMultiTexCoord2d (GL_TEXTURE0, fx+bx, fy+by); glVertex2i (NX, NY);
	glMultiTexCoord2d (GL_TEXTURE0, fx   , fy+by); glVertex2i ( 0, NY);
	glEnd ();

	glUseProgram (0);
}


void mysavepic (void)
{
	int y;
	static int bildnr = 1;
	char buf[128];
	char *buffer;
	FILE *file;
	BITMAPFILEHEADER *bmfh = NULL;
	BITMAPINFOHEADER *bmih = NULL;

	bmfh = (BITMAPFILEHEADER *)calloc (sizeof(BITMAPFILEHEADER), sizeof(char));
	bmfh->bfType = 'M' * 256 + 'B';
	bmfh->bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (long)NX*NY*3;
	bmfh->bfReserved1 = 0;
	bmfh->bfReserved2 = 0;
	bmfh->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	bmih = (BITMAPINFOHEADER *)calloc (sizeof(BITMAPINFOHEADER), sizeof(char));
	bmih->biSize = sizeof(BITMAPINFOHEADER);
	bmih->biWidth = NX;
	bmih->biHeight = NY;
	bmih->biPlanes = 1;
	bmih->biBitCount = 24;
	bmih->biCompression = BI_RGB;
	bmih->biSizeImage = (long)NX*NY*3;
	bmih->biXPelsPerMeter = 0;
	bmih->biYPelsPerMeter = 0;
	bmih->biClrUsed = 0;
	bmih->biClrImportant = 0;

	create_render_buffer ();
	drawa_render_buffer (AA);

	buffer = (char*)calloc (3*NX, sizeof(char));
	sprintf (buf, "SmoothLife%02d.bmp", bildnr);
	file = fopen (buf, "wb");
	fwrite (bmfh, 1, sizeof(BITMAPFILEHEADER), file);
	fwrite (bmih, 1, sizeof(BITMAPINFOHEADER), file);
	for (y=0; y<NY; y++)
	{
		glReadPixels (0, y, NX, 1, GL_BGR, GL_UNSIGNED_BYTE, buffer);
		fwrite (buffer, 1, 3*NX, file);
	}
	fclose (file);
	free (buffer);

	delete_render_buffer ();

	if (bmih) free (bmih);
	if (bmfh) free (bmfh);

	bildnr++;
}


bool printShaderInfoLog (GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetShaderiv (obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc (infologLength);
        glGetShaderInfoLog (obj, infologLength, &charsWritten, infoLog);
		fprintf (logfile, "%s\n", infoLog); fflush (logfile);
        free (infoLog);
    }
	return infologLength > 2;
}


bool printProgramInfoLog (GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetProgramiv (obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc (infologLength);
		glGetProgramInfoLog (obj, infologLength, &charsWritten, infoLog);
		fprintf (logfile, "%s\n", infoLog); fflush (logfile);
		free (infoLog);
	}
	return infologLength > 2;
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

	fprintf (logfile, "setting shader: %s\n", fname);

	sprintf (filename, "%s\\%s.vert", dname, fname);
	fp = fopen (filename, "rt");
	if (fp == NULL) { fprintf (logfile, "couldn't open .vert\n"); fflush (logfile); return false; }
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
	if (fp == NULL) { fprintf (logfile, "couldn't open .frag\n"); fflush (logfile); return false; }
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
		fprintf (logfile, "error in vertex shader!\n\n"); fflush (logfile);
		ret |= true;
	}
	else
	{
		fprintf (logfile, "vertex shader ok\n\n"); fflush (logfile);
	}

	glCompileShader (f);
	if (printShaderInfoLog (f))
	{
		fprintf (logfile, "error in fragment shader!\n\n"); fflush (logfile);
		ret |= true;
	}
	else
	{
		fprintf (logfile, "fragment shader ok\n\n"); fflush (logfile);
	}

	prog = glCreateProgram ();
	glAttachShader (prog, v);
	glAttachShader (prog, f);

	glLinkProgram (prog);
	if (printProgramInfoLog (prog))
	{
		fprintf (logfile, "shader program error!\n\n"); fflush (logfile);
		ret |= true;
	}
	else
	{
		fprintf (logfile, "shader program ok\n\n"); fflush (logfile);
	}

	return ret;
}


void dosystem (void)
{
	MSG msg;
	while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}


void splat3D (float *buf)
{
	double mx, my, mz, dx, dy, dz, u, l;
	int ix, iy, iz;

	mx = RND(NX);
	my = RND(NY);
	mz = RND(NZ);
	u = ra*(RND(0.5) + 0.5);

	for (iz=(int)(mz-u-1); iz<=(int)(mz+u+1); iz++)
	for (iy=(int)(my-u-1); iy<=(int)(my+u+1); iy++)
	for (ix=(int)(mx-u-1); ix<=(int)(mx+u+1); ix++)
	{
		dx = mx-ix;
		dy = my-iy;
		dz = mz-iz;
		l = sqrt(dx*dx+dy*dy+dz*dz);
		if (l<u)
		{
			int px = ix;
			int py = iy;
			int pz = iz;
			while (px<  0) px+=NX;
			while (px>=NX) px-=NX;
			while (py<  0) py+=NY;
			while (py>=NY) py-=NY;
			while (pz<  0) pz+=NZ;
			while (pz>=NZ) pz-=NZ;
			if (px>=0 && px<NX && py>=0 && py<NY && pz>=0 && pz<NZ)
			{
				*(buf + NX*NY*pz + NX*py + px) = 1.0;
			}
		}
		dosystem ();
	}
}


void inita3D (int a)
{
	float *buf = (float*)calloc(NX*NY*NZ, sizeof(float));

	double mx, my, mz;

	mx = 2*ra; if (mx>NX) mx=NX;
	my = 2*ra; if (my>NY) my=NY;
	mz = 2*ra; if (mz>NZ) mz=NZ;

	for (int t=0; t<=(int)(NX*NY*NZ/(mx*my*mz)); t++)
	{
		splat3D (buf);
	}

	glBindTexture (GL_TEXTURE_3D, tr[a]);
	glTexImage3D (GL_TEXTURE_3D, 0, GL_R32F, NX,NY,NZ, 0, GL_RED, GL_FLOAT, buf);

	free (buf);
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
		dosystem ();
	}
}


void inita2D (int a)
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


void splat1D (float *buf)
{
	double mx, dx, u, l;
	int ix;

	mx = RND(NX);
	u = ra*(RND(0.5) + 0.5);

	for (ix=(int)(mx-u-1); ix<=(int)(mx+u+1); ix++)
	{
		dx = mx-ix;
		l = sqrt(dx*dx);
		if (l<u)
		{
			int px = ix;
			while (px<  0) px+=NX;
			while (px>=NX) px-=NX;
			if (px>=0 && px<NX)
			{
				*(buf + px) = 1.0;
			}
		}
		dosystem ();
	}
}


void inita1D (int a)
{
	float *buf = (float*)calloc(NX, sizeof(float));

	double mx;

	mx = 2*ra; if (mx>NX) mx=NX;

	for (int t=0; t<=(int)(NX/mx); t++)
	{
		splat1D (buf);
	}

	glBindTexture (GL_TEXTURE_1D, tr[a]);
	glTexImage1D (GL_TEXTURE_1D, 0, GL_R32F, NX, 0, GL_RED, GL_FLOAT, buf);

	free (buf);
}


void inita (int a)
{
	if (dims==1) inita1D (a);
	if (dims==2) inita2D (a);
	if (dims==3) inita3D (a);
}


bool create_buffers (void)
{
	unsigned int err;
	int t, s, eb;

	if (dims==1)
	{
		fprintf (logfile, "create buffers 1D %d\n", NX); fflush (logfile);
	}
	else if (dims==2)
	{
		fprintf (logfile, "create buffers 2D %d %d\n", NX, NY); fflush (logfile);
	}
	else // dims==3
	{
		fprintf (logfile, "create buffers 3D %d %d %d\n", NX, NY, NZ); fflush (logfile);
	}


	// Fourier (complex) buffers

	glGenTextures (AFB, &tb[0]);
	err = glGetError (); fprintf (logfile, "GenTextures err %d\n", err); fflush (logfile); if (err) return false;

	glGenFramebuffers (AFB, &fb[0]);
	err = glGetError (); fprintf (logfile, "GenFramebuffers err %d\n", err); fflush (logfile); if (err) return false;

	for (t=0; t<AFB; t++)
	{
		fprintf (logfile, "complex buffer %d\n", t); fflush (logfile);

		glBindTexture (ttd, tb[t]);
		err = glGetError (); fprintf (logfile, "BindTexture err %d\n", err); fflush (logfile); if (err) return false;

		glTexParameterf (ttd, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf (ttd, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf (ttd, GL_TEXTURE_WRAP_S, GL_REPEAT);
		if (dims>1)
		{
			glTexParameterf (ttd, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		if (dims>2)
		{
			glTexParameterf (ttd, GL_TEXTURE_WRAP_R, GL_REPEAT);
		}

		if (dims==1) glTexImage1D (GL_TEXTURE_1D, 0, GL_RG32F, NX/2+1, 0, GL_RG, GL_FLOAT, NULL);
		if (dims==2) glTexImage2D (GL_TEXTURE_2D, 0, GL_RG32F, NX/2+1,NY, 0, GL_RG, GL_FLOAT, NULL);
		if (dims==3) glTexImage3D (GL_TEXTURE_3D, 0, GL_RG32F, NX/2+1,NY,NZ, 0, GL_RG, GL_FLOAT, NULL);
		err = glGetError (); fprintf (logfile, "TexImage err %d\n", err); fflush (logfile); if (err) return false;


		glBindFramebuffer (GL_FRAMEBUFFER, fb[t]);
		err = glGetError (); fprintf (logfile, "BindFramebuffer err %d\n", err); fflush (logfile); if (err) return false;

		if (dims==1) glFramebufferTexture1D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, tb[t], 0);
		if (dims==2) glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[t], 0);
		if (dims==3) glFramebufferTexture3D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tb[t], 0, 0);
		err = glGetError (); fprintf (logfile, "FramebufferTexture err %d\n", err); fflush (logfile); if (err) return false;

		err = glCheckFramebufferStatus (GL_FRAMEBUFFER);
		fprintf (logfile, "FramebufferStatus 0x%x\n", err); fflush (logfile); if (err!=0x8cd5) return false;
	}


	// real buffers

	glGenTextures (ARB, &tr[0]);
	err = glGetError (); fprintf (logfile, "GenTextures err %d\n", err); fflush (logfile); if (err) return false;

	glGenFramebuffers (ARB, &fr[0]);
	err = glGetError (); fprintf (logfile, "GenFramebuffers err %d\n", err); fflush (logfile); if (err) return false;

	for (t=0; t<ARB; t++)
	{
		fprintf (logfile, "real buffer %d\n", t); fflush (logfile);

		glBindTexture (ttd, tr[t]);
		err = glGetError (); fprintf (logfile, "BindTexture err %d\n", err); fflush (logfile); if (err) return false;

		glTexParameterf (ttd, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf (ttd, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf (ttd, GL_TEXTURE_WRAP_S, GL_REPEAT);
		if (dims>1)
		{
			glTexParameterf (ttd, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		if (dims>2)
		{
			glTexParameterf (ttd, GL_TEXTURE_WRAP_R, GL_REPEAT);
		}

		if (dims==1) glTexImage1D (GL_TEXTURE_1D, 0, GL_R32F, NX, 0, GL_RED, GL_FLOAT, NULL);
		if (dims==2) glTexImage2D (GL_TEXTURE_2D, 0, GL_R32F, NX,NY, 0, GL_RED, GL_FLOAT, NULL);
		if (dims==3) glTexImage3D (GL_TEXTURE_3D, 0, GL_R32F, NX,NY,NZ, 0, GL_RED, GL_FLOAT, NULL);
		err = glGetError (); fprintf (logfile, "TexImage err %d\n", err); fflush (logfile); if (err) return false;


		glBindFramebuffer (GL_FRAMEBUFFER, fr[t]);
		err = glGetError (); fprintf (logfile, "BindFramebuffer err %d\n", err); fflush (logfile); if (err) return false;

		if (dims==1) glFramebufferTexture1D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, tr[t], 0);
		if (dims==2) glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr[t], 0);
		if (dims==3) glFramebufferTexture3D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tr[t], 0, 0);
		err = glGetError (); fprintf (logfile, "FramebufferTexture err %d\n", err); fflush (logfile); if (err) return false;

		err = glCheckFramebufferStatus (GL_FRAMEBUFFER);
		fprintf (logfile, "FramebufferStatus 0x%x\n", err); fflush (logfile); if (err!=0x8cd5) return false;
	}


	// FFT plan texture

	glGenTextures ((BX-1+2)*2, &planx[0][0]);
	for (s=0; s<=1; s++)
	{
		for (eb=0; eb<=BX-1+1; eb++)
		{
			glBindTexture (GL_TEXTURE_1D, planx[eb][s]);
			err = glGetError (); fprintf (logfile, "BindTexture err %d\n", err); fflush (logfile); if (err) return false;

			glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

			glTexImage1D (GL_TEXTURE_1D, 0, GL_RGBA32F, NX/2+1, 0, GL_RGBA, GL_FLOAT, NULL);
			err = glGetError (); fprintf (logfile, "TexImage err %d\n", err); fflush (logfile); if (err) return false;
		}
	}

	if (dims>1)
	{
		glGenTextures ((BY+1)*2, &plany[0][0]);
		for (s=0; s<=1; s++)
		{
			for (eb=1; eb<=BY; eb++)
			{
				glBindTexture (GL_TEXTURE_1D, plany[eb][s]);
				err = glGetError (); fprintf (logfile, "BindTexture err %d\n", err); fflush (logfile); if (err) return false;

				glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

				glTexImage1D (GL_TEXTURE_1D, 0, GL_RGBA32F, NY, 0, GL_RGBA, GL_FLOAT, NULL);
				err = glGetError (); fprintf (logfile, "TexImage err %d\n", err); fflush (logfile); if (err) return false;
			}
		}
	}

	if (dims>2)
	{
		glGenTextures ((BZ+1)*2, &planz[0][0]);
		for (s=0; s<=1; s++)
		{
			for (eb=1; eb<=BZ; eb++)
			{
				glBindTexture (GL_TEXTURE_1D, planz[eb][s]);
				err = glGetError (); fprintf (logfile, "BindTexture err %d\n", err); fflush (logfile); if (err) return false;

				glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

				glTexImage1D (GL_TEXTURE_1D, 0, GL_RGBA32F, NZ, 0, GL_RGBA, GL_FLOAT, NULL);
				err = glGetError (); fprintf (logfile, "TexImage err %d\n", err); fflush (logfile); if (err) return false;
			}
		}
	}

	fprintf (logfile, "all buffers ok\n"); fflush (logfile);


	return true;
}


bool delete_buffers (void)
{
	unsigned int err;

	fprintf (logfile, "delete buffers\n"); fflush (logfile);

	glBindTexture (ttd, 0);
	err = glGetError (); fprintf (logfile, "BindTexture 0 err %d\n", err); fflush (logfile); if (err) return false;
	glDeleteTextures (AFB, &tb[0]);
	err = glGetError (); fprintf (logfile, "DeleteTextures err %d\n", err); fflush (logfile); if (err) return false;
	glDeleteTextures (ARB, &tr[0]);
	err = glGetError (); fprintf (logfile, "DeleteTextures err %d\n", err); fflush (logfile); if (err) return false;
	glDeleteTextures ((BX-1+2)*2, &planx[0][0]);
	err = glGetError (); fprintf (logfile, "DeleteTextures err %d\n", err); fflush (logfile); if (err) return false;
	if (dims>1)
	{
		glDeleteTextures ((BY+1)*2, &plany[0][0]);
		err = glGetError (); fprintf (logfile, "DeleteTextures err %d\n", err); fflush (logfile); if (err) return false;
	}
	if (dims>2)
	{
		glDeleteTextures ((BZ+1)*2, &planz[0][0]);
		err = glGetError (); fprintf (logfile, "DeleteTextures err %d\n", err); fflush (logfile); if (err) return false;
	}

	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	err = glGetError (); fprintf (logfile, "BindFramebuffer 0 err %d\n", err); fflush (logfile); if (err) return false;
	glDeleteFramebuffers (AFB, &fb[0]);
	err = glGetError (); fprintf (logfile, "DeleteFramebuffers err %d\n", err); fflush (logfile); if (err) return false;
	glDeleteFramebuffers (ARB, &fr[0]);
	err = glGetError (); fprintf (logfile, "DeleteFramebuffers err %d\n", err); fflush (logfile); if (err) return false;

	return true;
}


int cube[6][4][3] =
{
	{ { 1, 1,-1}, {-1, 1,-1}, {-1, 1, 1}, { 1, 1, 1} },
	{ { 1,-1, 1}, {-1,-1, 1}, {-1,-1,-1}, { 1,-1,-1} },
	{ { 1, 1, 1}, {-1, 1, 1}, {-1,-1, 1}, { 1,-1, 1} },
	{ { 1,-1,-1}, {-1,-1,-1}, {-1, 1,-1}, { 1, 1,-1} },
	{ {-1, 1, 1}, {-1, 1,-1}, {-1,-1,-1}, {-1,-1, 1} },
	{ { 1, 1,-1}, { 1, 1, 1}, { 1,-1, 1}, { 1,-1,-1} }
};


void drawa (int a)
{
	if (dims==1)
	{
		static int ypos=0;

		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0, SX, 0, SY, -1, 1);
		glViewport (0, 0, SX, SY);

		glBindFramebuffer (GL_FRAMEBUFFER, 0);

		//glClearColor (0.5, 0.5, 0.5, 1.0);
		//glClear (GL_COLOR_BUFFER_BIT);
		
		glActiveTexture (GL_TEXTURE0);
		glEnable (GL_TEXTURE_1D);
		glBindTexture (GL_TEXTURE_1D, tr[a]);

		glActiveTexture (GL_TEXTURE1);
		glDisable (GL_TEXTURE_1D);

		glUseProgram (shader_draw);
		glUniform1i (glGetUniformLocation (shader_draw, "tex0"), 0);
		glUniform1f (loc_colscheme, (float)colscheme);
		glUniform1f (loc_phase, (float)phase);

		glDisable (GL_DEPTH_TEST);

		glBegin (GL_LINES);
		glMultiTexCoord1d (GL_TEXTURE0, fx   ); glVertex2d (ox   , oy+ypos+0.5);
		glMultiTexCoord1d (GL_TEXTURE0, fx+bx); glVertex2d (ox+qx, oy+ypos+0.5);
		glEnd ();

		glUseProgram (0);

		ypos++;
		if (ypos>SY) ypos=0;

	}
	else if (dims==2)
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
	else // dims==3
	{

		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		gluPerspective (30, (double)SX/SY, 0.1, 1000);
		glViewport (0, 0, SX, SY);

		glBindFramebuffer (GL_FRAMEBUFFER, 0);

		glActiveTexture (GL_TEXTURE0);
		glEnable (GL_TEXTURE_3D);
		glBindTexture (GL_TEXTURE_3D, tr[a]);
		glUseProgram (shader_draw);
		glUniform1i (glGetUniformLocation (shader_draw, "tex0"), 0);

		glUniform1f (glGetUniformLocation (shader_draw, "fx"), (float)fx);
		glUniform1f (glGetUniformLocation (shader_draw, "fy"), (float)fy);
		glUniform1f (glGetUniformLocation (shader_draw, "fz"), (float)fz);

		glUniform1f (glGetUniformLocation (shader_draw, "nx"), (float)NX);
		glUniform1f (glGetUniformLocation (shader_draw, "ny"), (float)NY);
		glUniform1f (glGetUniformLocation (shader_draw, "nz"), (float)NZ);

		glUniform1f (loc_colscheme, (float)colscheme);
		glUniform1f (loc_phase, (float)phase);
		glUniform1f (loc_visscheme, (float)visscheme);

		glTexParameterf (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glEnable (GL_DEPTH_TEST);

		glClearColor (0.5, 0.5, 0.5, 1.0);
		glClearDepth (1.0);
		glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();

		glPushMatrix ();
		glTranslated (0, 0, qq*10);

		glRotated (-(wj+wy), 1, 0, 0);
		glRotated (-(wi+wx), 0, 1, 0);

		wi += dw;
		phase += dphase;

		for (int s=0; s<6; s++)
		{
			glBegin (GL_QUADS);
			for (int c=0; c<4; c++)
			{
				glVertex3d (NX/2*cube[s][c][0], NY/2*cube[s][c][1], NZ/2*cube[s][c][2]);
			}
			glEnd ();
		}

		glPopMatrix ();

		glUseProgram (0);
		glDisable (GL_DEPTH_TEST);
		glTexParameterf (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	}
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
	if (ad==0) { fprintf (logfile, "ad failed\n"); fflush (logfile); return; }

	ar = (float*)calloc (NX*NY, sizeof(float));
	if (ar==0) { fprintf (logfile, "ar failed\n"); fflush (logfile); return; }

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
		if (dims>2)
		{
			if (iz<NZ/2) z = iz; else z = iz-NZ;
		}
		else
		{
			z = 0;
		}
		if (z>=-Ra && z<=Ra)
		{
			for (iy=0; iy<NY; iy++)
			{
				if (dims>1)
				{
					if (iy<NY/2) y = iy; else y = iy-NY;
				}
				else
				{
					y = 0;
				}
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

		if (dims==1)
		{
			glBindTexture (GL_TEXTURE_1D, tr[kd]);
			glTexSubImage1D (GL_TEXTURE_1D, 0, 0, NX, GL_RED, GL_FLOAT, ad);

			glBindTexture (GL_TEXTURE_1D, tr[kr]);
			glTexSubImage1D (GL_TEXTURE_1D, 0, 0, NX, GL_RED, GL_FLOAT, ar);
		}
		else if (dims==2)
		{
			glBindTexture (GL_TEXTURE_2D, tr[kd]);
			glTexSubImage2D (GL_TEXTURE_2D, 0, 0,0, NX,NY, GL_RED, GL_FLOAT, ad);

			glBindTexture (GL_TEXTURE_2D, tr[kr]);
			glTexSubImage2D (GL_TEXTURE_2D, 0, 0,0, NX,NY, GL_RED, GL_FLOAT, ar);
		}
		else // dims==3
		{
			glBindTexture (GL_TEXTURE_3D, tr[kd]);
			glTexSubImage3D (GL_TEXTURE_3D, 0, 0,0,iz, NX,NY,1, GL_RED, GL_FLOAT, ad);

			glBindTexture (GL_TEXTURE_3D, tr[kr]);
			glTexSubImage3D (GL_TEXTURE_3D, 0, 0,0,iz, NX,NY,1, GL_RED, GL_FLOAT, ar);
		}
	}	// for iz

	free (ar);
	free (ad);

	fprintf (logfile, "ra=%lf rr=%lf rb=%lf ri=%lf bb=%lf kflr=%lf kfld=%lf kflr/kfld=%lf\n", ra, rr, rb, ri, bb, kflr, kfld, kflr/kfld); fflush (logfile);
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


void fft_planz (void)
{
	float *p = (float*)calloc (4*NZ, sizeof(float));

	for (int s=0; s<=1; s++)
	{
		int si = s*2-1;
		for (int eb=1; eb<=BZ; eb++)
		{

			for (int x=0; x<NZ; x++)
			{
				int l = 1<<eb;
				int j = x%l;
				double w = si*PI*j/l;
				if (j<l/2)
				{
					if (eb==1)
					{
						*(p + 4*x + 0) = bitreverse(x    ,BZ)/(float)NZ;
						*(p + 4*x + 1) = bitreverse(x+l/2,BZ)/(float)NZ;
					}
					else
					{
						*(p + 4*x + 0) = (x    )/(float)NZ;
						*(p + 4*x + 1) = (x+l/2)/(float)NZ;
					}
				}
				else
				{
					if (eb==1)
					{
						*(p + 4*x + 0) = bitreverse(x-l/2,BZ)/(float)NZ;
						*(p + 4*x + 1) = bitreverse(x    ,BZ)/(float)NZ;
					}
					else
					{
						*(p + 4*x + 0) = (x-l/2)/(float)NZ;
						*(p + 4*x + 1) = (x    )/(float)NZ;
					}
				}
				*(p + 4*x + 2) = (float)cos(w);
				*(p + 4*x + 3) = (float)sin(w);
			}

			glBindTexture (GL_TEXTURE_1D, planz[eb][s]);
			glTexSubImage1D (GL_TEXTURE_1D, 0, 0, NZ, GL_RGBA, GL_FLOAT, p);
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

	if (dims==1)
	{
		glFramebufferTexture1D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, tb[na], 0);
		glBegin (GL_LINES);
		glMultiTexCoord1d (GL_TEXTURE0, 0-1.0/NX); glMultiTexCoord1d (GL_TEXTURE1, 0-0.0/NX); glVertex2d (   0, 0.5);
		glMultiTexCoord1d (GL_TEXTURE0, 1-1.0/NX); glMultiTexCoord1d (GL_TEXTURE1, 1-0.0/NX); glVertex2d (NX/2, 0.5);
		glEnd ();
		/*
		glBegin (GL_QUADS);
		glMultiTexCoord2d (GL_TEXTURE0, 0-1.0/NX, 0); glMultiTexCoord2d (GL_TEXTURE1, 0-0.0/NX, 0); glVertex2i (   0, 0);
		glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/NX, 0); glMultiTexCoord2d (GL_TEXTURE1, 1-0.0/NX, 0); glVertex2i (NX/2, 0);
		glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/NX, 1); glMultiTexCoord2d (GL_TEXTURE1, 1-0.0/NX, 1); glVertex2i (NX/2, 1);
		glMultiTexCoord2d (GL_TEXTURE0, 0-1.0/NX, 1); glMultiTexCoord2d (GL_TEXTURE1, 0-0.0/NX, 1); glVertex2i (   0, 1);
		glEnd ();
		*/
	}
	else if (dims==2)
	{
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[na], 0);
		glBegin (GL_QUADS);
		glMultiTexCoord2d (GL_TEXTURE0, 0-1.0/NX, 0); glMultiTexCoord2d (GL_TEXTURE1, 0-0.0/NX, 0); glVertex2i (   0,  0);
		glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/NX, 0); glMultiTexCoord2d (GL_TEXTURE1, 1-0.0/NX, 0); glVertex2i (NX/2,  0);
		glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/NX, 1); glMultiTexCoord2d (GL_TEXTURE1, 1-0.0/NX, 1); glVertex2i (NX/2, NY);
		glMultiTexCoord2d (GL_TEXTURE0, 0-1.0/NX, 1); glMultiTexCoord2d (GL_TEXTURE1, 0-0.0/NX, 1); glVertex2i (   0, NY);
		glEnd ();
	}
	else // dims==3
	{
		for (int t=0; t<NZ; t++)
		{
			double l = (t+0.5)/NZ;
			glFramebufferTexture3D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tb[na], 0, t);
			glBegin (GL_QUADS);
			glMultiTexCoord3d (GL_TEXTURE0, 0-1.0/NX, 0, l); glMultiTexCoord3d (GL_TEXTURE1, 0-0.0/NX, 0, l); glVertex3i (   0,  0, t);
			glMultiTexCoord3d (GL_TEXTURE0, 1-1.0/NX, 0, l); glMultiTexCoord3d (GL_TEXTURE1, 1-0.0/NX, 0, l); glVertex3i (NX/2,  0, t);
			glMultiTexCoord3d (GL_TEXTURE0, 1-1.0/NX, 1, l); glMultiTexCoord3d (GL_TEXTURE1, 1-0.0/NX, 1, l); glVertex3i (NX/2, NY, t);
			glMultiTexCoord3d (GL_TEXTURE0, 0-1.0/NX, 1, l); glMultiTexCoord3d (GL_TEXTURE1, 0-0.0/NX, 1, l); glVertex3i (   0, NY, t);
			glEnd ();
		}
	}

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

	if (dims==1)
	{
		glFramebufferTexture1D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, tr[na], 0);
		glBegin (GL_LINES);
		glMultiTexCoord1d (GL_TEXTURE0,   0.0/(NX/2+1)); glMultiTexCoord1d (GL_TEXTURE1,  0); glVertex2d ( 0, 0.5);
		glMultiTexCoord1d (GL_TEXTURE0, 1-1.0/(NX/2+1)); glMultiTexCoord1d (GL_TEXTURE1, NX); glVertex2d (NX, 0.5);
		glEnd ();
		/*
		glBegin (GL_QUADS);
		glMultiTexCoord2d (GL_TEXTURE0,   0.0/(NX/2+1), 0); glMultiTexCoord2d (GL_TEXTURE1,  0, 0); glVertex2i ( 0, 0);
		glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/(NX/2+1), 0); glMultiTexCoord2d (GL_TEXTURE1, NX, 0); glVertex2i (NX, 0);
		glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/(NX/2+1), 1); glMultiTexCoord2d (GL_TEXTURE1, NX, 1); glVertex2i (NX, 1);
		glMultiTexCoord2d (GL_TEXTURE0,   0.0/(NX/2+1), 1); glMultiTexCoord2d (GL_TEXTURE1,  0, 1); glVertex2i ( 0, 1);
		glEnd ();
		*/
	}
	else if (dims==2)
	{
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr[na], 0);
		glBegin (GL_QUADS);
		glMultiTexCoord2d (GL_TEXTURE0,   0.0/(NX/2+1), 0); glMultiTexCoord2d (GL_TEXTURE1,  0, 0); glVertex2i ( 0,  0);
		glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/(NX/2+1), 0); glMultiTexCoord2d (GL_TEXTURE1, NX, 0); glVertex2i (NX,  0);
		glMultiTexCoord2d (GL_TEXTURE0, 1-1.0/(NX/2+1), 1); glMultiTexCoord2d (GL_TEXTURE1, NX, 1); glVertex2i (NX, NY);
		glMultiTexCoord2d (GL_TEXTURE0,   0.0/(NX/2+1), 1); glMultiTexCoord2d (GL_TEXTURE1,  0, 1); glVertex2i ( 0, NY);
		glEnd ();
	}
	else // dims==3
	{
		for (int t=0; t<NZ; t++)
		{
			double l = (t+0.5)/NZ;
			glFramebufferTexture3D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tr[na], 0, t);
			glBegin (GL_QUADS);
			glMultiTexCoord3d (GL_TEXTURE0,   0.0/(NX/2+1), 0, l); glMultiTexCoord3d (GL_TEXTURE1,  0, 0, l); glVertex3i ( 0,  0, t);
			glMultiTexCoord3d (GL_TEXTURE0, 1-1.0/(NX/2+1), 0, l); glMultiTexCoord3d (GL_TEXTURE1, NX, 0, l); glVertex3i (NX,  0, t);
			glMultiTexCoord3d (GL_TEXTURE0, 1-1.0/(NX/2+1), 1, l); glMultiTexCoord3d (GL_TEXTURE1, NX, 1, l); glVertex3i (NX, NY, t);
			glMultiTexCoord3d (GL_TEXTURE0,   0.0/(NX/2+1), 1, l); glMultiTexCoord3d (GL_TEXTURE1,  0, 1, l); glVertex3i ( 0, NY, t);
			glEnd ();
		}
	}

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
	if (dim==3) glBindTexture (GL_TEXTURE_1D, planz[eb][(si+1)/2]);
	glUniform1i (glGetUniformLocation (shader_fft, "tex1"), 1);

	if (dims==1)
	{
		glFramebufferTexture1D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, tb[ffto], 0);
		glBegin (GL_LINES);
		glMultiTexCoord1d (GL_TEXTURE0,  0); glMultiTexCoord1d (GL_TEXTURE1,  0); glVertex2d ( 0, 0.5);
		glMultiTexCoord1d (GL_TEXTURE0, gd); glMultiTexCoord1d (GL_TEXTURE1, gd); glVertex2d (gi, 0.5);
		glEnd ();
		/*
		glBegin (GL_QUADS);
		glMultiTexCoord2d (GL_TEXTURE0,  0, 0); glMultiTexCoord2d (GL_TEXTURE1,  0, 0); glVertex2i ( 0, 0);
		glMultiTexCoord2d (GL_TEXTURE0, gd, 0); glMultiTexCoord2d (GL_TEXTURE1, gd, 0); glVertex2i (gi, 0);
		glMultiTexCoord2d (GL_TEXTURE0, gd, 1); glMultiTexCoord2d (GL_TEXTURE1, gd, 1); glVertex2i (gi, 1);
		glMultiTexCoord2d (GL_TEXTURE0,  0, 1); glMultiTexCoord2d (GL_TEXTURE1,  0, 1); glVertex2i ( 0, 1);
		glEnd ();
		*/
	}
	else if (dims==2)
	{
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[ffto], 0);
		glBegin (GL_QUADS);
		glMultiTexCoord2d (GL_TEXTURE0,  0, 0); glMultiTexCoord2d (GL_TEXTURE1,  0, 0); glVertex2i ( 0,  0);
		glMultiTexCoord2d (GL_TEXTURE0, gd, 0); glMultiTexCoord2d (GL_TEXTURE1, gd, 0); glVertex2i (gi,  0);
		glMultiTexCoord2d (GL_TEXTURE0, gd, 1); glMultiTexCoord2d (GL_TEXTURE1, gd, 1); glVertex2i (gi, NY);
		glMultiTexCoord2d (GL_TEXTURE0,  0, 1); glMultiTexCoord2d (GL_TEXTURE1,  0, 1); glVertex2i ( 0, NY);
		glEnd ();
	}
	else // dims==3
	{
		for (int t=0; t<NZ; t++)
		{
			double l = (t+0.5)/NZ;
			glFramebufferTexture3D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tb[ffto], 0, t);
			glBegin (GL_QUADS);
			glMultiTexCoord3d (GL_TEXTURE0,  0, 0, l); glMultiTexCoord3d (GL_TEXTURE1,  0, 0, l); glVertex3i ( 0,  0, t);
			glMultiTexCoord3d (GL_TEXTURE0, gd, 0, l); glMultiTexCoord3d (GL_TEXTURE1, gd, 0, l); glVertex3i (gi,  0, t);
			glMultiTexCoord3d (GL_TEXTURE0, gd, 1, l); glMultiTexCoord3d (GL_TEXTURE1, gd, 1, l); glVertex3i (gi, NY, t);
			glMultiTexCoord3d (GL_TEXTURE0,  0, 1, l); glMultiTexCoord3d (GL_TEXTURE1,  0, 1, l); glVertex3i ( 0, NY, t);
			glEnd ();
		}
	}

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
			if (dims==1 && t==BX) fft_stage (1, t, si, fftcur, na);
			else fft_stage (1, t, si, fftcur, fftoth);
			s=fftcur; fftcur=fftoth; fftoth=s;
		}

		for (t=1; t<=BY; t++)
		{
			if (dims==2 && t==BY) fft_stage (2, t, si, fftcur, na);
			else fft_stage (2, t, si, fftcur, fftoth);
			s=fftcur; fftcur=fftoth; fftoth=s;
		}

		for (t=1; t<=BZ; t++)
		{
			if (t==BZ) fft_stage (3, t, si, fftcur, na);
			else fft_stage (3, t, si, fftcur, fftoth);
			s=fftcur; fftcur=fftoth; fftoth=s;
		}
	}
	else	// si==1
	{
		for (t=1; t<=BZ; t++)
		{
			if (t==1) fft_stage (3, t, si, vo, fftoth);
			else fft_stage (3, t, si, fftcur, fftoth);
			s=fftcur; fftcur=fftoth; fftoth=s;
		}

		for (t=1; t<=BY; t++)
		{
			if (dims==2 && t==1) fft_stage (2, t, si, vo, fftoth);
			else fft_stage (2, t, si, fftcur, fftoth);
			s=fftcur; fftcur=fftoth; fftoth=s;
		}

		for (t=0; t<=BX-1; t++)
		{
			if (dims==1 && t==0) fft_stage (1, t, si, vo, fftoth);
			else fft_stage (1, t, si, fftcur, fftoth);
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

	if (dims==1)
	{
		glFramebufferTexture1D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, tb[na], 0);
		glBegin (GL_LINES);
		glMultiTexCoord1d (GL_TEXTURE0, 0); glMultiTexCoord1d (GL_TEXTURE1, 0); glVertex2d (     0, 0.5);
		glMultiTexCoord1d (GL_TEXTURE0, 1); glMultiTexCoord1d (GL_TEXTURE1, 1); glVertex2d (NX/2+1, 0.5);
		glEnd ();
		/*
		glBegin (GL_QUADS);
		glMultiTexCoord2d (GL_TEXTURE0, 0, 0); glMultiTexCoord2d (GL_TEXTURE1, 0, 0); glVertex2i (     0, 0);
		glMultiTexCoord2d (GL_TEXTURE0, 1, 0); glMultiTexCoord2d (GL_TEXTURE1, 1, 0); glVertex2i (NX/2+1, 0);
		glMultiTexCoord2d (GL_TEXTURE0, 1, 1); glMultiTexCoord2d (GL_TEXTURE1, 1, 1); glVertex2i (NX/2+1, 1);
		glMultiTexCoord2d (GL_TEXTURE0, 0, 1); glMultiTexCoord2d (GL_TEXTURE1, 0, 1); glVertex2i (     0, 1);
		glEnd ();
		*/
	}
	else if (dims==2)
	{
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[na], 0);
		glBegin (GL_QUADS);
		glMultiTexCoord2d (GL_TEXTURE0, 0, 0); glMultiTexCoord2d (GL_TEXTURE1, 0, 0); glVertex2i (     0,  0);
		glMultiTexCoord2d (GL_TEXTURE0, 1, 0); glMultiTexCoord2d (GL_TEXTURE1, 1, 0); glVertex2i (NX/2+1,  0);
		glMultiTexCoord2d (GL_TEXTURE0, 1, 1); glMultiTexCoord2d (GL_TEXTURE1, 1, 1); glVertex2i (NX/2+1, NY);
		glMultiTexCoord2d (GL_TEXTURE0, 0, 1); glMultiTexCoord2d (GL_TEXTURE1, 0, 1); glVertex2i (     0, NY);
		glEnd ();
	}
	else // dims==3
	{
		for (int t=0; t<NZ; t++)
		{
			double l = (t+0.5)/NZ;
			glFramebufferTexture3D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tb[na], 0, t);
			glBegin (GL_QUADS);
			glMultiTexCoord3d (GL_TEXTURE0, 0, 0, l); glMultiTexCoord3d (GL_TEXTURE1, 0, 0, l); glVertex3i (     0,  0, t);
			glMultiTexCoord3d (GL_TEXTURE0, 1, 0, l); glMultiTexCoord3d (GL_TEXTURE1, 1, 0, l); glVertex3i (NX/2+1,  0, t);
			glMultiTexCoord3d (GL_TEXTURE0, 1, 1, l); glMultiTexCoord3d (GL_TEXTURE1, 1, 1, l); glVertex3i (NX/2+1, NY, t);
			glMultiTexCoord3d (GL_TEXTURE0, 0, 1, l); glMultiTexCoord3d (GL_TEXTURE1, 0, 1, l); glVertex3i (     0, NY, t);
			glEnd ();
		}
	}

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

	if (dims==1)
	{
		glFramebufferTexture1D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, tr[na], 0);
		glBegin (GL_LINES);
		glMultiTexCoord1d (GL_TEXTURE0, 0); glMultiTexCoord1d (GL_TEXTURE1, 0); glMultiTexCoord2d (GL_TEXTURE2, 0, 0); glVertex2d ( 0, 0.5);
		glMultiTexCoord1d (GL_TEXTURE0, 1); glMultiTexCoord1d (GL_TEXTURE1, 1); glMultiTexCoord2d (GL_TEXTURE2, 1, 0); glVertex2d (NX, 0.5);
		glEnd ();
		/*
		glBegin (GL_QUADS);
		glMultiTexCoord2d (GL_TEXTURE0, 0, 0); glMultiTexCoord2d (GL_TEXTURE1, 0, 0); glMultiTexCoord2d (GL_TEXTURE2, 0, 0); glVertex2i ( 0, 0);
		glMultiTexCoord2d (GL_TEXTURE0, 1, 0); glMultiTexCoord2d (GL_TEXTURE1, 1, 0); glMultiTexCoord2d (GL_TEXTURE2, 1, 0); glVertex2i (NX, 0);
		glMultiTexCoord2d (GL_TEXTURE0, 1, 1); glMultiTexCoord2d (GL_TEXTURE1, 1, 1); glMultiTexCoord2d (GL_TEXTURE2, 1, 1); glVertex2i (NX, 1);
		glMultiTexCoord2d (GL_TEXTURE0, 0, 1); glMultiTexCoord2d (GL_TEXTURE1, 0, 1); glMultiTexCoord2d (GL_TEXTURE2, 0, 1); glVertex2i ( 0, 1);
		glEnd ();
		*/
	}
	else if (dims==2)
	{
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr[na], 0);
		glBegin (GL_QUADS);
		glMultiTexCoord2d (GL_TEXTURE0, 0, 0); glMultiTexCoord2d (GL_TEXTURE1, 0, 0); glMultiTexCoord2d (GL_TEXTURE2, 0, 0); glVertex2i ( 0,  0);
		glMultiTexCoord2d (GL_TEXTURE0, 1, 0); glMultiTexCoord2d (GL_TEXTURE1, 1, 0); glMultiTexCoord2d (GL_TEXTURE2, 1, 0); glVertex2i (NX,  0);
		glMultiTexCoord2d (GL_TEXTURE0, 1, 1); glMultiTexCoord2d (GL_TEXTURE1, 1, 1); glMultiTexCoord2d (GL_TEXTURE2, 1, 1); glVertex2i (NX, NY);
		glMultiTexCoord2d (GL_TEXTURE0, 0, 1); glMultiTexCoord2d (GL_TEXTURE1, 0, 1); glMultiTexCoord2d (GL_TEXTURE2, 0, 1); glVertex2i ( 0, NY);
		glEnd ();
	}
	else // dims==3
	{
		for (int t=0; t<NZ; t++)
		{
			double l = (t+0.5)/NZ;
			glFramebufferTexture3D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tr[na], 0, t);
			glBegin (GL_QUADS);
			glMultiTexCoord3d (GL_TEXTURE0, 0, 0, l); glMultiTexCoord3d (GL_TEXTURE1, 0, 0, l); glMultiTexCoord3d (GL_TEXTURE2, 0, 0, l); glVertex3i ( 0,  0, t);
			glMultiTexCoord3d (GL_TEXTURE0, 1, 0, l); glMultiTexCoord3d (GL_TEXTURE1, 1, 0, l); glMultiTexCoord3d (GL_TEXTURE2, 1, 0, l); glVertex3i (NX,  0, t);
			glMultiTexCoord3d (GL_TEXTURE0, 1, 1, l); glMultiTexCoord3d (GL_TEXTURE1, 1, 1, l); glMultiTexCoord3d (GL_TEXTURE2, 1, 1, l); glVertex3i (NX, NY, t);
			glMultiTexCoord3d (GL_TEXTURE0, 0, 1, l); glMultiTexCoord3d (GL_TEXTURE1, 0, 1, l); glMultiTexCoord3d (GL_TEXTURE2, 0, 1, l); glVertex3i ( 0, NY, t);
			glEnd ();
		}
	}

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


#define WM_MOUSEWHEEL  0x020A

LRESULT CALLBACK PrgWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int xp, yp;
	static int lbutton=0, rbutton=0;
	static int oxp, oyp;
	static int oox, ooy;

	switch (message)
	{

	case WM_CHAR:
	if (wParam == 27) PostQuitMessage (0);

	// 'b' 'n' and 'space' in main loop

	if (wParam=='p') pause ^= 1;

	if (wParam=='c') { anz++; if (anz>4) anz=1; }
	if (wParam=='v') timing ^= 1;

	if (wParam=='C') { visscheme++; if (visscheme>6) visscheme=0; }
	if (wParam=='V') { visscheme--; if (visscheme<0) visscheme=6; }

	if (wParam=='x') { colscheme++; if (colscheme>7) colscheme=1; }
	if (wParam=='y') { colscheme--; if (colscheme<1) colscheme=7; }

	if (wParam=='X') phase += 0.01;
	if (wParam=='Y') phase -= 0.01;

	if (wParam=='t') { sigmode++; if (sigmode>4) sigmode=1; }
	if (wParam=='g') { sigmode--; if (sigmode<1) sigmode=4; }

	if (wParam=='z') { sigtype++; if (sigtype>9) sigtype=0; }
	if (wParam=='h') { sigtype--; if (sigtype<0) sigtype=9; }

	if (wParam=='u') { mixtype++; if (mixtype>7) mixtype=0; }
	if (wParam=='j') { mixtype--; if (mixtype<0) mixtype=7; }

	if (wParam=='0') mode = 0;
	if (wParam=='1') mode = 1;
	if (wParam=='2') mode = 2;
	if (wParam=='3') mode = 3;
	if (wParam=='4') mode = 4;

	if (wParam=='T')
	{
		ra += 0.1;
		makekernel (KR, KD);
		fft (KR, KRF, -1);
		fft (KD, KDF, -1);
	}
	if (wParam=='G')
	{
		ra -= 0.1;
		if (ra<1.0) ra=1.0;
		makekernel (KR, KD);
		fft (KR, KRF, -1);
		fft (KD, KDF, -1);
	}

	if (wParam=='Z') dt += 0.001;
	if (wParam=='H') { dt -= 0.001; if (dt<0.001) dt = 0.001; }

	if (dims==1)
	{
		if (wParam=='5') { delete_buffers (); NX= 512; BX= 9; neu = true; }
		if (wParam=='6') { delete_buffers (); NX=1024; BX=10; neu = true; }
		if (wParam=='7') { delete_buffers (); NX=2048; BX=11; neu = true; }
		if (wParam=='8') { delete_buffers (); NX=4096; BX=12; neu = true; }
		if (wParam=='9') { delete_buffers (); NX=8192; BX=13; neu = true; }
	}
	else if (dims==2)
	{
		if (wParam=='5') { delete_buffers (); NX= 128; NY= 128; BX= 7; BY= 7; neu = true; }
		if (wParam=='6') { delete_buffers (); NX= 256; NY= 256; BX= 8; BY= 8; neu = true; }
		if (wParam=='7') { delete_buffers (); NX= 512; NY= 512; BX= 9; BY= 9; neu = true; }
		if (wParam=='8') { delete_buffers (); NX=1024; NY=1024; BX=10; BY=10; neu = true; }
		if (wParam=='9') { delete_buffers (); NX=2048; NY=2048; BX=11; BY=11; neu = true; }
	}
	else // dims==3
	{
		if (wParam=='5') { delete_buffers (); NX= 32; NY= 32; NZ= 32; BX=5; BY=5; BZ=5; neu = true; }
		if (wParam=='6') { delete_buffers (); NX= 64; NY= 64; NZ= 64; BX=6; BY=6; BZ=6; neu = true; }
		if (wParam=='7') { delete_buffers (); NX=128; NY=128; NZ=128; BX=7; BY=7; BZ=7; neu = true; }
		if (wParam=='8') { delete_buffers (); NX=256; NY=256; NZ=256; BX=8; BY=8; BZ=8; neu = true; }
		if (wParam=='9') { delete_buffers (); NX=512; NY=512; NZ=512; BX=9; BY=9; BZ=9; neu = true; }
	}

	if (wParam=='q') b1 += 0.001;
	if (wParam=='a') b1 -= 0.001;
	if (wParam=='w') b2 += 0.001;
	if (wParam=='s') b2 -= 0.001;

	if (wParam=='e') d1 += 0.001;
	if (wParam=='d') d1 -= 0.001;
	if (wParam=='r') d2 += 0.001;
	if (wParam=='f') d2 -= 0.001;

	if (wParam=='i') sn += 0.001;
	if (wParam=='k') sn -= 0.001;

	if (wParam=='o') sm += 0.001;
	if (wParam=='l') sm -= 0.001;

	if (wParam=='Q') b1 += 0.01;
	if (wParam=='A') b1 -= 0.01;
	if (wParam=='W') b2 += 0.01;
	if (wParam=='S') b2 -= 0.01;

	if (wParam=='E') d1 += 0.01;
	if (wParam=='D') d1 -= 0.01;
	if (wParam=='R') d2 += 0.01;
	if (wParam=='F') d2 -= 0.01;

	if (wParam=='I') sn += 0.01;
	if (wParam=='K') sn -= 0.01;

	if (wParam=='O') sm += 0.01;
	if (wParam=='L') sm -= 0.01;

	if (wParam=='m')
	{
		write_config ();
		savedispcnt = 1.0;
	}

	if (wParam=='-')
	{
		if (dims==2) mysavepic ();
	}

	if (wParam=='.')
	{
		if (maximized)
		{
			SetWindowLong (hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
			SetWindowPos (hWnd, HWND_TOP, oldx, oldy, oldw, oldh, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
			RECT rect;
			GetClientRect (hWnd, &rect);
			SX = rect.right;
			SY = rect.bottom;
			maximized = 0;
			//fprintf (logfile, "restored to %d %d\n", SX, SY); fflush (logfile);
		}
		else
		{
			ShowWindow (hWnd, SW_MAXIMIZE);
		}
	}

	if (wParam==',')
	{
		if (! maximized)
		{
			//SetWindowLong (hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
			SetWindowPos (hWnd, HWND_TOP, 300-8, 100-30, 512+16, 512+60+38, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
			oy = 60;
			//SetWindowPos (hWnd, HWND_TOP, 100-8, 100-30, 640+16, 480+38, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
			//SetWindowPos (hWnd, HWND_TOP, 8-8, 30-30, 960+16, 720+38, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
			RECT rect;
			GetClientRect (hWnd, &rect);
			SX = rect.right;
			SY = rect.bottom;
			maximized = 0;
			//fprintf (logfile, "restored to %d %d\n", SX, SY); fflush (logfile);
		}
	}
	break;


	case WM_LBUTTONDOWN:
	oxp = LOWORD(lParam);
	oyp = HIWORD(lParam);
	oox = ox;
	ooy = oy;
	lbutton=1;
	break;


	case WM_LBUTTONUP:
	lbutton=0;
	wi = wi+wx; wx = 0;
	wj = wj+wy; wy = 0;
	break;


	case WM_RBUTTONDOWN:
	oxp = LOWORD(lParam);
	oyp = HIWORD(lParam);
	rbutton=1;
	break;


	case WM_RBUTTONUP:
	rbutton=0;
	break;


	case WM_MOUSEMOVE:
	xp = LOWORD(lParam);
	yp = HIWORD(lParam);
	if (rbutton)
	{
	}
	if (lbutton)
	{
		ox = oox+(xp-oxp);
		oy = ooy-(yp-oyp);
		wy = oyp-yp;
		wx = oxp-xp;
	}
	break;


	case WM_MOUSEWHEEL:
	POINT pnt;
	xp = LOWORD(lParam);
	yp = HIWORD(lParam);
	pnt.x = xp;
	pnt.y = yp;
	ScreenToClient (hWnd, &pnt);
	xp = pnt.x;
	yp = pnt.y;
	if (HIWORD(wParam)>255)
	{
		qq--;
	}
	else
	{
		qq++;
	}
	qx = (int)pow(2,qq/16.0);
	qy = (int)pow(2,qq/16.0);
	break;


	case WM_EXITSIZEMOVE:
	RECT rect;
	GetWindowRect (hWnd, &rect);
	oldx = rect.left;
	oldy = rect.top;
	oldw = rect.right-rect.left;
	oldh = rect.bottom-rect.top;
	GetClientRect (hWnd, &rect);
	if (rect.right!=SX || rect.bottom!=SY)
	{
		SX = rect.right;
		SY = rect.bottom;
		//fprintf (logfile, "resized to %d %d\n", SX, SY); fflush (logfile);
	}
	break;


	case WM_SIZE:
	if (wParam==SIZE_MAXIMIZED)
	{
		SX = GetSystemMetrics (SM_CXSCREEN);
		SY = GetSystemMetrics (SM_CYSCREEN);
		SetWindowLong (hWnd, GWL_STYLE, WS_POPUP);
		SetWindowPos (hWnd, HWND_TOP, 0, 0, SX, SY, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
		maximized = 1;
		//fprintf (logfile, "maximized to %d %d\n", SX, SY); fflush (logfile);
	}
	break;

	case WM_SETFOCUS: windowfocus = 1; break;
	case WM_KILLFOCUS: windowfocus = 0; break;


	case WM_CLOSE:
	PostQuitMessage (0);
	break;

	case WM_QUERYENDSESSION:
	return (long)TRUE;

	default:
	return DefWindowProc (hWnd, message, wParam, lParam);

	}

	return 0L;
}


void registerprg (HINSTANCE hinst)
{
	WNDCLASS wcPrgClass;

	wcPrgClass.lpszClassName = (char *)prgname;
	wcPrgClass.hInstance     = hinst;
	wcPrgClass.lpfnWndProc   = PrgWndProc;
	wcPrgClass.hCursor       = LoadCursor (0, IDC_ARROW);
	wcPrgClass.hIcon         = 0;
	wcPrgClass.lpszMenuName  = 0;
	wcPrgClass.hbrBackground = (HBRUSH)GetStockObject (BLACK_BRUSH);
	wcPrgClass.style         = CS_OWNDC;
	wcPrgClass.cbClsExtra    = 0;
	wcPrgClass.cbWndExtra    = 0;
	RegisterClass (&wcPrgClass);
}


// Main
//
int WINAPI WinMain (HINSTANCE hi, HINSTANCE hpi, LPSTR lpszCmdLine, int cmdShow)
{
	MSG msg;
	HGLRC hglrc, nhglrc;
	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		//WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, 0,
		0
	};
	const ubyte *GLVersionString;

	hwnd = 0;
	hdc = 0;
	hglrc = 0;
	nhglrc = 0;

	QueryPerformanceFrequency ((LARGE_INTEGER *)&freq);

	srand ((unsigned)time (0));

	_fmode = O_TEXT;
	logfile = fopen ("SmoothLifeLog.txt", "w");
	if (logfile==0) goto ende;
	fprintf (logfile, "starting SmoothLife\n"); fflush (logfile);

	if (! read_config ()) { fprintf (logfile, "couldn't read config file\n"); fflush (logfile); goto ende; }

	maximized = 0;

	if (hpi) { fprintf (logfile, "only one instance allowed\n"); fflush (logfile); goto ende; }

	registerprg (hi);

	hwnd = CreateWindow ((char *)prgname, (const char *)prgname,
		WS_OVERLAPPEDWINDOW,
		//WS_POPUP,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		//0, 0, SX, SY,
		NULL, NULL, hi, NULL);

	if (hwnd==NULL) { fprintf (logfile, "couldn't open window\n"); fflush (logfile); goto ende; }

	ShowWindow (hwnd, SW_SHOWNORMAL);
	windowfocus = 1;

	hdc = GetDC (hwnd);
	if (hdc==NULL) goto ende;
	fprintf (logfile, "hdc\n"); fflush (logfile);

	RECT rect;

	SX = GetSystemMetrics (SM_CXSCREEN);
	SY = GetSystemMetrics (SM_CYSCREEN);

	GetClientRect (hwnd, &rect);

	if (rect.right==SX && rect.bottom==SY)
	{
		fprintf (logfile, "window is maximized\n"); fflush (logfile);
		maximized = 1;
		oldx = (int)(0.1*SX);
		oldy = (int)(0.1*SY);
		oldw = (int)(0.75*SX);
		oldh = (int)(0.75*SY);
	}
	else
	{
		fprintf (logfile, "window is not maximized\n"); fflush (logfile);
		GetWindowRect (hwnd, &rect);
		oldx = rect.left;
		oldy = rect.top;
		oldw = rect.right-rect.left;
		oldh = rect.bottom-rect.top;
	}

	GetClientRect (hwnd, &rect);
	SX = rect.right;
	SY = rect.bottom;

	fprintf (logfile, "window  sx %d  sy %d\n", SX, SY); fflush (logfile);


	PIXELFORMATDESCRIPTOR pfd, cpfd;
	int pixelformat;

	memset (&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	memset (&cpfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE;
	pfd.dwLayerMask = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.cAccumBits = 0;

	pixelformat = ChoosePixelFormat (hdc, &pfd);
	fprintf (logfile, "choose pixelformat %d\n", pixelformat); fflush (logfile);
	if (pixelformat==0) goto ende;

	//pixelformat = GetPixelFormat (hdc);
	DescribePixelFormat (hdc, pixelformat, sizeof(PIXELFORMATDESCRIPTOR), &cpfd);
	fprintf (logfile, "describe\n"); fflush (logfile);
	fprintf (logfile, "color bits %d\n", cpfd.cColorBits); fflush (logfile);
	fprintf (logfile, "depth bits %d\n", cpfd.cDepthBits); fflush (logfile);
	fprintf (logfile, "stencil bits %d\n", cpfd.cStencilBits); fflush (logfile);
	fprintf (logfile, "accum bits %d\n", cpfd.cAccumBits); fflush (logfile);
	fprintf (logfile, "pixel type %d\n", cpfd.iPixelType); fflush (logfile);
	fprintf (logfile, "flags %x\n", cpfd.dwFlags); fflush (logfile);

	//if (cpfd.cColorBits != pfd.cColorBits) goto ende;
	//if (cpfd.cDepthBits != pfd.cDepthBits) goto ende;
	//if (cpfd.iPixelType != pfd.iPixelType) goto ende;
	//if (cpfd.dwFlags != pfd.dwFlags) goto ende;

	if (! SetPixelFormat (hdc, pixelformat, &cpfd)) goto ende;
	fprintf (logfile, "set pixel format\n"); fflush (logfile);

	hglrc = wglCreateContext (hdc); if (hglrc==NULL) goto ende;
	if (! wglMakeCurrent (hdc, hglrc)) goto ende;

    if (GLEE_WGL_ARB_create_context)
    {
		fprintf (logfile, "create context supported\n"); fflush (logfile);
		nhglrc = wglCreateContextAttribsARB (hdc, 0, attribs);
	}
	else
	{
		fprintf (logfile, "create context not supported\n"); fflush (logfile);
		nhglrc = hglrc;
	}

	GLVersionString = glGetString (GL_VERSION);
	fprintf (logfile, "version string %s\n", GLVersionString); fflush (logfile);

	if (! nhglrc)
	{
		fprintf (logfile, "no nhglrc\n"); fflush (logfile);
		goto ende;
	}
	else
	{
		fprintf (logfile, "making new context current\n"); fflush (logfile);
		wglMakeCurrent (NULL, NULL);
		wglDeleteContext (hglrc);
		hglrc = 0;
		wglMakeCurrent (hdc, nhglrc);
		fprintf (logfile, "new context is current\n"); fflush (logfile);
	}

	SelectObject (hdc, GetStockObject (SYSTEM_FONT));
	wglUseFontBitmaps (hdc, 0, 255, 1000);

	//wglSwapIntervalEXT (1);		// vsync on
	wglSwapIntervalEXT (0);		// vsync off

	if (dims==1) ttd = GL_TEXTURE_1D;
	if (dims==2) ttd = GL_TEXTURE_2D;
	if (dims==3) ttd = GL_TEXTURE_3D;


	char ddir[16];

	if (dims==1) strcpy (ddir, "1D");
	if (dims==2) strcpy (ddir, "2D");
	if (dims==3) strcpy (ddir, "3D");

	if (setShaders (ddir, "copybufferrc", shader_copybufferrc)) goto ende;
	if (setShaders (ddir, "copybuffercr", shader_copybuffercr)) goto ende;
	if (setShaders (ddir, "fft", shader_fft)) goto ende;
	if (setShaders (ddir, "kernelmul", shader_kernelmul)) goto ende;
	if (setShaders (ddir, "snm", shader_snm)) goto ende;
	if (setShaders (ddir, "draw", shader_draw)) goto ende;

	loc_dim = glGetUniformLocation (shader_fft, "dim");
	loc_tang = glGetUniformLocation (shader_fft, "tang");
	loc_tangsc = glGetUniformLocation (shader_fft, "tangsc");

	loc_sc = glGetUniformLocation (shader_kernelmul, "sc");

	loc_mode = glGetUniformLocation (shader_snm, "mode");
	loc_dt = glGetUniformLocation (shader_snm, "dt");
	loc_b1 = glGetUniformLocation (shader_snm, "b1");
	loc_b2 = glGetUniformLocation (shader_snm, "b2");
	loc_d1 = glGetUniformLocation (shader_snm, "d1");
	loc_d2 = glGetUniformLocation (shader_snm, "d2");
	loc_sigmode = glGetUniformLocation (shader_snm, "sigmode");
	loc_sigtype = glGetUniformLocation (shader_snm, "sigtype");
	loc_mixtype = glGetUniformLocation (shader_snm, "mixtype");
	loc_sn = glGetUniformLocation (shader_snm, "sn");
	loc_sm = glGetUniformLocation (shader_snm, "sm");

	loc_colscheme = glGetUniformLocation (shader_draw, "colscheme");
	loc_phase = glGetUniformLocation (shader_draw, "phase");
	loc_visscheme = glGetUniformLocation (shader_draw, "visscheme");


	glActiveTexture (GL_TEXTURE0); glEnable (ttd);
	glActiveTexture (GL_TEXTURE1); glEnable (ttd);

	glClampColor (GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
	glClampColor (GL_CLAMP_VERTEX_COLOR, GL_FALSE);
	glClampColor (GL_CLAMP_READ_COLOR, GL_FALSE);

	glEnable (GL_DEPTH_TEST);
	glEnable (GL_CULL_FACE);

	savedispcnt = 0.0;
	timing = 1;
	colscheme = 1;
	visscheme = 2;
	anz = 1;
	pause = 0;

	if (dims==1)
	{
		NX = 1024; BX = 10;  // 262144, 18
		NY = 1; BY = 0;
		NZ = 1; BZ = 0;
	}
	else if (dims==2)
	{
		NX = 512; BX = 9;
		NY = 512; BY = 9;
		NZ = 1; BZ = 0;
	}
	else // dims==3
	{
		NX = 64; BX = 6;
		NY = 64; BY = 6;
		NZ = 64; BZ = 6;
	}
	fx = 0; fy = 0; fz = 0;
	bx = 1; by = 1;
	phase = 0.0; dphase = 0.0;
	wi = 0; wj = 0; wx = 0; wy = 0; dw = 0.0;

	nochmal:
	ox = 0; oy = 0; qx = NX; qy = NY;
	if (dims==1) qq = BX*16;
	if (dims==2) qq = BX*16;
	if (dims==3) qq = -NX*3/10;

	if (! create_buffers ()) goto ende;

	fft_planx ();
	if (dims>1) fft_plany ();
	if (dims>2) fft_planz ();
	makekernel (KR, KD);
	fft (KR, KRF, -1);
	fft (KD, KDF, -1);

	inita (AA);

	neu = false;
	for (;;)
	{
		if (timing)
		{
			glFlush ();
			glFinish ();
			QueryPerformanceCounter ((LARGE_INTEGER *)&tim);
		}

		while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) goto ende;
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}

		if (neu) goto nochmal;

		if (! pause)
		{
			if (anz==1)
			{
				drawa (AA);
				fft (AA, AF, -1);
				kernelmul (AF, KRF, ANF, sqrt(NX*NY*NZ)/kflr);
				kernelmul (AF, KDF, AMF, sqrt(NX*NY*NZ)/kfld);
				fft (ANF, AN, 1);
				fft (AMF, AM, 1);
				snm (AN, AM, AA);
			}
			else if (anz==2)
			{
				if (dims==2) makesnm (AN, AM, AA);
				drawa (AA);
			}
			else if (anz==3)
			{
				drawa (KD);
			}
			else if (anz==4)
			{
				drawa (KR);
			}
		}

		if (windowfocus)
		{
			if (GetAsyncKeyState(VK_LEFT )<0) fx -= 0.01;
			if (GetAsyncKeyState(VK_RIGHT)<0) fx += 0.01;
			if (GetAsyncKeyState(VK_DOWN )<0) fy -= 0.01;
			if (GetAsyncKeyState(VK_UP   )<0) fy += 0.01;
			if (GetAsyncKeyState(VK_PRIOR)<0) fz -= 0.01;
			if (GetAsyncKeyState(VK_NEXT )<0) fz += 0.01;

			if (GetAsyncKeyState(VK_F1)<0) dw += 0.05;
			if (GetAsyncKeyState(VK_F2)<0) dw -= 0.05;

			if (GetAsyncKeyState(VK_F3)<0) dphase += 0.005;
			if (GetAsyncKeyState(VK_F4)<0) dphase -= 0.005;

			if (GetAsyncKeyState('B')<0 ||
				GetAsyncKeyState('N')<0 ||
				GetAsyncKeyState(VK_SPACE)<0)
			{
				inita (AA);
			}
		}

		if (timing)
		{
			glFlush ();
			glFinish ();
			QueryPerformanceCounter ((LARGE_INTEGER *)&tima);

			glMatrixMode (GL_PROJECTION);
			glLoadIdentity ();
			glOrtho (0, SX, 0, SY, -1, 1);
			glViewport (0, 0, SX, SY);

			glBindFramebuffer (GL_FRAMEBUFFER, 0);
			glActiveTexture (GL_TEXTURE0); glDisable (ttd);
			glActiveTexture (GL_TEXTURE1); glDisable (ttd);

			char buf[256];
			int t;
			SIZE size;

			sprintf (buf, "%.3f  %d   ra=%.1f  rr=%.1f  rb=%.1f  dt=%.3f", (double)(tima-tim)/freq*85, (int)mode, ra, rr, rb, dt);
			t = strlen(buf);
			GetTextExtentPoint32 (hdc, buf, t, &size);

			glColor3d (0, 0, 0);
			glBegin (GL_QUADS);
			glVertex2i (10-5, 30);
			glVertex2i (10+size.cx+5, 30);
			glVertex2i (10+size.cx+5, 50);
			glVertex2i (10-5, 50);
			glEnd ();

			glColor3d (1, 1, 1);
			glRasterPos2i (10, 35);
			glListBase (1000);
			glCallLists (t, GL_UNSIGNED_BYTE, buf);

			sprintf (buf, "b1=%.3f  b2=%.3f  d1=%.3f  d2=%.3f   %d %d %d   sn=%.3f  sm=%.3f", b1, b2, d1, d2, (int)sigmode, (int)sigtype, (int)mixtype, sn, sm);
			t = strlen(buf);
			GetTextExtentPoint32 (hdc, buf, t, &size);

			if (savedispcnt>0.0) glColor3d (1, 1, 1);
			else glColor3d (0, 0, 0);
			glBegin (GL_QUADS);
			glVertex2i (10-5, 30-20);
			glVertex2i (10+size.cx+5, 30-20);
			glVertex2i (10+size.cx+5, 50-20);
			glVertex2i (10-5, 50-20);
			glEnd ();

			if (savedispcnt>0.0) glColor3d (0, 0, 0);
			else glColor3d (1, 1, 1);
			glRasterPos2i (10, 35-20);
			glListBase (1000);
			glCallLists (t, GL_UNSIGNED_BYTE, buf);

			glActiveTexture (GL_TEXTURE0); glEnable (ttd);
			glActiveTexture (GL_TEXTURE1); glEnable (ttd);

			if (savedispcnt>0.0)
			{
				savedispcnt -= (double)(tima-tim)/freq;
				if (savedispcnt<0.0) savedispcnt = 0.0;
			}
		}

		glBindFramebuffer (GL_FRAMEBUFFER, 0);	// for Taksi

		SwapBuffers (hdc);
	}

	ende:
	if (nhglrc) { wglMakeCurrent (NULL, NULL); wglDeleteContext (nhglrc); nhglrc = 0; }
	if (hglrc) { wglMakeCurrent (NULL, NULL); wglDeleteContext (hglrc); hglrc = 0; }
	if (hdc) { ReleaseDC (hwnd, hdc); hdc = 0; }
	if (hwnd) { DestroyWindow (hwnd); hwnd = 0; }
	if (logfile)
	{
		fprintf (logfile, "ending\n"); fflush (logfile);
		fclose (logfile);
	}
	return 0;
}
