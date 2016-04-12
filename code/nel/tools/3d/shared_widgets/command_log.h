/*

Copyright (C) 2010-2015  by authors
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

#ifndef NLQT_COMMAND_LOG_H
#define NLQT_COMMAND_LOG_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes
#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>

// NeL includes
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>
#include <nel/misc/callback.h>

// Project includes

namespace NLQT {

typedef NLMISC::CCallback<void, const std::string &> TCommandExecute;

class CCommandLog : public QWidget
{
	Q_OBJECT
	
public:
	CCommandLog(QWidget *parent);
	virtual ~CCommandLog();

	void setExecCommand(const TCommandExecute &func) { m_Func = func; }
	void doDisplay(const NLMISC::CLog::TDisplayInfo& args, const char *message);

	void clear() { m_DisplayerOutput->clear(); }

signals:
	void tSigDisplay(const QColor &c, const QString &text);
	void execCommand(const QString &cmd);

private slots:
	void returnPressed();
	void tSlotDisplay(const QColor &c, const QString &text);

private:
	QTextEdit *m_DisplayerOutput;
	QLineEdit *m_CommandInput;
	TCommandExecute m_Func;

private:
	CCommandLog(const CCommandLog &);
	CCommandLog &operator=(const CCommandLog &);
	
}; /* class CCommandLog */

class CCommandLogDisplayer : public CCommandLog, public NLMISC::IDisplayer
{
	Q_OBJECT

public:
	CCommandLogDisplayer(QWidget *parent);
	virtual ~CCommandLogDisplayer();

protected:
	virtual void doDisplay(const NLMISC::CLog::TDisplayInfo& args, const char *message);

private slots:
	void execCommandLog(const QString &cmd);

private:
	NLMISC::CLog m_Log;

private:
	CCommandLogDisplayer(const CCommandLogDisplayer &);
	CCommandLogDisplayer &operator=(const CCommandLogDisplayer &);

}; /* class CCommandLogDisplayer */

} /* namespace NLQT */

#endif /* #ifndef NLQT_COMMAND_LOG_H */

/* end of file */
