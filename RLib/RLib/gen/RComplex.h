/*****************************************************************************

	Projet	: RLib

	Fichier	: RComplex.h
	Partie	: core complex (simplified)

	Auteur	: RM
	Date	: 061899
	Format	: tabs==4

*****************************************************************************/


#ifndef _H_RCOMPLEX
#define _H_RCOMPLEX

#include <math.h>
#include <stdio.h>

//------------------------------------------------------------

/// A complex, using a template for the basic storage definition (fp32 or fp64)

//********************************
template <class FP> class TComplex
//********************************
{
public:

	FP re;
	FP im;

	// ----

	TComplex(void)					{ re = 0.; im = 0.;		}
	TComplex(FP r, FP i)			{ re = r; im = i;		}
	TComplex(const TComplex<FP> &c)	{ re = c.re; im = c.im; }

	TComplex<FP>	&operator=(const TComplex<FP> &c)	{ re = c.re; im = c.im; return *this; }
	void			set(FP r, FP i)	 { re = r; im = i; }

	void			printToStream(void) const { printf("TComplex : re = %f, im = %f\n", re, im); }

	TComplex<FP>	operator+ (const TComplex<FP>& s) const	{ return TComplex<FP>(re+s.re, im+s.im); }
	TComplex<FP>	operator- (const TComplex<FP>& s) const	{ return TComplex<FP>(re-s.re, im-s.im); }
	TComplex<FP>	operator* (const TComplex<FP>& s) const	{ return TComplex<FP>(re*s.re - im*s.im, re*s.im + im*s.re); }
	TComplex<FP>	operator* (const FP			   n) const	{ return TComplex<FP>(re*n, im*n);		}
	TComplex<FP>&	operator+=(const TComplex<FP>& s) 		{ re += s.re; im += s.im; return *this; }
	TComplex<FP>&	operator-=(const TComplex<FP>& s) 		{ re -= s.re; im -= s.im; return *this; }
	TComplex<FP>&	operator*=(const TComplex<FP>& s) 		{ FP r2 = re*s.re - im*s.im; im=re*s.im + im*s.re; re=r2; return *this; }
	TComplex<FP>&	operator*=(const FP			   n) 		{ re *= n; im *= n; return *this; }

	rbool			operator!=(const TComplex<FP>& c) const { return !(this->operator==(c)); }
	rbool			operator==(const TComplex<FP>& c) const { return (fabs(re-c.re) < K_EPSILON && fabs(im-c.im) < K_EPSILON); }

	FP				module(void) const		{ return sqrt(re*re + im*im); }
	FP				operator!(void) const	{ return module(); }
};


//------------------------------------------------------------
// Usages of TComplex<FP> are designed for fp32 or fp64.
// Say default is for fp32.

/// A float (fp32) complex

typedef TComplex<fp32> RComplex32;

/// A double (fp64) complex

typedef TComplex<fp64> RComplex64;

/// The default complex data types is a fp32-based complex

typedef RComplex32	   RComplex;



//------------------------------------------------------------

#endif // _H_RCOMPLEX

//------------------------------------------------------------

/****************************************************************

	$Log: RComplex.h,v $
	Revision 1.1  2000/03/13 08:55:13  Ralf
	First revision
	
****************************************************************/

//eoh
