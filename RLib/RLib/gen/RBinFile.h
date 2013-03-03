/*****************************************************************

	Projet	: RLib

	Fichier	: RBinFile.h
	Partie	: file access

	Auteur	: RM
	Date	: 180100 -- updated with a RIBin
	Date	: 180298
	Date	: 090999 -- moved implementation to .cpp
	Format	: tabs==4

	Actually this is implemented over FILE * of C stdlib
	or FSSpec/DataFork for MacOS

****************************************************************/

#ifndef _H_RBINFILE_
#define _H_RBINFILE_

#include "RIBin.h"

//-------------------------------------------------------------

/** Binary File class.
	Implemented over FILE * for the C stdlib
	or FSSpec/DataFork for MacOS.
*/

//***************************
class RBinFile : public RIBin
//***************************
{
public:
				RBinFile(void);		// need to init with Open()
virtual			~RBinFile(void);	// close file if needed

		// open for reading or writing
		// writing is create-mode (not append mode)

virtual	rbool	Open(const sptr   path, EFileMode mode);
virtual	rbool	Open(const RPath &path, EFileMode mode);
virtual void	Close(void);

		// random access
		// Tell returns -1 on error.

virtual	void	Rewind(void) = 0;
virtual	rbool	Seek  (si32 position, EFileSeek mode = kFileSeekSet) = 0;
virtual	si32	Tell  (void) = 0;
virtual	rbool	Eof	  (void) = 0;
virtual	si32	Size  (void) = 0;

		// read/write sequential access

virtual	rbool	Read (char *buffer, si32 size) = 0;
virtual	rbool	Write(char *buffer, si32 size) = 0;

		// return the path name used to open this file
const	RPath & GetOpenPath(void);

		// misc utilities for the files types
		// setFileType sets the mimetype (on beos) or the
		// creator/filetype (on MacOS) of the specified file.
		// setDefaultFileType tries to setup the file type
		// depending on the file name extension.

static	rbool	SetFileType(const sptr filename,
							const sptr mimetype=NULL,
							ui32 creator=0, ui32 type=0);
static	rbool	SetFileType(RPath &path,
							const sptr mimetype=NULL,
							ui32 creator=0, ui32 type=0);

static	rbool	SetDefaultFileType(const sptr filename, const sptr ext = NULL);
static	rbool	SetDefaultFileType(RPath &path, const sptr ext = NULL);

		// Debug

virtual	void	PrintToStream(const sptr name = NULL);
		
protected:

	FILE * 			mFile;
	RPath			mPath;

	#ifdef __MAC_OS
		rbool		mDataFork;
		short		mRefNum;		// 0 is an invalid ref num
	#endif

};

//-------------------------------------------------------------


#endif // _H_RBINFILE_


/****************************************************************

	$Log: RBinFile.h,v $
	Revision 1.2  2000/04/01 21:47:33  Ralf
	Using RIBin interface
	
	Revision 1.1  2000/03/13 08:55:13  Ralf
	First revision
	
****************************************************************/

// eoh
