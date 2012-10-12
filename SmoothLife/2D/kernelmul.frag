// SmoothLife
//
// 2D kernelmul


uniform float sc;

uniform sampler2D tex0;
uniform sampler2D tex1;

void main()
{
	vec2 a, b;

	a = texture2D (tex0, gl_TexCoord[0].xy).rg;
	b = texture2D (tex1, gl_TexCoord[1].xy).rg*sc;
	gl_FragColor.r = a.r*b.r - a.g*b.g;
	gl_FragColor.g = a.r*b.g + a.g*b.r;
}
