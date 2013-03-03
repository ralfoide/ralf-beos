/************************************************************
	Author	: X
	1997	: XD
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, X <x.com>

************************************************************/

/*****************************************************************************

	Projet	: Pulsar Torus AddOn

	Fichier	: Torus_addon.cpp
	Partie	: Add-ons

	Auteur	: X
	Date	: 040597 -- version 0.2 (C funcs)
	Date	: 050300 -- Ralf - 32 bpp

*****************************************************************************/

#include "Basic_addon.h"
#include "SinTable.h"

#define min(a,b) ((a)>(b)?(b):(a))
#define max(a,b) ((a)>(b)?(a):(b))


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

#define DEC 5

//*******************************
bool CSampleFftFilter::load(void)
//*******************************
{
	sFilter.name = "PowerTorus";
	sFilter.author = "X";
	sFilter.info = "x.com";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 4;
	sFilter.supportedMode = (EColorSpace)(kColor8Bit | kRgb32Bit);

	//sFilter.icon = loadBitmap("./add-ons.icons/fft.tga");


	int32 i;

	Minor=10;
	Major=60;
	nb_point = 1<<DEC;
	nb_lat = 12;
	// Tableau de cos et sin
	tetaMCos = (float*)malloc(sizeof(float)*nb_point);
	tetaMSin = (float*)malloc(sizeof(float)*nb_point);
	
	for (i = 0 ; i<nb_point ; i++) {
		tetaMCos[i] = cos(2.0*PI*(0.5+i)/nb_point); // 3 cause 2*1.5 cause le premier et le dernier doivent pas etre colle.
		tetaMSin[i] = sin(2.0*PI*(0.5+i)/nb_point);
	}

	tetamCos = (float*)malloc(sizeof(float)*nb_lat);
	tetamSin = (float*)malloc(sizeof(float)*nb_lat);
	
	for (i = 0 ; i<nb_lat ; i++) {
		tetamCos[i] = cos(2*PI*i/nb_lat);
		tetamSin[i] = sin(2*PI*i/nb_lat);
	}

	// on deplie tout le torus en un tableau de points.
	// creation du tableau en fonction du nombre de faces
	// tab[(latitude<<DEC)+longitude]
	donuts = (Point3D*)malloc(sizeof(Point3D)*nb_point*nb_lat);

	// on cree la table de sin/cos.
	sintab = new SinTable;
	for (i=0;i<nb_point*nb_lat;i++)
		donuts[i].sintab = sintab;

	// table de fft pour une frame
	// on s'en sert deux fois, pas la peine de la recalculer comme ca.
	FFT = (float*)malloc(sizeof(float)*nb_point);

	return true;
}  // end of load for CSampleFftFilter


//**********************************
bool CSampleFftFilter::prepare(void)
//**********************************
{
	rX = rY = rZ = 	R = 0;

	y1 = y2 = y3 = -1;
	Y1 = Y2 = Y3 = -1;

	return true;
} // end of prepare for CSampleFftFilter


//*****************************************************
void CSampleFftFilter::processFrame8(SFrameInfo &frame)
//****************************************************
{
	pre_update(frame.fftBass);

	// Affichage.
	for(int x=0;x<nb_point;x++) {

		int couleur2;

		if (x==y1 || x==Y1 || x==y2 || x==Y2 || x==y3 || x==Y3) {
			couleur2 = 250;
		}
		else {
			if (FFT[x]<10)
				couleur2= 32;
			else if (FFT[x]<90)
				couleur2 = 49-((FFT[x]-10.0)*(8.0/80.0));
			else
				couleur2 = 42;
		}

		for(int y=0;y<nb_lat;y++) {
			long Xa,Ya,Xb,Yb;
			
			long X=donuts[(y<<DEC)+x].x;
			long Y=donuts[(y<<DEC)+x].y;
		
			if (x==0) {
				if (y==0) {
					Xa=donuts[(y<<DEC)+(nb_point-1)].x;
					Ya=donuts[(y<<DEC)+(nb_point-1)].y;
					Xb=donuts[x+((nb_lat-1)<<DEC)].x;
					Yb=donuts[x+((nb_lat-1)<<DEC)].y;
				}
				else {
					Xa=donuts[(nb_point-1)+(y<<DEC)].x;
					Ya=donuts[(nb_point-1)+(y<<DEC)].y;
					Xb=donuts[((y-1)<<DEC)+x].x;
					Yb=donuts[((y-1)<<DEC)+x].y;
				}
			}
			else {
				if (y==0) {
					Xa=donuts[(x-1)+(y<<DEC)].x;
					Ya=donuts[(x-1)+(y<<DEC)].y;
					Xb=donuts[x+((nb_lat-1)<<DEC)].x;
					Yb=donuts[x+((nb_lat-1)<<DEC)].y;
				}
				else {
					Xa=donuts[(x-1)+(y<<DEC)].x;
					Ya=donuts[(x-1)+(y<<DEC)].y;
					Xb=donuts[x+((y-1)<<DEC)].x;
					Yb=donuts[x+((y-1)<<DEC)].y;
				}
			}
			// trace des reliures des anneaux.
			int x1;
			if (x==0)
				x1 = nb_point-1;
			else
				x1=x-1;
			if (FFT[x]>10 || FFT[x1]>10) {
				int coul;
				float cormax = max(FFT[x],FFT[x1]);
				if (cormax<90)
					coul = 49-((cormax-10.0)*(8.0/80.0));
				else
					coul = 42;
				blitLine8(X,Y,Xa,Ya,coul);
			}
			else
				blitLine8(X,Y,Xa,Ya,32);
			// traces des anneaux.
			blitLine8(X,Y,Xb,Yb,couleur2);
		}
	}

	post_update();

}  // end of processFrame8 for CSampleFftFilter


//******************************************************
void CSampleFftFilter::processFrame32(SFrameInfo &frame)
//*****************************************************
{
	pre_update(frame.fftBass);

	// Affichage.
	for(int x=0;x<nb_point;x++) {

		const int couleur32 = (  0<<16)|(  0<<8)|(248<<0);	// 32=blue
		const int couleur42 = (248<<16)|(  0<<8)|(  0<<0);	// 42=rouge
		const int couleur250= (250<<16)|(250<<8)|( 51<<0);	// 250=jaune
		int couleur2;

		if (x==y1 || x==Y1 || x==y2 || x==Y2 || x==y3 || x==Y3) {
			couleur2 = couleur250;
		}
		else {
			if (FFT[x]<10)
				couleur2= couleur32;
			else if (FFT[x]<90)
				//couleur2 = 49-((FFT[x]-10.0)*(8.0/80.0));
				couleur2 = (int)(80+(175*(FFT[x]-10.0))/8)<<16; // red: 80->255 (delta 175)
			else
				couleur2 = couleur42;
		}

		for(int y=0;y<nb_lat;y++) {
			long Xa,Ya,Xb,Yb;
			
			long X=donuts[(y<<DEC)+x].x;
			long Y=donuts[(y<<DEC)+x].y;
		
			if (x==0) {
				if (y==0) {
					Xa=donuts[(y<<DEC)+(nb_point-1)].x;
					Ya=donuts[(y<<DEC)+(nb_point-1)].y;
					Xb=donuts[x+((nb_lat-1)<<DEC)].x;
					Yb=donuts[x+((nb_lat-1)<<DEC)].y;
				}
				else {
					Xa=donuts[(nb_point-1)+(y<<DEC)].x;
					Ya=donuts[(nb_point-1)+(y<<DEC)].y;
					Xb=donuts[((y-1)<<DEC)+x].x;
					Yb=donuts[((y-1)<<DEC)+x].y;
				}
			}
			else {
				if (y==0) {
					Xa=donuts[(x-1)+(y<<DEC)].x;
					Ya=donuts[(x-1)+(y<<DEC)].y;
					Xb=donuts[x+((nb_lat-1)<<DEC)].x;
					Yb=donuts[x+((nb_lat-1)<<DEC)].y;
				}
				else {
					Xa=donuts[(x-1)+(y<<DEC)].x;
					Ya=donuts[(x-1)+(y<<DEC)].y;
					Xb=donuts[x+((y-1)<<DEC)].x;
					Yb=donuts[x+((y-1)<<DEC)].y;
				}
			}
			// trace des reliures des anneaux.
			int x1;
			if (x==0)
				x1 = nb_point-1;
			else
				x1=x-1;
			if (FFT[x]>10 || FFT[x1]>10) {
				int coul;
				float cormax = max(FFT[x],FFT[x1]);
				if (cormax<90)
					//coul = 49-((cormax-10.0)*(8.0/80.0));
					coul = (80+(175*(cormax-10.0))/8); // red: 80->255 (delta 175)
				else
					coul = couleur42;
				blitLine32(X,Y,Xa,Ya,coul);
			}
			else
				blitLine32(X,Y,Xa,Ya,couleur32);
			// traces des anneaux.
			blitLine32(X,Y,Xb,Yb,couleur2);
		}
	}

	post_update();

}  // end of processFrame8 for CSampleFftFilter


//**************************************
void CSampleFftFilter::post_update(void)
//**************************************
{
	// eventuellement on deplace les tours jaunes ...
	if (y1!=-1) {
		if (!(c1%4)) {
			if (++y1>(nb_point>>1)-1)
				y1=-1;
			if (--Y1<(nb_point>>1))
				Y1=-1;
		}
	}
	if (y2!=-1) {
		if (!(c2%4)) {
			if (++y2>(nb_point>>1)-1)
				y2=-1;
			if (--Y2<(nb_point>>1))
				Y2=-1;
		}
	}
	if (y3!=-1) {
		if (!(c3%4)) {
			if (++y3>(nb_point>>1)-1)
				y3=-1;
			if (--Y3<(nb_point>>1))
				Y3=-1;
		}
	}
	c1++;
	c2++;
	c3++;
	return;
}  // end of post_update for CSampleFftFilter



//*********************************************
void CSampleFftFilter::pre_update(float *chanR)
//*********************************************
{
	int w2 = sPrepare.sx>>1;
	int h2 = sPrepare.sy>>1;

	int indice;
	float correction;
	
	rZ = (rZ+5)%360;
	rX = (rX+4)%360;
	rY = (rY+3)%360;

	R = (R+1)%360;

	// decalage pour le centrage
	int hsin = 120.0*sintab->Sin(R)+h2;
	int wcos = 160.0*sintab->Cos(R)+w2;

	// modif Major
	float cor2 = chanR[10]*(1.0/30000.0);
	float cor3 = chanR[9]*(1.0/30000.0);
	
	if (cor2>35.0 || cor3>35.0) {
		if (y1==-1) {
			y1 = 0;
			Y1 = nb_point-1;
			c1=0;
		}
		else if (y2==-1 && y1>1) {
			y2 = 0;
			Y2 = nb_point-1;
			c2=0;
		}
		else if (y3==-1 && y1>1 && y2>1) {
			y3 = 0;
			Y3 = nb_point-1;
			c3=0;
		}
	}

	// remplissage du tableau
	for (int i=0;i<(nb_point>>1);i++) {
		indice = (i<<8)/nb_point;

		if (i) {
			correction = chanR[indice-2];
			correction += chanR[indice-1];
			correction += chanR[indice];
			correction += chanR[indice+1];
			correction += chanR[indice+2];
			correction /= (125000.0-5000.0*i);
		}
		else {
			correction = chanR[indice+1];
			correction += chanR[indice+2];
			correction += chanR[indice+3];
			correction = correction * (1.0/75000.0);
		}

		FFT[i] = correction;
		FFT[nb_point-1-i] = correction;

		for (int j=0;j<nb_lat;j++) {
			int id1 = (j<<DEC)+i;
			int id2 = (j<<DEC)+(nb_point-1-i);
			donuts[id1].SetTo((Major+cor2+(Minor+correction)*tetamCos[j])*tetaMCos[i],(Major+cor3+(Minor+correction)*tetamCos[j])*tetaMSin[i],(Minor+correction)*tetamSin[j]);
			donuts[id2].x = donuts[id1].x;
			donuts[id2].y = 0.0-donuts[id1].y;
			donuts[id2].z = donuts[id1].z;

			//rotation des deux points ...
			donuts[id1].FastRotateZ(rZ);
			donuts[id1].FastRotateX(rX);
			donuts[id1].FastRotateZ(rY);
			donuts[id2].FastRotateZ(rZ);
			donuts[id2].FastRotateX(rX);
			donuts[id2].FastRotateZ(rY);
			// + centrage;
			donuts[id1].x+=wcos;
			donuts[id1].y+=hsin;
			donuts[id2].x+=wcos;
			donuts[id2].y+=hsin;
		}
	}

}  // end of pre_update for CSampleFftFilter

