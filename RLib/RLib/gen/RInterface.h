/*****************************************************************************

	Projet	: Portable Lib

	Fichier	: RInterface.h
	Partie	: RLib basics

	Auteur	: RM
	Date	: 100798
	Format	: tabs==4

	The RInterface class is the archetype of the null-object.
	An "interface", in my terminlogy, is a pure abstract class.
	Eventually the class declares no constructor/destructor and all methods
	are required to be abstract.
	Every class in the RLib hierarchy should derive virtually from RInterface.

	The main idea behind this root class is portability on platforms that may
	require all instances/classes to share some methods or members (in which case
	the class may become non-abstract... thus the need of a virtual derivation.)
	Another side effect is that, by using the RTTI, one can know if some instance
	is RLib or platform-specific...

*****************************************************************************/

#ifndef _H_RINTERFACE_
#define _H_RINTERFACE_


//---------------------------------------------------------------------------



//**************
class RInterface
//**************
{
public:

	virtual ~RInterface(void) = 0;

}; // end of class defs for RInterface


//---------------------------------------------------------------------------

#endif // of _H_RINTERFACE_

/****************************************************************

	$Log: RInterface.h,v $
	Revision 1.2  2000/04/01 21:50:44  Ralf
	Adapted to this RLib
	
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eoh
