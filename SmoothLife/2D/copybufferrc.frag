// SmoothLife
//
// 2D copybuffer real complex


uniform sampler2D tex0;
uniform sampler2D tex1;

void main()
{
	gl_FragColor.r = texture2D (tex0, gl_TexCoord[0].xy).r;
	gl_FragColor.g = texture2D (tex1, gl_TexCoord[1].xy).r;
}
