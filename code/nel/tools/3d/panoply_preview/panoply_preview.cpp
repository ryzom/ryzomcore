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

#include <nel/misc/types_nl.h>
#include "panoply_preview.h"

// STL includes

// Qt includes
#include <QVBoxLayout>
#include <QDockWidget>
#include <QMenu>
#include <QGroupBox>
#include <QLineEdit>
#include <QSlider>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/command.h>
#include <nel/misc/path.h>
#include <nel/misc/thread.h>
#include <nel/misc/mutex.h>
#include <nel/misc/bitmap.h>

// Project includes
#include "main_window.h"
#include "../panoply_maker/color_modifier.h"

using namespace std;
using namespace NLMISC;

namespace NLTOOLS {

class CColorThread : public NLMISC::IRunnable
{
public:
	// Called when a thread is run.
	virtual void run()
	{
		while (Running)
		{
			SettingsMutex.enter();
			if (!Running)
			{
				SettingsMutex.leave();
				return;
			}
			if (!Process)
			{
				SettingsMutex.leave();
				nlSleep(100);
				continue;
			}
			nldebug("Update color modifier");
			m_ColorModifier.Hue = Hue;
			m_ColorModifier.Lightness = Lightness;
			m_ColorModifier.Saturation = Saturation;
			m_ColorModifier.Luminosity = Luminosity;
			m_ColorModifier.Saturation = Saturation;
			SettingsMutex.leave();

			BitmapMutex.enter();
			if (!Running)
			{
				BitmapMutex.leave();
				return;
			}
			if (!m_BitmapsOk)
			{
				nldebug("Bitmaps not ready");
				BitmapMutex.leave();
				nlSleep(100);
				continue;
			}
			float retDeltaHue;
			m_ColorModifier.convertBitmap(DestBitmap, ColorBitmap, MaskBitmap, retDeltaHue);
			BitmapMutex.leave();

			PanoplyPreview->displayBitmap(DestBitmap);

			nlSleep(100);
		}
	}

	CColorThread() : m_BitmapsOk(false), Hue(0), Lightness(0), Saturation(0), Luminosity(0), Contrast(0), Process(false), Running(true) { }
	virtual ~CColorThread() { }
	virtual void getName (std::string &result) const { result = "CColorThread"; }

private:
	CColorModifier m_ColorModifier;
	bool m_BitmapsOk;

public:
	CPanoplyPreview *PanoplyPreview;

	NLMISC::CMutex BitmapMutex;
	NLMISC::CBitmap ColorBitmap;
	NLMISC::CBitmap MaskBitmap;
	NLMISC::CBitmap DestBitmap;

	NLMISC::CMutex SettingsMutex;
	float Hue;
	float Lightness;
	float Saturation;
	float Luminosity;
	float Contrast;
	bool Process;

	bool Running;
};

// *****************************************************************

CPanoplyPreview::CPanoplyPreview(CMainWindow *parent) : QWidget(parent)
{
	m_DisplayerOutput = new QTextEdit();
	m_DisplayerOutput->setReadOnly(true);
	m_DisplayerOutput->setFocusPolicy(Qt::NoFocus);
	m_CommandInput = new QLineEdit();

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_DisplayerOutput);
	layout->addWidget(m_CommandInput);
	setLayout(layout);

	// connect(m_CommandInput, SIGNAL(returnPressed()), this, SLOT(returnPressed()));

	createDockWindows(parent);

	m_ColorThread = new CColorThread();
	m_ColorThread->PanoplyPreview = this;
	m_Thread = IThread::create(m_ColorThread);
	m_Thread->start();
}

CPanoplyPreview::~CPanoplyPreview()
{
	m_ColorThread->BitmapMutex.enter();
	m_ColorThread->SettingsMutex.enter();
	m_ColorThread->Running = false;
	m_ColorThread->SettingsMutex.leave();
	m_ColorThread->BitmapMutex.leave();
	m_Thread->wait();
	delete m_Thread;
	delete m_ColorThread;
}

void CPanoplyPreview::displayBitmap(const CBitmap &bitmap) // Called from thread!
{
	nldebug("display bitmap!");
}

void CPanoplyPreview::colorEdited(const QString &text)
{
	
}

void CPanoplyPreview::maskEdited(const QString &text)
{
	
}

void CPanoplyPreview::goPushed()
{
	
}

void CPanoplyPreview::hueChanged(int value)
{
	float v = (float)value;
	m_ColorThread->SettingsMutex.enter();
	m_ColorThread->Hue = v;
	m_ColorThread->Process = true;
	m_ColorThread->SettingsMutex.leave();
}

void CPanoplyPreview::lightnessChanged(int value)
{
	float v = (float)value * 0.01f;
	m_ColorThread->SettingsMutex.enter();
	m_ColorThread->Lightness = v;
	m_ColorThread->Process = true;
	m_ColorThread->SettingsMutex.leave();
}

void CPanoplyPreview::saturationChanged(int value)
{
	float v = (float)value * 0.01f;
	m_ColorThread->SettingsMutex.enter();
	m_ColorThread->Saturation = v;
	m_ColorThread->Process = true;
	m_ColorThread->SettingsMutex.leave();
}

void CPanoplyPreview::luminosityChanged(int value)
{
	float v = (float)value;
	m_ColorThread->SettingsMutex.enter();
	m_ColorThread->Luminosity = v;
	m_ColorThread->Process = true;
	m_ColorThread->SettingsMutex.leave();
}

void CPanoplyPreview::contrastChanged(int value)
{
	float v = (float)value;
	m_ColorThread->SettingsMutex.enter();
	m_ColorThread->Contrast = v;
	m_ColorThread->Process = true;
	m_ColorThread->SettingsMutex.leave();
}

// *****************************************************************

CSliderTextEdit::CSliderTextEdit(QWidget *parent, QLineEdit *lineEdit, float scale) : QSlider(Qt::Horizontal, parent), m_LineEdit(lineEdit), m_Updating(false), m_Scale(scale)
{
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
	connect(lineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(lineEditTextEdited(const QString &)));
}

CSliderTextEdit::~CSliderTextEdit()
{
	
}

void CSliderTextEdit::lineEditTextEdited(const QString &text)
{
	if (!m_Updating)
	{
		m_Updating = true;
		setValue((int)(text.toFloat() * m_Scale));
		m_Updating = false;
	}
}

void CSliderTextEdit::sliderValueChanged(int value)
{
	if (!m_Updating)
	{
		m_Updating = true;
		m_LineEdit->setText(QString::number((double)value / (double)m_Scale));
		m_Updating = false;
	}
}

// *****************************************************************

void CPanoplyPreview::createDockWindows(CMainWindow *mainWindow)
{
	nlassert(mainWindow);

	// Color Modifier
	{
		QDockWidget *dockWidget = new QDockWidget(mainWindow);
		nlassert(dockWidget);
		dockWidget->setWindowTitle(tr("Color Modifier"));
		dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		QScrollArea *scrollArea = new QScrollArea(dockWidget);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setWidgetResizable(true);
		QWidget *widget = new QWidget(scrollArea);
		QVBoxLayout *vboxLayout = new QVBoxLayout(widget);

		// Input File Paths
		{
			QGroupBox *groupBox = new QGroupBox(widget);
			groupBox->setTitle(tr("Input File Paths"));
			QGridLayout *groupLayout = new QGridLayout(groupBox);

			QLabel *colorLabel = new QLabel(groupBox);
			colorLabel->setText(tr("Color: "));
			groupLayout->addWidget(colorLabel, 0, 0);

			QLineEdit *colorFile = new QLineEdit(groupBox);
			groupLayout->addWidget(colorFile, 0, 1);

			QLabel *maskLabel = new QLabel(groupBox);
			maskLabel->setText(tr("Mask: "));
			groupLayout->addWidget(maskLabel, 1, 0);

			QLineEdit *maskFile = new QLineEdit(groupBox);
			groupLayout->addWidget(maskFile, 1, 1);

			QPushButton *go = new QPushButton(groupBox);
			go->setText(tr("Go"));
			groupLayout->addWidget(go, 2, 0, 1, 2);

			groupBox->setLayout(groupLayout);
			vboxLayout->addWidget(groupBox);
		}

		// Color Modifier
		{
			QGroupBox *groupBox = new QGroupBox(widget);
			groupBox->setTitle(tr("Color Modifier"));
			QGridLayout *groupLayout = new QGridLayout(groupBox);

			QLabel *label;
			QLineEdit *edit;
			CSliderTextEdit *slider;

			label = new QLabel(groupBox);
			label->setText(tr("Hue [0, 360]: "));
			groupLayout->addWidget(label, 0, 0);
			
			edit = new QLineEdit(groupBox);
			edit->setText("0");
			groupLayout->addWidget(edit, 0, 1);

			slider = new CSliderTextEdit(groupBox, edit, 1.0f);
			slider->setMinimum(0);
			slider->setMaximum(360);
			slider->setValue(0);
			groupLayout->addWidget(slider, 1, 0, 1, 2);

			connect(slider, SIGNAL(valueChanged(int)), this, SLOT(hueChanged(int)));

			label = new QLabel(groupBox);
			label->setText(tr("Lightness [-1, 1]: "));
			groupLayout->addWidget(label, 2, 0);
			
			edit = new QLineEdit(groupBox);
			edit->setText("0");
			groupLayout->addWidget(edit, 2, 1);

			slider = new CSliderTextEdit(groupBox, edit, 100.0f);
			slider->setMinimum(-100);
			slider->setMaximum(100);
			slider->setValue(0);
			groupLayout->addWidget(slider, 3, 0, 1, 2);

			connect(slider, SIGNAL(valueChanged(int)), this, SLOT(lightnessChanged(int)));

			label = new QLabel(groupBox);
			label->setText(tr("Saturation [-1, 1]: "));
			groupLayout->addWidget(label, 4, 0);
			
			edit = new QLineEdit(groupBox);
			edit->setText("0");
			groupLayout->addWidget(edit, 4, 1);

			slider = new CSliderTextEdit(groupBox, edit, 100.0f);
			slider->setMinimum(-100);
			slider->setMaximum(100);
			slider->setValue(0);
			groupLayout->addWidget(slider, 5, 0, 1, 2);

			connect(slider, SIGNAL(valueChanged(int)), this, SLOT(saturationChanged(int)));

			label = new QLabel(groupBox);
			label->setText(tr("Luminosity [-100, 100]: "));
			groupLayout->addWidget(label, 6, 0);
			
			edit = new QLineEdit(groupBox);
			edit->setText("0");
			groupLayout->addWidget(edit, 6, 1);

			slider = new CSliderTextEdit(groupBox, edit, 1.0f);
			slider->setMinimum(-100);
			slider->setMaximum(100);
			slider->setValue(0);
			groupLayout->addWidget(slider, 7, 0, 1, 2);

			connect(slider, SIGNAL(valueChanged(int)), this, SLOT(luminosityChanged(int)));

			label = new QLabel(groupBox);
			label->setText(tr("Contrast [-100, 100]: "));
			groupLayout->addWidget(label, 8, 0);
			
			edit = new QLineEdit(groupBox);
			edit->setText("0");
			groupLayout->addWidget(edit, 8, 1);

			slider = new CSliderTextEdit(groupBox, edit, 1.0f);
			slider->setMinimum(-100);
			slider->setMaximum(100);
			slider->setValue(0);
			groupLayout->addWidget(slider, 9, 0, 1, 2);

			connect(slider, SIGNAL(valueChanged(int)), this, SLOT(contrastChanged(int)));
			
			groupBox->setLayout(groupLayout);
			vboxLayout->addWidget(groupBox);
		}

		vboxLayout->addStretch();
		widget->setLayout(vboxLayout);
		scrollArea->setWidget(widget);
		dockWidget->setWidget(scrollArea);
		mainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
		mainWindow->widgetsMenu()->addAction(dockWidget->toggleViewAction());
	}
}

} /* namespace NLTOOLS */

/* end of file */
