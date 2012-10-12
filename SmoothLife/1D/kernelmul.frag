// SmoothLife
//
// 1D kernelmul


uniform float sc;

uniform sampler1D tex0;
uniform sampler1D tex1;

void main()
{
	vec2 a, b;

	a = texture1D (tex0, gl_TexCoord[0].x).rg;
	b = texture1D (tex1, gl_TexCoord[1].x).rg*sc;
	gl_FragColor.r = a.r*b.r - a.g*b.g;
	gl_FragColor.g = a.r*b.g + a.g*b.r;
}
