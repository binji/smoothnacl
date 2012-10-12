// SmoothLife
//
// 3D copybuffer real complex


uniform sampler3D tex0;
uniform sampler3D tex1;

void main()
{
	gl_FragColor.r = texture3D (tex0, gl_TexCoord[0].xyz).r;
	gl_FragColor.g = texture3D (tex1, gl_TexCoord[1].xyz).r;
}
