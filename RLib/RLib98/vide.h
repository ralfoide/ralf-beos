/*****************************************************************************

	Projet	: Droids

	Fichier	:	vide.h
	Partie	: Interface Messagerie Encapsulation Video Loader Son Camera Interpreteur
						Programmation Moteur Fichiers Magnetoscope Timing Defs Stockage Reseau

	Auteur	: RM
	Date		: 120297
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CCLASSE_
#define _H_CCLASSE_

#include "machine.h"


//---------------------------------------------------------------------------


//********************************
class CClasse : public CBaseClasse
//********************************
{
//==== methodes publiques ----
public:
	CClasse(void);
	virtual ~CClasse(void);

	void foo(void);

//---- membres publiques ----
	int a;


//==== methodes protected ====
protected:


//---- membres protected ----



//==== methodes private ====
private:


//---- membres private ----



}; // end of class defs for CClasse


//---------------------------------------------------------------------------

#endif // of _H_CCLASSE_

// eoh
