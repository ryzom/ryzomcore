#include "nel/gui/dbview_bar3.h"
#include "nel/gui/dbview_number.h"

namespace NLGUI
{
	/// Necessary so the linker doesn't drop the code of these classes from the library
	void LinkHack()
	{
		CDBViewBar3::forceLink();
		CDBViewNumber::forceLink();
	}
}