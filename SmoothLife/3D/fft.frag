// SmoothLife
//
// 3D fft


uniform int dim, tang;
uniform float tangsc;

uniform sampler3D tex0;
uniform sampler1D tex1;


vec2 cmul (vec2 a, vec2 b)
{
	return vec2 (a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}


void main()
{
	vec2 a, b;
	vec3 v;
	vec4 p;

	v = gl_TexCoord[0].xyz;

	if (dim==1)
	{
		p = texture1D (tex1, gl_TexCoord[1].x).rgba;
		a = texture3D (tex0, vec3 (p.r, v.y, v.z)).rg;
		b = texture3D (tex0, vec3 (p.g, v.y, v.z)).rg;
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
	else if (dim==2)
	{
		p = texture1D (tex1, gl_TexCoord[1].y).rgba;
		a = texture3D (tex0, vec3 (v.x, p.r, v.z)).rg;
		b = texture3D (tex0, vec3 (v.x, p.g, v.z)).rg;
		//gl_FragColor.rg = (a + cmul (p.ba,b))*(1.0/sqrt(2.0));
		gl_FragColor.r = (a.r + p.b*b.r - p.a*b.g)*(1.0/sqrt(2.0));
		gl_FragColor.g = (a.g + p.b*b.g + p.a*b.r)*(1.0/sqrt(2.0));
	}
	else // dim==3
	{
		p = texture1D (tex1, gl_TexCoord[1].z).rgba;
		a = texture3D (tex0, vec3 (v.x, v.y, p.r)).rg;
		b = texture3D (tex0, vec3 (v.x, v.y, p.g)).rg;
		gl_FragColor.r = (a.r + p.b*b.r - p.a*b.g)*(1.0/sqrt(2.0));
		gl_FragColor.g = (a.g + p.b*b.g + p.a*b.r)*(1.0/sqrt(2.0));
	}

}
