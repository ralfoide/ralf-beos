/*****************************************************************************

	Projet	: RLib

	Fichier	: RGenApp.h
	Partie	: Main app class

	Auteur	: RM
	Date	: 160598
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_RGENAPP_
#define _H_RGENAPP_

#include "RPath.h"

//---------------------------------------------------------------------------
/**
	RGenApp is an interface (abstract class) defining a generic basis
	for RApp.
*/

//***********
class RGenApp
//***********
{
public:
		// there is no constructor in this interface
		// RGenApp(void);

		/// Virtual destructor, does nothing
virtual	~RGenApp(void) = 0;


	// --- public information about the app ---

virtual	const sptr	AppMimeType(void) const	= 0;	// for BeOS
virtual const sptr	AppRegKey  (void) const	= 0;	// for Win32
virtual		  ui32	AppCreator (void) const	= 0;	// for MacOS



	// --- virtual callbacks ---

		/// Callback to display an about box
virtual	void AboutRequested(void) = 0;

		/// Called to initialize the instance of the application, just after construction.
virtual	void Init(void) = 0;

		/// Called once the app as quit, before the instance is destroyed.
virtual	void Terminate(void) = 0;

		/** Process one msg in the main app loop.
			@param msg		the RMessage to be processed. Note that the caller owns the RMessage,
							this method will copy the data if it needs to keep it around. The
							msg pointer cannot be NULL.
		*/
virtual	void ProcessMessage(RMessage *msg) = 0;

		/** Process one system request to open a document.
			@param path	the path of the document to be open
			@param last	indicates if this is the last document to be open
						when multiple documents are requested.
		*/
virtual	void OpenDocument(RPath &path, rbool last=true) = 0;


private:

}; // end of class defs for RGenApp



//---------------------------------------------------------------------------

#endif // of _H_RGENAPP_


/****************************************************************

	$Log: RGenApp.h,v $
	Revision 1.2  2000/04/05 06:24:30  Ralf
	Switched ProcessMessage to RMessage
	
	Revision 1.1  2000/03/13 08:55:13  Ralf
	First revision
	
****************************************************************/

// eoh
