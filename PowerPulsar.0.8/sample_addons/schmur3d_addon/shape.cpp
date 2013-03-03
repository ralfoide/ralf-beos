//******************************************************************************
//
//	File:		shape.cpp
//
//
//	Written by:	B. S.
//
//	Copyright 1996, Be Incorporated
//
//	Change History:
//
//	5/15/93		bgs	new today
//
//******************************************************************************


#include "PP_Be.h"
#include "t3d.h"

/*------------------------------------------------------------*/


TShape::TShape(T3d *aowner)
{
	owner = aowner;
	num_point = 0;
	num_poly = 0;
	cur_alpha = 0.0;
	cur_delta = 0.0;
	cur_zeta  = 0.0;
}

/*------------------------------------------------------------*/

TShape::~TShape()
{
}

/*------------------------------------------------------------*/

int	TShape::add_point(long x, long y, long z)
{
	x *= 64;
	y *= 64;
	z *= 64;

	pt_x[num_point] = x;
	pt_y[num_point] = y;
	pt_z[num_point] = z;
	num_point++;
	return(num_point - 1);
}

/*------------------------------------------------------------*/

void	TShape::add_triangle(long p1, long p2, long p3, rgb_color co)
{
	l1[num_poly] = p1;
	l2[num_poly] = p2;
	l3[num_poly] = p3;
	l4[num_poly] = p3;
	acolor[num_poly] = co;
	num_poly++;
}


void	TShape::add_poly(long p1, long p2, long p3, long p4, rgb_color co)
{
	l1[num_poly] = p1;
	l2[num_poly] = p2;
	l3[num_poly] = p3;
	l4[num_poly] = p4;
	acolor[num_poly] = co;
	num_poly++;
}

//------------------------------------------------------------------------------

long	TShape::calc_mid(long pn)
{
	long	sum;

	sum = screen_z[l1[pn]];
	sum += screen_z[l2[pn]];
	sum += screen_z[l3[pn]];
	sum += screen_z[l4[pn]];
	sum /= 4;
	return(sum);
}

//------------------------------------------------------------------------------

void	TShape::sort_polys()
{
	long		i;
	long		j;
	rgb_color	tmp_color;
	long		tmp;
	long		step;

	for (i = 0; i < (num_poly); i++)
		zs[i] = calc_mid(i);


	for (step = num_poly / 2; step > 0; step /= 2)
		for (i = step; i < num_poly; i++)
			for (j=i - step; j >= 0 && zs[j] > zs[j+step]; j -= step) {
				tmp = zs[j];
				zs[j] = zs[j + step];
				zs[j + step] = tmp;

				tmp = l1[j];	
				l1[j] = l1[j + step];
				l1[j + step] = tmp;

				tmp = l2[j];	
				l2[j] = l2[j + step];
				l2[j + step] = tmp;

				tmp = l3[j];	
				l3[j] = l3[j + step];
				l3[j + step] = tmp;

				tmp = l4[j];	
				l4[j] = l4[j + step];
				l4[j + step] = tmp;
				
				tmp_color = acolor[j];	
				acolor[j] = acolor[j + step];
				acolor[j + step] = tmp_color;
			}
}

//------------------------------------------------------------------------------
#include <Screen.h>
long	TShape::calc_color(long i, rgb_color a_color)
{
	long	dx;
	//long	dy;
	//long	dz;
	long	br;
	long	r, g, b;
	
	r = a_color.red;
	g = a_color.green;
	b = a_color.blue;
	
	dx = screen_x[l1[i]] - screen_x[l3[i]];

	br = 0;
	br = dx * dx;
	br = br / 20;
	br += 10;
	if (br > 32)
		br = 32;
		
	a_color.red = (r*br)/32;
	a_color.green = (g*br)/32;
	a_color.blue = (b*br)/32;

	//return 15+(i&0xF);
	 
	// return (owner->filter->sPrepare.interfaceScreen->IndexForColor(a_color));
	return BScreen().IndexForColor(a_color);
}

//------------------------------------------------------------------------------

void	TShape::draw(float new_alpha, float new_delta, float new_zeta, float pos_x, float pos_y)
{
	BPoint	p1;
	BPoint	p2;
	BPoint	p3;
	BPoint	p4;
	long	i;
	long	tmp;
	ulong	color;


	change_view_point(new_alpha, new_delta, new_zeta);
	sort_polys();


	for (i = 0; i < num_poly; i++) {
		p1.x = pos_x + screen_x[l1[i]];
		p1.y = pos_y + screen_y[l1[i]];

		p2.x = pos_x + screen_x[l2[i]];
		p2.y = pos_y + screen_y[l2[i]];
	
		p3.x = pos_x + screen_x[l3[i]];
		p3.y = pos_y + screen_y[l3[i]];
		
		p4.x = pos_x + screen_x[l4[i]];
		p4.y = pos_y + screen_y[l4[i]];
		tmp = calc_color(i, acolor[i]);
		//tmp = acolor[i].red;
		color = (tmp << 24) | (tmp << 16) | (tmp << 8) | tmp;

		if (l3[i] == l4[i])
			owner->fill_triangle(p1, p2, p3, color);
		else
			owner->fill_4(p1, p2, p3, p3, color);
	}
}

//------------------------------------------------------------------------------

void	TShape::change_view_point(float new_alpha, float new_delta, float new_zeta)
{
	long	sina;
	long	cosa;
	long	sinb;
	long	cosb;
	long	sinc;
	long	cosc;

	long	i;
	long	x, y, z;
	long	x0, y0, z0;
	//long	hs, vs, ms;
	long	k;
	//float	kk;

	
	cur_alpha = new_alpha;
	cur_delta = new_delta;
	cur_zeta  = new_zeta;


	//ms = 1500;

	sina = sin(new_alpha) * 65535;
	cosa = cos(new_alpha) * 65535;

	sinb = sin(new_delta) * 65535;
	cosb = cos(new_delta) * 65535;

	sinc = sin(new_zeta) * 65535;
	cosc = cos(new_zeta) * 65535;

	for (i = 0; i < num_point; i++) {
		
		

		x = pt_x[i];
		y = pt_y[i];
		z = pt_z[i];

		x0 = x * cosa - y * sina;
		y0 = x * sina + y * cosa;

		x = x0 >> 16;
		y = y0 >> 16;
		
		x0 = x * cosc - z * sinc;
		z0 = x * sinc + z * cosc;

		if (x0 < 0)
			x0 -= 32760;
		else
			x0 += 32760;

		x = x0 >> 16;
		z = z0 >> 16;

		y0 = y * cosb - z * sinb;
		z0 = y * sinb + z * cosb;

		y = y0 >> 16;

		if (y0 < 0)
			y0 -= 32760;
		else
			y0 += 32760;

		y = y0 >> 16;

		z = z0 >> 16;
		
		z = z - 22000;		// observer distance

		k = -z / 155;

		z = z + 22000;		// observer distance
		screen_x[i] = x / k;
		screen_y[i] = y / k;
		screen_z[i] = z / 64;
	}
}

//------------------------------------------------------------------------------

