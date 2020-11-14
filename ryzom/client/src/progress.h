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


#ifndef CL_PROGRESS_H
#define CL_PROGRESS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/progress_callback.h"
//
#include "nel/misc/event_listener.h"


namespace NL3D
{
	class UTextureFile;
}



#define PROGRESS_BAR_BORDER 0
#define PROGRESS_BAR_LEFT 0.25f
#define PROGRESS_BAR_TOP 0.09f
#define PROGRESS_BAR_WIDTH (1.f-2.f*PROGRESS_BAR_LEFT)
#define PROGRESS_BAR_HEIGHT 0.02f
#define PROGRESS_BAR_UPDATE 200
#define PROGRESS_BAR_BORDER_COLOR (CRGBA (255, 255, 255))
#define PROGRESS_BAR_COLOR (CRGBA (224, 200, 157))
#define PROGRESS_BAR_BG_COLOR (CRGBA (224/4, 200/4, 157/4))


// ***************************************************************************
// Client progress class
class CProgress : public NLMISC::IProgressCallback,
				  public NLMISC::IEventListener
{
public:

	CProgress ();
	virtual ~CProgress();

	void release();

	// Update the progress bar
	virtual void	progress (float value);

	// Reset the root progress bar
	void			reset (uint rootNodeCount);

	// Finish the progress bar
	void			finish ();

	// New message
	void			newMessage (const ucstring& message);

	void			setFontFactor(float f);

	// display some custom text messages(text defined in client.cfg)
	bool			ApplyTextCommands;

	// Set teleport specific message
	void			setTPMessages(const ucstring &tpReason, const ucstring &tpCancelText, const std::string &iconName);

	bool			getTPCancelFlag(bool clearFlag = true);

private:

	// Internal progress
	void			internalProgress (float value);

	// Display a text to describe what is the application going to do.
	// this function can be call even if texture is NULL, driver or textcontext not initialised
	ucstring		_ProgressMessage;

	// Time since last update
	sint64			_LastUpdate;

	float			_FontFactor;
	uint			_CurrentRootStep;
	uint			_RootStepCount;

	ucstring		_TPReason;
	ucstring		_TPCancelText;

	bool			_TPCancelFlag;

protected:
	// from IEventListener
	virtual void operator ()(const NLMISC::CEvent& event);
};



#endif // CL_PROGRESS_H

/* End of progress.h */
