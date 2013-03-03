

#ifndef	TSB
#define	TSB

#include "CFilter.h"

/*------------------------------------------------------------*/

class	T3d
{
public:
		char 		*bitbase;
		long		bitbpr;
		long		*poly_scratch;
		long		size_x;
		long		size_y;
		CFilter	*filter;
		bool		use_gk;
		

		T3d(long size_x, long size_y);
		virtual ~T3d();

		// info from prepare in CFilter for the addon
		void		setFrame(char *base, long bpr) { bitbase = base; bitbpr = bpr; }
		void 		useGk(bool _usegk, CFilter *_filter) { use_gk = _usegk; filter = _filter; }

		void		fill_triangle(BPoint pt_a, BPoint pt_b, BPoint pt_c, ulong color);
		void		fill_4(BPoint pt_a, BPoint pt_b, BPoint pt_c, BPoint pt_d, ulong color);
		void		clear();
};

/*------------------------------------------------------------*/

#ifndef	SHAPE_H
#define	SHAPE_H
#endif



/*------------------------------------------------------------*/

#define		MAX_POINT	5000
#define		MAX_S		5000

/*------------------------------------------------------------*/


class	TShape {
public:
		T3d			*owner;
		long		num_point;
		long		num_poly;

		long		spt_x[MAX_POINT];
		long		spt_y[MAX_POINT];

		long		pt_x[MAX_POINT];
		long		pt_y[MAX_POINT];
		long		pt_z[MAX_POINT];

		long		screen_x[MAX_POINT];
		long		screen_y[MAX_POINT];
		long		screen_z[MAX_POINT];

		long		l1[MAX_S];	
		long		l2[MAX_S];	
		long		l3[MAX_S];	
		long		l4[MAX_S];	
		rgb_color	acolor[MAX_S];
		long		zs[MAX_S];

		float		cur_alpha;
		float		cur_delta;
		float		cur_zeta;

					TShape(T3d *aowner);
virtual				~TShape();
		void		change_view_point(float new_alpha, float new_delta, float new_zeta);
		int			add_point(long x, long y, long z);
		void		add_poly(long p1, long p2, long p3, long p4, rgb_color c);
		void		add_triangle(long p1, long p2, long p3, rgb_color co);
		long		calc_mid(long pn);
		void		sort_polys();
		void		draw(float new_alpha, float new_delta, float new_zeta, float pos_x, float pos_y);
		long		calc_color(long i, rgb_color c);
};


#endif


