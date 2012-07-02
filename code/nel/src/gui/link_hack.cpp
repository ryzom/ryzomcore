#include "nel/gui/dbview_bar3.h"
#include "nel/gui/dbview_number.h"
#include "nel/gui/dbview_quantity.h"

namespace NLGUI
{
	void ifexprufct_forcelink();

	/// Necessary so the linker doesn't drop the code of these classes from the library
	void LinkHack()
	{
		CDBViewBar3::forceLink();
		CDBViewNumber::forceLink();
		CDBViewQuantity::forceLink();
		ifexprufct_forcelink();
	}
}