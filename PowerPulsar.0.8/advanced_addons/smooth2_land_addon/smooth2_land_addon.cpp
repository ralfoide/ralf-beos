/*****************************************************************************

	Projet	: Pulsar Sample FFT AddOn

	Fichier	: sample_addon.cpp
	Partie	: End

	Auteur	: RM
	Date		: 040597 -- version 0.2 (C funcs)
	Date		: 010797 -- version 0.3 (CFilter)
	Format	: tabs==2

*****************************************************************************/

#include "PP_Be.h"
#include "CFilter.h"
#include <math.h>

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" _ADDON_IMPEXP void * filterInit(uint32 index);

#define ADDON_NAME "SmoothLand2"

static const bool debug=false;

#ifndef M_PI
#define M_PI 3.141592654
#endif

const float FOV=M_PI/2;   // half of the xy field of view
#define KSX 320
#define KSX1 (KSX-1)
#define KSY 240
#define KSY1 (KSY-1)

//--------------------------------------------------------------------


//*************************************
class CVoxelFilter : public CFilter
//*************************************
{
public:

	CVoxelFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CVoxelFilter(void) 		{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void);
	virtual void processFrame8(SFrameInfo &frame);

private:
	uchar *HMap;
	uchar *CMap;
	uchar *Video;

	int lasty[KSX],       // Last pixel drawn on a given column
    lastc[KSX];         // Color of last pixel on a column

	void Line(int x0,int y0,int x1,int y1,int hy,int s);
	void ComputeVoxel(int x0,int y0,float aa);
	int Clamp(int x);
	void ComputeMap(void);

	float angle;
	int32 speed;
	int x0,y0;
	int lasth;

}; // end of class defs for CVoxelFilter


//--------------------------------------------------------------------

//**********************************************
int main(int, char **)
//**********************************************
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


//**********************************************
void * filterInit(uint32 index)
//**********************************************
{
	// this add-on only declares ONE filter
	if (index > 0) return NULL;

	CVoxelFilter *info = new CVoxelFilter;
	if (!info) return NULL;		// memory error, give up

	// returns the instance -- the caller will check that this instance
	// is derived from CFilter but is _not_ a CFilter...
	return (void *)info;
} // end of filterInit


//*******************************
bool CVoxelFilter::load(void)
//*******************************
{
	sFilter.name = "Smooth2 Land";
	sFilter.author = "SpH & R'alf";
	sFilter.info = "Da code";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionFirst;
	sFilter.supportedMode = (color_space)(B_COLOR_8_BIT);

	sFilter.icon = loadBitmap("./add-ons.icons/fft.tga");

	HMap = new uchar[256*256];
	CMap = new uchar[256*256];
	Video = new uchar[KSX*KSY];

	return true;
}  // end of load for CVoxelFilter


//**********************************
bool CVoxelFilter::prepare(void)
//**********************************
{

	angle = 0.0;
	speed = 0;
	x0=y0=0;
	lasth = 0;

  memset(Video,0,KSX*KSY);
	ComputeMap();

	return true;
} // end of prepare for CVoxelFilter


//***************************************
void CVoxelFilter::processFrame8(SFrameInfo &frame)
//***************************************
{
	if(1)
	{
		double	*base;
		long		bpr;
		int			x0bis, y0bis;
		float 	a;
		static float lastRaie = 0;
		static float idealraie;
		
		base	= (double *)frame.screen;
		bpr		= sPrepare.bpr;
		x0bis = x0;
		y0bis = y0;

		float raie = frame.meanFftBass[0]/8192;
		float diffraie = (raie-lastRaie);
		idealraie = idealraie+(diffraie-idealraie)/10;
		lastRaie = raie;

		a = angle;

		ComputeVoxel(x0bis,y0bis,a);

#ifdef DISPLAY_NORMAL
		base--;
		double *video_ptr = ((double *)Video)-1;
		long offset = bpr/sizeof(double);
		for (int j=0;j<KSY;j++)
		{
			double *base2 = base;
			for (int i=0;i<KSX/sizeof(double);i++)
				*(++base2) = *(++video_ptr);
			base += offset;
		}
#else // display doubling

	#ifdef __POWERPC__
		if(1)
		{
			int source_height = KSY;
			double temp[1];
			register double temp2;
			char * source_bitmap = (char *)Video;
			char * dest_bitmap = (char *)base;
			while (--source_height >= 0)
			{
				unsigned long * src = ((unsigned long *)source_bitmap);
				double * dst1 = ((double *)dest_bitmap)-1;
				double * dst2 = dst1+bpr/8;
				int w = KSX;
				while ((w-=4) >= 0)
				{
					unsigned long pixx = *src;
					*(src++) = 0;
					unsigned char * mid = (unsigned char *)&temp[1];
					*(--mid) = pixx;
					*(--mid) = pixx;
					pixx >>= 8;
					*(--mid) = pixx;
					*(--mid) = pixx;
					pixx >>= 8;
					*(--mid) = pixx;
					*(--mid) = pixx;
					pixx >>= 8;
					*(--mid) = pixx;
					*(--mid) = pixx;
					temp2 = temp[0];
					*(++dst1) = temp2;
					*(++dst2) = temp2;
				}
				source_bitmap += KSX;
				dest_bitmap += 2*bpr;
			}
		}
	#elif __INTEL__
		if(1)
		{
			int source_height;
			uchar * source_bitmap = (uchar *)Video;
			uchar * dest_bitmap = (uchar *)base;
			int dest_bpr = 2*bpr; //*2/sizeof(uint16);
			for(source_height=0; source_height<KSY; source_height++)
			{
				#ifdef _ENABLE_INTEL_ASM
					asm
					{
						push	edi
						push	esi
						push	ebx
	
						mov		esi,source_bitmap
						mov		edi,dest_bitmap
						mov		ebx,bpr
						mov		ecx,KSX
	
						pf8_x86_duploop:
						
						lodsb						// *(source_bitmap++) --> AL
						mov		ah,al
						mov		word ptr [edi+ebx],ax
						stosw						// *(dest_bitmap++) <-- AX
						
						loop	pf8_x86_duploop
						
						mov		edi,source_bitmap
						xor		eax,eax
						mov		ecx,KSX/4
						rep		stosd
						
						pop		ebx
						pop		esi
						pop		edi
					}
					source_bitmap += KSX;
					dest_bitmap += dest_bpr;
				#else
					uchar *src = source_bitmap;
					uint16 *dest1= (uint16 *)dest_bitmap;
					uint16 *dest2= (uint16 *)(dest_bitmap+bpr);
					int i;
					
					for(i=0; i<KSX; i++)
					{
						uint16 a = *(src);
						a = a | (a<<8);
						*(dest1++) = a;
						*(dest2++) = a;
						*(src++) = 0;
					}
					
					source_bitmap += KSX;
					dest_bitmap += dest_bpr;
				#endif
			}
		}
	#endif

#endif

		long sp = (1<<speed) + frame.averageFftBass;
		x0 = x0bis+sp*cos(a);
		y0 = y0bis+sp*sin(a);
		float a2 = a+idealraie*FOV/20.0;
		a = (a+a2)/2;
		if (a > M_PI) a -= M_PI;
		if (a < -M_PI) a += M_PI;
	
		if (modifiers() & B_LEFT_SHIFT_KEY) a -= FOV/10.0;
		else if (modifiers() & B_RIGHT_SHIFT_KEY) a += FOV/10.0;
				
		if (modifiers() & B_CONTROL_KEY && speed < 16) speed++;
		else if (modifiers() & B_COMMAND_KEY && speed > 1) speed--;
	
		angle = a;
	}
}  // end of processFrame8 for CVoxelFilter

//******************************************
int CVoxelFilter::Clamp(int x)
//******************************************
{
  return (x<0 ? 0 : (x>255 ? 255 : x));
}

//******************************************
void CVoxelFilter::ComputeMap(void)
//******************************************
{
  int p,i,j,k,k2,p2;

  //
  // Start from a plasma clouds fractal
  //
  HMap[0]=128;
  for ( p=256; p>1; p=p2 )
  {
    p2=p>>1;
    k=p*8+20; k2=k>>1;
    for ( i=0; i<256; i+=p )
    {
      for ( j=0; j<256; j+=p )
      {
		int a,b,c,d;

		a=HMap[(i<<8)+j];
		b=HMap[(((i+p)&255)<<8)+j];
		c=HMap[(i<<8)+((j+p)&255)];
		d=HMap[(((i+p)&255)<<8)+((j+p)&255)];

		HMap[(i<<8)+((j+p2)&255)]=
		  	Clamp(((a+c)>>1)+(rand()%k-k2));
		HMap[(((i+p2)&255)<<8)+((j+p2)&255)]=
	 		Clamp(((a+b+c+d)>>2)+(rand()%k-k2));
		HMap[(((i+p2)&255)<<8)+j]=
	  		Clamp(((a+b)>>1)+(rand()%k-k2));
      }
    }
  }

  //
  // Smoothing
  //
  for ( k=0; k<3; k++ )
    for ( i=0; i<256*256; i+=256 )
      for ( j=0; j<256; j++ )
      {
		HMap[i+j]=(HMap[((i+256)&0xFF00)+j]+HMap[i+((j+1)&0xFF)]+
		   HMap[((i-256)&0xFF00)+j]+HMap[i+((j-1)&0xFF)])>>2;
      }

  //
  // Color computation (derivative of the height field)
  //
  for ( i=0; i<256*256; i+=256 )
    for ( j=0; j<256; j++ )
    {
      k=128+(HMap[((i+256)&0xFF00)+((j+1)&255)]-HMap[i+j])*4;
      if ( k<0 ) k=0; if (k>255) k=255;
      CMap[i+j]=k;
    }
 
 	memset(HMap,0,256*256); // HOOK BOURRIN
    
}

//******************************************
void CVoxelFilter::Line(int x0,int y0,int x1,int y1,int hy,int s)
//******************************************
{
  int i,sx,sy;
	int32 startline=sFrame->startingLine;

  // Compute xy speed
  sx=(x1-x0)/KSX; sy=(y1-y0)/KSX;
  for ( i=0; i<KSX; i++ )
  {
    int c,y,h,u0,v0,u1,v1,a,b,h0,h1,h2,h3;

    //
    // Compute the xy coordinates; a and b will be the position inside the
    // single map cell (0..255).
    //
    u0=(x0>>16)&0xFF;    a=(x0>>8)&255;
    v0=((y0>>8)&0xFF00); b=(y0>>8)&255;
    u1=(u0+1)&0xFF;
    v1=(v0+256)&0xFF00;

    //
    // Fetch the height at the four corners of the square the point is in
    //
    h0=HMap[u0+v0]; h2=HMap[u0+v1];
    h1=HMap[u1+v0]; h3=HMap[u1+v1];

    //
    // Compute the height using bilinear interpolation
    //
    h0=(h0<<8)+a*(h1-h0);
    h2=(h2<<8)+a*(h3-h2);
    h=((h0<<8)+b*(h2-h0))>>16;

    //
    // Fetch the color at the four corners of the square the point is in
    //
    h0=CMap[u0+v0]; h2=CMap[u0+v1];
    h1=CMap[u1+v0]; h3=CMap[u1+v1];

    //
    // Compute the color using bilinear interpolation (in 16.16)
    //
    h0=(h0<<8)+a*(h1-h0);
    h2=(h2<<8)+a*(h3-h2);
    c=((h0<<8)+b*(h2-h0));

    //
    // Compute screen height using the scaling factor
    //
    y=(((h-hy)*s)>>11)+100;

    //
    // Draw the column
    //
    if ( y<(a=lasty[i]) )
    {
      unsigned char *b=Video+a*KSX+i;
      int sc,cc;


      if ( lastc[i]==-1 )
		lastc[i]=c;

      sc=(c-lastc[i])/(a-y);
      cc=lastc[i];

      if ( a>KSY1 ) { b-=(a-KSY1)*KSX; cc+=(a-KSY1)*sc; a=KSY1; }
      if ( y<0 ) y=0;

#undef INTERP
#ifdef INTERP
      
      while ( y<a )
      {
		// *b=cc>>18; // --> color map entre 0 et 63
		*b=cc>>19;		// be clut : 32 gray
		cc+=sc;
		b-=KSX; a--;
      }
      lasty[i]=y;


#else
	if(1)
	{
	  uchar col=c>>19;
      while ( y<a )
      {
		// *b=cc>>18; // --> color map entre 0 et 63
		*b=col;
		b-=KSX; a--;
      }
      lasty[i]=y;
    }
#endif

    }
    lastc[i]=c;

    //
    // Advance to next xy position
    //
    x0+=sx; y0+=sy;
  }
}

//******************************************
void CVoxelFilter::ComputeVoxel(int x0,int y0,float aa)
//******************************************
{
  int d;
  int a,b,h,u0,v0,u1,v1,h0,h1,h2,h3;

  //
  // Clear offscreen buffer
  //
  //memset(Video,0,KSX*KSY);

  //
  // Initialize last-y and last-color arrays
  //
  for ( d=0; d<KSX; d++ )
  {
    lasty[d]=KSY;
    lastc[d]=-1;
  }

  //
  // Compute viewpoint height value
  //

  //
  // Compute the xy coordinates; a and b will be the position inside the
  // single map cell (0..255).
  //
  u0=(x0>>16)&0xFF;    a=(x0>>8)&255;
  v0=((y0>>8)&0xFF00); b=(y0>>8)&255;
  u1=(u0+1)&0xFF;
  v1=(v0+256)&0xFF00;

  //
  // Fetch the height at the four corners of the square the point is in
  //
  h0=HMap[u0+v0]; h2=HMap[u0+v1];
  h1=HMap[u1+v0]; h3=HMap[u1+v1];

  //
  // Compute the height using bilinear interpolation
  //
  h0=(h0<<8)+a*(h1-h0);
  h2=(h2<<8)+a*(h3-h2);
  h=((h0<<8)+b*(h2-h0))>>16;

	int hbis = lasth;
	if (h > hbis) h = hbis+(h-hbis)/8;
	lasth = h;

  //
  // Draw the landscape from near to far without overdraw
  //
  for ( d=0; d<100; d+=1+(d>>6) )
  {
    Line(x0+d*65536*cos(aa-FOV),y0+d*65536*sin(aa-FOV),
         x0+d*65536*cos(aa+FOV),y0+d*65536*sin(aa+FOV),
         h-30,100*256/(d+1));
  }

/*  float	xa,ya,xb,yb;
  xa = cos(aa-FOV);
  ya = sin(aa-FOV);
  xb = cos(aa+FOV);
  yb = sin(aa+FOV);
  for(d=0; d<100; d+=1+(d>>6) )
  {
  	float s=1/(d+1);
    Line(x0+d*65536*xa,y0+d*65536*ya,
         x0+d*65536*xb,y0+d*65536*yb,
         h-30,100*256*s);
  }
*/
  //
  // Blit the final image to the screen
  //
//  memcpy((unsigned char *)0xA0000,Video,KSX*KSY);
}

// eoc
