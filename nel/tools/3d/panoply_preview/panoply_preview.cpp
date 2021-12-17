// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014-2016  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include <QPainter>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/command.h>
#include <nel/misc/path.h>
#include <nel/misc/thread.h>
#include <nel/misc/mutex.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/file.h>

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
				nlSleep(10); // TODO: Should wait on an event signal...
				continue;
			}
			// nldebug("Update color modifier");
			m_ColorModifier.Hue = Hue;
			m_ColorModifier.Lightness = Lightness;
			m_ColorModifier.Saturation = Saturation;
			m_ColorModifier.Luminosity = Luminosity;
			m_ColorModifier.Contrast = Contrast;
			Process = false;
			SettingsMutex.leave();

			BitmapMutex.enter();
			if (!Running)
			{
				BitmapMutex.leave();
				return;
			}
			if (!BitmapsOk)
			{
				nldebug("Bitmaps not ready");
				BitmapMutex.leave();
				nlSleep(500);
				continue;
			}
			float retDeltaHue;
			DestBitmap = ColorBitmap;
			m_ColorModifier.convertBitmap(DestBitmap, ColorBitmap, MaskBitmap, retDeltaHue);
			BitmapMutex.leave();

			PanoplyPreview->displayBitmap(DestBitmap);

			nlSleep(10); // TODO: Should wait on an event signal...
		}
	}

	CColorThread() : PanoplyPreview(NULL), BitmapsOk(false), Hue(0), Lightness(0), Saturation(0), Luminosity(0), Contrast(0), Process(false), Running(true) { }
	virtual ~CColorThread() { }
	virtual void getName (std::string &result) const { result = "CColorThread"; }

private:
	CColorModifier m_ColorModifier;

public:
	CPanoplyPreview *PanoplyPreview;

	NLMISC::CMutex BitmapMutex;
	NLMISC::CBitmap ColorBitmap;
	NLMISC::CBitmap MaskBitmap;
	bool BitmapsOk;
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
	connect(this, SIGNAL(tSigBitmap()), this, SLOT(tSlotBitmap()));

	createDockWindows(parent);

	m_Image = new QImage(512, 512, QImage::Format_RGB32);
	m_Pixmap = new QPixmap(512, 512);

	setMinimumWidth(512);
	setMinimumHeight(512);

	m_ColorThread = new CColorThread();
	m_ColorThread->PanoplyPreview = this;
	m_Thread = IThread::create(m_ColorThread);
	m_Thread->start();
}

CPanoplyPreview::~CPanoplyPreview()
{
	m_ColorThread->SettingsMutex.enter();
	m_ColorThread->BitmapMutex.enter();
	m_ColorThread->Running = false;
	m_ColorThread->BitmapMutex.leave();
	m_ColorThread->SettingsMutex.leave();
	m_Thread->wait();
	delete m_Thread;
	delete m_ColorThread;
}

void CPanoplyPreview::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, *m_Pixmap);
}

void CPanoplyPreview::displayBitmap(const CBitmap &bitmap) // Called from thread!
{
	// nldebug("received bitmap");

	m_ColorThread->BitmapMutex.enter();
	m_ImageMutex.enter();

	const char *buffer = (const char *)&bitmap.getPixels()[0];

	if (bitmap.getWidth() != m_Image->width() || bitmap.getHeight() != m_Image->height())
	{
		QImage *image = m_Image;
		m_Image = new QImage(bitmap.getWidth(), bitmap.getHeight(), QImage::Format_RGB32);
		delete image;
	}
	
	for (uint32 y = 0; y < bitmap.getHeight(); ++y)
	{
		uint8 *dst = (uint8 *)m_Image->scanLine(y);
		const uint8 *src = (const uint8 *)&buffer[y * bitmap.getWidth() * sizeof(uint32)];
		for (uint32 x = 0; x < bitmap.getWidth(); ++x)
		{
			uint32 xb = x * 4;
			dst[xb + 0] = src[xb + 2];
			dst[xb + 1] = src[xb + 1];
			dst[xb + 2] = src[xb + 0];
			dst[xb + 3] = src[xb + 3];
		}

		//memcpy(m_Image->scanLine(y), &buffer[y * bitmap.getWidth() * sizeof(uint32)], sizeof(uint32) * bitmap.getWidth());
	}

	m_ImageMutex.leave();
	m_ColorThread->BitmapMutex.leave();

	tSigBitmap();
}

void CPanoplyPreview::tSlotBitmap()
{
	// nldebug("display bitmap");

	m_ImageMutex.enter();

	if (m_Image->width() != m_Pixmap->width()
		|| m_Image->height() != m_Pixmap->height())
	{
		QPixmap *pixmap = m_Pixmap;
		m_Pixmap = new QPixmap(m_Image->width(), m_Image->height());
		setMinimumWidth(m_Pixmap->width());
		setMinimumHeight(m_Pixmap->height());
		delete pixmap;
	}
	m_Pixmap->convertFromImage(*m_Image);
	repaint();

	m_ImageMutex.leave();
	
}

void CPanoplyPreview::colorEdited(const QString &text)
{
	m_ColorFile = text;
}

void CPanoplyPreview::maskEdited(const QString &text)
{
	m_MaskFile = text;
}

void CPanoplyPreview::goPushed(bool)
{
	// nldebug("push bitmaps");
	m_ColorThread->SettingsMutex.enter();
	m_ColorThread->BitmapMutex.enter();
	m_ColorThread->BitmapsOk = false;

	try
	{
		{
			NLMISC::CIFile is;
			if (!is.open(m_ColorFile.toLocal8Bit().data()))
				throw NLMISC::Exception("Cannot open file '%s'", m_ColorFile.toLocal8Bit().data());
			uint32 depth = m_ColorThread->ColorBitmap.load(is);
			if (depth == 0 || m_ColorThread->ColorBitmap.getPixels().empty())
				throw NLMISC::Exception("Failed to load bitmap '%s'", m_ColorFile.toLocal8Bit().data());
			if (m_ColorThread->ColorBitmap.PixelFormat != NLMISC::CBitmap::RGBA)
				m_ColorThread->ColorBitmap.convertToType(NLMISC::CBitmap::RGBA);
		}
		{
			NLMISC::CIFile is;
			if (!is.open(m_MaskFile.toLocal8Bit().data()))
				throw NLMISC::Exception("Cannot open file '%s'", m_MaskFile.toLocal8Bit().data());
			uint32 depth = m_ColorThread->MaskBitmap.load(is);
			if (depth == 0 || m_ColorThread->MaskBitmap.getPixels().empty())
				throw NLMISC::Exception("Failed to load bitmap '%s'", m_MaskFile.toLocal8Bit().data());
			if (m_ColorThread->MaskBitmap.PixelFormat != NLMISC::CBitmap::Luminance)
				m_ColorThread->MaskBitmap.convertToType(NLMISC::CBitmap::Luminance);
		}
		{
			m_ColorThread->BitmapsOk = true;
			m_ColorThread->Process = true;
		}
	}
	catch (const NLMISC::Exception &e)
	{
		nlwarning("Exception: '%s'", e.what());
	}

	m_ColorThread->BitmapMutex.leave();
	m_ColorThread->SettingsMutex.leave();
	// nldebug("done pushing butmaps");
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
			colorFile->setText("W:\\database\\stuff\\fyros\\agents\\_textures\\actors\\fy_hof_armor00_arm01_c1.png");
			groupLayout->addWidget(colorFile, 0, 1);

			m_ColorFile = colorFile->text();
			connect(colorFile, SIGNAL(textEdited(const QString &)), this, SLOT(colorEdited(const QString &)));

			QLabel *maskLabel = new QLabel(groupBox);
			maskLabel->setText(tr("Mask: "));
			groupLayout->addWidget(maskLabel, 1, 0);

			QLineEdit *maskFile = new QLineEdit(groupBox);
			maskFile->setText("W:\\database\\stuff\\fyros\\agents\\_textures\\actors\\mask\\fy_hof_armor00_arm01_c1_skin.png");
			groupLayout->addWidget(maskFile, 1, 1);

			m_MaskFile = maskFile->text();
			connect(maskFile, SIGNAL(textEdited(const QString &)), this, SLOT(maskEdited(const QString &)));

			QPushButton *go = new QPushButton(groupBox);
			go->setText(tr("Go"));
			groupLayout->addWidget(go, 2, 0, 1, 2);

			connect(go, SIGNAL(clicked(bool)), this, SLOT(goPushed(bool)));

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
