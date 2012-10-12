// SmoothLife
//
// 2D fft


uniform int dim, tang;
uniform float tangsc;

uniform sampler2D tex0;
uniform sampler1D tex1;


vec2 cmul (vec2 a, vec2 b)
{
	return vec2 (a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}


void main()
{
	vec2 a, b;
	vec2 v;
	vec4 p;

	v = gl_TexCoord[0].xy;

	if (dim==1)
	{
		p = texture1D (tex1, gl_TexCoord[1].x).rgba;
		a = texture2D (tex0, vec2 (p.r, v.y)).rg;
		b = texture2D (tex0, vec2 (p.g, v.y)).rg;
		if (tang==1)
		{
			b.y = -b.y;
			gl_FragColor.rg = (a+b + cmul (a-b, p.ba))*tangsc;
		}
		else
		{
			gl_FragColor.r = (a.r + p.b*b.r - p.a*b.g)*(1.0/sqrt(2.0));
			gl_FragColor.g = (a.g + p.b*b.g + p.a*b.r)*(1.0/sqrt(2.0));
		}
	}
	else //if (dim==2)
	{
		p = texture1D (tex1, gl_TexCoord[1].y).rgba;
		a = texture2D (tex0, vec2 (v.x, p.r)).rg;
		b = texture2D (tex0, vec2 (v.x, p.g)).rg;
		//gl_FragColor.rg = (a + cmul (p.ba,b))*(1.0/sqrt(2.0));
		gl_FragColor.r = (a.r + p.b*b.r - p.a*b.g)*(1.0/sqrt(2.0));
		gl_FragColor.g = (a.g + p.b*b.g + p.a*b.r)*(1.0/sqrt(2.0));
	}

}
