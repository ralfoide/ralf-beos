/************************************************************
	Author	: Laurent Pontier
	1997	: LP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Laurent Pontier

************************************************************/

// Texture mathematique.

#ifndef _TEXTURE_NOISE_H_
#include "Texture_noise.h"
#endif

#include <stdlib.h>
#include <string.h>

// RM 050300
#ifndef bzero
#define bzero(s, l)  memset(s, 0, l) 
#endif

Texture_noise::Texture_noise(int octave)
{
	start=1;
	bzero(p,sizeof(int)*(BB+BB+2));
	bzero(g,sizeof(float)*(BB+BB+2)*3);

	Octave=octave;
}

Texture_noise::~Texture_noise()
{}



void Texture_noise::init(void)
{
	int i, j, k;
	float v[3], s;
	
	srand(1);
	for (i = 0; i < BB; i++)
		{
		do
			{
			for (j = 0; j < 3; j++)
				v[j] = (float)((rand() % (BB + BB)) - BB) / BB;
			s = DOT(v, v);
			} while (s > 1.0);
		s = sqrt(s);
		for (j = 0; j < 3; j++)
			g[i][j] = v[j] / s;
		}
	for (i = 0; i < BB; i++)
		p[i] = i;
	for (i = BB; i > 0; i -= 2)
		{
		j = rand() % BB;
		k = p[i];
		p[i] = p[j];
		p[j] = k;
		}
	for (i = 0; i < BB + 2; i++)
		{
		p[BB + i] = p[i];
		for (j = 0; j < 3; j++)
			g[BB + i][j] = g[i][j];
		}
}

float Texture_noise::noise(float x, float y, float z)
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, rz0, rz1, *q, sx, sy, sz, a, b, c, d, t, u, v;
	float n;
	register int i, j;
	
	if (start)
		{
		start = 0;
		init();
		}
	
	setup(x, bx0, bx1, rx0, rx1);
	setup(y, by0, by1, ry0, ry1);
	setup(z, bz0, bz1, rz0, rz1);
	
	i = p[bx0];
	j = p[bx1];
	
	b00 = p[i + by0];
	b10 = p[j + by0];
	b01 = p[i + by1];
	b11 = p[j + by1];
	

	sx = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);
	
	q = g[b00 + bz0]; u = at(rx0, ry0, rz0);
	q = g[b10 + bz0]; v = at(rx1, ry0, rz0);
	a = lerp(sx, u, v);
	
	q = g[b01 + bz0]; u = at(rx0, ry1, rz0);
	q = g[b11 + bz0]; v = at(rx1, ry1, rz0);
	b = lerp(sx, u, v);
	
	c = lerp(sy, a, b);
	
	q = g[b00 + bz1]; u = at(rx0, ry0, rz1);
	q = g[b10 + bz1]; v = at(rx1, ry0, rz1);
	a = lerp(sx, u, v);
	
	q = g[b01 + bz1]; u = at(rx0, ry1, rz1);
	q = g[b11 + bz1]; v = at(rx1, ry1, rz1);
	b = lerp(sx, u, v);
	
	d = lerp(sy, a, b);

	n = 1.5 * lerp(sz, c, d);
	n = (n + 1.0) / 2.0;
	
	return n;
}

float Texture_noise::turbulence(float x, float y, float z)
{
	float s, t, ts, xx, yy, zz;
	int i;

	if (Octave == 0) return 0.5;
	if (Octave == 1) return noise(x, y, z);

	xx = x;
	yy = y;
	zz = z;

	s = 1.0;

	t = 0.0;
	ts = 0.0;
	for (i = 0; i < Octave; i++)
		{
		t += s * noise(xx, yy, zz);
		ts += s;
		xx *= 2.0;
		yy *= 2.0;
		zz *= 2.0;
		s /= 2.0;
		}
	return t / ts;
}
