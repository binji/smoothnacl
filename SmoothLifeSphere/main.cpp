/*
	SmoothLifeSphere

	ESC			Quit
	
	.			maximize to full screen / restore window
	,			resize window to client area size 640x480 for video recording
	v			show timing information and values
	m			save values
	b			fill buffer with random boxes
	c			hold
	-			grid
	x/y			color scheme
	5/6			sphere rotation speed
	0/1			time step mode

	q/a			increase/decrease b1
	w/s			increase/decrease b2
	e/d			increase/decrease d1
	r/f			increase/decrease d2

	t/g			sigmode
	z/h			sigtype
	u/j			mixtype
	i/k			sn
	o/l			sm

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

const double PI = 6.283185307;

typedef unsigned char ubyte;

ubyte prgname[] = "SmoothLifeSphere";
FILE *logfile=0;
HWND hwnd;
HDC hdc;
int maximized;
int SX, SY;
_int64 freq, tim, tima;


double ri, bi, ra, ba, Ra, flr, fld;
double b1, d1, b2, d2, sn, sm;

double mode, sigmode, sigtype, mixtype, colscheme;

const double FOV = 15;		// field of view
const double SR = 150;		// sphere radius (graphics)
const double SD = 1200;		// sphere distance
const int SN = 16;			// number of quads for one cube side is (SN*2)^2

int ox, oy;
int oldx, oldy, oldw, oldh;
double wx, wy, wi, wj, dw;
int windowfocus;

int timing, anz, grid;

const int K = 128;			// texture size (one cube side)

//const double R = pow(3/(2*PI),1.0/3.0)*K;		// sphere radius (internal) (volume equal)
//const double R = sqrt(3/PI)*K;		// sphere radius (internal) (area equal)
//const double R = 4*K/PI;			// sphere radius (internal) (circumference equal)
const double R = K/2;				// sphere radius (internal) (old way)

const int NB = 3;			// n buffers

unsigned int fb[NB];
unsigned int tb[NB];

GLuint shaderp, shaderc;

GLint loc_dx, loc_dy;
GLint loc_b1, loc_b2;
GLint loc_d1, loc_d2;
GLint loc_sn, loc_sm;
GLint loc_mode, loc_sigmode, loc_sigtype, loc_mixtype;
GLint loc_colscheme, loc_flr, loc_fld, loc_grid, loc_side;



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

	fscanf (file, "%d", &t); mode = t;

	fscanf (file, "%d", &t); sigmode = t;
	fscanf (file, "%d", &t); sigtype = t;
	fscanf (file, "%d", &t); mixtype = t;
	fscanf (file, "%lf", &d); ra = d;
	fscanf (file, "%lf", &d); b1 = d;
	fscanf (file, "%lf", &d); b2 = d;
	fscanf (file, "%lf", &d); d1 = d;
	fscanf (file, "%lf", &d); d2 = d;
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

	fprintf (file, "%d ", (int)mode);

	fprintf (file, "%d ", (int)sigmode);
	fprintf (file, "%d ", (int)sigtype);
	fprintf (file, "%d ", (int)mixtype);
	fprintf (file, "%.1f ", ra);
	fprintf (file, "%.3f ", b1);
	fprintf (file, "%.3f ", b2);
	fprintf (file, "%.3f ", d1);
	fprintf (file, "%.3f ", d2);
	fprintf (file, "%.3f ", sn);
	fprintf (file, "%.3f ", sm);
	fprintf (file, "\n");

	fclose (file);
	return true;
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


bool setShaders (char *fname, GLuint &prog)
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

	sprintf (filename, "%s.vert", fname);
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

	sprintf (filename, "%s.frag", fname);
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

	char w4[256], ins[256];
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

		ins[0] = '\0';
		if (strcmp (w4, "XX_ra")==0) sprintf (ins, "%f", ra);
		if (strcmp (w4, "XX_ri")==0) sprintf (ins, "%f", ri);
		if (strcmp (w4, "XX_ba")==0) sprintf (ins, "%f", ba);
		if (strcmp (w4, "XX_bi")==0) sprintf (ins, "%f", bi);
		if (strcmp (w4, "XX_Ra")==0) sprintf (ins, "%f", Ra);
		if (strcmp (w4, "XX_R" )==0) sprintf (ins, "%f", R );
		if (ins[0])
		{
			w = 0;
			while (ins[w]!='\0')
			{
				gs[d] = ins[w];
				d++; w++;
			}
		}
		else
		{
			w = 0;
			while (w4[w]!='\0')
			{
				gs[d] = w4[w];
				d++; w++;
			}
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


bool create_buffers (int k)
{
	unsigned int err, t;

	fprintf (logfile, "create buffers %d\n", k); fflush (logfile);


	// Texturebuffer

	glGenTextures (NB, &tb[0]);
	err = glGetError (); fprintf (logfile, "GenTextures err %d\n", err); fflush (logfile); if (err) return false;

	glBindTexture (GL_TEXTURE_2D, tb[0]);
	err = glGetError (); fprintf (logfile, "BindTexture err %d\n", err); fflush (logfile); if (err) return false;
	glTexImage2D (GL_TEXTURE_2D, 0, GL_R32F, 3*k*6, 3*k, 0, GL_RED, GL_FLOAT, NULL);
	err = glGetError (); fprintf (logfile, "TexImage err %d\n", err); fflush (logfile); if (err) return false;

	glBindTexture (GL_TEXTURE_2D, tb[1]);
	err = glGetError (); fprintf (logfile, "BindTexture err %d\n", err); fflush (logfile); if (err) return false;
	glTexImage2D (GL_TEXTURE_2D, 0, GL_R32F, 3*k*6, 3*k, 0, GL_RED, GL_FLOAT, NULL);
	err = glGetError (); fprintf (logfile, "TexImage err %d\n", err); fflush (logfile); if (err) return false;

	glBindTexture (GL_TEXTURE_2D, tb[2]);
	err = glGetError (); fprintf (logfile, "BindTexture err %d\n", err); fflush (logfile); if (err) return false;
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, 3*k*6, 3*k, 0, GL_RGBA, GL_FLOAT, NULL);
	err = glGetError (); fprintf (logfile, "TexImage err %d\n", err); fflush (logfile); if (err) return false;


	// Framebuffer

	glGenFramebuffers (NB, &fb[0]);
	err = glGetError (); fprintf (logfile, "GenFramebuffers err %d\n", err); fflush (logfile); if (err) return false;

	for (t=0; t<NB; t++)
	{
		glBindFramebuffer (GL_FRAMEBUFFER, fb[t]);
		err = glGetError (); fprintf (logfile, "BindFramebuffer err %d\n", err); fflush (logfile); if (err) return false;

		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tb[t], 0);
		err = glGetError (); fprintf (logfile, "FramebufferTexture2D err %d\n", err); fflush (logfile); if (err) return false;

		err = glCheckFramebufferStatus (GL_FRAMEBUFFER);
		fprintf (logfile, "FramebufferStatus 0x%x\n", err); fflush (logfile); if (err!=0x8cd5) return false;
	}

	return true;
}


bool delete_buffers (void)
{
	unsigned int err;

	fprintf (logfile, "delete buffers\n"); fflush (logfile);

	glBindTexture (GL_TEXTURE_2D, 0);
	err = glGetError (); fprintf (logfile, "BindTexture 0 err %d\n", err); fflush (logfile); if (err) return false;
	glDeleteTextures (NB, &tb[0]);
	err = glGetError (); fprintf (logfile, "DeleteTextures err %d\n", err); fflush (logfile); if (err) return false;

	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	err = glGetError (); fprintf (logfile, "BindFramebuffer 0 err %d\n", err); fflush (logfile); if (err) return false;
	glDeleteFramebuffers (NB, &fb[0]);
	err = glGetError (); fprintf (logfile, "DeleteFramebuffers err %d\n", err); fflush (logfile); if (err) return false;

	return true;
}


struct vector
{
	double x;
	double y;
	double z;
};

inline vector vec (double x, double y, double z)
{
	vector w;
	w.x = x;
	w.y = y;
	w.z = z;
	return w;
}

inline vector plus (vector a, vector b)
{
	vector w;
	w.x = a.x + b.x;
	w.y = a.y + b.y;
	w.z = a.z + b.z;
	return w;
}

inline vector minus (vector a, vector b)
{
	vector w;
	w.x = a.x - b.x;
	w.y = a.y - b.y;
	w.z = a.z - b.z;
	return w;
}

inline vector mal (double a, vector v)
{
	vector w;
	w.x = a * v.x;
	w.y = a * v.y;
	w.z = a * v.z;
	return w;
}

inline double skalar (vector a, vector b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline vector kreuz (vector a, vector b)
{
	vector w;
	w.x = a.y*b.z - a.z*b.y;
	w.y = a.z*b.x - a.x*b.z;
	w.z = a.x*b.y - a.y*b.x;
	return w;
}

inline double betrag (vector v)
{
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline vector normiert (vector v)
{
	vector w;
	double b;
	b = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	w.x = v.x/b;
	w.y = v.y/b;
	w.z = v.z/b;
	return w;
}

inline double winkel (vector a, vector b)
{
	return acos((a.x*b.x+a.y*b.y+a.z*b.z)/sqrt((a.x*a.x+a.y*a.y+a.z*a.z)*(b.x*b.x+b.y*b.y+b.z*b.z)));
}


double fldr (vector aa, vector bb, vector cc)
{
	double a, b, c, al, be, ga;
	a = winkel (aa, bb);
	b = winkel (bb, cc);
	c = winkel (cc, aa);
	al = acos ( (cos(a) - cos(b)*cos(c)) / (sin(b)*sin(c)) );
	be = acos ( (cos(b) - cos(c)*cos(a)) / (sin(c)*sin(a)) );
	ga = acos ( (cos(c) - cos(a)*cos(b)) / (sin(a)*sin(b)) );
	return al + be + ga - PI/2;
}


void machcoswinkelpre (int buf, int RR)
{
	vector v1, v2, v3, v4;
	vector r, u, v, p;
	int t, x, y;
	float *coswinkelpre;

	glViewport (0, 0, 3*K*6, 3*K);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, 3*K*6, 0, 3*K, -1, 1);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glBindFramebuffer (GL_FRAMEBUFFER, fb[buf]);

	glClearColor (0,0,0,0);
	glClear (GL_COLOR_BUFFER_BIT);

	coswinkelpre = (float*)calloc (4*2*RR*2*RR, sizeof(float));

	for (t=0; t<6; t++)
	{
		if (t==0) { r = vec( 1, 0, 0); u = vec( 0, 1, 0); v = vec( 0, 0, 1); }
		if (t==1) { r = vec( 0, 1, 0); u = vec(-1, 0, 0); v = vec( 0, 0, 1); }
		if (t==2) { r = vec(-1, 0, 0); u = vec( 0,-1, 0); v = vec( 0, 0, 1); }
		if (t==3) { r = vec( 0,-1, 0); u = vec( 1, 0, 0); v = vec( 0, 0, 1); }
		if (t==4) { r = vec( 0, 0, 1); u = vec( 1, 0, 0); v = vec( 0, 1, 0); }
		if (t==5) { r = vec( 0, 0,-1); u = vec( 1, 0, 0); v = vec( 0,-1, 0); }

		for (y=0; y<2*RR; y++)
		for (x=0; x<2*RR; x++)
		{
			/*
			v1 = vec (x-RR  , y-RR  , RR);
			v2 = vec (x-RR+1, y-RR  , RR);
			v3 = vec (x-RR+1, y-RR+1, RR);
			v4 = vec (x-RR  , y-RR+1, RR);
			*/

			v1 = vec (tan((double)(x-RR  )/RR*PI/8)*RR, tan((double)(y-RR  )/RR*PI/8)*RR, RR);
			v2 = vec (tan((double)(x-RR+1)/RR*PI/8)*RR, tan((double)(y-RR  )/RR*PI/8)*RR, RR);
			v3 = vec (tan((double)(x-RR+1)/RR*PI/8)*RR, tan((double)(y-RR+1)/RR*PI/8)*RR, RR);
			v4 = vec (tan((double)(x-RR  )/RR*PI/8)*RR, tan((double)(y-RR+1)/RR*PI/8)*RR, RR);
			
			//p = normiert (plus (r, plus (mal ((x-RR+0.5)/RR,u), mal ((y-RR+0.5)/RR,v))));
			p = normiert (plus (r, plus (mal (tan((x-RR+0.5)/RR*PI/8),u), mal (tan((y-RR+0.5)/RR*PI/8),v))));
			
			coswinkelpre[4*(y*2*RR+x)+0] = (float)(p.x);
			coswinkelpre[4*(y*2*RR+x)+1] = (float)(p.y);
			coswinkelpre[4*(y*2*RR+x)+2] = (float)(p.z);
			coswinkelpre[4*(y*2*RR+x)+3] = (float)(R*R*(fldr (v1, v2, v3) + fldr (v1, v3, v4)));
		}

		glBindTexture (GL_TEXTURE_2D, tb[buf]);
		glTexSubImage2D (GL_TEXTURE_2D, 0, t*3*K+K, K, K, K, GL_RGBA, GL_FLOAT, coswinkelpre);
	}

	if (coswinkelpre) free (coswinkelpre);
}


void drawsphere (double x1, double y1, double z1)
{
	int t;
	double x, y;
	vector r, u, v, s1, s2, s3, s4, d;

	d = vec (x1, y1, z1);
	for (t=0; t<6; t++)
	{
		if (t==0) { r = vec( 1, 0, 0); u = vec( 0, 1, 0); v = vec( 0, 0, 1); }
		if (t==1) { r = vec( 0, 1, 0); u = vec(-1, 0, 0); v = vec( 0, 0, 1); }
		if (t==2) { r = vec(-1, 0, 0); u = vec( 0,-1, 0); v = vec( 0, 0, 1); }
		if (t==3) { r = vec( 0,-1, 0); u = vec( 1, 0, 0); v = vec( 0, 0, 1); }
		if (t==4) { r = vec( 0, 0, 1); u = vec( 1, 0, 0); v = vec( 0, 1, 0); }
		if (t==5) { r = vec( 0, 0,-1); u = vec( 1, 0, 0); v = vec( 0,-1, 0); }
		
		for (y=-SN; y<SN; y++)
		for (x=-SN; x<SN; x++)
		{
			//if (! (((int)x)%4==0 || ((int)y)%4==0)) continue;

			s1 = mal (SR, normiert (plus (r, plus (mal (tan((x  )/SN*PI/8), u), mal (tan((y  )/SN*PI/8), v)))));
			s2 = mal (SR, normiert (plus (r, plus (mal (tan((x+1)/SN*PI/8), u), mal (tan((y  )/SN*PI/8), v)))));
			s3 = mal (SR, normiert (plus (r, plus (mal (tan((x+1)/SN*PI/8), u), mal (tan((y+1)/SN*PI/8), v)))));
			s4 = mal (SR, normiert (plus (r, plus (mal (tan((x  )/SN*PI/8), u), mal (tan((y+1)/SN*PI/8), v)))));
			
			/*
			s1 = mal (SR, normiert (plus (r, plus (mal ((x  )/SN, u), mal ((y  )/SN, v)))));
			s2 = mal (SR, normiert (plus (r, plus (mal ((x+1)/SN, u), mal ((y  )/SN, v)))));
			s3 = mal (SR, normiert (plus (r, plus (mal ((x+1)/SN, u), mal ((y+1)/SN, v)))));
			s4 = mal (SR, normiert (plus (r, plus (mal ((x  )/SN, u), mal ((y+1)/SN, v)))));
			*/
			s1 = plus (s1, d);
			s2 = plus (s2, d);
			s3 = plus (s3, d);
			s4 = plus (s4, d);

			glBegin (GL_QUADS);
			/*
			glMultiTexCoord2d (GL_TEXTURE0, t + (tan((x  )/SN*PI/8)*0.5+0.5)/3.0+1.0/3.0, (tan((y  )/SN*PI/8)*0.5+0.5)/3.0+1.0/3.0); glVertex3d (s1.x, s1.y, s1.z);
			glMultiTexCoord2d (GL_TEXTURE0, t + (tan((x+1)/SN*PI/8)*0.5+0.5)/3.0+1.0/3.0, (tan((y  )/SN*PI/8)*0.5+0.5)/3.0+1.0/3.0); glVertex3d (s2.x, s2.y, s2.z);
			glMultiTexCoord2d (GL_TEXTURE0, t + (tan((x+1)/SN*PI/8)*0.5+0.5)/3.0+1.0/3.0, (tan((y+1)/SN*PI/8)*0.5+0.5)/3.0+1.0/3.0); glVertex3d (s3.x, s3.y, s3.z);
			glMultiTexCoord2d (GL_TEXTURE0, t + (tan((x  )/SN*PI/8)*0.5+0.5)/3.0+1.0/3.0, (tan((y+1)/SN*PI/8)*0.5+0.5)/3.0+1.0/3.0); glVertex3d (s4.x, s4.y, s4.z);
			*/

			glMultiTexCoord2d (GL_TEXTURE0, t + ((x  )/SN*0.5+0.5)/3.0+1.0/3.0, ((y  )/SN*0.5+0.5)/3.0+1.0/3.0); glVertex3d (s1.x, s1.y, s1.z);
			glMultiTexCoord2d (GL_TEXTURE0, t + ((x+1)/SN*0.5+0.5)/3.0+1.0/3.0, ((y  )/SN*0.5+0.5)/3.0+1.0/3.0); glVertex3d (s2.x, s2.y, s2.z);
			glMultiTexCoord2d (GL_TEXTURE0, t + ((x+1)/SN*0.5+0.5)/3.0+1.0/3.0, ((y+1)/SN*0.5+0.5)/3.0+1.0/3.0); glVertex3d (s3.x, s3.y, s3.z);
			glMultiTexCoord2d (GL_TEXTURE0, t + ((x  )/SN*0.5+0.5)/3.0+1.0/3.0, ((y+1)/SN*0.5+0.5)/3.0+1.0/3.0); glVertex3d (s4.x, s4.y, s4.z);

			glEnd ();
		}
	}

}


void update_viewport (void)
{
	glViewport (0, 0, SX, SY);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, SX, 0, SY, -1, 1);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}


void timestep (void)
{
	int t;
	double dx, dy;

	glViewport (0, 0, 3*K*6, 3*K);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, 3*K*6, 0, 3*K, -1, 1);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glBindFramebuffer (GL_FRAMEBUFFER, fb[0]);
	
	glUseProgram (shaderp);

	dx = 1.0/(3*K*6);
	dy = 1.0/(3*K);

	glUniform1f (loc_mode, (float)mode);
	glUniform1f (loc_sigmode, (float)sigmode);
	glUniform1f (loc_sigtype, (float)sigtype);
	glUniform1f (loc_mixtype, (float)mixtype);
	glUniform1f (loc_dx, (float)dx);
	glUniform1f (loc_dy, (float)dy);
	glUniform1f (loc_b1, (float)b1);
	glUniform1f (loc_b2, (float)b2);
	glUniform1f (loc_d1, (float)d1);
	glUniform1f (loc_d2, (float)d2);
	glUniform1f (loc_sn, (float)sn);
	glUniform1f (loc_sm, (float)sm);
	glUniform1f (loc_flr, (float)flr);
	glUniform1f (loc_fld, (float)fld);
	
	glActiveTexture (GL_TEXTURE0); glEnable (GL_TEXTURE_2D);
	glActiveTexture (GL_TEXTURE1); glEnable (GL_TEXTURE_2D);

	glActiveTexture (GL_TEXTURE0); glBindTexture (GL_TEXTURE_2D, tb[1]);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glUniform1i (glGetUniformLocation (shaderp, "tex0"), 0);

	glActiveTexture (GL_TEXTURE1); glBindTexture (GL_TEXTURE_2D, tb[2]);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glUniform1i (glGetUniformLocation (shaderp, "tex1"), 1);

	for (t=0; t<6; t++)
	{
		glBegin (GL_QUADS);
		glMultiTexCoord2d (GL_TEXTURE0, (double)(t*3*K+K  )/(3*K*6), (double)(K  )/(3*K)); glMultiTexCoord2d (GL_TEXTURE1, (double)(t*3*K+K  )/(3*K*6), (double)(K  )/(3*K)); glVertex2i (t*3*K+K  , K  );
		glMultiTexCoord2d (GL_TEXTURE0, (double)(t*3*K+K+K)/(3*K*6), (double)(K  )/(3*K)); glMultiTexCoord2d (GL_TEXTURE1, (double)(t*3*K+K+K)/(3*K*6), (double)(K  )/(3*K)); glVertex2i (t*3*K+K+K, K  );
		glMultiTexCoord2d (GL_TEXTURE0, (double)(t*3*K+K+K)/(3*K*6), (double)(K+K)/(3*K)); glMultiTexCoord2d (GL_TEXTURE1, (double)(t*3*K+K+K)/(3*K*6), (double)(K+K)/(3*K)); glVertex2i (t*3*K+K+K, K+K);
		glMultiTexCoord2d (GL_TEXTURE0, (double)(t*3*K+K  )/(3*K*6), (double)(K+K)/(3*K)); glMultiTexCoord2d (GL_TEXTURE1, (double)(t*3*K+K  )/(3*K*6), (double)(K+K)/(3*K)); glVertex2i (t*3*K+K  , K+K);
		glEnd ();
	}

	glUseProgram (0);

	glActiveTexture (GL_TEXTURE0); glDisable (GL_TEXTURE_2D);
	glActiveTexture (GL_TEXTURE1); glDisable (GL_TEXTURE_2D);
}


void initaa (int buf)
{
	int t, qx, qy, r;
	GLUquadric *nq;

	glViewport (0, 0, 3*K*6, 3*K);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, 3*K*6, 0, 3*K, -1, 1);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glBindFramebuffer (GL_FRAMEBUFFER, fb[buf]);

	nq = gluNewQuadric ();
	
	for (t=0; t<1000; t++)
	{
		qx = rand()%K; qy = rand()%K;
		qx += (rand()%6)*3*K+K;
		qy += K;
		r = rand()%10+5;

		glColor3d (rand()%2, 0, 0);
		//glColor3d (RND(1), 0, 0);

		glPushMatrix ();
		glTranslatef ((float)qx, (float)qy, 0);
		gluDisk (nq, 0, r/2, 32, 2);
		glPopMatrix ();

		/*
		glBegin (GL_QUADS);
		glVertex2i (qx  , qy  );
		glVertex2i (qx+r, qy  );
		glVertex2i (qx+r, qy+r);
		glVertex2i (qx  , qy+r);
		glEnd ();
		*/
	}
	
	gluDeleteQuadric (nq);
	
	/*
	for (int t=0; t<6; t++)
	for (int x=0; x<K; x+=4)
	for (int y=0; y<K; y+=4)
	{
		int qx, qy, gx, gy;

		qx = x; qy = y;
		qx += t*3*K+K;
		qy += K;
		gx = 4; gy = 4;

		if ((x/4)%2==0 && (y/4)%2==0) glColor3d (0, 0, 0);
		else glColor3d (1, 0, 0);
		glBegin (GL_QUADS);
		glVertex2i (qx   , qy   );
		glVertex2i (qx+gx, qy   );
		glVertex2i (qx+gx, qy+gy);
		glVertex2i (qx   , qy+gy);
		glEnd ();
	}
	*/
}


int cs[6*4][2][1+4*2] =
{
	0, 1,1, 2,1, 2,2, 1,2,  1, 0,1, 1,1, 1,2, 0,2,
	0, 1,1, 2,1, 2,2, 1,2,  3, 2,1, 3,1, 3,2, 2,2,
	0, 1,1, 2,1, 2,2, 1,2,  4, 3,1, 3,2, 2,2, 2,1,
	0, 1,1, 2,1, 2,2, 1,2,  5, 2,2, 2,1, 3,1, 3,2,

	1, 1,1, 2,1, 2,2, 1,2,  0, 2,1, 3,1, 3,2, 2,2,
	1, 1,1, 2,1, 2,2, 1,2,  2, 0,1, 1,1, 1,2, 0,2,
	1, 1,1, 2,1, 2,2, 1,2,  4, 2,3, 1,3, 1,2, 2,2,
	1, 1,1, 2,1, 2,2, 1,2,  5, 2,1, 1,1, 1,0, 2,0,

	2, 1,1, 2,1, 2,2, 1,2,  1, 2,1, 3,1, 3,2, 2,2,
	2, 1,1, 2,1, 2,2, 1,2,  3, 0,1, 1,1, 1,2, 0,2,
	2, 1,1, 2,1, 2,2, 1,2,  4, 0,2, 0,1, 1,1, 1,2,
	2, 1,1, 2,1, 2,2, 1,2,  5, 1,1, 1,2, 0,2, 0,1,

	3, 1,1, 2,1, 2,2, 1,2,  0, 0,1, 1,1, 1,2, 0,2,
	3, 1,1, 2,1, 2,2, 1,2,  2, 2,1, 3,1, 3,2, 2,2,
	3, 1,1, 2,1, 2,2, 1,2,  4, 1,0, 2,0, 2,1, 1,1,
	3, 1,1, 2,1, 2,2, 1,2,  5, 1,2, 2,2, 2,3, 1,3,

	4, 1,1, 2,1, 2,2, 1,2,  0, 1,3, 1,2, 2,2, 2,3,
	4, 1,1, 2,1, 2,2, 1,2,  1, 2,3, 1,3, 1,2, 2,2,
	4, 1,1, 2,1, 2,2, 1,2,  2, 2,2, 2,3, 1,3, 1,2,
	4, 1,1, 2,1, 2,2, 1,2,  3, 1,2, 2,2, 2,3, 1,3,

	5, 1,1, 2,1, 2,2, 1,2,  0, 2,0, 2,1, 1,1, 1,0,
	5, 1,1, 2,1, 2,2, 1,2,  1, 2,1, 1,1, 1,0, 2,0,
	5, 1,1, 2,1, 2,2, 1,2,  2, 1,1, 1,0, 2,0, 2,1,
	5, 1,1, 2,1, 2,2, 1,2,  3, 1,0, 2,0, 2,1, 1,1
};


void copysides (int buf)
{
	int t;

	glViewport (0, 0, 3*K*6, 3*K);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, 3*K*6, 0, 3*K, -1, 1);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glBindFramebuffer (GL_FRAMEBUFFER, fb[buf]);

	glActiveTexture (GL_TEXTURE0);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, tb[buf]);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	for (t=0; t<6*4; t++)
	{
		int s0,ax0,ay0,bx0,by0,cx0,cy0,dx0,dy0;
		int s1,ax1,ay1,bx1,by1,cx1,cy1,dx1,dy1;

		s0 = cs[t][0][0]*3*K;
		ax0 = cs[t][0][1]*K; ay0 = cs[t][0][2]*K;
		bx0 = cs[t][0][3]*K; by0 = cs[t][0][4]*K;
		cx0 = cs[t][0][5]*K; cy0 = cs[t][0][6]*K;
		dx0 = cs[t][0][7]*K; dy0 = cs[t][0][8]*K;

		s1 = cs[t][1][0]*3*K;
		ax1 = cs[t][1][1]*K; ay1 = cs[t][1][2]*K;
		bx1 = cs[t][1][3]*K; by1 = cs[t][1][4]*K;
		cx1 = cs[t][1][5]*K; cy1 = cs[t][1][6]*K;
		dx1 = cs[t][1][7]*K; dy1 = cs[t][1][8]*K;

		glBegin (GL_QUADS);
		glTexCoord2d ((double)(s0+ax0)/(3*K*6),(double)(ay0)/(3*K)); glVertex2i (s1+ax1,ay1);
		glTexCoord2d ((double)(s0+bx0)/(3*K*6),(double)(by0)/(3*K)); glVertex2i (s1+bx1,by1);
		glTexCoord2d ((double)(s0+cx0)/(3*K*6),(double)(cy0)/(3*K)); glVertex2i (s1+cx1,cy1);
		glTexCoord2d ((double)(s0+dx0)/(3*K*6),(double)(dy0)/(3*K)); glVertex2i (s1+dx1,dy1);
		glEnd ();
	}

	glActiveTexture (GL_TEXTURE0); glDisable (GL_TEXTURE_2D);
}


void drawaa (int buf)
{
	glBindFramebuffer (GL_FRAMEBUFFER, 0);

	glActiveTexture (GL_TEXTURE0);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, tb[buf]);
	//glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glUseProgram (shaderc);
	glUniform1f (loc_colscheme, (float)colscheme);
	glUniform1i (loc_grid, grid);

	/*
	glViewport (0, 0, SX, SY);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, SX, 0, SY, -1, 1);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glClearColor (0.5, 0.5, 0.5, 1.0);
	glClearDepth (1.0);
	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable (GL_DEPTH_TEST);

	glUniform1i (loc_side, 1);
	glBegin (GL_QUADS);
	glTexCoord2d (0,0); glVertex2i ( 0,100+   0);
	glTexCoord2d (6,0); glVertex2i (SX,100+   0);
	glTexCoord2d (6,1); glVertex2i (SX,100+SX/6);
	glTexCoord2d (0,1); glVertex2i ( 0,100+SX/6);
	glEnd ();

	glUseProgram (0);

	glActiveTexture (GL_TEXTURE0); glDisable (GL_TEXTURE_2D);

	return;
	*/

	glViewport (0, 0, SX, SY);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (FOV, (double)SX/SY, SD-(SR+1), SD+(SR+1));
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	
	glClearColor (0.5, 0.5, 0.5, 1.0);
	glClearDepth (1.0);
	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glPushMatrix ();
	glTranslated (0, 0, -SD);
	glRotated (-(wj+wy), 1, 0, 0);
	glRotated (-(wi+wx), 0, 1, 0);

	glEnable (GL_DEPTH_TEST);
	glEnable (GL_CULL_FACE);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	
	glUniform1i (loc_side, 0);
	glCullFace (GL_FRONT);
	drawsphere (0,0,0);

	glUniform1i (loc_side, 1);
	glCullFace (GL_BACK);
	drawsphere (0,0,0);

	glDisable (GL_DEPTH_TEST);
	glDisable (GL_CULL_FACE);
	glDisable (GL_BLEND);
	//glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

	glPopMatrix ();
	
	wi += dw;

	glUseProgram (0);

	glActiveTexture (GL_TEXTURE0); glDisable (GL_TEXTURE_2D);
}



#define WM_MOUSEWHEEL  0x020A

LRESULT CALLBACK PrgWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int xp, yp;
	static int lbutton=0, rbutton=0;
	static int oxp, oyp;
	static int oox, ooy;
	const double dp = 0.001;


	switch (message)
	{

	case WM_CHAR:
	if (wParam == 27) PostQuitMessage (0);

	if (wParam=='v') timing ^= 1;

	if (wParam=='-') grid ^= 1;

	if (wParam=='c') anz ^= 1;

	if (wParam=='5') dw += 1;
	if (wParam=='6') dw -= 1;

	if (wParam=='0') mode = 0;
	if (wParam=='1') mode = 1;

	// 'b' in main loop

	if (wParam=='x') { colscheme++; if (colscheme>7) colscheme=1; }
	if (wParam=='y') { colscheme--; if (colscheme<1) colscheme=7; }
	
	if (wParam=='t') { sigmode++; if (sigmode>4) sigmode=1; }
	if (wParam=='g') { sigmode--; if (sigmode<1) sigmode=4; }

	if (wParam=='z') { sigtype++; if (sigtype>9) sigtype=0; }
	if (wParam=='h') { sigtype--; if (sigtype<0) sigtype=9; }

	if (wParam=='u') { mixtype++; if (mixtype>7) mixtype=0; }
	if (wParam=='j') { mixtype--; if (mixtype<0) mixtype=7; }

	if (wParam=='q') b1 += dp;
	if (wParam=='a') b1 -= dp;
	if (wParam=='w') b2 += dp;
	if (wParam=='s') b2 -= dp;

	if (wParam=='e') d1 += dp;
	if (wParam=='d') d1 -= dp;
	if (wParam=='r') d2 += dp;
	if (wParam=='f') d2 -= dp;

	if (wParam=='i') sn += dp;
	if (wParam=='k') sn -= dp;

	if (wParam=='o') sm += dp;
	if (wParam=='l') sm -= dp;

	if (wParam=='Q') b1 += 10*dp;
	if (wParam=='A') b1 -= 10*dp;
	if (wParam=='W') b2 += 10*dp;
	if (wParam=='S') b2 -= 10*dp;

	if (wParam=='E') d1 += 10*dp;
	if (wParam=='D') d1 -= 10*dp;
	if (wParam=='R') d2 += 10*dp;
	if (wParam=='F') d2 -= 10*dp;

	if (wParam=='I') sn += 10*dp;
	if (wParam=='K') sn -= 10*dp;

	if (wParam=='O') sm += 10*dp;
	if (wParam=='L') sm -= 10*dp;

	if (wParam=='m')
	{
		write_config ();
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
			update_viewport ();
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
			SetWindowPos (hWnd, HWND_TOP, 100-8, 100-30, 640+16, 480+38, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
			RECT rect;
			GetClientRect (hWnd, &rect);
			SX = rect.right;
			SY = rect.bottom;
			update_viewport ();
			maximized = 0;
			//fprintf (logfile, "restored to %d %d\n", SX, SY); fflush (logfile);
		}
	}
	break;


	case WM_LBUTTONDOWN:
	oxp = LOWORD(lParam);
	oyp = HIWORD(lParam);
	lbutton=1;
	oox = ox;
	ooy = oy;
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
	}
	else
	{
	}
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
		update_viewport ();
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

		update_viewport ();
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
	char buf[128];


	QueryPerformanceFrequency ((LARGE_INTEGER *)&freq);

	srand ((unsigned)time (0));

	_fmode = O_TEXT;
	logfile = fopen ("SmoothLifeLog.txt", "w");
	if (logfile==0) goto ende;
	fprintf (logfile, "starting SmoothLife\n"); fflush (logfile);

	if (! read_config ()) { fprintf (logfile, "couldn't read config file\n"); fflush (logfile); goto ende; }
	
	fprintf (logfile, "%d %d %d %d %f %f %f %f %f %f %f\n", (int)mode, (int)sigmode, (int)sigtype, (int)mixtype, ra, b1, b2, d1, d2, sn, sm); fflush (logfile);

	hwnd = 0;
	hdc = 0;
	hglrc = 0;
	nhglrc = 0;

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

	ri = ra/3;
	bi = 1.0; ba = 1.0;
	Ra = ceil(ra*2);
	//Ra = ceil(ra+ba/2);

	fprintf (logfile, "R = %f  K = %d  C %f %d  A %f %d  V %f %d\n", R, K, PI*R, 4*K, 2*PI*R*R, 6*K*K, 2*PI/3*R*R*R, K*K*K);

	fprintf (logfile, "ri = %f  ra = %f\n", ri, ra);

	fld = 0.5*PI*ri*ri;
	flr = 0.5*PI*ra*ra - fld;
	fprintf (logfile, "areas flat:  disk %f  ring %f\n", fld, flr);

	ri = R*acos(1-0.5*ri*ri/(R*R));
	ra = R*acos(1-0.5*ra*ra/(R*R));

	fprintf (logfile, "ri = %f  ra = %f\n", ri, ra);

	fld = PI*R*R*(1-cos(ri/R));
	flr = PI*R*R*(1-cos(ra/R)) - fld;
	
	fprintf (logfile, "areas sphere:  disk %f  ring %f\n", fld, flr);


	if (! create_buffers (K)) goto ende;

	if (setShaders ("program", shaderp)) goto ende;
	if (setShaders ("color", shaderc)) goto ende;

	loc_b1 = glGetUniformLocation (shaderp, "b1");
	loc_b2 = glGetUniformLocation (shaderp, "b2");
	loc_d1 = glGetUniformLocation (shaderp, "d1");
	loc_d2 = glGetUniformLocation (shaderp, "d2");
	loc_sn = glGetUniformLocation (shaderp, "sn");
	loc_sm = glGetUniformLocation (shaderp, "sm");

	loc_mode = glGetUniformLocation (shaderp, "mode");
	loc_sigmode = glGetUniformLocation (shaderp, "sigmode");
	loc_sigtype = glGetUniformLocation (shaderp, "sigtype");
	loc_mixtype = glGetUniformLocation (shaderp, "mixtype");
	loc_dx = glGetUniformLocation (shaderp, "dx");
	loc_dy = glGetUniformLocation (shaderp, "dy");
	loc_flr = glGetUniformLocation (shaderp, "flr");
	loc_fld = glGetUniformLocation (shaderp, "fld");

	loc_colscheme = glGetUniformLocation (shaderc, "colscheme");
	loc_grid = glGetUniformLocation (shaderc, "grid");
	loc_side = glGetUniformLocation (shaderc, "side");

	glClampColor (GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
	glClampColor (GL_CLAMP_VERTEX_COLOR, GL_FALSE);
	glClampColor (GL_CLAMP_READ_COLOR, GL_FALSE);

	wx = 0; wy = 0; wi = 0; wj = 0; ox = 0; oy = 0; dw = 0;
	anz = 1;
	timing = 1;
	colscheme = 1;
	grid = 1;

	machcoswinkelpre (2, K/2);
	copysides (2);

	initaa (1);

	for (;;)
	{
		while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) goto ende;
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}

		if (timing)
		{
			glFlush ();
			glFinish ();
			QueryPerformanceCounter ((LARGE_INTEGER *)&tim);
		}

		if (windowfocus && GetAsyncKeyState('B')<0) initaa (1);

		copysides (1);

		drawaa (1);

		if (anz)
		{
			timestep ();
			glBindFramebuffer (GL_READ_FRAMEBUFFER, fb[0]);
			glBindFramebuffer (GL_DRAW_FRAMEBUFFER, fb[1]);
			glBlitFramebuffer (0,0,3*K*6,3*K, 0,0,3*K*6,3*K, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		if (timing)
		{
			glFlush ();
			glFinish ();
			QueryPerformanceCounter ((LARGE_INTEGER *)&tima);

			glBindFramebuffer (GL_FRAMEBUFFER, 0);
			update_viewport ();

			sprintf (buf, "%.3f  %d  %d %d %d  r=%.1f   b1=%.3f  b2=%.3f  d1=%.3f  d2=%.3f   sn=%.3f  sm=%.3f", (double)(tima-tim)/freq*85, (int)mode, (int)sigmode, (int)sigtype, (int)mixtype, ra, b1, b2, d1, d2, sn, sm);

			int t = strlen(buf);
			glColor3d (0, 0, 0);
			glBegin (GL_QUADS);
			glVertex2i (5, 35);
			glVertex2i (5+8*t, 35);
			glVertex2i (5+8*t, 55);
			glVertex2i (5, 55);
			glEnd ();

			glColor3d (1, 1, 1);
			glRasterPos2i (10, 40);
			glListBase (1000);
			glCallLists (strlen(buf), GL_UNSIGNED_BYTE, buf);
		}

		glBindFramebuffer (GL_FRAMEBUFFER, 0);
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
