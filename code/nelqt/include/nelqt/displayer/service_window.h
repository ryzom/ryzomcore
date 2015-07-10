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

#ifndef NLQT_SERVICE_WINDOW_H
#define NLQT_SERVICE_WINDOW_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes
#include <QMainWindow>
#include <QFont>

// NeL includes
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>
#include <nel/misc/callback.h>

// Project includes

class QHBoxLayout;
class QVBoxLayout;

namespace NLQT {
	class CCommandLog;

typedef NLMISC::CCallback<void, QWidget *> TDisplayerButtonCallback;
typedef NLMISC::CCallback<void> TDisplayerTimerCallback;
typedef NLMISC::CCallback<void> TDisplayerExitCallback;

class CServiceWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	CServiceWindow(QWidget *parent = NULL, Qt::WindowFlags flags = 0);
	virtual ~CServiceWindow();

	inline void setButtonCallback(const TDisplayerButtonCallback &cb) { m_ButtonCallback = cb; }
	inline void setTimerCallback(const TDisplayerTimerCallback &cb) { m_TimerCallback = cb; }
	inline void setExitCallback(const TDisplayerExitCallback &cb) { m_ExitCallback = cb; }
	inline CCommandLog *commandLog() { return m_CommandLog; }

	QWidget *addLabel();
	QWidget *addButton();
	void addLine();

private slots:
	void buttonCallback();
	void timerCallback();

private:
	CCommandLog *m_CommandLog;
	TDisplayerButtonCallback m_ButtonCallback;
	TDisplayerTimerCallback m_TimerCallback;
	TDisplayerExitCallback m_ExitCallback;

	QVBoxLayout *m_LabelVBox;
	QHBoxLayout *m_LabelHBox;

	QFont m_Font;

private:
	CServiceWindow(const CServiceWindow &);
	CServiceWindow &operator=(const CServiceWindow &);
	
}; /* class CServiceWindow */

} /* namespace NLQT */

#endif /* #ifndef NLQT_SERVICE_WINDOW_H */

/* end of file */
