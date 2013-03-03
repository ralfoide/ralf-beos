/*****************************************************************************

	Projet	: RLib

	Fichier	: RMacros.cpp
	Partie	: Portable / Macros

	Auteur	: RM
	Date	: 250798 (Win32)
	Date	: 150298 -- 050398
	Format	: tabs==4

	Part of this file should be OS-dependant.

*****************************************************************************/

#include "RLib.h"

#ifdef RLIB_WIN32

#include "RMacros.h"
#include "RErrors.h"

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

#if 1 // -- status_t is BeOS specific and is not emulated.
			// Should be replaced by my_favourite_win32_error_type later.

//*****************************************************
void gThrowAlertStatusT(const char *name, status_t err)
//*****************************************************
{
	BAlert *box;
	char s2[256];

	sprintf(s2,	"%s Error\n" \
				"Exception catched : %d (%08xh)\n"
				"(probably a %s error number)",
				name, (int32)err, (int32)err,
				((err >= K_ERROR_BASE && err <= K_ERROR_MAX) ? "RErrEx" : "BeOS"));
	box = new BAlert("Exception Error box", s2, "    OK    ", NULL, NULL,
					B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
	if (box) box->Go();
} // end of gThrowAlertStatusT

#endif

//********************************************
void gThrowAlertInt(const char *name, int err)
//********************************************
{
	BAlert *box;
	char s2[256];

	sprintf(s2,	"%s Error\n" \
				"Exception catched : %d (%08xh)\n"
				"(probably a %s error number)",
				name, err, err,
				((err >= K_ERROR_BASE && err <= K_ERROR_MAX) ? "RErrEx" : "BeOS"));
	box = new BAlert("Exception Error box", s2, "    OK    ", NULL, NULL,
					B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
	if (box) box->Go();
} // end of gThrowAlertInt

//************************************************
void gThrowAlertCharPtr(const char *name, char *s)
//************************************************
{
	BAlert *box;
	char s2[256];
	if (strlen(s) > 128) s[127] = '\0';
	sprintf(s2,	"%s Error\n" \
				"Exception catched : %s\n", name, s);

	box = new BAlert("Exception Error box", s2, "    OK    ", NULL, NULL,
					B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
	if (box) box->Go();

} // end of gThrowAlertCharPtr


//***************************************
void gThrowAlertUntyped(const char *name)
//***************************************
{
	BAlert *box;
	char s2[256];
	sprintf(s2, "%s Error\nUnhandled Exception thrown", name);

	box = new BAlert("Exception Error box", s2, "    OK    ", NULL, NULL,
					B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
	if (box) box->Go();
} // end of gThrowAlertUntyped


//***************************************************
void gThrowAlertRErrEx (const char *name, RErrEx &ex)
//***************************************************
{
	BAlert *box;
	if (ex.str() && !ex.what()) gThrowAlertCharPtr(name, ex.str());
	else if (!ex.str() && ex.what()) gThrowAlertInt(name, ex.what());
	else
	{
		char s2[256];
		sprintf(s2,	"%s Error\n" \
					"Exception catched : %d (%08xh)\n"
					"Exception catched : %s\n", name, ex.what(), ex.what() , ex.str());

		box = new BAlert("Exception Error box", s2, "    OK    ", NULL, NULL,
						B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
		if (box) box->Go();
	}

} // end of gThrowAlertCharPtr




//---------------------------------------------------------------------------

#endif // of RLIB_WIN32


/****************************************************************

	$Log: RMacros.cpp,v $
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eoh
