/****************
*	main.cpp	*
****************/

#include "CPulsarApp.h"

#include <stdio.h>
#include <stdlib.h>

bool debug = FALSE;

CPulsarApp *gApplication = NULL;

/*****************************/
int main(int argc,char *argv[])
/*****************************/
{
CPulsarApp gApp;

	debug = (argc>1);

	gApplication = &gApp; // hairy and ballsy but works... 

	try
	{
		if (gApp.init()) gApp.Run();
		else throw("Error during initialization...\n");
	}
	catch(status_t err)
	{
		BAlert *box;
		char s2[256];
		sprintf(s2,	K_APP_NAME " Error\n"
							"Exception catched : %d (%08xh)\n"
							"(can be a BeOS error number)", (int32)err, (int32)err);
		box = new BAlert("Abcd - Error box", s2, "    OK    ", NULL, NULL,
									B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
		if (box) box->Go();
	}
	catch(char *s)
	{
		BAlert *box;
		char s2[1024];
		if (strlen(s) > 800) s[800] = '\0';	// ugly but safe... :-/
		sprintf(s2,	K_APP_NAME " Error\n"
							"Exception catched : %s\n", s);
		box = new BAlert(K_APP_NAME " Error", s2, "    OK    ", NULL, NULL,
									B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
		if (box) box->Go();
	}
	catch(...)
	{
		BAlert *box = new BAlert(K_APP_NAME " Error",
													K_APP_NAME " Error\nUnhandled Exception thrown,"
													"\ncatched in main()",
													"   OK   ");
		if(box) box->Go();
	}

	return 0;
}

/* eof */