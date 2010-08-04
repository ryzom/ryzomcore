#include "stdopengl.h"

#include <nel/misc/types_nl.h>

#ifdef NL_OS_MAC

#include <nel/3d/driver.h>

#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

namespace NL3D
{

static int bppFromDisplayMode(CGDisplayModeRef mode)
{
	CFStringRef pixelEncoding = CGDisplayModeCopyPixelEncoding(mode);
	
	if(CFStringCompare(pixelEncoding, CFSTR(IO32BitDirectPixels), 
			kCFCompareCaseInsensitive) == kCFCompareEqualTo) 
		return 32;
	
	else if(CFStringCompare(pixelEncoding, CFSTR(IO16BitDirectPixels), 
			kCFCompareCaseInsensitive) == kCFCompareEqualTo)
		return 16;
	
	return 0;
}

bool getMacModes(std::vector<GfxMode> &modes)
{
	static const CGDisplayCount kMaxDisplays = 16;
	CGDirectDisplayID display[kMaxDisplays];
	CGDisplayCount numDisplays;

	CGDisplayErr err = CGGetActiveDisplayList(kMaxDisplays, display, &numDisplays);
	if (err != CGDisplayNoErr)
	{
		nlwarning("Cannot get displays (%d)", err);
		return false;
	}

	nldebug("3D: %d displays found", (int)numDisplays);

	for (CGDisplayCount i = 0; i < numDisplays; ++i)
	{
		CGDirectDisplayID dspy = display[i];
		CFArrayRef modeList = CGDisplayCopyAllDisplayModes(dspy, NULL);
		
		if (modeList == NULL)
		{
			nlwarning("Display is invalid");
			continue;
		}

		for (CFIndex j = 0; j < CFArrayGetCount(modeList); ++j)
		{
			CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(modeList, j);
			int bpp = bppFromDisplayMode(mode);
			
			if (bpp >= 16)
			{
				int w = CGDisplayModeGetWidth(mode);
				int h = CGDisplayModeGetHeight(mode);

				// Add this mode
				GfxMode mode;
				mode.Width = (uint16)w;
				mode.Height = (uint16)h;
				mode.Depth = (uint8)bpp;

				// frequency stays at 0 because on mac cocoa, display resolution
				// is never really changed. if rendering resolution < display resolution
				// cocoa interpolates and keeps the display at it's original resolution
				mode.Frequency = 0;
				modes.push_back (mode);

				nldebug( " Display 0x%x: Mode %dx%d, %d BPP", dspy, w, h, bpp);
			}
		}
	}
	return true;
}

}

#endif
