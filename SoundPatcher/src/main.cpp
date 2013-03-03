/*****************************************************************************

	Projet	: Nerdkill 3d

	Fichier	: main.cpp
	Partie	: Loader for the Nerdkill 3d App

	Auteur	: RM
	Date		: 010598
	Format	: tabs==2

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

#include "CApp.h"
#include "gMacros.h"
#include "externs.h"

//------------------------------------------------------------------------

// prototypes
int main(int argc,char *argv[]);

// hack
char *gFilename = NULL;

//---------------------------------------------------------------------------

//*****************************
int main(int argc,char *argv[])
//*****************************
{
CApp gApp;

	printf(K_APP_NAME " starting\n");
	printf("Hacked usage : %s ip\n", argv[0]);
	printf("SETENV EXPORT: NO_SOUND=1 or SEND_ONLY=1 or RECEIVE_ONLY=1\n");

	//debug = (argc>1);						// defined in CApp.cpp/.h
	debug=1;
	gApplication = &gApp; 			// defined in CApp.cpp/.h

	printf("Nb argc %d\n", argc);
	if (argc>1) gFilename = argv[1];
	if (!gFilename) gFilename = "127.0.0.1";

	try
	{
		gApp.init();
		gApp.Run();
	}
	M_CATCH_STATIC(K_APP_NAME " main");

	printf(K_APP_NAME " end\n");

	return 0;

} // end of main

//---------------------------------------------------------------------------



// eoc

