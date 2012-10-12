// SmoothLife
//
// 3D draw (raycast) and color


struct Ray
{
	vec3 d;
	vec3 o;
};

uniform sampler3D tex0;

uniform float fx, fy, fz;
uniform float nx, ny, nz;

const float pi = 6.283185307;

uniform float colscheme, phase, visscheme;

vec3 rainbow (float f)
{
	float r, g, b;

	     if (f>=0.0 && f<1.0) { r=1.0;   g=f-0.0; b=0.0;   }
	else if (f>=1.0 && f<2.0) { r=2.0-f; g=1.0;   b=0.0;   }
	else if (f>=2.0 && f<3.0) { r=0.0;   g=1.0;   b=f-2.0; }
	else if (f>=3.0 && f<4.0) { r=0.0;   g=4.0-f; b=1.0;   }
	else if (f>=4.0 && f<5.0) { r=f-4.0; g=0.0;   b=1.0;   }
	else if (f>=5.0 && f<6.0) { r=1.0;   g=0.0;   b=6.0-f; }
	else { r=0.0; g=0.0; b=0.0; }

	return vec3(r,g,b);
}

vec3 rainbowph (float f, float ph)
{
	return 0.5*sin(1.7*cos(pi*(f+vec3(0,1,2)/3.0+ph)))+0.5;
}


vec3 color (float f)
{
	//     if (colscheme==1.0) return vec3(f,.0,.0);		// red on black
	     if (colscheme==1.0) return rainbow (fract(phase)*6.0)*f;
	else if (colscheme==2.0) return vec3(f,f,f);		// white on black
	else if (colscheme==3.0) return vec3(1.0-f,1.0-f,1.0-f);		// black on white
	else if (colscheme==4.0) return rainbow (6.0*sqrt(sqrt(1.0-f)))*sqrt(sqrt(f));		// rainbow
	else if (colscheme==5.0) return mix (vec3(.5,.3,.0), vec3(.5,.75,.1), f);		// brown/green
	else if (colscheme==6.0) return mix (vec3(.5,.3,.0), mix (vec3(1.0,.95,.0), vec3(.3,.2,.0), f), sqrt(sqrt(f)));		// gold and brown
	else if (colscheme==7.0) return rainbowph (sqrt(sqrt(1.0-f)), phase)*sqrt(sqrt(f));		// rainbow with adjustable phase shift
}


float trueb (float f)
{
	return mix (0.99, 0.9, f);
}


vec3 licht (float f)
{
	return color (1.0-f)*0.01;
}



void main()
{
	Ray r;
	vec3 boxmin = vec3 (-nx/2.0, -ny/2.0, -nz/2.0);
	vec3 boxmax = vec3 ( nx/2.0,  ny/2.0,  nz/2.0);
	const float stepsize = 0.5;
	const float brightness = 4.0;
	const float density = 0.25;
	const float threshold = 0.99;

	float tnear, tfar;

	r.o = gl_TexCoord[0].xyz;
	r.d = gl_TexCoord[1].xyz;

	r.d = normalize (r.d);

	// compute intersection of ray with all six bbox planes
	vec3 invR = 1.0 / r.d;
	vec3 tbot = invR * (boxmin.xyz - r.o);
	vec3 ttop = invR * (boxmax.xyz - r.o);

	// re-order intersections to find smallest and largest on each axis
	vec3 tmin = min (ttop, tbot);
	vec3 tmax = max (ttop, tbot);

	// find the largest tmin and the smallest tmax
	vec2 t0 = max (tmin.xx, tmin.yz);
	float largest_tmin = max (t0.x, t0.y);
	t0 = min (tmax.xx, tmax.yz);
	float smallest_tmax = min (t0.x, t0.y);

	tnear = largest_tmin;
	tfar = smallest_tmax;

	if (tnear < 0.0) tnear = 0.0;

	// calculate intersection points

	vec3 Pnear = r.o + r.d*tnear;
	vec3 Pfar = r.o + r.d*tfar;

	// convert to texture space

	vec3 Pmul = vec3 (1.0/nx, 1.0/ny, 1.0/nz);
	vec3 Padd = vec3 (0.5, 0.5, 0.5);

	// march along ray, accumulating color


	if (visscheme==6.0)
	{

		float random = fract(sin(gl_FragCoord.x * 12.9898 + gl_FragCoord.y * 78.233) * 43758.5453);

		vec3 Pstep = r.d * stepsize;
		vec3 P = Pnear + vec3(fx,fy,fz)*vec3(nx,ny,nz) + random*Pstep;

		vec4 c = vec4(0,0,0,0);

		int steps = int((tfar-tnear)/stepsize);		// rainbow with laplacian
		float t = tnear;

		for (int i=0; i<steps; i++)
		{
			float s = texture3D (tex0, P*Pmul+Padd).r;
			
			float d;
			
			d = texture3D (tex0, (P+vec3(-1, 0, 0))*Pmul+Padd).r + texture3D (tex0, (P+vec3(1, 0, 0))*Pmul+Padd).r
			  + texture3D (tex0, (P+vec3( 0,-1, 0))*Pmul+Padd).r + texture3D (tex0, (P+vec3(0, 1, 0))*Pmul+Padd).r
			  + texture3D (tex0, (P+vec3( 0, 0,-1))*Pmul+Padd).r + texture3D (tex0, (P+vec3(0, 0, 1))*Pmul+Padd).r
			  - 6.0*s;

			s *= density;

			c.rgb += (1.0 - c.a)*s*s * 2.0*rainbowph(d*0.7,phase) * (6.0-t/(nx*0.5))*0.5;
			c.a += (1.0 - c.a)*s;
			if (c.a > threshold) break;

			P += Pstep;
			t += stepsize;
		}
		c.rgb *= brightness;

		gl_FragColor.rgb = c.rgb;

	}
	else if (visscheme==5.0)
	{

		vec3 P = Pnear + vec3(fx,fy,fz)*vec3(nx,ny,nz);
		vec3 Pstep = r.d * stepsize;

		vec4 c = vec4(0,0,0,0);

		int steps = int((tfar-tnear)/stepsize);		// rainbow with gradient
		float t = tnear;

		for (int i=0; i<steps; i++)
		{
			float s = texture3D (tex0, P*Pmul+Padd).r*density;
			
			vec3 d;
			
			d.x = texture3D (tex0, (P+vec3(-1, 0, 0))*Pmul+Padd).r - texture3D (tex0, (P+vec3(1, 0, 0))*Pmul+Padd).r;
			d.y = texture3D (tex0, (P+vec3( 0,-1, 0))*Pmul+Padd).r - texture3D (tex0, (P+vec3(0, 1, 0))*Pmul+Padd).r;
			d.z = texture3D (tex0, (P+vec3( 0, 0,-1))*Pmul+Padd).r - texture3D (tex0, (P+vec3(0, 0, 1))*Pmul+Padd).r;

			c.rgb += (1.0 - c.a)*s*s * 2.0*rainbowph(dot(r.d,d)*0.7,phase) * (6.0-t/(nx*0.5))*0.5;
			c.a += (1.0 - c.a)*s;
			if (c.a > threshold) break;

			P += Pstep;
			t += stepsize;
		}
		c.rgb *= brightness;

		gl_FragColor.rgb = c.rgb;

	}
	else if (visscheme==4.0)
	{

		vec3 P = Pnear + vec3(fx,fy,fz)*vec3(nx,ny,nz);
		vec3 Pstep = r.d * stepsize;

		vec2 c = vec2(0,0);

		int steps = int((tfar-tnear)/stepsize);		// raymarch with darkening

		float t = tnear;

		for (int i=0; i<steps; i++)
		{
			float s = texture3D (tex0, P*Pmul+Padd).r*density;

			c.y += (1.0 - c.x)*s*s*(6.0-t/(nx*0.5))*0.5;
			c.x += (1.0 - c.x)*s;
			if (c.x > threshold) break;

			P += Pstep;
			t += stepsize;
		}
		c.y *= brightness;

		gl_FragColor.rgb = color(c.y);

	}
	else if (visscheme==3.0)
	{

		vec3 P = Pnear + vec3(fx,fy,fz)*vec3(nx,ny,nz);
		vec3 Pstep = r.d * stepsize;

		vec2 c = vec2(0,0);

		int steps = int((tfar-tnear)/stepsize);		// raymarch with threshold fog

		for (int i=0; i<steps; i++)
		{
			float s = texture3D (tex0, P*Pmul+Padd).r*density;

			c.y += (1.0 - c.x)*s*s;
			c.x += (1.0 - c.x)*s;
			if (c.x > threshold) break;

			P += Pstep;
		}
		c.y *= brightness;

		vec4 p = vec4(P, 1);
		p = gl_ModelViewMatrix * p;

		gl_FragColor.rgb = color(length(p/7.0/(nx*0.5)));

	}
	else if (visscheme==2.0)
	{

		vec3 P = Pnear + vec3(fx,fy,fz)*vec3(nx,ny,nz);
		vec3 Pstep = r.d * stepsize;

		vec2 c = vec2(0,0);

		int steps = int((tfar-tnear)/stepsize);		// generic raymarch

		for (int i=0; i<steps; i++)
		{
			float s = texture3D (tex0, P*Pmul+Padd).r*density;

			c.y += (1.0 - c.x)*s*s;
			c.x += (1.0 - c.x)*s;
			if (c.x > threshold) break;

			P += Pstep;
		}
		c.y *= brightness;

		gl_FragColor.rgb = color(c.y);

	}
	else if (visscheme==1.0)
	{

		vec3 P = Pfar + vec3(fx,fy,fz)*vec3(nx,ny,nz);
		vec3 Pstep = r.d * stepsize;

		int steps = int((tfar-tnear)/stepsize);		// fog

		vec3 c = vec3(0.5,0.5,0.5);

		for (int i=0; i<steps; i++)
		{
			float f = texture3D (tex0, P*Pmul+Padd).r;
			P -= Pstep;

			c *= trueb (f);
			c += licht (f);
		}

		gl_FragColor.rgb = c;

	}
	else if (visscheme==0.0)
	{

		vec3 P = Pnear + vec3(fx,fy,fz)*vec3(nx,ny,nz);
		vec3 Pstep = r.d * stepsize;

		int steps = int((tfar-tnear)/stepsize);		// simple

		float f, c = 0.0;

		for (int i=0; i<steps; i++)
		{
			f = texture3D (tex0, P*Pmul+Padd).r;
			P += Pstep;

			c += 0.01*f;
		}

		gl_FragColor.rgb = color(c);

	}
}
