/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRPath.h
	Partie	: Portable Path Wrapper

	Auteur	: RM
	Date		: 010698
	Format	: tabs==2

*****************************************************************************/

#ifdef RLIB_BEOS

#ifndef _H_BRPATH_
#define _H_BRPATH_

#include "gMachine.h"
#include <Path.h>

//---------------------------------------------------------------------------


//***************************************************
class RPath : public BPath, virtual public RInterface
//***************************************************
{
public:
	RPath(void)
		: BPath() { }
	RPath(const char *dir, const char *leaf = NULL, bool normalize = false)
		: BPath(dir, leaf, normalize) { }
	RPath(const RPath &path)
		: BPath(path) { }

	virtual ~RPath(void)  { }

	inline const char *path() const { return Path(); }
	inline const char *leaf() const { return Leaf(); }
	inline si32 getParent(RPath *path) const { return (si32)GetParent(path); }

	//RPath &		operator=(const RPath &item)
	//RPath &		operator=(const char *path) 

}; // end of class defs for RPath


//---------------------------------------------------------------------------

#endif // of _H_BRPATH_

#endif // of RLIB_BEOS

// eoh
