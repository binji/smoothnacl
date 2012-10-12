// SmoothLife
//
// 3D kernelmul


uniform float sc;

uniform sampler3D tex0;
uniform sampler3D tex1;

void main()
{
	vec2 a, b;

	a = texture3D (tex0, gl_TexCoord[0].xyz).rg;
	b = texture3D (tex1, gl_TexCoord[1].xyz).rg*sc;
	gl_FragColor.r = a.r*b.r - a.g*b.g;
	gl_FragColor.g = a.r*b.g + a.g*b.r;
}
