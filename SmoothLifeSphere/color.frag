// SmoothLifeSphere

const float pi = 6.283185307;

uniform float colscheme;
uniform int grid;
uniform int side;

uniform sampler2D tex0;

vec3 rainbow (float f)
{
	float r, g, b;

	     if (f>=0.0 && f<1.0) { r=1.0;   g=f-0.0; b=0.0;   }
	else if (f>=1.0 && f<2.0) { r=2.0-f; g=1.0;   b=0.0;   }
	else if (f>=2.0 && f<3.0) { r=0.0;   g=1.0;   b=f-2.0; }
	else if (f>=3.0 && f<4.0) {	r=0.0;   g=4.0-f; b=1.0;   }
	else if (f>=4.0 && f<5.0) {	r=f-4.0; g=0.0;   b=1.0;   }
	else if (f>=5.0 && f<6.0) { r=1.0;   g=0.0;   b=6.0-f; }
	else { r=0.0; g=0.0; b=0.0; }

	return vec3(r,g,b);
}

vec3 rainbowph (float f, float ph)
{
	float r, g, b;

	r = 0.5*sin(1.7*cos(pi*(f+0.0/3.0+ph)))+0.5;
	g = 0.5*sin(1.7*cos(pi*(f+1.0/3.0+ph)))+0.5;
	b = 0.5*sin(1.7*cos(pi*(f+2.0/3.0+ph)))+0.5;

	return vec3(r,g,b);
}


void main()
{
	vec2 t;
	vec3 c;
	float x, y, fx, fy, fc, f;

	t = gl_TexCoord[0].xy;

	x = (fract(t.x)-1.0/3.0)*3.0;
	y = (      t.y -1.0/3.0)*3.0;

	fc = 0.0;
	if (grid>0)
	{
		fx = fract (x*10.0);
		fy = fract (y*10.0);
		if (fx<0.1 || fx>0.9 || fy<0.1 || fy>0.9) fc = 0.25;
	}

	f = texture2D (tex0, vec2(t.x/6.0, t.y)).r;

	c = vec3(0,0,0);
	if (colscheme==1.0)		// red/blue on black
	{
		//if (side==0) c = vec3(0,0,f);
		//else         c = vec3(f,0,0);
		if (side==0) c = vec3(0,0,1);
		else         c = vec3(1,0,0);
	}
	else if (colscheme==2.0) c = vec3(f,f,f);		// white on black
	else if (colscheme==3.0) c = vec3(1.0-f,1.0-f,1.0-f);		// black on white
	else if (colscheme==4.0) c = rainbow (6.0*sqrt(sqrt(1.0-f)))*sqrt(sqrt(f));		// rainbow
	else if (colscheme==5.0) c = mix (vec3(.5,.3,.0), vec3(.5,.75,.1), f);		// brown/green
	else if (colscheme==6.0) c = mix (vec3(.5,.3,.0), mix (vec3(1.0,.95,.0), vec3(.3,.2,.0), f), sqrt(sqrt(f)));		// gold and brown
	else if (colscheme==7.0) c = rainbowph (sqrt(sqrt(1.0-f)), 0.75)*sqrt(sqrt(f));		// rainbow with adjustable phase shift

	gl_FragColor.rgb = c*(1.0-gl_FragCoord.z*0.5);
	gl_FragColor.a = clamp (f+0.25+fc, 0.0, 1.0);
}
