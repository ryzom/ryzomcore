// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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


#ifndef GROUP_CONTAINER_BASE_H
#define GROUP_CONTAINER_BASE_H

#include "nel/gui/interface_group.h"
#include "nel/misc/rgba.h"

namespace NLGUI
{

	class CGroupContainerBase : public CInterfaceGroup
	{
	public:
		DECLARE_UI_CLASS( CGroupContainerBase )

		CGroupContainerBase( const TCtorParam &param );
		virtual ~CGroupContainerBase();

		virtual void removeAllContainers();
		virtual void setLocked( bool locked );
		bool isLocked() const { return _Locked; }

		uint8  getContainerAlpha() const { return _ContainerAlpha; }
		uint8  getContentAlpha() const { return _ContentAlpha; }
		uint8  getRolloverAlphaContent() const { return _RolloverAlphaContent; }
		uint8  getRolloverAlphaContainer() const { return _RolloverAlphaContainer; }

		void setContainerAlpha( uint8 alpha );
		void setContentAlpha( uint8 alpha );
		void setRolloverAlphaContent( uint8 alpha );
		void setRolloverAlphaContainer( uint8 alpha );

		// for export
		sint32  getContainerAlphaAsSInt32() const{ return (sint32)_ContainerAlpha; }
		sint32  getContentAlphaAsSInt32() const{ return (sint32)_ContentAlpha; }
		sint32  getRolloverAlphaContentAsSInt32() const{ return (sint32)_RolloverAlphaContent; }
		sint32  getRolloverAlphaContainerAsSInt32() const{ return (sint32)_RolloverAlphaContainer; }

		// sin32 versions for export
		void setContainerAlpha( sint32 alpha ){ setContainerAlpha((uint8) alpha); }
		void setContentAlpha( sint32 alpha ){ setContentAlpha((uint8) alpha); }
		void setRolloverAlphaContent( sint32 alpha ){ setRolloverAlphaContent((uint8) alpha); }
		void setRolloverAlphaContainer( sint32 alpha ){ setRolloverAlphaContainer((uint8) alpha); }

		void setUseGlobalAlpha( bool use );
		bool isUsingGlobalAlpha() const{ return _UseGlobalAlpha; }

		std::string getAHOnAlphaSettingsChanged() const{ return CAHManager::getInstance()->getAHName( _AHOnAlphaSettingsChanged ); }
		std::string getAHOnAlphaSettingsChangedParams() const{ return _AHOnAlphaSettingsChangedParams; }

		void setAHOnAlphaSettingsChanged( const std::string &h ){ _AHOnAlphaSettingsChanged = CAHManager::getInstance()->getAH( h, _AHOnAlphaSettingsChangedParams ); }
		void setAHOnAlphaSettingsChangedParams( const std::string &p ){ _AHOnAlphaSettingsChangedParams = p; }

		REFLECT_EXPORT_START( CGroupContainerBase, CInterfaceGroup )
			REFLECT_SINT32("container_alpha", getContainerAlphaAsSInt32, setContainerAlpha);
			REFLECT_SINT32("content_alpha", getContentAlphaAsSInt32, setContentAlpha);
			REFLECT_SINT32("rollover_content_alpha", getRolloverAlphaContentAsSInt32, setRolloverAlphaContent);
			REFLECT_SINT32("rollover_container_alpha", getRolloverAlphaContainerAsSInt32, setRolloverAlphaContainer);
			REFLECT_BOOL("use_global_alpha_settings", isUsingGlobalAlpha, setUseGlobalAlpha);
			REFLECT_STRING("on_alpha_settings_changed", getAHOnAlphaSettingsChanged, setAHOnAlphaSettingsChanged);
			REFLECT_STRING("on_alpha_settings_changed_params", getAHOnAlphaSettingsChangedParams, setAHOnAlphaSettingsChangedParams);
		REFLECT_EXPORT_END

		virtual bool isMoving() const{ return false; }

		// Get the header color draw. NB: depends if grayed, and if active.
		virtual NLMISC::CRGBA getDrawnHeaderColor () const{ return NLMISC::CRGBA(); };

		uint8 getCurrentContainerAlpha() const { return _CurrentContainerAlpha; }
		uint8 getCurrentContentAlpha() const { return _CurrentContentAlpha; }

		virtual bool isGrayed() const { return false; }
		virtual bool getTouchFlag(bool /* clearFlag */) const { return false; }
		virtual void backupPosition() {}
		virtual void restorePosition() {}

	protected:
		void triggerAlphaSettingsChangedAH();

		uint8 _CurrentContainerAlpha;
		uint8 _CurrentContentAlpha;
		uint8 _ContainerAlpha;
		uint8 _ContentAlpha;
		uint8 _RolloverAlphaContainer; // Alpha for the window when mouse not over it
		uint8 _RolloverAlphaContent;   // Alpha for the content when mouse not over it
		bool _Locked : 1; // Is the container locked (ie override movable, openable ...)
		bool _UseGlobalAlpha : 1;

		IActionHandler *_AHOnAlphaSettingsChanged;
		CStringShared  _AHOnAlphaSettingsChangedParams;

	private:

	};



}


#endif
