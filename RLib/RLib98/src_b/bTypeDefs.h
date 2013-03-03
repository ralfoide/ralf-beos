/*****************************************************************************

	Projet	: Droids

	Fichier	:	bTypeDefs.h
	Partie	: Defs

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifdef RLIB_BEOS

#ifndef _H_BTYPEDEFS_
#define _H_BTYPEDEFS_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <ByteOrder.h>
#include <SupportDefs.h>

//---------------------------------------------------------------------------
// Definitions des types les plus standards.
//
// L'absence de support des entiers 64 bits est autorise. Dans quel cas on peut
// soit l'emuler avec une structure (mais l'arithmetique entiere ne marcherait plus) soit
// tout simplement ne pas le definir. Ce dernier cas, preferable imho, signifie que jusuqu'a
// preuve contraire du support universel de ce type (quid de VC++ 5.0 ?), il ne doit etre utilise
// que dans des implementations specifiques a une machine.

#define ui8		unsigned char
#define si8		  signed char
#define ui16	unsigned short int
#define si16	  signed short int
#define ui32	unsigned long  int
#define si32	  signed long  int
#define ui64	unsigned long  long
#define si64	  signed long  long
#define fp32					 float
#define fp64					 double
#define fx16					 si32

// bool, true and false sont definis dans CW/Be
// ils doivent l'etre en ANSI C++ (a voir)
// le type bool doit il etre un char (compact a stocker) ou un int (format natif du compilo) ?
// A priori, prendre un octet permet de garantir la taille du type dans divers implementations.

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

// PFV : Pointer on Function returning Void -- definit dans Be/SupportDefs.h

#ifndef PFV
	#ifdef B_PFV
		#define PFV B_PFV
	#else
		typedef void (*PFV)(void);
	#endif
#endif

// strupr : utilise entre autre par CTokenisor, n'est pas une fct ANSI C universelle

#if !defined(strupr)
	inline void strupr(char *s) {for(;*s;s++) *s = toupper((*s));}
#endif

//---------------------------------------------------------------------------
// Big/Little Endian swaping to native mode

#define M_TO_BIG_UINT32(x) 		B_HOST_TO_BENDIAN_INT32(x)
#define M_FROM_BIG_UINT32(x) 	B_BENDIAN_TO_HOST_INT32(x)

//---------------------------------------------------------------------------
// some base math stuff

#define K_EPSILON	(1e-6)

//---------------------------------------------------------------------------

#endif // of _H_BTYPEDEFS_

#endif // of RLIB_BEOS

// eoh
