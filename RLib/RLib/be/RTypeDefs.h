/*****************************************************************************

	Projet	: RLib

	Fichier	: RTypeDefs.h
	Partie	: Defs

	Auteur	: RM
	Format	: tabs==4

*****************************************************************************/

#ifdef RLIB_BEOS

#ifndef _H_RTYPEDEFS_
#define _H_RTYPEDEFS_

#include "PP_Be.h"

#ifdef _DEBUG
	#define DEBUG
#endif

//---------------------------------------------------------------------------
// adjust warning for the compiler


//---------------------------------------------------------------------------
// Definitions des types les plus standards.
//
// L'absence de support des entiers 64 bits est autorise. Dans quel cas on peut
// soit l'emuler avec une structure (mais l'arithmetique entiere ne marcherait plus) soit
// tout simplement ne pas le definir. Ce dernier cas, preferable imho, signifie que jusqu'a
// preuve contraire du support universel de ce type (quid de VC++ 5.0 ?), il ne doit etre
// utilise que dans des implementations specifiques a une machine.

#define ui8  unsigned char
#define si8    signed char
#define ui16 unsigned short int
#define si16   signed short int
#define ui32 unsigned long  int
#define si32   signed long  int
#define ui64 unsigned long  long
#define si64   signed long  long
#define fp32          float
#define fp64          double
#define fx16          si32

// bool, true and false sont definis dans CW/Be
// ils doivent l'etre en ANSI C++ (a voir)
// le type bool doit il etre un char (compact a stocker) ou un int (format natif du compilo) ?
// A priori, prendre un octet permet de garantir la taille du type dans diverses
// implementations mais en contrepartie cela ralenti probablement l'acces a certaines
// structure pour cause d'alignment non natif.
// Dans l'ideal, ce devrait etre un int, mais stocke sur un seul octet (cf RFlatBlock, TBDL)

#define rbool ui8

#ifndef true
	#define true 1
#endif

#ifndef false
	#define false 0
#endif

// format pour les chaines de caracteres et pointeur void
// Suppose qu'un 'char' est signe par defaut, l'absence de signed/unsigned simplifant
// la cross-compilations pour tout ce qui est stdio en general.

#define vptr void *
#define sptr char *

// PFV : Pointer on Function returning Void -- defini dans Be/SupportDefs.h

#ifndef PFV
	typedef void (*PFV)(void);
#endif

// strupr : utilise entre autre par CTokenisor, n'est pas une fct ANSI C universelle
//
//#if !defined(strupr)
//	inline void strupr(char *s) {for(;*s;s++) *s = toupper((*s));}
//#endif

//---------------------------------------------------------------------------
// Big/Little Endian swaping to native mode
// WIN32 : Host is always Little-Endian (always swap ;-) ?)

inline ui16 rlib_swap_ui16(ui16 x) { return ((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8); };
inline ui32 rlib_swap_ui32(ui32 x) { return ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | ((x & 0xFF) << 24); };


#define M_TO_BIG_UI16(x) 	rlib_swap_ui16(x)
#define M_TO_BIG_UI32(x) 	rlib_swap_ui32(x)
#define M_FROM_BIG_UI16(x) 	rlib_swap_ui16(x)
#define M_FROM_BIG_UI32(x) 	rlib_swap_ui32(x)


#ifdef __INTEL__
	#define M_SWAP_LE_TO_HOST16(x)	/* nop */
	#define M_SWAP_BE_TO_HOST16(x)	rlib_swap_ui16(x)
	#define M_SWAP_HOST_TO_LE16(x)	/* nop */
	#define M_SWAP_HOST_TO_BE16(x)	rlib_swap_ui16(x)

	#define M_SWAP_LE_TO_HOST32(x)	/* nop */
	#define M_SWAP_BE_TO_HOST32(x)	rlib_swap_ui32(x)
	#define M_SWAP_HOST_TO_LE32(x)	/* nop */
	#define M_SWAP_HOST_TO_BE32(x)	rlib_swap_ui32(x)
#else
	#define M_SWAP_LE_TO_HOST16(x)	rlib_swap_ui16(x)
	#define M_SWAP_BE_TO_HOST16(x)	/* nop */
	#define M_SWAP_HOST_TO_LE16(x)	rlib_swap_ui16(x)
	#define M_SWAP_HOST_TO_BE16(x)	/* nop */

	#define M_SWAP_LE_TO_HOST32(x)	rlib_swap_ui32(x)
	#define M_SWAP_BE_TO_HOST32(x)	/* nop */
	#define M_SWAP_HOST_TO_LE32(x)	rlib_swap_ui32(x)
	#define M_SWAP_HOST_TO_BE32(x)	/* nop */
#endif

//---------------------------------------------------------------------------
// some base math stuff

#define K_EPSILON	(1e-6)

//---------------------------------------------------------------------------

#endif // of _H_RTYPEDEFS_
#endif // of RLIB_BEOS


/****************************************************************

	$Log: RTypeDefs.h,v $
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eoh
