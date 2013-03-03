#include <support/Debug.h>

#include "CFilter.h"

// some constants :

// desired frame time in µs
// NOT USED in bigfft_addon
//const float frame=30000; 

class bigfft_addon : public CFilter
{
private:
// common	
	BRect fBounds;
	
public:

	bigfft_addon();
	
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

};
