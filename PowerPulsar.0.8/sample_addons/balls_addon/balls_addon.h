/************************************************************
	Author	: Jonathan Perret
	1997	: JP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Jonathan Perret <jperret@cybercable.fr>

************************************************************/

#include <support/Debug.h>
#include "3dVector.h"
#include "CFilter.h"

struct PixelSprite
{
	int width,height;
	uchar *data;

	PixelSprite() { width=height=0; data=NULL; }
};

struct Ball
{
	BPoint pos;
	float ix,iy;
	int anim_step;
};

// some constants :
// max number of balls (size of array)
const int n_balls_max=300;
// desired frame time in µs
// NOT USED in balls_addon
//const float frame=30000; 

const int anim_frames = 14;
const int anim_div = 1;

// VectorBalls max number of divisions
const int NP = 60;

class balls_addon : public CFilter
{
private:
// common	
	PixelSprite *the_sprite;
	
	PixelSprite the_anim[anim_frames];
	
	BRect fBounds;
	
	uchar *fBits; // careful - it's changing
	uint32 fBpr;

	uint32 fWidth,fHeight;

	bool animate;
	bool back;
	bool bench;
	
	bigtime_t start_time;
	bigtime_t last_calc;
	bigtime_t last_frame;
	
	bool vectormode; // classic(false) or vector ?
	
// "classic balls" only

	int n_balls;
	int n_visible;
	
	Ball balls[n_balls_max];
	
// VectorBalls only

	int xcenter,ycenter;

	float distance;
	float f;
	float pasangle;
	
	float alpha,cosalpha,sinalpha;
	float beta,cosbeta,sinbeta;
	float gamma,cosgamma,singamma;

	int np,next_np,nballs;
	
	float morph_factor;
	float morph_inc;

	float t;
	float expand_factor;

	B3dVector points[NP*NP];
	int ordre[NP*NP];

	bool teapot_mode;
	int teapot_npoints;
	B3dVector *teapot_points;
	
	float projette(float x,float y,float z,short *xe,short *ye);
	void put3dball(float x1,float y1,float z1);
	void put3dball(B3dVector &v) { put3dball(v.x,v.y,v.z); };
	void newalpha(float a);
	void newbeta(float b);
	void newgamma(float g);
	void dessin();
	void trace(float x,float z);
	void calcul();
	void tri();
		
public:

	balls_addon(bool vectormode);
	
	virtual bool load();
	virtual bool prepare();
	
	virtual void processFrame8(SFrameInfo&);
	
	virtual void terminate();
	virtual void unload();
	
	void *Bits() const { return sFrame->screen; }
	int32 BytesPerRow() const { return sPrepare.bpr; }
	BRect Bounds() const { return fBounds; }
	
	inline void Box(int left,int top,int right,int bottom,uchar col)
	{ Box(BRect(left,top,right,bottom),col); };
	void Box(BRect,uchar);

	inline void PutSprite(PixelSprite *spr,BPoint where)
	{ PutSprite(spr,int(where.x),int(where.y)); };
	void PutSprite(PixelSprite *spr,int32 x,int32 y);
	
};

#define FOR_EACH_BALL(counter) \
	for (int counter=0;counter<n_balls;counter++)

