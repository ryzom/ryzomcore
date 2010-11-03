// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "app_bundle_utils.h"

#if defined(NL_OS_MAC)
#include <CoreFoundation/CoreFoundation.h>

std::string getAppBundlePath() 
{
	static std::string cachedPathToBundle;

	if(cachedPathToBundle.size())
		return cachedPathToBundle;

	// get the bundle
	CFBundleRef bundle = CFBundleGetMainBundle();

	if(bundle)
	{
		// get the url to the bundles root
		CFURLRef url = CFBundleCopyBundleURL(bundle);

		if(url)
		{
			// get the file system path
			CFStringRef str;
			str = CFURLCopyFileSystemPath(
				CFURLCopyAbsoluteURL(url), kCFURLPOSIXPathStyle);
			CFRelease(url);
	
			if(str)
			{
				cachedPathToBundle = CFStringGetCStringPtr(
					str, CFStringGetSmallestEncoding(str));
				CFRelease(str);

				return cachedPathToBundle;
			}
			else
				nlerror("CFStringGetCStringPtr");
		}
		else
			nlerror("CFBundleCopyBundleURL");
	}
	else
		nlerror("CFBundleGetMainBundle");
	
	return std::string();
}

#endif // defined(NL_OS_MAC)
