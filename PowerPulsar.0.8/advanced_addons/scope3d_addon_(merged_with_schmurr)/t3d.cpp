//******************************************************************************
//
//	File:		t3d.cpp
//
//	Description:	show pixmap main test program.
//
//	Written by:	B. S.
//
//	Copyright 1992, Be Incorporated
//
//	Change History:
//
//	7/31/92		bgs	new today
//
//******************************************************************************

#pragma once on
#include "t3d.h"
//#define DEBUG 1


/*------------------------------------------------------------*/


T3d::T3d(long psize_x, long psize_y)
{
	size_x = psize_x;
	size_y = psize_y;
	filter = NULL;
	use_gk = false;
	poly_scratch = new long[4096];
}

/*------------------------------------------------------------*/

T3d::~T3d()
{
	delete poly_scratch;
}


/*------------------------------------------------------------*/
void	hline(char *base, long count, ulong color);
void	hline(char *base, long count, ulong color)
{
	long	c;

	c = color;

	if (count < 8) {
		while(count-- >= 0)
			*base++ = c;
		return;
	}

	while ((long)base & 0x03) {
		*base++ = c;
		count--;
	}
	while(count > 3) {
		*(long *)base = c;
		base += 4;
		count -= 4;
	}

	while(count-- >= 0) 
		*base++ = c;
}

/*------------------------------------------------------------*/

void	T3d::fill_triangle(BPoint pt_a, BPoint pt_b, BPoint pt_c, ulong color)
{
#define compare(a,b,c)  {if (a<b) b = a; if (a>c) c = a;}

	long* 	coord0;
	long* 	coord1;
	long	*tmp0;
	long	*tmp1;
	long	dx;
	long	dy;
	long	i;
	long	j;
	long	x;
	long	xe;
	long	xpoint[4];
	long	ypoint[4];
	long	xs;
	long	y;
	long	y_sign;
	long	decision;
	long	ye;
	long	ys;
   	long	scanlines;
	long	top;
	long	bottom;
	char	*ptr;
	long	left, right;   
	long	rowbyte = bitbpr; 

	xpoint[3] = xpoint[0] = pt_a.x;
	ypoint[3] = ypoint[0] = pt_a.y;
	xpoint[1] = pt_b.x;
	ypoint[1] = pt_b.y;
	xpoint[2] = pt_c.x;
	ypoint[2] = pt_c.y;


	bottom = -10000;
	top = 10000;
	left = 10000;
	right = -100000;

	for (i = 0; i < 3; i++) {
		if (ypoint[i] > bottom)
			bottom = ypoint[i];
		if (ypoint[i] < top)
			top = ypoint[i];
		if (xpoint[i] < left)
			left = xpoint[i];
		if (xpoint[i] > right)
			right = xpoint[i];
	}
	if (top > (size_y-1))
		return;
	if (bottom < 0)
		return;
	if (right < 0)
		return;
	if (left > (size_x-1))
		return;
	
	scanlines = 1 + bottom - top;

	coord0 = (long*)poly_scratch;
	coord1 = coord0 + scanlines;
		
	tmp0 = coord0;
	tmp1 = coord1;

    	for (i = 0; i <= bottom - top; i++) {
		*tmp0++ = 128000;
		*tmp1++ = -128000;
	}


	for (i = 0; i < 3; i++) {
		xs = xpoint[i];
		xe = xpoint[i + 1];
		ys = ypoint[i];
		ye = ypoint[i + 1];
		
		if (xs > xe) {
			j    = xs;
			xs   = xe;
			xe   = j;
			j    = ys;
			ys   = ye;
			ye   = j;
		}

		y = ys - top;
		tmp0 = coord0 + y;
		tmp1 = coord1 + y;

		compare(xs, *tmp0, *tmp1);

		dx = abs(xe - xs);
		dy = abs(ye - ys);

		if ((ye - ys) < 0)
			y_sign = -1;
		else
			y_sign = 1;

		x = xs;
	
		if (dy == 0) {
			*tmp0 = xs;
			*tmp1 = xe;
			continue;
		}
		
		if (dx >= dy) {
			decision = dx / 2;

			while (x <= xe) {
				if (decision >= dx) {
					decision -= dx;
					y += y_sign;
					tmp0 += y_sign;
					tmp1 += y_sign;
				}

				if (x < *tmp0)
					*tmp0 = x;

				decision += dy;
				
				if (x > *tmp1)
					*tmp1 = x;

				x++;
			}
		}
		else {
			ye -= top;
			decision = dy / 2;
			
			while (y != ye) {
				if (decision >= dy) {
					decision -= dy;
					x++;
				}

				if (x < *tmp0)
					*tmp0 = x;

				tmp0 += y_sign;
				
				if (x > *tmp1)
					*tmp1 = x;

				decision += dx;
				tmp1 += y_sign;
				y += y_sign;
			}
			ye += top;
		}
	}

	tmp0 = coord0;
	tmp1 = coord1;

	if (use_gk)
	{
		for (i = top; i <= bottom; i++)
		{
			filter->blitLine8(*tmp0, i, *tmp1, i, color);
			tmp0++;
			tmp1++;
		}
	}
	else
	{
		ptr = (char *)bitbase;
		ptr = ptr + (top * rowbyte);
		for (i = top; i <= bottom; i++)
		{
			if (*tmp0 < 0)
				*tmp0 = 0;
			if (*tmp1 > (size_x-1))
				*tmp1 = (size_x-1);
			if ((i >= 0) && (i < (size_y - 1))) 
				hline(ptr + *tmp0, *tmp1 - *tmp0, color);

			tmp0++;
			tmp1++;

			ptr += size_x;
		}
	}
}

/*------------------------------------------------------------*/

void	T3d::fill_4(BPoint pt_a, BPoint pt_b, BPoint pt_c, BPoint pt_d, ulong color)
{
	#define compare(a,b,c)  {if (a<b) b = a; if (a>c) c = a;}
 
	long* 	coord0;
	long* 	coord1;
	long	*tmp0;
	long	*tmp1;
	long	dx;
	long	dy;
	long	i;
	long	j;
	long	x;
	long	xe;
	long	xpoint[5];
	long	ypoint[5];
	long	xs;
	long	y;
	long	y_sign;
	long	decision;
	long	ye;
	long	ys;
  long	scanlines;
	long	top;
	long	bottom;
	char	*ptr;
  long	rowbyte = bitbpr; 

	xpoint[4] = xpoint[0] = pt_a.x;
	ypoint[4] = ypoint[0] = pt_a.y;
	xpoint[1] = pt_b.x;
	ypoint[1] = pt_b.y;
	xpoint[2] = pt_c.x;
	ypoint[2] = pt_c.y;
	xpoint[3] = pt_d.x;
	ypoint[3] = pt_d.y;
	
	bottom = -10000;
	top = 10000;

	for (i = 0; i < 4; i++) {
		if (ypoint[i] > bottom)
			bottom = ypoint[i];
		if (ypoint[i] < top)
			top = ypoint[i];
	}
	if (top > size_y)
		return;
	if (bottom < 0)
		return;
	
	scanlines = 1 + bottom - top;

	coord0 = (long*)poly_scratch;
	coord1 = coord0 + scanlines;
		
	tmp0 = coord0;
	tmp1 = coord1;

    for (i = 0; i <= bottom - top; i++) {
		*tmp0++ = 640;
		*tmp1++ = -1;
	}

	for (i = 0; i < 4; i++) {
		xs = xpoint[i];
		xe = xpoint[i + 1];
		ys = ypoint[i];
		ye = ypoint[i + 1];
		
		if (xs > xe) {
			j    = xs;
			xs   = xe;
			xe   = j;
			j    = ys;
			ys   = ye;
			ye   = j;
		}

		y = ye - top;
		tmp0 = coord0 + y;
		tmp1 = coord1 + y;
		compare(xe, *tmp0, *tmp1);

		y = ys - top;
		tmp0 = coord0 + y;
		tmp1 = coord1 + y;

		compare(xs, *tmp0, *tmp1);

		dx = abs(xe - xs);
		dy = abs(ye - ys);

		if ((ye - ys) < 0)
			y_sign = -1;
		else
			y_sign = 1;

		x = xs;
	
		if (dy == 0) {
			*tmp0 = xs;
			*tmp1 = xe;
			continue;
		}
		
		if (dx >= dy) {
			decision = dx / 2;

			while (x <= xe) {
				if (decision >= dx) {
					decision -= dx;
					y += y_sign;
					tmp0 += y_sign;
					tmp1 += y_sign;
				}

				if (x < *tmp0)
					*tmp0 = x;

				decision += dy;
				
				if (x > *tmp1)
					*tmp1 = x;

				x++;
			}
		}
		else {
			ye -= top;
			decision = dy / 2;
			
			do {
				if (decision >= dy) {
					decision -= dy;
					x++;
				}

				if (x < *tmp0)
					*tmp0 = x;

				tmp0 += y_sign;
				
				if (x > *tmp1)
					*tmp1 = x;

				decision += dx;
				tmp1 += y_sign;
				y += y_sign;
			} while(y != ye);
			ye += top;
		}
	}

	tmp0 = coord0;
	tmp1 = coord1;

	if (use_gk)
	{
		for (i = top; i <= bottom; i++)
		{
			filter->blitLine8(*tmp0, i, *tmp1, i, color);
			tmp0++;
			tmp1++;
		}
	}
	else
	{	
		ptr = bitbase;
		ptr = ptr + (top * rowbyte);
		for (i = top; i <= bottom; i++) {
			if (*tmp0 < 0)
				*tmp0 = 0;
			if (*tmp1 > (size_x-1))
				*tmp1 = (size_x-1);
			if ((i >= 0) && (i < (size_y - 1))) { 
				hline(ptr + *tmp0, *tmp1 - *tmp0, color);
			}
	
			tmp0++;
			tmp1++;
			ptr += size_x;
		}
	}
}

/*------------------------------------------------------------*/

void T3d::clear()
{
	char	*bits;
	
	bits = bitbase;
	memset(bits, 0x00, (size_x*size_y));
}


	
