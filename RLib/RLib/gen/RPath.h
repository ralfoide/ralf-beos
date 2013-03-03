/*****************************************************************************

	Projet	: RLib

	Fichier	: RPath.h
	Partie	: Portable Path Wrapper

	Auteur	: RM
	Date	: 011800 Merged all platforms in the generic file
	Date	: 061499 (MacOS)
	Date	: 300798 (Win32)
	Date	: 010698
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_RPATH_
#define _H_RPATH_


#include "RString.h"


//--------------------------------------------------------------------
//------------------------   BeOS version  ---------------------------
//--------------------------------------------------------------------

#if defined(RLIB_BEOS)

#include <Path.h>

// the standard bunch of drive and directory separators, for a Unix-like mode
// these defines are to be a little platform specific in the code
// but they are not yet always used the correct way in the real code.

#define RPATH_USES_DRIVE_LETTER 0		// 1 for DOS, 0 for everything else :-)
#define RPATH_DRIVE_SEP			''		// empty for non-DOS
#define RPATH_DRIVE_SEP_STR		""		// empty for non-DOS
#define RPATH_DIR_SEP			'/'		// '\\' for DOS, '/' for Unix, ':' for MacOS
#define RPATH_DIR_SEP_STR		"/"		// '\\' for DOS, '/' for Unix, ':' for MacOS
#define	RPATH_DEFAULT_VALUE		RPATH_DRIVE_SEP_STR


/// A Portable Path Wrapper

//************************
class RPath : public BPath
//************************
{
public:
		RPath(void)				 : BPath() { }
		RPath(const sptr dir, const sptr leaf = NULL, rbool normalize = false)
								 : BPath(dir, leaf, normalize) { }
		RPath(const RPath &path) : BPath(path) { }

virtual	~RPath(void) { }

		si32 GetParent(RPath& path) const	{ return (si32)BPath::GetParent(&path); }

	/*	These are deactivated, since they obviously do exactly the same thing than
		their BPath counterpart !

const	sptr Path(void) const 				{ return BPath::Path(); }
const	sptr Leaf(void) const 				{ return BPath::Leaf(); }
		RPath &	operator=(const RPath &item){ *this = item.Path(); return *this; }
		RPath &	operator=(const sptr  path)	{ *this = path; return *this; }
	*/

	// all other BPath calls are not supposedly portables

}; // end of class defs for RPath


//--------------------------------------------------------------------
//------------------------  MacOS version  ---------------------------
//--------------------------------------------------------------------

#elif defined(RLIB_MACOS)

#include <Files.h>	// for FSSpec

// the standard bunch of drive and directory separators, for a MacOS-like mode

#define RPATH_USES_DRIVE_LETTER 0		// 1 for DOS, 0 for everything else :-)
#define RPATH_DRIVE_SEP			':'		// empty for non-DOS
#define RPATH_DRIVE_SEP_STR		":"		// empty for non-DOS
#define RPATH_DIR_SEP			':'		// '\\' for DOS, '/' for Unix, ':' for MacOS
#define RPATH_DIR_SEP_STR		":"		// '\\' for DOS, '/' for Unix, ':' for MacOS
#define	RPATH_DEFAULT_VALUE		RPATH_DRIVE_SEP_STR


//*********
class RPath
//*********
{
public:
		RPath(void);
		RPath(const sptr dir, const sptr leaf = NULL, rbool normalize = false);
		RPath(const RPath &path);

		~RPath(void)  { }		// destructor is explicitly not virtual

		// --- portable & universal methods to handle pathnames ---

		RPath& operator=(const RPath &path);
		RPath& operator=(const sptr dir)	{ affect(dir); mUseFSSpec = false; return *this; }

const	RString& PathString(void)	{ return (const RString&) mFullPathLeaf;	}
const	sptr	 Path(void) const	{ return (const sptr)((sptr)mFullPathLeaf); }
const	sptr	 Leaf(void)			{ return (const sptr)getLeaf();				}
		void  	 GetParent(RPath& path);

		// --- MacOS specific methods ---
		// methods return an si32 return kErr, kErrFileNotFound or kNoErr.

				RPath(FSSpec &spec)								{ SetTo(spec);								}
				RPath(short ref, long dir, ConstStr255Param f)	{ SetTo(ref, dir, f); 						}
				RPath& operator=(const FSSpec &spec)			{ SetTo(*((FSSpec *)&spec)); return *this;	}
		rbool	HasFSSpec(void) const							{ return mUseFSSpec; 						}
const	FSSpec &GetFSSpec(void)	const							{ M_ASSERT(mUseFSSpec); return mFSSpec;		}

		si32	SetTo(FSSpec &spec);
		si32	SetTo(short vRefNum, long dirId, ConstStr255Param fileName);

		// --- common routines ---
		// needed to take care of specific FSSpec internal actions
		// but that also work on pathnames

		// set this path to the filepath of the given filename into the current path
		// (i.e. choose a file in a directory)
	
		si32	SetToFile(const sptr leaf);

		// --- misc ---

		void	PrintToStream(void);

protected:

		rbool	mUseFSSpec;
		FSSpec	mFSSpec;

		RString	mFullPathLeaf;

		sptr	getLeaf(void);
		void	affect(const sptr dir, const sptr leaf = NULL, rbool normalize = false);

}; // end of class defs for RPath


//--------------------------------------------------------------------
//------------------------  Win32 version  ---------------------------
//--------------------------------------------------------------------
// Windows handling. This is the same than MacOS code but
// without the FSSpec-handling routines.


#elif defined(RLIB_WIN32)

// the standard bunch of drive and directory separators, for a MacOS-like mode

#define RPATH_USES_DRIVE_LETTER 1		// 1 for DOS, 0 for everything else :-)
#define RPATH_DRIVE_SEP			':'		// empty for non-DOS
#define RPATH_DRIVE_SEP_STR		":"		// empty for non-DOS
#define RPATH_DIR_SEP			'\\'	// '\\' for DOS, '/' for Unix, ':' for MacOS
#define RPATH_DIR_SEP_STR		"\\"	// '\\' for DOS, '/' for Unix, ':' for MacOS
#define	RPATH_DEFAULT_VALUE		"C" RPATH_DRIVE_SEP_STR RPATH_DIR_SEP_STR

//*********
class RPath
//*********
{
public:
		RPath(void);
		RPath(const sptr dir, const sptr leaf = NULL, rbool normalize = false);
		RPath(const RPath &path);

		~RPath(void)  { }		// destructor is explicitly not virtual

		RPath& operator=(const RPath &path);
		RPath& operator=(const sptr dir) { affect(dir); return *this; }

		// --- methods handling pathnames ---

const	RString& PathString(void)		{ return (const RString&) mFullPathLeaf;		}
const	sptr	 Path(void) const		{ return (const sptr)((sptr)mFullPathLeaf); 	}
const	sptr	 Leaf(void)				{ return (const sptr)getLeaf();					}
		void	 GetParent(RPath& path);

		void	 PrintToStream(void);

protected:

	RString mFullPathLeaf;
	sptr	getLeaf(void);
	void	affect(const sptr dir, const sptr leaf = NULL, rbool normalize = false);

}; // end of class defs for RPath

#endif // RLIB_WIN32


//------------------------------------------------------------

#endif // of _H_RPATH_

/*****************************************************************************

	$Log: RPath.h,v $
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
*****************************************************************************/

// eof
