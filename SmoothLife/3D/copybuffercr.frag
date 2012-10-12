// SmoothLife
//
// 3D copybuffer complex real


uniform sampler3D tex0;
uniform sampler3D tex1;

void main()
{
	int a;

	a = int(gl_TexCoord[1].x);
	if ((a/2)*2==a)
	{
		gl_FragColor.r = texture3D (tex0, gl_TexCoord[0].xyz).r;
	}
	else
	{
		gl_FragColor.r = texture3D (tex0, gl_TexCoord[0].xyz).g;
	}
}
