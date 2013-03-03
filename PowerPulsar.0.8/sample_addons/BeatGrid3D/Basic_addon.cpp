/*****************************************************************************

	Projet	: Pulsar Fog AddOn

	Fichier	: TechnoFog_addon.cpp
	Partie	: End

	Auteur	: Laurent Pontier
	Date	: 040597 -- version 0.2 (C funcs)

*****************************************************************************/

/************************************************************
	Author	: Laurent Pontier
	1997	: LP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Laurent Pontier

************************************************************/


#include "Basic_addon.h"

//--------------------------------------------------------------------

//********************
int main(int, char **)
//********************
// Code to help a little the fellow user double-clicking the
// add-on icon directly.
{
	new BApplication("application/x-vnd.ralf-vanilla");
	BAlert *box = new BAlert(ADDON_NAME,
							 "Thanks for using " ADDON_NAME " !\n\n"
							 "Please move the " ADDON_NAME " executable into\n"
							 "the PowerPulsar add-on directory\n"
							 "and try again !",
							 "Great !");
	box->Go();
	delete be_app;
	return 0;
}

//--------------------------------------------------------------------


//*****************************
void * filterInit(uint32 index)
//*****************************
{
	// this add-on only declares ONE filter
	if (index > 0) return NULL;

	CSampleFftFilter *info = new CSampleFftFilter;
	if (!info) return NULL;		// memory error, give up

	// returns the instance -- the caller will check that this instance
	// is derived from CFilter but is _not_ a CFilter...
	return info;
} // end of filterInit


//*******************************
bool CSampleFftFilter::load(void)
//*******************************
{
	sFilter.name = "Beat 3DGrid";
	sFilter.author = "Laurent Pontier AKA BeLou";
	sFilter.info = "Have a glance to my 3D FFT grid...";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 1;
	sFilter.supportedMode = (EColorSpace)(kColor8Bit | kRgb32Bit);

	// sFilter.icon = loadBitmap("./add-ons.icons/fft.tga");

	mUseGk8 = false;
	mUseGk32 = false;
	
	// Preparation point3d.
	
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			p[x][y]=new Point3D(20.0*(x-DOTNUMBER/2),20.0*(y-DOTNUMBER/2),0.0);
			res[x][y]=new Point3D(20.0*(x-DOTNUMBER/2),20.0*(y-DOTNUMBER/2),0.0);
		}

	counter=0;
	// configuration;
	Epaisseur=1;
	Croix=FALSE;
	TailleCroix=1;
	NombreImageRotation=50;
	AmplitudeVagues = 0.1;
	AmplitudeOscillation = 0.6;
	VitesseOscillation = 0.15;
	
	return true;
}  // end of load for CSampleFftFilter


//*********************************
void CSampleFftFilter::unload(void)
//*********************************
{
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			delete p[x][y];
			delete res[x][y];
		}
}


//**********************************
bool CSampleFftFilter::prepare(void)
//**********************************
{
	return true;
} // end of prepare for CSampleFftFilter


//*****************************************************
void CSampleFftFilter::processFrame8(SFrameInfo &frame)
//*****************************************************
{
	float *chanL;
	float *chanR;
	int i,j;
	long nframe;
	int y1;
	int h = sPrepare.sy;
	int w = sPrepare.sx;
	int w2 = w/2;
	int h2 = h/2;

	chanL	= frame.fftHigh;
	chanR	= frame.fftBass;
	nframe	= frame.frame;

	
	// determination de la forme de debut.
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
			{
				// on amplifi un peu les aigues...
				p[x][y]->z=(chanR[x+y*DOTNUMBER]) / (20000-250*y);
				if (p[x][y]->z <0) p[x][y]->z=-p[x][y]->z;
				if (p[x][y]->z > 150) p[x][y]->z=150; 
			}

	p[0][0]->z=0;
	counter++;
	// deplacements 3D.
	double coef1=(1.0/5.0);
	double coef2=3.14/6;
	double coef3=3.14/3;
	double coef4=1/50000.0;

	// deformation outrageuses...
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			res[x][y]->x=p[x][y]->x;
			res[x][y]->y=p[x][y]->y;
			res[x][y]->z=p[x][y]->z;
			
			res[x][y]->RotateX(AmplitudeVagues*sin(y*0.5-coef1*nframe));
			res[x][y]->RotateY(AmplitudeVagues*sin(x*0.5-coef1*nframe));
		}
	
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			// positionnement 3/4
			res[x][y]->RotateZ(coef2);
			res[x][y]->RotateX(coef3);
		}

	double coef5=(6.28/NombreImageRotation)*(counter-200);
	// galipettes...
	if ( ((counter>199)&&(counter<200+NombreImageRotation))
		|| ((counter>399)&&(counter<=400+NombreImageRotation)))
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			// galipette 1 !!!!
			if ((counter>200)&&(counter<200+NombreImageRotation))
			{
				res[x][y]->RotateX(coef5);
				res[x][y]->RotateZ(coef5);
			}
			// galipette 2 !!!!
			if ((counter>400)&&(counter<400+NombreImageRotation))
			{
				res[x][y]->RotateY(coef5);
				res[x][y]->RotateZ(-coef5);
			}
			if (counter>=400+NombreImageRotation) counter=0;
		}
	
	double coef6=AmplitudeOscillation*sin(VitesseOscillation*nframe);
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			// Oscillation !!!
			res[x][y]->RotateY(coef6);
			// Hop !
			res[x][y]->y+=(((double)chanR[10])*coef4);	
		}

	long X;
	long Y;
	long Xa;
	long Ya;
	long Xb;
	long Yb;
	double coef7=1.0/150.0;
	
	const float zoom=150;
	const float dist=300;
	
	// Affichage.
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			X=res[x][y]->x;
			Y=res[x][y]->y;
			X+=w2;
			Y+=h2;
			Xa=res[x-1][y]->x;
			Ya=res[x-1][y]->y;
			Xa+=w2;
			Ya+=h2;
			Xb=res[x][y-1]->x;
			Yb=res[x][y-1]->y;
			Xb+=w2;
			Yb+=h2;
			
			// les 4 points qui entourent le point de traitement.
			// trace rouge...
			if (p[x][y]->z > 20)
			{
				if (x>0)
					blitLine8(X,Y,Xa,Ya,48-(p[x][y]->z *coef7)*6.0);

				if (y>0)
					blitLine8(X,Y,Xb,Yb,48-(p[x][y]->z *coef7)*6.0);

				// petite croix
				if (Croix)
				{
					blitLine8(X-TailleCroix,Y,X+TailleCroix,Y,255);
					blitLine8(X,Y-TailleCroix,X,Y+TailleCroix,255);
				}
			}
			else // affichage en vert sinon.
			{
				if (x>0)
				{
					if (p[x-1][y]->z > 20) blitLine8(X,Y,Xa,Ya,48-(p[x][y]->z *coef7)*6.0);
					else blitLine8(X,Y,Xa,Ya,57-y/4.0);
				}
				if (y>0)
				{
					if (p[x][y-1]->z > 20) blitLine8(X,Y,Xb,Yb,48-(p[x][y]->z *coef7)*6.0);
					else blitLine8(X,Y,Xb,Yb,57-y/4.0);
				}
			}
		}
}  // end of processFrame8 for CSampleFftFilter




//******************************************************
void CSampleFftFilter::processFrame32(SFrameInfo &frame)
//******************************************************
{
	float *chanL;
	float *chanR;
	int i,j;
	long nframe;
	int y1;
	int h = sPrepare.sy;
	int w = sPrepare.sx;
	int w2 = w/2;
	int h2 = h/2;

	chanL	= frame.fftHigh;
	chanR	= frame.fftBass;
	nframe	= frame.frame;

	// determination de la forme de debut.
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
			{
				// on amplifi un peu les aigues...
				p[x][y]->z=(chanR[x+y*DOTNUMBER]) / (20000-250*y);
				if (p[x][y]->z <0) p[x][y]->z=-p[x][y]->z;
				if (p[x][y]->z > 150) p[x][y]->z=150; 
			}

	p[0][0]->z=0;
	counter++;
	// deplacements 3D.
	double coef1=(1.0/5.0);
	double coef2=3.14/6;
	double coef3=3.14/3;
	double coef4=1/50000.0;

	// deformation outrageuses...
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			res[x][y]->x=p[x][y]->x;
			res[x][y]->y=p[x][y]->y;
			res[x][y]->z=p[x][y]->z;
			
			res[x][y]->RotateX(AmplitudeVagues*sin(y*0.5-coef1*nframe));
			res[x][y]->RotateY(AmplitudeVagues*sin(x*0.5-coef1*nframe));
		}
	
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			// positionnement 3/4
			res[x][y]->RotateZ(coef2);
			res[x][y]->RotateX(coef3);
		}

	double coef5=(6.28/NombreImageRotation)*(counter-200);
	// galipettes...
	if ( ((counter>199)&&(counter<200+NombreImageRotation))
		|| ((counter>399)&&(counter<=400+NombreImageRotation)))
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			// galipette 1 !!!!
			if ((counter>200)&&(counter<200+NombreImageRotation))
			{
				res[x][y]->RotateX(coef5);
				res[x][y]->RotateZ(coef5);
			}
			// galipette 2 !!!!
			if ((counter>400)&&(counter<400+NombreImageRotation))
			{
				res[x][y]->RotateY(coef5);
				res[x][y]->RotateZ(-coef5);
			}
			if (counter>=400+NombreImageRotation) counter=0;
		}
	
	double coef6=AmplitudeOscillation*sin(VitesseOscillation*nframe);
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			// Oscillation !!!
			res[x][y]->RotateY(coef6);
			// Hop !
			res[x][y]->y+=(((double)chanR[10])*coef4);	
		}

	long X;
	long Y;
	long Xa;
	long Ya;
	long Xb;
	long Yb;
	double coef7=1.0/150.0;
	
	const float zoom=150;
	const float dist=300;
	
	// Affichage.
	for(int x=0;x<DOTNUMBER;x++)
		for(int y=0;y<DOTNUMBER;y++)
		{
			X=res[x][y]->x;
			Y=res[x][y]->y;
			X+=w2;
			Y+=h2;
			Xa=res[x-1][y]->x;
			Ya=res[x-1][y]->y;
			Xa+=w2;
			Ya+=h2;
			Xb=res[x][y-1]->x;
			Yb=res[x][y-1]->y;
			Xb+=w2;
			Yb+=h2;
			
			const int32 col255 = (255<<16)|(255<<8)|(255<<0);		// blanc
			int32 col48 = (int)(100+155*(p[x][y]->z *coef7))<<16;	// rouge 42->48 : 255->80
			int32 col57 = (int)(100+(155*y)/16)<<8;					// vert  52->57 : 255->80, [y:0-15]

			// les 4 points qui entourent le point de traitement.
			// trace rouge...
			if (p[x][y]->z > 20)
			{
				if (x>0)
					blitLine32(X,Y,Xa,Ya,col48);

				if (y>0)
					blitLine32(X,Y,Xb,Yb,col48);

				// petite croix
				if (Croix)
				{
					blitLine32(X-TailleCroix,Y,X+TailleCroix,Y,col255);
					blitLine32(X,Y-TailleCroix,X,Y+TailleCroix,col255);
				}
			}
			else // affichage en vert sinon.
			{
				if (x>0)
				{
					if (p[x-1][y]->z > 20) blitLine32(X,Y,Xa,Ya,col48);
					else blitLine32(X,Y,Xa,Ya,col57);
				}
				if (y>0)
				{
					if (p[x][y-1]->z > 20) blitLine32(X,Y,Xb,Yb,col48);
					else blitLine32(X,Y,Xb,Yb,col57);
				}
			}
		}
}  // end of processFrame32 for CSampleFftFilter


















