/*****************************************************************************

	Projet	: Pulsar

	Fichier	:	CPreference.cpp
	Partie	: Prefs

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Path.h>
#include <File.h>
#include <ByteOrder.h>
#include <FindDirectory.h>
#include "CPreference.h"
#include "globals.h"

#define K_PREF_VERS_MIN		100
#define K_PREF_VERS_MAX		100

#define K_PREF_FILE_SIG		'PwPf'
#define K_PREF_FILE_NAME	"preferences.bin"

#define FULL_DEBUG	if (debug)
//#define FULL_DEBUG		if (0)
#define NORMAL_DEBUG	if (debug)
//#define NORMAL_DEBUG	if (0)

//---------------------------------------------------------------------------


//*****************************************************************
CPreference::CPreference(const char *dirname, const char *prefname)
//*****************************************************************
{
	if (!dirname) throw "CPreference dirname not set";
	if (strchr(dirname, '/')) throw "CPreference dirname shoud not contains any '/' chars !";
	if (prefname && strchr(prefname, '/')) throw "CPreference prefname shoud not contains any '/' chars !";

	mPrefName = (prefname ? prefname : K_PREF_FILE_NAME);
	mDirName = dirname;

	mPref.MakeEmpty();
}


//*****************************
CPreference::~CPreference(void)
//*****************************
{
}


//**************************
bool CPreference::load(void)
//**************************
{
	status_t err;
	BPath bpath;
	BString path;
	BFile f;

	NORMAL_DEBUG printf("CPreference::load\n");

	err = find_directory(B_USER_SETTINGS_DIRECTORY, &bpath, true, NULL);
	if (err < B_OK) throw "Can't find_directory B_USER_SETTINGS_DIRECTORY";

	// create a directory 'submime' in the Settings, if not already existing
	path = bpath.Path();
	path += '/';
	path += mDirName;
	mkdir(path.String(), S_IRWXU+S_IRWXG+S_IRWXO);

	FULL_DEBUG printf("CPreference::load -> select dir %s\n", path.String());

	// get the file name
	path += '/';
	path += mPrefName;

	FULL_DEBUG printf("CPreference::load -> select file %s\n", path.String());

	try
	{
		if (f.SetTo(path.String(), B_READ_ONLY) != B_OK)
		{
			FULL_DEBUG printf("CPreference::load -> can't open file\n");
			return false;
		}

		FULL_DEBUG printf("CPreference::load -> pref file open\n");
	
		// get header, check it
		uint32 sig=0, vers=0, size=0;
	
		if (   f.Read(&sig , 4) != 4
			  || f.Read(&vers, 4) != 4
			  || f.Read(&size, 4) != 4
			  || swap_data(B_UINT32_TYPE, &sig,  1, B_SWAP_BENDIAN_TO_HOST) != B_OK
			  || swap_data(B_UINT32_TYPE, &vers, 1, B_SWAP_BENDIAN_TO_HOST) != B_OK
			  || swap_data(B_UINT32_TYPE, &size, 1, B_SWAP_BENDIAN_TO_HOST) != B_OK
			  || sig != K_PREF_FILE_SIG
			  || vers < K_PREF_VERS_MIN
			  || vers > K_PREF_VERS_MAX)
		{
			FULL_DEBUG printf("CPreference::load -> BAD HEADER; sig 0x%08x, vers %d (0x%04x), size %d\n", sig, vers, vers, size);
			return false;
		}
	
		FULL_DEBUG printf("CPreference::load -> good header; sig 0x%08x, vers %d (0x%04x), size %d\n", sig, vers, vers, size);
	
		if (mPref.Unflatten(&f) != B_OK) return false;

		FULL_DEBUG printf("CPreference::load -> msg unflatten OK\n");
	}
	catch(...)
	{
		NORMAL_DEBUG printf("CPreference::load -> exception catched\n");
		return false;
	}

	return true;
}


//**************************
bool CPreference::save(void)
//**************************
{
	status_t err;
	BPath bpath;
	BString path;
	BFile f;

	NORMAL_DEBUG printf("CPreference::save\n");

	err = find_directory(B_USER_SETTINGS_DIRECTORY, &bpath, true, NULL);
	if (err < B_OK) throw "Can't find_directory B_USER_SETTINGS_DIRECTORY";

	// select directory
	path = bpath.Path();
	path += '/';
	path += mDirName;

	// get the file name
	path += '/';
	path += mPrefName;

	FULL_DEBUG printf("CPreference::save -> select file %s\n", path.String());

	err = f.SetTo(path.String(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (err != B_OK)
	{
		FULL_DEBUG printf("CPreference::save -> can't open file\nerr = %ld -- %s\n", err, strerror(err));
		return false;
	}

	// set header, write it
	uint32 sig  = K_PREF_FILE_SIG;
	uint32 vers = K_PREF_VERS_MAX;
	uint32 size = mPref.FlattenedSize();

	if (	 swap_data(B_UINT32_TYPE, &sig , 1, B_SWAP_HOST_TO_BENDIAN) != B_OK
			|| swap_data(B_UINT32_TYPE, &vers, 1, B_SWAP_HOST_TO_BENDIAN) != B_OK
			|| swap_data(B_UINT32_TYPE, &size, 1, B_SWAP_HOST_TO_BENDIAN) != B_OK
			|| f.Write(&sig , 4) != 4
			|| f.Write(&vers, 4) != 4
			|| f.Write(&size, 4) != 4)
		return false;

	err = mPref.Flatten(&f);

	FULL_DEBUG printf("CPreference::save -> flatten %s\n", (err == B_OK) ? "OK" : "ERROR");

	return err == B_OK;
}



//********************************************************
bool CPreference::getPref(const char *name, BMessage &msg)
//********************************************************
{
	NORMAL_DEBUG printf("CPreference::getPref, name: %s\n", name);
	if (!name) return false;
	return mPref.FindMessage(name, &msg) == B_OK;
}


//********************************************************
bool CPreference::setPref(const char *name, BMessage &msg)
//********************************************************
{
	NORMAL_DEBUG printf("CPreference::setPref, name: %s\n", name);
	if (!name) return false;
	mPref.RemoveName(name);
	return mPref.AddMessage(name, &msg) == B_OK;
}



//---------------------------------------------------------------------------

// eoc


