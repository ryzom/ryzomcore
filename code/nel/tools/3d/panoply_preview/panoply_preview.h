// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014  Jan BOON (jan.boon@kaetemi.be)
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

#ifndef NLTOOLS_PANOPLY_PREVIEW_H
#define NLTOOLS_PANOPLY_PREVIEW_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes
#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QSlider>
#include <QImage>
#include <QPixmap>

// NeL includes
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>

// Project includes

namespace NLMISC {
	class CBitmap;
	class IThread;
}

namespace NLTOOLS {
	class CMainWindow;
	class CColorThread;

/**
 * CPanoplyPreview
 * \brief CPanoplyPreview
 * \date 2014-09-19 09:38GMT
 * \author Jan BOON (jan.boon@kaetemi.be)
 */
class CPanoplyPreview : public QWidget
{
	Q_OBJECT
	
public:
	CPanoplyPreview(CMainWindow *parent);
	virtual ~CPanoplyPreview();

	void displayBitmap(const NLMISC::CBitmap &bitmap); // Called from thread!

protected:
	virtual void paintEvent(QPaintEvent *e);

signals:
	void tSigBitmap();

private slots:
	void tSlotBitmap();

	void colorEdited(const QString &text);
	void maskEdited(const QString &text);
	void goPushed(bool);

	void hueChanged(int value);
	void lightnessChanged(int value);
	void saturationChanged(int value);
	void luminosityChanged(int value);
	void contrastChanged(int value);

private:
	void createDockWindows(CMainWindow *mainWindow);

private:
	NLMISC::IThread *m_Thread;
	CColorThread *m_ColorThread;

	QString m_ColorFile;
	QString m_MaskFile;

	QImage *m_Image;
	QPixmap *m_Pixmap;

	NLMISC::CMutex m_ImageMutex;

private:
	CPanoplyPreview(const CPanoplyPreview &);
	CPanoplyPreview &operator=(const CPanoplyPreview &);
	
}; /* class CPanoplyPreview */

class CSliderTextEdit : public QSlider
{
	Q_OBJECT
	
public:
	CSliderTextEdit(QWidget *parent, QLineEdit *lineEdit, float scale);
	virtual ~CSliderTextEdit();

private slots:
	void lineEditTextEdited(const QString &text);
	void sliderValueChanged(int value);

private:
	QLineEdit *m_LineEdit;
	bool m_Updating;
	float m_Scale;

};

} /* namespace NLTOOLS */

#endif /* #ifndef NLTOOLS_PANOPLY_PREVIEW_H */

/* end of file */
