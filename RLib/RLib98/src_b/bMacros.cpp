/*****************************************************************************

	Projet	: Portable Lib

	Fichier	: gMacros.cpp
	Partie	: Portable / Macros

	Auteur	: RM
	Date		: 150298 -- 050398
	Format	: tabs==2

	Part of this file should be OS-dependant.

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "gMacros.h"
#include "gErrors.h"
#include <Alert.h>

//---------------------------------------------------------------------------
// private global (referenced as "debug" in source code)

bool _gDebug = false;

//---------------------------------------------------------------------------
// debug setup routine

//******************************
void gDebugSetup(bool condition)
//******************************
{
	_gDebug = condition;

} // end of gDebugSetup


//---------------------------------------------------------------------------
// typical handlers for exception in throw


//********************************************************
void gThrowAlertStatusT(const char *name,
												const char *classname,
												const char *file,
												si32 line,
												status_t err)
//********************************************************
{
	BAlert *box;
	char s2[512];
	sprintf(s2,	"Error: %s\nClass: %s\nFile: %s\nLine: %ld\n"
							"Exception catched : %ld (%08lxh)\n"
							"(probably a %s error number)",
							name, classname, file, line,
							err, err,
							((err >= K_ERROR_BASE && err <= K_ERROR_MAX) ? "RErrEx" : "BeOS"));
	box = new BAlert("Exception Error box", s2, "    OK    ", NULL, NULL,
								B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
	if (box) box->Go();

} // end of gThrowAlertStatusT


//***************************************************
void gThrowAlertCharPtr(const char *name, 
												const char *classname,
												const char *file,
												si32 line,
												char *s)
//***************************************************
{
	BAlert *box;
	char s2[512];
	if (strlen(s) > 128) s[127] = '\0';
	sprintf(s2,	"Error: %s\nClass: %s\nFile: %s\nLine: %ld\n"
							"Exception catched : %s\n",
							name, classname, file, line,
							s);
	box = new BAlert("Exception Error box", s2, "    OK    ", NULL, NULL,
								B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
	if (box) box->Go();

} // end of gThrowAlertCharPtr


//******************************************
void gThrowAlertUntyped(const char *name, 
												const char *classname,
												const char *file,
												si32 line)
//******************************************
{
	BAlert *box;
	char s2[512];
	sprintf(s2, "Error: %s\nClass: %s\nFile: %s\nLine: %ld\n"
							"Unhandled Exception thrown",
							name, classname, file, line);
	box = new BAlert("Exception Error box", s2, "   OK   ", NULL, NULL,
									B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
	if (box) box->Go();

} // end of gThrowAlertUntyped


//***************************************************
void gThrowAlertRErrEx (const char *name, 
												const char *classname,
												const char *file, 
												si32 line,
												RErrEx &ex)
//***************************************************
{
	if (ex.str() && !ex.what()) gThrowAlertCharPtr(name, classname, file, line, ex.str());
	else if (!ex.str() && ex.what()) gThrowAlertStatusT(name, classname, file, line, ex.what());
	else
	{
		BAlert *box;
		char s2[512];
		sprintf(s2,	"Error: %s\nClass: %s\nFile: %s\nLine: %ld\n"
								"Exception catched : %ld (%08lxh)\n"
								"Exception catched : %s\n",
								name, classname, file, line,
								ex.what(), ex.what() , ex.str());
		box = new BAlert("Exception Error box", s2, "    OK    ", NULL, NULL,
									B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
		if (box) box->Go();
	}

} // end of gThrowAlertCharPtr




//---------------------------------------------------------------------------

#endif // of RLIB_BEOS

// eoh
