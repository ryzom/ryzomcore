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
#include <nel/misc/sha1.h>
#include <nel/pipeline/project_config.h>

// Project includes
#include "../shared_widgets/event_loop.h"

// See also: studio/.../gui_editor/texture_chooser.cpp

// UTILITY ->
QImage qImageFromCBitmap(NLMISC::CBitmap &bitmap, bool alpha)
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

	return img;
}
// <- UTILITY

CTextureBrowser::CTextureBrowser(QWidget *parent) : QListWidget(parent)
{
	qRegisterMetaType<CStdFunctionVoid>("CStdFunctionVoid");
	m_Thread = new CEventLoop();
	m_Thread->run();

	setViewMode(QListWidget::IconMode);
	setIconSize(QSize(128, 128));
	setResizeMode(QListWidget::Adjust);

	setGridSize(QSize(144, 160));

	// setDirectory("W:/database/stuff/fyros/agents/_textures/actors/");
}

CTextureBrowser::~CTextureBrowser()
{
	m_Thread->clear();
	delete m_Thread;
}

std::string CTextureBrowser::getSelectedTextureFile() const
{
	std::string res;
	QList<QListWidgetItem *> items = selectedItems();
	if (items.size() > 0)
		res = items[0]->text().toUtf8().data();
	return res;
}

void CTextureBrowser::setDirectory(const QString &dir)
{
	if (dir == m_CurrentDirectory)
		return;

	// Remove any pending stuff
	m_Thread->clear();

	// Sync up, clear, and start processing
	m_Thread->immediate([this, dir]() -> void {
		invokeStdFunction([this, dir]() -> void {
			clear();
			std::string cacheDir = NLMISC::CPath::getApplicationDirectory("NeL", true) + "cache/thumbnails/";
			NLMISC::CFile::createDirectoryTree(cacheDir);
			std::vector<std::string> files;
			NLMISC::CPath::getPathContent(dir.toUtf8().data(), false, false, true, files);
			QImage dummyimg = QImage(128, 128, QImage::Format_ARGB32);
			dummyimg.fill(0);
			QIcon dummy = QIcon(QPixmap::fromImage(dummyimg));
			for (size_t i = 0; i < files.size(); ++i)
			{
				std::string &file = files[i];
				QString fileName = QFileInfo(QString::fromUtf8(file.c_str())).fileName();
				std::string ext = NLMISC::toLower(NLMISC::CFile::getExtension(file));
				if (ext == "dds" || ext == "tga" || ext == "png" || ext == "jpg" || ext == "jpeg")
				{
					QListWidgetItem *item = new QListWidgetItem(dummy, fileName);
					item->setSizeHint(gridSize());
					addItem(item);
					m_Thread->immediate([this, file, cacheDir, item]() -> void {
						CHashKey filePathHash = getSHA1((uint8 *)file.c_str(), file.size()); // Get SHA1 of filepath
						std::string hash = NLMISC::toLower(filePathHash.toString()); // Hash in text format
						std::string cacheFile = cacheDir + hash + ".png";
						QString cacheFilePath = QString::fromUtf8(cacheFile.c_str());
						uint32 assetModification = NLMISC::CFile::getFileModificationDate(file);
						uint32 assetSize = NLMISC::CFile::getFileSize(file);
						QImage image;
						if (NLMISC::CFile::isExists(cacheFile))
						{
							if (image.load(QString::fromUtf8(cacheFile.c_str()), "PNG"))
							{
								// Use thumnbail if it matches only
								uint32 thumbnailModification = image.text("NL_ASSET_MODIFICATION").toUInt();
								uint32 thumbnailSize = image.text("NL_ASSET_SIZE").toUInt();
								if (thumbnailSize != assetSize || thumbnailModification != assetModification)
								{
									image = QImage();
								}
							}
							else
							{
								image = QImage();
							}
						}
						if (image.isNull())
						{
							if (NLMISC::CFile::isExists(cacheFile))
								NLMISC::CFile::deleteFile(cacheFile);
							NLMISC::CIFile f;
							if (f.open(file))
							{
								NLMISC::CBitmap bitmap;
								bitmap.load(f);
								uint w = bitmap.getWidth();
								uint h = bitmap.getHeight();
								if (w == h) bitmap.resample(128, 128);
								else if (w > h) bitmap.resample(128, h * 128 / w);
								else bitmap.resample(w * 128 / h, 128);
								image = qImageFromCBitmap(bitmap, false);
								image.setText("NL_ASSET_MODIFICATION", QString::number(assetModification));
								image.setText("NL_ASSET_SIZE", QString::number(assetSize));
								image.save(cacheFilePath, "PNG");
							}
						}
						if (!image.isNull())
						{
							QIcon icon = QIcon(QPixmap::fromImage(image));
							invokeStdFunction([this, icon, item]() -> void {
								item->setIcon(icon); // IMPORANT: Must set icon with exact size of existing so the Qt UI doesn't flicker...
							});
						}
					});
				}
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
