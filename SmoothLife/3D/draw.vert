// SmoothLife
//
// 3D draw vertex shader

void main()
{
	gl_Position = ftransform();

	vec4 o = gl_ModelViewMatrixInverse * vec4(0,0,0,1);
	vec3 d = gl_Vertex.xyz - o.xyz;

	gl_TexCoord[0].xyz = o.xyz;
	gl_TexCoord[1].xyz = d;
}
