/************************************************************
	Author	: Laurent Pontier
	1997	: LP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Laurent Pontier

************************************************************/

// texture noise.


#ifndef _TEXTURE_NOISE_H_
#define _TEXTURE_NOISE_H_

#include "Point3D.h"

#define DOT(a,b) (a[0] * b[0] + a[1] * b[1] + a[2] * b[2])
#define BB 256
#define at(rx,ry,rz) (rx*q[0] + ry*q[1] + rz*q[2])
#define s_curve(t) ((t) * (t) * (3. - 2.0 * (t)))
#define lerp(t, a, b) ((a) + (t) * ((b) - (a)))
#define setup(i,b0,b1,r0,r1) \
	t = i + 10000.; \
	b0 = ((int)t) & (BB - 1); \
	b1 = (b0 + 1) & (BB - 1); \
	r0 = t - (int)t; \
	r1 = r0 - 1; 

class Texture_noise{

private:
	int p[BB + BB + 2];
	float g[BB + BB + 2][3];
	int start;
	int Octave;
	float noise(float, float, float);

public:
	Texture_noise(int);
	~Texture_noise();
	void SetOctave(int octave){Octave=octave;};
	int GetOctave(void){return Octave;};

	void init(void);
	float turbulence(float, float, float);
	

};


#endif






