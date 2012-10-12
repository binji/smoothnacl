// SmoothLife
//
// 1D draw and color


uniform sampler1D tex0;

const float pi = 6.283185307;

uniform float colscheme, phase;

vec3 rainbow (float f)
{
	float r, g, b;

	     if (f>=0.0 && f<1.0) { r=1.0;   g=f-0.0; b=0.0;   }
	else if (f>=1.0 && f<2.0) { r=2.0-f; g=1.0;   b=0.0;   }
	else if (f>=2.0 && f<3.0) { r=0.0;   g=1.0;   b=f-2.0; }
	else if (f>=3.0 && f<4.0) { r=0.0;   g=4.0-f; b=1.0;   }
	else if (f>=4.0 && f<5.0) { r=f-4.0; g=0.0;   b=1.0;   }
	else if (f>=5.0 && f<6.0) { r=1.0;   g=0.0;   b=6.0-f; }
	else { r=0.0; g=0.0; b=0.0; }

	return vec3(r,g,b);
}

vec3 rainbowph (float f, float ph)
{
	return 0.5*sin(1.7*cos(pi*(f+vec3(0,1,2)/3.0+ph)))+0.5;
}


vec3 color (float f)
{
	//     if (colscheme==1.0) return vec3(f,.0,.0);		// red on black
	     if (colscheme==1.0) return rainbow (fract(phase)*6.0)*f;
	else if (colscheme==2.0) return vec3(f,f,f);		// white on black
	else if (colscheme==3.0) return vec3(1.0-f,1.0-f,1.0-f);		// black on white
	else if (colscheme==4.0) return rainbow (6.0*sqrt(sqrt(1.0-f)))*sqrt(sqrt(f));		// rainbow
	else if (colscheme==5.0) return mix (vec3(.5,.3,.0), vec3(.5,.75,.1), f);		// brown/green
	else if (colscheme==6.0) return mix (vec3(.5,.3,.0), mix (vec3(1.0,.95,.0), vec3(.3,.2,.0), f), sqrt(sqrt(f)));		// gold and brown
	else if (colscheme==7.0) return rainbowph (sqrt(sqrt(1.0-f)), phase)*sqrt(sqrt(f));		// rainbow with adjustable phase shift
}


void main()
{
	gl_FragColor.rgb = color (texture1D (tex0, gl_TexCoord[0].x).r);
}
