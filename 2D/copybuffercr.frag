// SmoothLife
//
// 2D copybuffer complex real


uniform sampler2D tex0;
uniform sampler2D tex1;

void main()
{
	int a;

	a = int(gl_TexCoord[1].x);
	if ((a/2)*2==a)
	{
		gl_FragColor.r = texture2D (tex0, gl_TexCoord[0].xy).r;
	}
	else
	{
		gl_FragColor.r = texture2D (tex0, gl_TexCoord[0].xy).g;
	}
}
