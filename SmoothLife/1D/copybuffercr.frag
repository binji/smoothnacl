// SmoothLife
//
// 1D copybuffer complex real


uniform sampler1D tex0;
uniform sampler1D tex1;

void main()
{
	int a;

	a = int(gl_TexCoord[1].x);
	if ((a/2)*2==a)
	{
		gl_FragColor.r = texture1D (tex0, gl_TexCoord[0].x).r;
	}
	else
	{
		gl_FragColor.r = texture1D (tex0, gl_TexCoord[0].x).g;
	}
}
