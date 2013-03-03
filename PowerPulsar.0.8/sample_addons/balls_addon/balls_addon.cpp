/************************************************************
	Author	: Jonathan Perret
	1997	: JP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Jonathan Perret <jperret@cybercable.fr>

************************************************************/

#include "PP_Be.h"
#include "CFilter.h"
#include <math.h>

//#define DEBUG 1
#include <support/Debug.h>

#include "balls_addon.h"


#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" void *filterInit(uint32 index);

//---------------------------------------------------------


// filter-specific info that can be in a constant
struct MyFilterInfo
{
	char *name;			// name of filter
	char *author;		// author (name, email... one line)
	char *info;			// one line of extra info
	uint32	majorVersion;
	uint32	minorVersion;

	bool			eraseScreen;
	EFilterPosition position;				// defaults to kFilterPositionAny
	EColorSpace 	supportedMode;			// kColor8Bit

	void FillSFilterInfo(SFilterInfo& sFilter) 
	{
		sFilter.name = name;
		sFilter.author = author;
		sFilter.info = info;
		sFilter.majorVersion = majorVersion;
		sFilter.minorVersion = minorVersion;
		sFilter.eraseScreen = eraseScreen;
		sFilter.position = position;
		sFilter.supportedMode = supportedMode;
	};
	
};

static MyFilterInfo classic_info =
{
	"Balls", "Jonathan Perret <jperret@cybercable.fr>",
	"Tons of bouncing balls !", // extra info
	0,2, // version
	false, // eraseScreen
	kFilterPositionAny,   // position
	kColor8Bit // supportedMode
};

static MyFilterInfo vector_info =
{
	"VectorBalls", "Jonathan Perret <jperret@cybercable.fr>",
	"Mmm... flying Balls ?", // extra info
	0,1, // version
	false, // eraseScreen
	kFilterPositionAny,   // position
	kColor8Bit // supportedMode
};

//--------------------------------------------------------------------


//********************
int main(int, char **)
//********************
// Code to help a little the fellow user double-clicking the
// add-on icon directly.
{
	#define ADDON_NAME "VectorBalls"
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

void *filterInit(uint32 index)
{
	switch(index)
	{
		case 0:	return new balls_addon(true); // vector balls
		case 1:	return new balls_addon(false); // classic balls
	}
	return 0;
}

//--------------------------------------------------------------------

balls_addon::balls_addon(bool vectormode)
: CFilter(),
	the_sprite(0),
	animate(false), back(true), bench(false),  
	last_frame(0), start_time(0), last_calc(0),
	vectormode(vectormode),
	// classic balls
	n_balls(0), n_visible(0),
	// vector balls
	distance(3),f(400),pasangle(0.15),
	alpha(0),cosalpha(1),sinalpha(0),
	beta(0),cosbeta(1),sinbeta(0),
	gamma(0),cosgamma(1),singamma(0),
	np(20), next_np(np),
	morph_factor(1), morph_inc(0.05),
	t(0),expand_factor(1),
	teapot_mode(false), teapot_npoints(0),
	teapot_points(0)
{
}

bool balls_addon::load()
{
//	(new BAlert(0,"load",
//		"clik"))->Go();

	if(!vectormode)
		classic_info.FillSFilterInfo(sFilter);
	else
		vector_info.FillSFilterInfo(sFilter);

	// load things from resources
	the_sprite = new PixelSprite;
	the_sprite->width=16;
	the_sprite->height=16;
	
	// let's load
	BFile *rfile = new BFile(&sLoad.addonEntry,B_READ_ONLY);
	BResources *res = new BResources(rfile);
	
	size_t datalength;
	
	void *icon_data = 
		res->FindResource('ICON',"BEOS:L:STD_ICON",&datalength);
	ASSERT_WITH_MESSAGE(datalength==1024,"failed to load the icon");
	
	sFilter.icon = new BBitmap(BRect(0,0,31,31),B_COLOR_8_BIT);
	sFilter.icon->SetBits(icon_data,datalength,0,B_COLOR_8_BIT);
	
	free(icon_data);
	
	the_sprite->data =
		(uchar *)res->FindResource('RAW8',"Ball",&datalength);
	ASSERT_WITH_MESSAGE(datalength==256,"failed to load the Ball");
		
	for(int i=0;i<anim_frames;i++) {
		the_anim[i].width =
		the_anim[i].height = 16;
		the_anim[i].data = 
			(uchar *)res->FindResource('MICN',10+i,&datalength);
		ASSERT_WITH_MESSAGE(datalength==256,"failed to load a TennisBall");
		}
		
	delete res;
	delete rfile;
	// done loading
	
	// now load the TEAPOT !!!
	FILE *f=fopen("/boot/beos/etc/teapot.data","r");
	if(f) {
		char s[255];
		
		fgets(s,sizeof(s)-1,f);
		
		sscanf(s,"%d",&teapot_npoints);
		
		teapot_points = new B3dVector[teapot_npoints];
		
		for(int i=0;i<teapot_npoints;i++) {
				fgets(s,sizeof(s)-1,f);
				
				sscanf(s,"%f %f %f",
					&(teapot_points[i].x),
					&(teapot_points[i].y),
					&(teapot_points[i].z)
					);
					
				teapot_points[i].x/=2.5;
				teapot_points[i].y/=2.5;
				teapot_points[i].z/=2.5;
				teapot_points[i].z-=.5;
							
			}
		}
	fclose(f);
	
	// for animation in vectorballs
	balls->anim_step = 0;

	return true;
}

bool balls_addon::prepare()
{
	fBounds.Set(0,0,sPrepare.sx-1,sPrepare.sy-1);
	
	fBits = (uchar *)Bits();
	fBpr = BytesPerRow();
	fWidth = sPrepare.sx;
	fHeight = sPrepare.sy;

	const float minspeed=3;
	const float maxspeed=10;
	
	if(!vectormode) {
		n_balls=100;
		for(int i=0;i<n_balls;i++) {
			Ball &ball = balls[i];
			ball.pos.Set(rand()%sPrepare.sx,rand()%(sPrepare.sy/2));
			ball.ix = (minspeed+rand()*(maxspeed-minspeed)/RAND_MAX)
				*(rand()>RAND_MAX/2?1:-1);
			ball.iy = (minspeed+rand()*(maxspeed-minspeed)/RAND_MAX)
				*(rand()>RAND_MAX/2?1:-1);
			ball.anim_step = rand()%anim_frames;
			}
		}
	else {
		xcenter = sPrepare.sx/2;
		ycenter = sPrepare.sy/2;
		}	
	
	return true;
}

void balls_addon::processFrame8(SFrameInfo &frame)
{

//	printf("blittime=%Ld\n",last_frame-last_calc);	
	
	last_frame = system_time();
	
	fBits = (uchar *)Bits();
	
	if(!vectormode) {
			
		// Classic Balls
		
		int new_n_visible=0;
		
		FOR_EACH_BALL(i)
		{
			Ball &ball=balls[i];
			
			BPoint &pos=ball.pos;
			
			float &ix=ball.ix, &iy=ball.iy;
			
			pos.x+=ix;
			pos.y+=iy;
		
			if (pos.x<8) {pos.x=8;ix=-ix;}
			if (pos.x>fBounds.right-7) {pos.x=fBounds.right-7;ix*=-1;};
			if (pos.y>fBounds.bottom-7) {pos.y=2*(fBounds.bottom-7)-pos.y;iy*=-1;};		
		
			iy+=0.2;
			if (pos.y>-8) 
			{
				if(!animate) 
					PutSprite(the_sprite,pos-BPoint(8,8));
				else {
					PutSprite(&the_anim[ball.anim_step/anim_div],
						pos-BPoint(8,8));
					ball.anim_step = 
						(ball.anim_step + 1)%(anim_frames*anim_div);
					}
					
				new_n_visible++;
			}
		}
		}
	else { 
	
		// Vector Balls
		
		const float periode = 8;
		
		float u=fmod(t,periode)/periode;
		
		if(u<.5) {
			teapot_mode = false;
			}
		else if(u>=.5) {
			teapot_mode = true;
			}

		if(0/*teapot_mode*/) {
				newbeta(beta+0.01);
			} else {
				newalpha(alpha+0.03);
				newbeta(beta+0.03);
				newgamma(gamma-0.02);
			}
		t+=0.05;
		//fprintf(stderr,"t=%f\n",t);
		morph_factor += morph_inc;
		if(morph_factor<0) morph_factor=0;
		else if(morph_factor>1) morph_factor=1;
		np=next_np;	
		nballs = np*np;
		expand_factor = frame.meanFftBass[0]/1e6;
		calcul();
		tri();
		dessin();
		}
	
	last_calc = system_time()-start_time;

}

void balls_addon::terminate()
{
}

void balls_addon::unload()
{
	free(the_sprite->data);
	
	delete the_sprite;
			
	for(int i=0;i<anim_frames;i++)
		delete the_anim[i].data;
}

// graphics support routines
#pragma mark -- graphics support --

void balls_addon::Box(BRect rect,uchar col)
{
	if (rect_8 && sPrepare.isGameKit) {
		int32 a=sFrame->startingLine;
		rect_8(int(rect.left),a+int(rect.top),
			int(rect.right),a+int(rect.bottom),col);
		}
	else {
		int y;
		uchar *offset;
		
		rect = rect & fBounds;
		
		offset=((uchar *)Bits())+int(rect.top)*BytesPerRow()+int(rect.left);
		for (y=0;y<=rect.IntegerHeight();y++) {
			memset(offset,col,rect.IntegerWidth()+1);
			offset+=BytesPerRow();
			}
		}

}

// PutSprite draws a sprite at x y, skipping pixels of color zero.
//void balls_addon::PutSprite(PixelSprite *spr,int x,int y)
//{
//	ASSERT(spr);
//	ASSERT(spr->data);
//	
//	// eliminate trivial cases
//	if (x>Bounds().right || y>Bounds().bottom) return;
//
//	register uint i;
//	register uchar *source=spr->data;
//	register uchar *dest;
//	register uint destdelta,srcdelta;
//	register int w = spr->width, h = spr->height;
//	register uint bpr = BytesPerRow();	
//		
//	if (x+w<=0 || y+h<=0) return;
//	
//	// row clipping
//	if(y<0) {
//		source += -y*w;
//		h -= -y;
//		y = 0;
//		}
//	int overbot = y+h-1-int(Bounds().bottom);
//	if(overbot>0) {
//		h -= overbot;
//		}
//		
//	// column clipping
//	srcdelta = 0;
//	if(x<0) {
//		source += -x;
//		w -= -x;
//		srcdelta += -x;
//		x = 0;
//		}
//	int overright = x+w-1-int(Bounds().right);
//	if(overright>0) {
//		w -= overright;
//		srcdelta += overright;
//		}	
//	
//	dest = (uchar *)Bits()+y*bpr+x-1;
//	destdelta = bpr-w;
//	
//	
//#pragma warn_possunwant off // sometimes I know what I'm doing
//
//	register int32 c;
//	while(h)
//	{
//		for (i=0;i<w;i++)
//		{
//			if (c=*source++) *++dest=c; // hurry up !
////			dest++;
//		}
//		dest+=destdelta;
//		source+=srcdelta;
//		--h;
//	}
//	
//#pragma warn_possunwant reset 		
//}

// VectorBalls implementation stuff - the old 3d code

#pragma mark -- 3 D code--

#define fonc1(x,z) (cos(5*(x+t))*sin(5*(z+t))/4)
#define fonc2(x,z) (cos(5*(x+t+1))*log(5*(/*z+*/t+1))/8)

#define tourneX(x,y,z,xr,yr,zr) { \
	xr=x; \
	yr=y*cosalpha-z*sinalpha; \
	zr=y*sinalpha+z*cosalpha; }

#define tourneY(x,y,z,xr,yr,zr) { \
	xr=x*cosbeta-z*sinbeta;\
	yr=y;\
	zr=x*sinbeta+z*cosbeta; }

#define tourneZ(x,y,z,xr,yr,zr) { \
	xr=x*cosgamma-y*singamma;\
	yr=x*singamma+y*cosgamma;\
	zr=z; }

float balls_addon::projette(float x,float y,float z,short *xe,short *ye) {
	z+=distance;
	*xe=xcenter+f*x/z;
	*ye=ycenter-f*y/z;
  return z;
	}

void balls_addon::newalpha(float a) {
	alpha=a;
	cosalpha=cos(a);
	sinalpha=sin(a);
	}

void balls_addon::newbeta(float b) {
	beta=b;
	cosbeta=cos(b);
	sinbeta=sin(b);
	}

void balls_addon::newgamma(float g) {
	gamma=g;
	cosgamma=cos(g);
	singamma=sin(g);
	}

void balls_addon::put3dball(float x1,float y1,float z1) {
	short xe,ye;
  float d;
	d=projette(x1,y1,z1,&xe,&ye);
	
	if(!animate) 
		PutSprite(the_sprite,xe-8,ye-8);
	else {
		PutSprite(&the_anim[balls->anim_step/anim_div],
			xe-8,ye-8);
		balls->anim_step = 
			(balls->anim_step + 1)%(anim_frames*anim_div);
		}
	}

void balls_addon::dessin(void) 
{
	int i;
	int n=teapot_mode?teapot_npoints:nballs;
	for(i=0;i<n;i++)
		put3dball(points[ordre[i]]);
}

void balls_addon::calcul(void) 
{
	int i,j;

	if(!teapot_mode) {	
		for (i=0;i<np;i++)
			for (j=0;j<np;j++) {
				register float x,y,z,xr,yr,zr;
				x = (float)i/((float)np/2)-1;
				z = (float)j/((float)np/2)-1;
				y = expand_factor*(morph_factor*fonc1(x,z));// + (1-morph_factor)*fonc2(x,z));
				
				x*=expand_factor;
				z*=expand_factor;
				
				// y clamping
				if (fabs(y)>1) y=0;
				
				tourneX(x,y,z,xr,yr,zr);
				tourneY(xr,yr,zr,x,y,z);
				tourneZ(x,y,z,xr,yr,zr);
				
				points[j*np+i].Set(xr,yr,zr);
				
				}
		}
	else {
		for(i=0;i<teapot_npoints;i++) {
				register float x,y,z,xr,yr,zr;
				x = teapot_points[i].x;
				y = teapot_points[i].y;
				z = teapot_points[i].z;
				
				tourneX(x,y,z,xr,yr,zr);
				tourneY(xr,yr,zr,x,y,z);
				tourneZ(x,y,z,xr,yr,zr);
				
				points[i].Set(expand_factor*xr,expand_factor*yr,expand_factor*zr);
			}
		}
		
}

static B3dVector *gpoints;

int compare(const int*p1, const int*p2);
int compare0(const int*p1, const int*p2);

int compare0(const int*p1, const int*p2)
{
	if(gpoints[*p1].z > gpoints[*p2].z)
		return -1;
	else if(gpoints[*p1].z == gpoints[*p2].z)
		return 0;
	else return 1;
} 

#ifdef __MWERKS__
	asm int compare
		(const int*p1, const int*p2)
	{
		lfs f1,0(r3)
		lfs f2,0(r4)
		fcmpo cr0,f1,f2
		bge ge
		li r3,-1
		blr
	ge:
		beq eq
		li r3,1
		blr
	eq:
		li r3,0
		blr
	}
#else
	// hein ??!!
#endif
	
void balls_addon::tri(void) 
{
	int i;
	if(!teapot_mode) {
		for(i=0;i<nballs;i++)
			ordre[i]=i;
		gpoints = points;		
		qsort(ordre,nballs,sizeof(int),(_compare_function)&compare0);
		}
	else {
		for(i=0;i<teapot_npoints;i++)
			ordre[i]=i;
		gpoints = points;
		qsort(ordre,teapot_npoints,sizeof(int),(_compare_function)&compare0);
		}
}

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif

// eoc
