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

static int numberForKey( CFDictionaryRef desc, CFStringRef key )
{
    CFNumberRef value;
    int num = 0;

    if ( (value = (CFNumberRef)CFDictionaryGetValue(desc, key)) == NULL )
        return 0;
    CFNumberGetValue(value, kCFNumberIntType, &num);
    return num;
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

	nldebug("3D: %d displays found\n", (int)numDisplays);

	for (CGDisplayCount i = 0; i < numDisplays; ++i)
	{
		CGDirectDisplayID dspy = display[i];

		CFArrayRef modeList = CGDisplayAvailableModes(dspy);
		if (modeList == NULL)
		{
			nlwarning("Display is invalid");
			continue;
		}
		CFIndex cnt = CFArrayGetCount(modeList);
		nldebug("3D: Display 0x%x has %d modes:", (unsigned int)dspy, (int)cnt);
		for (CFIndex j = 0; j < cnt; ++j)
		{
			CFDictionaryRef desc = (CFDictionaryRef)CFArrayGetValueAtIndex(modeList, j);

			int w = numberForKey(desc, kCGDisplayWidth);
			int h = numberForKey(desc, kCGDisplayHeight);
			int bpp = numberForKey(desc, kCGDisplayBitsPerPixel);
			int freq = numberForKey(desc, kCGDisplayRefreshRate);

			if (bpp >= 16)
			{
				// Add this mode
				GfxMode mode;
				mode.Width = (uint16)w;
				mode.Height = (uint16)h;
				mode.Depth = (uint8)bpp;
				mode.Frequency = freq;
				modes.push_back (mode);
			}

			nldebug( "  > Mode %d: %dx%d, %d BPP, %d Hz", j, w, h, bpp, freq);
		}
	}
	return true;
}

}

#endif
