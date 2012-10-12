// SmoothLife
//
// 1D fft


uniform int dim, tang;
uniform float tangsc;

uniform sampler1D tex0;
uniform sampler1D tex1;


vec2 cmul (vec2 a, vec2 b)
{
	return vec2 (a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}


void main()
{
	vec2 a, b;
	//float v;
	vec4 p;

	//v = gl_TexCoord[0].x;

	p = texture1D (tex1, gl_TexCoord[1].x).rgba;
	a = texture1D (tex0, p.r).rg;
	b = texture1D (tex0, p.g).rg;
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
