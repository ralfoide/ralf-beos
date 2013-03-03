/***********************************************************
	Projet	: Pulsar

	Fichier	: CFilledView.h
	Partie	: Visu

	Auteur	: RM
	Date	: xxxx97
	Format	: tabs==4

***********************************************************/

#ifndef _H_CFILLEDVIEW_
#define _H_CFILLEDVIEW_


#include "globals.h"
#include "CPulsarApp.h"
#include "CPulsarMsg.h"

//--------------------------------------------------------

//******************************
class CFilledView : public BView
//******************************
{
public:
			CFilledView(BRect frame, char *title, long resize, long flags=B_WILL_DRAW);
			~CFilledView(void);

		bool init(void);

virtual void Pulse(void);
virtual	void KeyDown(const char *bytes, int32 numBytes)
			 { gApplication->touches(bytes, numBytes, Window()->CurrentMessage()); }
virtual void MouseDown(BPoint where) { MakeFocus(true); }

		void Draw(BRect upd);

		bool setColorSpace(EColorSpace mode, bool force_lock=true);
inline	void getInfos(int32 &sx, int32 &sy, EColorSpace &mode,
					  uint8 * &buffer, int32 &bpr)
					 { sx = mapSx; sy = mapSy; mode = colorSpace;
					   buffer = mapPtr; bpr = mapBpr; }

	void setZoom(int32 sx, int32 sy);

private:
	BRect	bounds;
	BRect	srcRect;
	BRect	blitRect;

	BBitmap	*map;
	uchar 	*mapPtr;
	long	mapBpr, mapSize;
	long	mapSx, mapSy;
	EColorSpace	colorSpace;
};


//--------------------------------------------------------
#endif // of _H_CFILLEDVIEW_

// eoh
