// SmoothLife
//
// 1D copybuffer real complex


uniform sampler1D tex0;
uniform sampler1D tex1;

void main()
{
	gl_FragColor.r = texture1D (tex0, gl_TexCoord[0].x).r;
	gl_FragColor.g = texture1D (tex1, gl_TexCoord[1].x).r;
}
