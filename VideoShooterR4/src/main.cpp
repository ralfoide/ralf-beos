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
rbool gPlaybackMode = false;
char *gFilename = NULL;

//---------------------------------------------------------------------------

//*****************************
int main(int argc,char *argv[])
//*****************************
{
CApp gApp;

	printf(K_APP_NAME " starting\n");
	printf("Hacked usage : %s [-r | -p] [filename]\n", argv[0]);

	//debug = (argc>1);						// defined in CApp.cpp/.h
	debug=1;
	gApplication = &gApp; 			// defined in CApp.cpp/.h

	printf("Nb argc %d\n", argc);
	gPlaybackMode = (argc<=1) || (strcmp(argv[1],"-r") != 0);
	if (argc>2) gFilename = argv[2];

	try
	{
		gApp.init();
		gApp.Run();
	}
	M_CATCH(K_APP_NAME " main");

	printf(K_APP_NAME " end\n");

	return 0;

} // end of main

//---------------------------------------------------------------------------



// eoc

