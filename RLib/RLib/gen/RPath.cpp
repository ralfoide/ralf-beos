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

#include "RLib.h"
#include "RPath.h"

#if defined(RLIB_WIN32) || defined(RLIB_MACOS)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef RLIB_MACOS
	#include "FullPath.h"		// for FSSpec handling
	#include "FSpCompat.h"		// from the MoreFile Lib (Apple Dev's)
	#include "MoreFilesExtras.h"
#endif


//------------------------------------------------------------
// Win32/MacOS implementation
// There is no BeOS implementation, it's just a BPath wrapper


//***************************************
RPath::RPath(void)
	  :mFullPathLeaf(RPATH_DEFAULT_VALUE)
//***************************************
//
// Simplified implementation for the Win32/DOS World
{
	#ifdef RLIB_MACOS
		mUseFSSpec = false;
	#endif
} // end of empty constructor for RPath


//************************************************************
RPath::RPath(const sptr dir, const sptr leaf, rbool normalize)
//************************************************************
//
// Simplified implementation for the Win32/DOS World
{
	#ifdef RLIB_MACOS
		mUseFSSpec = false;
	#endif
	affect(dir, leaf, normalize);
} // end of full param constructor for RPath


//*****************************
RPath::RPath(const RPath &path)
//*****************************
//
// Simplified implementation for the Win32/DOS World
{
	#ifdef RLIB_MACOS
		mUseFSSpec = path.mUseFSSpec;
		mFSSpec = path.mFSSpec;
	#endif
	mFullPathLeaf = path.mFullPathLeaf;
} // end of full param constructor for RPath


//****************************************
RPath &RPath::operator=(const RPath &path)
//****************************************
{
	#ifdef RLIB_MACOS
		mUseFSSpec = path.mUseFSSpec;
		mFSSpec = path.mFSSpec;
	#endif
	mFullPathLeaf = path.mFullPathLeaf;
	return *this;
} // end of operator= for RPath



//---------------------------------------------------------------------------
#pragma mark -


//******************************************************************
void RPath::affect(const sptr dir, const sptr leaf, rbool normalize)
//******************************************************************
//
// Simplified implementation for the Win32/DOS World
{
	mFullPathLeaf = dir;

	#ifdef RLIB_MACOS
		ThrowIf_(normalize);
	#else
		M_ASSERT(!normalize);
	#endif

	if (!dir) mFullPathLeaf = RPATH_DEFAULT_VALUE;
	if (leaf)
	{
		si32 len = mFullPathLeaf.Len();
		if (len && (mFullPathLeaf.Data())[len-1] != RPATH_DIR_SEP)
			mFullPathLeaf += RPATH_DIR_SEP_STR;
		mFullPathLeaf += leaf;
	}
} // end of affect for RPath



//***********************
sptr RPath::getLeaf(void)
//***********************
//
// Simplified implementation for the Win32/DOS World
{
	sptr s = mFullPathLeaf.Data();
	ui32 len = mFullPathLeaf.Len();

	// return either NULL or ptr on '\0' if string is empty
	if (!len || !s) return s;

	sptr s1 = s+len-1;
	while(s1 > s && *s1 != RPATH_DIR_SEP) s1--;
	return s1+1;

} // end of getLeaf for RPath


//********************************
void RPath::GetParent(RPath& path)
//********************************
//
// Simplified implementation for the Win32/DOS World
// TBDL RM 071399 : not yet adapted to FSSpec-based stuff
{
	sptr s = mFullPathLeaf.Data();
	ui32 len = mFullPathLeaf.Len();

	// override with default path
	path.mFullPathLeaf = RPATH_DEFAULT_VALUE;

	// don't continue if string is empty
	if (!len || !s) return;

	// get leaf start ptr
	// parent is everything before this
	sptr leaf = getLeaf();

	// make sure leaf ptr is valid
	if (leaf < s || leaf > s+len-1) return;

	// create parent string
	RString parent(mFullPathLeaf);

	// cut it to the correct length
	parent.Cut((ui32)(leaf-s));

	path.mFullPathLeaf = parent;

} // end of GetParent for RPath


//---------------------------------------------------------------------------
#pragma mark -


#ifdef RLIB_MACOS

//****************************************
si32 RPath::SetToFile(const sptr filename)
//****************************************
// set this path to the filepath of the given filename into the current path
// works with both FSSpec and pathname
{
	if (mUseFSSpec)
	{
		OSErr		err;
		long		dirID;
		Boolean		isDir;
		RString		pname(filename);

		err = FSpGetDirectoryID(&mFSSpec, &dirID, &isDir);
		if (err != noErr) return kErr;

		return SetTo(mFSSpec.vRefNum, dirID, (StringPtr)pname);
	}
	else
	{
		RString temp(mFullPathLeaf);
		affect(temp, filename);
	}
	return kNoErr;
}

#endif


//---------------------------------------------------------------------------
#pragma mark -


//*****************************
void RPath::PrintToStream(void)
//*****************************
{
	sptr s = mFullPathLeaf.Data();
	ui32 len = mFullPathLeaf.Len();

	printf("RPath <%p>(%d) = 's'\n", this, len, s);
	printf("     leaf = '%s'\n", getLeaf());

} // end of PrintToStream for RPath


//---------------------------------------------------------------------------
#pragma mark -

#ifdef RLIB_MACOS		// ------- MacOS Specifics begin here ---------------


//*****************************
si32 RPath::SetTo(FSSpec &spec)
//*****************************
{
	short fullPathLength = 0;
	Handle fullPath = NULL;

	mUseFSSpec = false;

	if (&mFSSpec != &spec)
		memcpy(&mFSSpec, &spec, sizeof(spec));

	OSErr err = FSpGetFullPath(&mFSSpec, &fullPathLength, &fullPath);
	if ((err == noErr || err == fnfErr) && fullPath && fullPathLength>0)
	{
		mUseFSSpec = true;
		HLock(fullPath);
		mFullPathLeaf.SetData(*fullPath, fullPathLength);
		HUnlock(fullPath);
		DisposeHandle(fullPath);
	}

	if (err == fnfErr)		return kErrFileNotFound;
	else if (err == noErr)	return kNoErr;
	mFullPathLeaf = RPATH_DEFAULT_VALUE;
	return kErr;
}


//*********************************************************************
si32 RPATH::SetTo(short vRefNum, long dirID, ConstStr255Param fileName)
//*********************************************************************
{
	OSErr err;
	err = FSMakeFSSpecCompat(vRefNum, dirID, fileName, &mFSSpec);
	if (err == noErr || err == fnfErr) return SetTo(mFSSpec);
	return kErr;
}


#endif // RLIB_MACOS	// ------- MacOS Specifics end here -----------------

//------------------------------------------------------------

#endif // if defined(RLIB_WIN32) || defined(RLIB_MACOS)

/*****************************************************************************

	$Log: RPath.cpp,v $
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
*****************************************************************************/

// eof
