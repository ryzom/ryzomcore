// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2016  Winch Gate Property Limited
// Author: Jan Boon <jan.boon@kaetemi.be>
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
#include "texture_browser.h"

// STL includes
#include <functional>

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPixmap>
#include <QListWidget>
#include <QFileInfo>

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/file.h>
#include <nel/pipeline/project_config.h>

// Project includes
#include "../shared_widgets/event_loop.h"

// See also: studio/.../gui_editor/texture_chooser.cpp

// UTILITY ->
QPixmap qPixmapFromCBitmap(NLMISC::CBitmap &bitmap, bool alpha)
{
	QImage img(bitmap.getWidth(), bitmap.getHeight(), alpha ? QImage::Format_ARGB32 : QImage::Format_RGB32);
	NLMISC::CObjectVector<uint8> &pixels = bitmap.getPixels();

	uint height = bitmap.getHeight();
	uint stride = bitmap.getWidth() * sizeof(NLMISC::CRGBA);
	for (uint y = 0; y < height; ++y)
	{
		// memcpy(img.scanLine(y), &pixels[y * stride], stride);
		// Convert from ABGR to ARGB
		uint8 *dst = img.scanLine(y);
		uint8 *src = &pixels[y * stride];
		for (uint x = 0; x < stride; x += 4)
		{
			dst[x] = src[x + 2];
			dst[x + 1] = src[x + 1];
			dst[x + 2] = src[x];
			dst[x + 3] = src[x + 3];
		}
	}

	return QPixmap::fromImage(img);
}
// <- UTILITY

CTextureBrowser::CTextureBrowser(QWidget *parent) : QListWidget(parent)
{
	qRegisterMetaType<CStdFunctionVoid>("CStdFunctionVoid");
	m_Thread = new CEventLoop();
	m_Thread->run();

	setViewMode(QListWidget::IconMode);
	setIconSize(QSize(200, 200));
	setResizeMode(QListWidget::Adjust);

	setDirectory("W:/database/stuff/fyros/agents/_textures/actors/");
}

CTextureBrowser::~CTextureBrowser()
{
	m_Thread->clear();
	delete m_Thread;
}

void CTextureBrowser::setDirectory(const QString &dir)
{
	// Remove any pending stuff
	m_Thread->clear();

	// Sync up, clear, and start processing
	m_Thread->immediate([this, dir]() -> void {
		invokeStdFunction([this, dir]() -> void {
			// m_listeWidget->addItem(new QListWidgetItem(QIcon("../earth.jpg"), "Earth"));
			clear();
			std::vector<std::string> files;
			NLMISC::CPath::getPathContent(dir.toUtf8().data(), false, false, true, files);
			for (size_t i = 0; i < files.size(); ++i)
			{
				std::string &file = files[i];
				m_Thread->immediate([this, file]() -> void {
					std::string ext = NLMISC::toLower(NLMISC::CFile::getExtension(file));
					if (ext == "dds" || ext == "tga" || ext == "png" || ext == "jpg" || ext == "jpeg")
					{
						NLMISC::CIFile f;
						if (f.open(file))
						{
							NLMISC::CBitmap bitmap;
							bitmap.load(f);
							bitmap.resample(128, 128);
							QPixmap pixmap = qPixmapFromCBitmap(bitmap, false);
							QString fileName = QFileInfo(QString::fromUtf8(file.c_str())).fileName();
							invokeStdFunction([this, pixmap, fileName]() -> void {
								addItem(new QListWidgetItem(QIcon(pixmap), fileName));
							});
						}
					}
				});
			}
		});
	});
}

void CTextureBrowser::invokeStdFunction(CStdFunctionVoid f)
{
	QMetaObject::invokeMethod(this, "callStdFunction", Qt::QueuedConnection, Q_ARG(CStdFunctionVoid, f));
}

void CTextureBrowser::callStdFunction(CStdFunctionVoid f)
{
	f();
}

/* end of file */
