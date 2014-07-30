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



#ifndef RZ_VIEW_BASE_H
#define RZ_VIEW_BASE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/factory.h"
#include "nel/gui/interface_element.h"

namespace NLGUI
{
	class CEventDescriptor;

	class CViewBase : public CInterfaceElement
	{
	public:

		// for factory construction
		struct TCtorParam
		{};

		/// Constructor
		CViewBase(const TCtorParam &/* param */) : CInterfaceElement()
		{
		}

		/// Destructor
		virtual ~CViewBase();

		// Returns 'true' if that element can be downcasted to a view
		virtual bool isView() const { return true; }

		/// Draw the view from XReal, YReal, WReal, HReal (implemented by derived classes)
		/// this coordinates are relative to the screen bottom left and begins the bottom left of the view
		virtual void draw () = 0;

		virtual void updateCoords() { CInterfaceElement::updateCoords(); }

		/// Debug
		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }

		/// Reflection
		virtual sint32 getAlpha() const  { return -1; }	// Not obliged to implement this
		virtual void setAlpha (sint32 /* a */) {}				// Not obliged to implement this


		void    copyOptionFrom(const CViewBase &other)
		{
			CInterfaceElement::copyOptionFrom(other);
		}


		REFLECT_EXPORT_START(CViewBase, CInterfaceElement)
			REFLECT_SINT32 ("alpha", getAlpha, setAlpha);
		REFLECT_EXPORT_END

		virtual void	dumpSize(uint depth = 0) const;

		// from CInterfaceElement
		virtual void visit(CInterfaceElementVisitor *visitor);

		// special for mouse over : return true and fill the name of the cursor to display
		virtual bool getMouseOverShape(std::string &/* texName */, uint8 &/* rot */, NLMISC::CRGBA &/* col */) { return false; }
		
		/// Handle all events (implemented by derived classes) (return true to signal event handled)
		virtual bool handleEvent (const NLGUI::CEventDescriptor &evnt);

	};

}

#endif // RZ_VIEW_BASE_H

/* End of view_base.h */
