/*

Copyright (C) 2015  by authors
Author: Jan Boon <jan.boon@kaetemi.be>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef NLQT_SERVICE_DISPLAYER_H
#define NLQT_SERVICE_DISPLAYER_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>
#include <nel/misc/window_displayer.h>

// Project includes

class QWidget;

namespace NLQT {
	class CServiceWindow;
	class CCommandLog;

class CServiceDisplayer : public NLMISC::CWindowDisplayer
{
public:
	CServiceDisplayer(const char *displayerName);
	virtual ~CServiceDisplayer();

	virtual void setTitleBar(const std::string &titleBar);

protected:
	virtual void doDisplay(const NLMISC::CLog::TDisplayInfo& args, const char *message);

	virtual void open(int argc, char **argv, std::string titleBar, bool iconified, sint x, sint y, sint w, sint h, sint hs, sint fs, const std::string &fn, bool ww, NLMISC::CLog *log);

	virtual void display_main();

	void commandExecute(const std::string &cmd);
	void buttonCallback(QWidget *sender);
	void timerCallback();
	void exitCallback();

private:
	CServiceWindow *m_ServiceWindow;
	CCommandLog *m_CommandLog;
	NLMISC::CLog *m_Log;
	NLMISC::CSynchronized<std::vector<std::pair<NLMISC::CLog::TDisplayInfo, std::string> > > m_DelayLog;

	NLMISC::CSynchronized<std::string> m_SetTitleBar;
	bool m_DoSetTitleBar;

private:
	CServiceDisplayer(const CServiceDisplayer &);
	CServiceDisplayer &operator=(const CServiceDisplayer &);
	
}; /* class CServiceDisplayer */

} /* namespace NLQT */

#endif /* #ifndef NLQT_SERVICE_DISPLAYER_H */

/* end of file */
