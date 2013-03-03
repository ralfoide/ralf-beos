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

#include "RLib.h"
#include "RBinFile.h"

#ifdef 	RLIB_BEOS
	#include <NodeInfo.h>
	#include <Mime.h>
#endif

#ifdef RLIB_MACOS
	#include <Files.h>
	#include "FullPath.h"
	#include "MoreFilesExtras.h"
	#include "UInternetConfig.h"	// for PP_ICMapFilename
#endif

#include <stdarg.h>
#include <stdio.h>

//-------------------------------------------------------------


//**********************
RBinFile::RBinFile(void)
//**********************
{
	mMode = kFileInvalidMode;
	mFile = NULL;

	// note that 0 is an invalid ref num number (-1 is valid for some fs items)
	#ifdef RLIB_MACOS
		mDataFork = false;
		mRefNum = 0;
	#endif
}


//***********************
RBinFile::~RBinFile(void)
//***********************
{
	Close();
}


//***************************************************
rbool RBinFile::Open(const sptr path, EFileMode mode)
//***************************************************
{
	sptr s = (mode == kFileReadOnly ? "rb" :
				(mode == kFileWriteOnly ? "wb" :
					(mode == kFileReadWrite ? "w+b" :
						NULL)));

	if (!path || !s) return false;

	// close file before reopen it
	Close();

	mMode = mode;
	mFile = fopen(path, s);
	mPath = path;
	return (mFile != NULL);
}


//*****************************************************
rbool RBinFile::Open(const RPath &path, EFileMode mode)
//*****************************************************
{
	#ifdef RLIB_MACOS
		if (path.HasFSSpec())
		{
			// close file before reopen it
			Close();

			OSErr err;
			char perm = (mode == kFileReadOnly ? fsRdPerm :
							(mode == kFileWriteOnly ? fsWrPerm :
								(mode == kFileReadWrite ? fsRdWrPerm :
									-1)));
			if (perm == -1) return false;

			const FSSpec &spec = path.GetFSSpec();
			err = ::FSpOpenDF(&spec, perm, &mRefNum);
			if (err == fnfErr && (mode == kFileWriteOnly || mode == kFileReadWrite))
			{
				err = ::FSpCreate(&spec, '????', '????', smSystemScript);
				if (err == noErr)
					err = ::FSpOpenDF(&spec, perm, &mRefNum);
			}
			mDataFork = (err == noErr);
			mPath = path;
			if (mDataFork) return true;
			mRefNum = 0;
		}
	#endif
	
	return Open(path.Path(), mode);
}


//************************
void RBinFile::Close(void)
//************************
{
	// close file if needed
	#ifdef RLIB_MACOS
		if (mDataFork)
		{
			::FSClose(mRefNum);
			mDataFork = false;
			mRefNum = 0;
		}
	#endif
	if (mFile) fclose(mFile);
	mFile = NULL;
	mMode = kFileInvalidMode;
}


//*************************
void RBinFile::Rewind(void)
//*************************
{
	#ifdef RLIB_MACOS
		if (mDataFork)
		{
			::SetFPos(mRefNum, fsFromStart, 0);
			return;
		}
	#endif

	if (mFile) rewind(mFile);
}


//*************************************************
rbool RBinFile::Seek(si32 position, EFileSeek mode)
//*************************************************
{
	#ifdef RLIB_MACOS
		if (mDataFork)
		{
			short macmod = (mode == kFileSeekSet	? fsFromStart :
								(mode == kFileSeekCur ? fsFromMark :
									(mode == kFileSeekEnd ? fsFromLEOF :
										fsAtMark)));

			// does fsFromLEOF needs a negative offset
			// to get back in direction of beginning of file ? I think so.
			// But that may be a bug, the doc is not explicit on this.
			if (mode == kFileSeekEnd) position = -position;

			return (::SetFPos(mRefNum, macmod, position) == 0);
		}
	#endif

	if (mFile) return fseek(mFile, position, mode) == 0;
	return false;
}


//***********************
rbool RBinFile::Eof(void)
//***********************
{
	#ifdef RLIB_MACOS
		if (mDataFork)
		{
			si32 size, fpos;
			OSErr err = ::GetEOF(mRefNum, &size);
			if (err != noErr) return true;
			err = ::GetFPos(mRefNum, &fpos);
			if (err != noErr) return true;
			return (fpos >= size);
		}
	#endif

	if (mFile) return feof(mFile) ? true : false;
	return true;	// eof always reached on an invalid file
}


//***********************
si32 RBinFile::Tell(void)
//***********************
{
	#ifdef RLIB_MACOS
		if (mDataFork)
		{
			si32 fpos;
			OSErr err = ::GetFPos(mRefNum, &fpos);
			if (err != noErr) return -1;
			return fpos;
		}
	#endif

	if (mFile) return ftell(mFile);
	return -1;
}


//***********************
si32 RBinFile::Size(void)
//***********************
{
	si32 size = -1;

	#ifdef RLIB_MACOS
		if (mDataFork)
		{
			OSErr err = ::GetEOF(mRefNum, &size);
			if (err != noErr) return -1;
			else return size;
		}
	#endif

	if (mFile)
	{
		si32 current = Tell();
		if (current < 0) return -1;
		if (Seek(0, kFileSeekEnd)) size = Tell();
		if (!Seek(current, kFileSeekSet)) return -1;
	}

	return size;
}


//*******************************************
rbool RBinFile::Read(char *buffer, si32 size)
//*******************************************
{
	if (!buffer || size <= 0) return false;

	#ifdef RLIB_MACOS
		if (mDataFork)
		{
			si32 total = size;
			OSErr err = ::FSRead(mRefNum, &total, (Ptr)buffer);
			return (err == noErr && size == total);
		}
	#endif

	if (mFile)
	{
		// this is faster but I want error checking
		if (size == 1)
		{
			int b = fgetc(mFile);
			*buffer = b;
			return b != EOF;
		}
		else
		{
			si32 read = fread(buffer, 1, size, mFile);
			return (read == size);
		}

	}

	return false;
}


//********************************************
rbool RBinFile::Write(char *buffer, si32 size)
//********************************************
{
	if (!buffer || size <= 0) return false;

	#ifdef RLIB_MACOS
		if (mDataFork)
		{
			si32 total = size;
			OSErr err = ::FSWrite(mRefNum, &total, (Ptr)buffer);
			return (err == noErr && size == total);
		}
	#endif

	if (mFile)
		return (((si32)fwrite(buffer, 1, size, mFile)) == size);

	return false;
}


//***************************************
const RPath & RBinFile::GetOpenPath(void)
//***************************************
{
	return mPath;
}


//**************************************************
rbool RBinFile::SetFileType(const sptr filename,
							const sptr mimetype,
							ui32 creator, ui32 type)
//**************************************************
{
#if defined(RLIB_BEOS)

	BNode node;
	BNodeInfo info;
	BMimeType mime;

	if (!filename || !mimetype) return false;

	// first make sure the mimetype is valid

	if (mime.SetTo(mimetype) < B_OK) return false;
	if (!mime.IsValid()) return false;
	// if !mime.IsInstalled()... TBDL. this is not the right place here to handle this

	if (node.SetTo(filename) < B_OK || node.InitCheck() < B_OK) return false;
	if (info.SetTo(&node)    < B_OK || info.InitCheck() < B_OK) return false;

	return (info.SetType(mimetype) >= B_OK);

#elif defined(RLIB_MACOS)

	OSErr	err;
	FSSpec	spec;
	FInfo	info;

	if (!filename || !type) return false;
	if (!creator) creator = gRGenApp->AppCreator();

	// first we need a real FSSpec
	err = FSpLocationFromFullPath(strlen(filename),
								  filename,
								  &spec);
	if (err != noErr) return false;

	err = ::FSpGetFInfo(&spec, &info);
	if (err == noErr)
	{
		info.fdType = type;
		info.fdCreator = creator;
		return (::FSpSetFInfo(&spec, &info) == noErr);
	}

	return false;

#elif defined(RLIB_WIN32)

	return false;

#endif

	return false;
}


//**************************************************
rbool RBinFile::SetFileType(RPath &path,
							const sptr mimetype,
							ui32 creator, ui32 type)
//**************************************************
{
#ifdef RLIB_MACOS

	if (path.HasFSSpec())
	{
		OSErr	err;
		FInfo	info;

		if (!type) return false;
		if (!creator) creator = gRGenApp->AppCreator();

		// use the FSSpec we already have
		const FSSpec &spec = path.GetFSSpec();

		err = ::FSpGetFInfo(&spec, &info);
		if (err == noErr)
		{
			info.fdType = type;
			info.fdCreator = creator;
			return (::FSpSetFInfo(&spec, &info) == noErr);
		}
	}
#endif

	return SetFileType(path.Path(), mimetype, creator, type);
}


//*********************************************************************
rbool RBinFile::SetDefaultFileType(const sptr filename, const sptr ext)
//*********************************************************************
{
#if defined(RLIB_BEOS)

	return false;

#elif defined(RLIB_MACOS)

	if (!filename || !UInternetConfig::PP_ICAvailable()) return false;

	ICMapEntry entry;
	ICError icerr = noErr;
	rbool second_try = true;

	if (ext && strlen(ext))
	{
		BWString pext(ext);
		icerr = UInternetConfig::PP_ICMapFilename((StringPtr)pext, entry);
		second_try = false;
	}
	if (second_try || icerr != noErr)
	{
		BWString pstr(filename);
		icerr = UInternetConfig::PP_ICMapFilename((StringPtr)pstr, entry);
	}

	if (icerr != noErr) return false;
	SetFileType(filename, NULL, entry.file_creator, entry.file_type);

#elif defined(RLIB_WIN32)

	return false;

#endif

	return true;
}


//*************************************************************
rbool RBinFile::SetDefaultFileType(RPath &path, const sptr ext)
//*************************************************************
{
#ifdef RLIB_MACOS

	if (!UInternetConfig::PP_ICAvailable()) return false;

	ICMapEntry entry;
	ICError icerr = noErr;
	rbool second_try = true;

	if (ext && strlen(ext))
	{
		BWString pext(ext);
		icerr = UInternetConfig::PP_ICMapFilename((StringPtr)pext, entry);
		second_try = false;
	}
	if (second_try || icerr != noErr)
	{
		BWString pstr(path.Leaf());
		icerr = UInternetConfig::PP_ICMapFilename((StringPtr)pstr, entry);
	}

	if (icerr != noErr) return false;
	SetFileType(path, NULL, entry.file_creator, entry.file_type);

	return true;

#else

	return SetDefaultFileType(path.Path(), ext);

#endif
}


//*******************************************
void RBinFile::PrintToStream(const sptr name)
//*******************************************
{
	printf("BWBinFile %s -- FILE * %p\n", (name ? name : ""), mFile);

	#ifdef RLIB_MACOS
		if (mDataFork)
			printf("Mac access : ref num %d\n", mRefNum);
		else
			printf("Use ANSI access\n");
	#endif

	printf("\tFile Mode %ld (%s)\n",
		mMode, (mMode == kFileReadOnly ? "Read Only" :
				 (mMode == kFileWriteOnly ? "Write Only" :
				 	(mMode == kFileReadWrite ? "Read/Wrtie" : "Error"))));
}


/****************************************************************

	$Log: RBinFile.cpp,v $
	Revision 1.2  2000/04/01 21:47:33  Ralf
	Using RIBin interface
	
	Revision 1.1  2000/03/13 08:55:13  Ralf
	First revision
	
****************************************************************/

// eoc
