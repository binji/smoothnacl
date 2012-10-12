// SmoothLife
//
// 2D snm


const float pi = 6.283185307;


uniform float b1, b2, d1, d2, sn, sm;

uniform float mode, sigmode, sigtype, mixtype, dt;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;


float func_hard (float x, float a)
{
	if (x>=a) return 1.0; else return 0.0;
}

float func_linear (float x, float a, float ea)
{
	if (x < a-ea/2.0) return 0.0;
	else if (x > a+ea/2.0) return 1.0;
	else return (x-a)/ea + 0.5;
}

float func_hermite (float x, float a, float ea)
{
	if (x < a-ea/2.0) return 0.0;
	else if (x > a+ea/2.0) return 1.0;
	else
	{
		float m = (x-(a-ea/2.0))/ea;
		return m*m*(3.0-2.0*m);
	}
}

float func_sin (float x, float a, float ea)
{
	if (x < a-ea/2.0) return 0.0;
	else if (x > a+ea/2.0) return 1.0;
	else return sin(pi/2.0*(x-a)/ea)*0.5+0.5;
}

float func_smooth (float x, float a, float ea)
{
	return 1.0/(1.0+exp(-(x-a)*4.0/ea));
}

float func_atan (float x, float a, float ea)
{
	return atan((x-a)*(pi/2.0)/ea)/(pi/2.0)+0.5;
}

float func_atancos (float x, float a, float ea)
{
	return (atan((x-a)/ea)/(pi/4.0)*cos((x-a)*1.4)*1.1 + 1.0)/2.0;
}

float func_overshoot (float x, float a, float ea)
{
	return (1.0/(1.0+exp(-(x-a)*4.0/ea))-0.5)*(1.0+exp(-(x-a)*(x-a)/ea/ea))+0.5;
}


float sigmoid_ab (float x, float a, float b)
{
	     if (sigtype==0.0) return func_hard      (x, a    )*(1.0-func_hard      (x, b    ));
	else if (sigtype==1.0) return func_linear    (x, a, sn)*(1.0-func_linear    (x, b, sn));
	else if (sigtype==2.0) return func_hermite   (x, a, sn)*(1.0-func_hermite   (x, b, sn));
	else if (sigtype==3.0) return func_sin       (x, a, sn)*(1.0-func_sin       (x, b, sn));
	else if (sigtype==4.0) return func_smooth    (x, a, sn)*(1.0-func_smooth    (x, b, sn));
	else if (sigtype==5.0) return func_atan      (x, a, sn)*(1.0-func_atan      (x, b, sn));
	else if (sigtype==6.0) return func_atancos   (x, a, sn)*(1.0-func_atancos   (x, b, sn));
	else if (sigtype==7.0) return func_overshoot (x, a, sn)*(1.0-func_overshoot (x, b, sn));
	else if (sigtype==8.0) return 1.0/(1.0+exp(-(x-a)*4.0/sn)) * 1.0/(1.0+exp((x-b)*4.0/sn)) * (1.0-0.2*exp(-((x-(a+b)/2.0)*20.0)*((x-(a+b)/2.0)*20.0)));
	else if (sigtype==9.0) return 1.0/(1.0+exp(-(x-a)*4.0/sn)) * 1.0/(1.0+exp((x-b)*4.0/sn)) * (1.0+0.2*exp(-((x-(a+b)/2.0)*20.0)*((x-(a+b)/2.0)*20.0)));
}

float sigmoid_mix (float x, float y, float m)
{
	     if (mixtype==0.0) return x*(1.0-func_hard      (m, 0.5    )) + y*func_hard      (m, 0.5    );
	else if (mixtype==1.0) return x*(1.0-func_linear    (m, 0.5, sm)) + y*func_linear    (m, 0.5, sm);
	else if (mixtype==2.0) return x*(1.0-func_hermite   (m, 0.5, sm)) + y*func_hermite   (m, 0.5, sm);
	else if (mixtype==3.0) return x*(1.0-func_sin       (m, 0.5, sm)) + y*func_sin       (m, 0.5, sm);
	else if (mixtype==4.0) return x*(1.0-func_smooth    (m, 0.5, sm)) + y*func_smooth    (m, 0.5, sm);
	else if (mixtype==5.0) return x*(1.0-func_atan      (m, 0.5, sm)) + y*func_atan      (m, 0.5, sm);
	else if (mixtype==6.0) return x*(1.0-func_atancos   (m, 0.5, sm)) + y*func_atancos   (m, 0.5, sm);
	else if (mixtype==7.0) return x*(1.0-func_overshoot (m, 0.5, sm)) + y*func_overshoot (m, 0.5, sm);
}


void main()
{
	float n, m, f;

	n = texture2D (tex0, gl_TexCoord[0].xy).r;
	m = texture2D (tex1, gl_TexCoord[1].xy).r;

	     if (sigmode==1.0) f = mix (sigmoid_ab (n, b1, b2), sigmoid_ab (n, d1, d2), m);
	else if (sigmode==2.0) f = sigmoid_mix (sigmoid_ab (n, b1, b2), sigmoid_ab (n, d1, d2), m);
	else if (sigmode==3.0) f = sigmoid_ab (n, mix (b1, d1, m), mix (b2, d2, m));
	else  /*sigmode==4.0*/ f = sigmoid_ab (n, sigmoid_mix (b1, d1, m), sigmoid_mix (b2, d2, m));

	if (mode>0.0)
	{
		float g = texture2D (tex2, gl_TexCoord[2].xy).r;

		     if (mode==1.0) f = g + dt*(2.0*f-1.0);
		else if (mode==2.0) f = g + dt*(f-g);
		else if (mode==3.0) f = m + dt*(2.0*f-1.0);
		else if (mode==4.0) f = m + dt*(f-m);
	}

	gl_FragColor.r = clamp (f, 0.0, 1.0);
}
