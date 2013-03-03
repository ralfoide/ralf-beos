/*****************************************************************************

	Projet	: RLib

	Fichier	: RMacros.h
	Partie	: Portable / Macros

	Auteur	: RM
	Date	: 250798 (Win32)
	Date	: 150298 -- 050398
	Format	: tabs==4

*****************************************************************************/


#ifdef RLIB_BEOS

#ifndef _H_RMACROS_
#define _H_RMACROS_

#include "RErrEx.h"

#include <typeinfo>		// for RTTI type_info stuff

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

#define M_ASSERT(x)		if (!(x)) throw(RErrEx( # x , __FILE__, __LINE__))
#define M_ASSERT_PTR(x) if (!(x)) throw(RErrEx( # x "\nCause : Ptr NULL or malloc error !" , __FILE__, __LINE__))

#define M_VERIFY(x)	M_ASSERT(x)
#define M_VERIFY_PTR(x)	M_ASSERT_PTR(x)

// #define M_ASSERT_PTR(x) if (!x) do { char s[1024]; sprintf(s, "PTR NULL !\nProbable malloc error\n" \
//	"File %s, Line %d\nOffending ptr :" #x, __FILE__, __LINE__); throw(s); } while(0)


//---------------------------------------------------------------------------
// typical handler for exception at end level
// this should map C++ un-classed routines (to have a more compact code herein)

extern void gThrowAlertInt    (const char *name, int err);
extern void gThrowAlertCharPtr(const char *name, char *s);
extern void gThrowAlertRErrEx (const char *name, RErrEx &ex);
extern void gThrowAlertUntyped(const char *name);

// extern void gThrowAlertStatusT(const char *name, status_t err); -- BeOS Specific

/**@name Exception Handling */
//@{

/// Catches an exception and display it to the user.

#define M_CATCH(name) 														  \
	catch(int __err)		{ gThrowAlertInt(name, __err);					} \
	catch(char *__s)		{ gThrowAlertCharPtr(name, __s);				} \
	catch(const char *__s)	{ gThrowAlertCharPtr(name, (char *)__s);		} \
	catch(status_t __err)	{ gThrowAlertStatusT(name, __err);				} \
	catch(RErrEx &__ex)		{ gThrowAlertRErrEx (name, __ex);				} \
	catch(RErrEx *__ex)		{ gThrowAlertRErrEx (name, *__ex); delete __ex;	} \
	catch(...)				{ gThrowAlertUntyped(name);						}

//	catch(CString *__s)		{ gThrowAlertCharPtr(name, (char *)((LPCTSTR) *__s)); delete __s; } \
//	-- Win32 Specific

/// Catches an exception, displays it to the user and rethrows it to the upper scope.

#define M_CATCH_RETHROW(name) 														 \
	catch(int __err)		{ gThrowAlertInt(name, __err);					throw; } \
	catch(char *__s)		{ gThrowAlertCharPtr(name, __s);				throw; } \
	catch(const char *__s)	{ gThrowAlertCharPtr(name, (char *)__s);		throw; } \
	catch(status_t __err)	{ gThrowAlertStatusT(name, __err);	throw; 			   } \
	catch(RErrEx &__ex)		{ gThrowAlertRErrEx (name, __ex);				throw; } \
	catch(RErrEx *__ex)		{ gThrowAlertRErrEx (name, *__ex); delete __ex;	throw; } \
	catch(...)				{ gThrowAlertUntyped(name);						throw; }

//	catch(CString *__s)		{ gThrowAlertCharPtr(name, (char *)((LPCTSTR) *__s)); delete __s; throw; } \
//	 -- BeOS Specific

/// Throws an RErrEx exception based on some kErrXXX code.

#define M_THROW(kerrcode) throw RErrEx(kerrcode, # kerrcode)

#define M_THROW_STR(s) throw RErrEx(s)

#define M_THROW_ERR(s) throw new RErrEx(s, __FILE__, __LINE__)

/// Win32 specific macro to throw an exception based on GetLastError(), but only in cases of failure.
/*
#define M_THROW_GETLASTERROR()														\
	do { char* lpMsgBuf; DWORD error = GetLastError();								\
	if (error != ERROR_SUCCESS)														\
	{	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, \
				NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),				\
				(LPTSTR) &lpMsgBuf, 0, NULL );										\
		CString *tempo = new CString;												\
		tempo->Format("GetLastError %d = '%s'", error, lpMsgBuf);					\
		LocalFree( lpMsgBuf ); throw((LPCTSTR)tempo); } } while(0)

//@}
*/
//---------------------------------------------------------------------------
// RTTI macros a-la-Be

/**@name RTTI Handling */
//@{

/// Gets the class name of a given instance.

#define M_CLASS_NAME(    ptr       )	((typeid(*(ptr))).name())

/// Casts a pointer into a given class or returns NULL.

#define M_CAST_AS(       ptr, class)	(dynamic_cast<class*>(ptr))

///	Indicates if the instance pointer is or derived from a given class.

#define M_IS_KIND_OF(    ptr, class)	(M_CAST_AS(ptr, class) != 0)

/// Indicates if the instance pointer is exactly an instance of a given class.

#define M_IS_INSTANCE_OF(ptr, class)	(typeid(*(ptr)) == typeid(class))

//@}

//---------------------------------------------------------------------------

#endif // of _H_RMACROS_
#endif // of RLIB_BEOS


/****************************************************************

	$Log: RMacros.h,v $
	Revision 1.2  2000/04/01 21:52:31  Ralf
	New M_THROW_STR and M_THROW_ERR
	
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eoh
