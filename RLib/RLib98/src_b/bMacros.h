/*****************************************************************************

	Projet	: Portable Lib

	Fichier	: macros.h
	Partie	: Portable / Macros

	Auteur	: RM
	Date		: 150298 -- 050398
	Format	: tabs==2

*****************************************************************************/


#ifdef RLIB_BEOS

#ifndef _H_CMACROS_
#define _H_CMACROS_

#include "gMachine.h"
#include "gTypeDefs.h"
#include "RErrEx.h"

//---------------------------------------------------------------------------

void gDebugSetup(bool condition);
extern bool _gDebug;

#define debug _gDebug
#define set_debug() gDebugSetup(true)

//---------------------------------------------------------------------------

#ifdef __cplusplus

	#ifdef min
		#undef min
	#endif
	#ifdef max
		#undef max
	#endif

	// derived from C++ efficace, Scott Meyers, p. 15
	template<class Ta, class Tb> inline Ta max(Ta a, Tb b) { return (a > b) ? a : b; }
	template<class Ta, class Tb> inline Ta min(Ta a, Tb b) { return (a < b) ? a : b; }

#else

	#ifndef min
		#define min(a,b) ((a)<(b)?(a):(b))
	#endif
	#ifndef max
		#define max(a,b) ((a)>(b)?(a):(b))
	#endif

#endif

//---------------------------------------------------------------------------

#define M_ASSERT_PTR(x) if (!(x)) throw(RErrEx( # x , __FILE__, __LINE__))
#define M_ASSERT(x) if (!(x)) throw(RErrEx( # x , __FILE__, __LINE__))

// #define M_ASSERT_PTR(x) if (!x) do { char s[1024]; sprintf(s, "PTR NULL !\nProbable malloc error\n" \
//										"File %s, Line %d\nOffending ptr :" #x, __FILE__, __LINE__); throw(s); } while(0)


//---------------------------------------------------------------------------
// RTTI macros a-la-Be

#define M_CLASS_NAME(    ptr       )	((typeid(*(ptr))).name())
#define M_CAST_AS(       ptr, class)	(dynamic_cast<class*>(ptr))
#define M_IS_KIND_OF(    ptr, class)	(M_CAST_AS(ptr, class) != 0)
#define M_IS_INSTANCE_OF(ptr, class)	(typeid(*(ptr)) == typeid(class))

//---------------------------------------------------------------------------
// typical handler for exception at end level
// this should map C++ un-classed routines (to have a more compact code herein)

extern void gThrowAlertStatusT(const char *name, const char *classname, const char *file, si32 line, status_t err);
extern void gThrowAlertCharPtr(const char *name, const char *classname, const char *file, si32 line, char *s);
extern void gThrowAlertRErrEx (const char *name, const char *classname, const char *file, si32 line, RErrEx &ex);
extern void gThrowAlertUntyped(const char *name, const char *classname, const char *file, si32 line);

#include <typeinfo>

#define M_CATCH_STATIC(name) 																				\
	catch(status_t __err)	{ gThrowAlertStatusT(name, "(static)", __FILE__, __LINE__, __err);	} \
	catch(char *__s)			{ gThrowAlertCharPtr(name, "(static)", __FILE__, __LINE__, __s);		} \
	catch(RErrEx &__ex)		{ gThrowAlertRErrEx (name, "(static)", __FILE__, __LINE__, __ex);		} \
	catch(RErrEx *__ex)		{ gThrowAlertRErrEx (name, "(static)", __FILE__, __LINE__, *__ex); delete __ex;	} \
	catch(...)						{ gThrowAlertUntyped(name, "(static)", __FILE__, __LINE__);					}

#define M_CATCH(name) 																				\
	catch(status_t __err)	{ gThrowAlertStatusT(name, M_CLASS_NAME(this), __FILE__, __LINE__, __err);	} \
	catch(char *__s)			{ gThrowAlertCharPtr(name, M_CLASS_NAME(this), __FILE__, __LINE__, __s);		} \
	catch(RErrEx &__ex)		{ gThrowAlertRErrEx (name, M_CLASS_NAME(this), __FILE__, __LINE__, __ex);		} \
	catch(RErrEx *__ex)		{ gThrowAlertRErrEx (name, M_CLASS_NAME(this), __FILE__, __LINE__, *__ex); delete __ex;	} \
	catch(...)						{ gThrowAlertUntyped(name, M_CLASS_NAME(this), __FILE__, __LINE__);					}

#define M_CATCH_RETHROW(name) 																			 \
	catch(status_t __err)	{ gThrowAlertStatusT(name, M_CLASS_NAME(this), __FILE__, __LINE__, __err);	throw; } \
	catch(char *__s)			{ gThrowAlertCharPtr(name, M_CLASS_NAME(this), __FILE__, __LINE__, __s);		throw; } \
	catch(RErrEx &__ex)		{ gThrowAlertRErrEx (name, M_CLASS_NAME(this), __FILE__, __LINE__, __ex);		throw; } \
	catch(RErrEx *__ex)		{ gThrowAlertRErrEx (name, M_CLASS_NAME(this), __FILE__, __LINE__, *__ex); delete __ex;	throw; } \
	catch(...)						{ gThrowAlertUntyped(name, M_CLASS_NAME(this), __FILE__, __LINE__);					throw; }

#define M_THROW(kerrcode) throw new RErrEx(kerrcode, # kerrcode)
//#define M_THROW(kerrcode) throw new RErrEx(# kerrcode, __FILE__, __LINE__)


//---------------------------------------------------------------------------

#endif // of _H_CMACROS_

#endif // of RLIB_BEOS

// eoh
