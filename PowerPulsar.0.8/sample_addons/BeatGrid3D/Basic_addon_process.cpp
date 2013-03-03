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




void CSampleFftFilter::DrawTriangleFlat(int xa,int ya, int xb, int yb, int xc, int yc, int color)
{
	float delta1=0,delta2=0;
	float xd,xf;
	int l;
	int test1=0,test2=0,test3=0,test4=0;
	
	// Tri sur les Y.
	if (ya>yb)
	{
		swap(ya,yb);
		swap(xa,xb);
	}
	if (ya>yc)
	{
		swap(ya,yc);
		swap(xa,xc);
	}
	if (yb>yc)
	{
		swap(yb,yc);
		swap(xb,xc);
	}

	// Remplissage
	if (ya!=yc) delta1=((float)(xa-xc))/((float)(ya-yc));
	if (ya!=yb) delta2=((float)(xb-xa))/((float)(yb-ya));

	xd=xa;
	xf=xa;

	// premiere partie.
	if (delta1<delta2)
	{
		for(l=ya+1;l<=yb;l++)
		{
			xd+=delta1;
			xf+=delta2;
			blitLine8(xd,l,xf,l,color);
		}
	}
	else
	{
		for(l=ya+1;l<=yb;l++)
		{
			xd+=delta1;
			xf+=delta2;
			blitLine8(xf,l,xd,l,color);	
		}
	}

	// Deuxieme partie... 
	// si b et c sont au meme niveau pas besin de continuer.
	if (yb!=yc)
	{
		delta2=((float)(xb-xc))/((float)(yb-yc));
		xf=xb;
		
		if(delta2<delta1)
		{
			for(l=yb+1;l<=yc;l++)
			{
				xd+=delta1;
				xf+=delta2;
				blitLine8(xd,l,xf,l,color);
				test3++;
			}
		}
		else
		{
			for(l=yb+1;l<=yc;l++)
			{
				xd+=delta1;
				xf+=delta2;
				blitLine8(xf,l,xd,l,color);
				test4++;
			}
		}	
	}
}