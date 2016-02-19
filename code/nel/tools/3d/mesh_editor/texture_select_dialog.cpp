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
#include "texture_select_dialog.h"

// STL includes
#include <functional>

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPixmap>
#include <QListWidget>
#include <QFileInfo>
#include <QSplitter>
#include <QPushButton>
#include <QDir>

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/file.h>
#include <nel/misc/sha1.h>
#include <nel/pipeline/project_config.h>

// Project includes
#include "texture_browser.h"

CTextureSelectDialog::CTextureSelectDialog(QWidget *parent) : QDialog(parent)
{
	resize(640, 400);
	setWindowTitle(tr("Select Texture"));

	m_TextureBrowser = new CTextureBrowser(this);

	QVBoxLayout *outer = new QVBoxLayout(this);
	setLayout(outer);

	QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
	outer->addWidget(splitter);

	QListWidget *folderList = new QListWidget(this);
	splitter->addWidget(folderList);
	splitter->addWidget(m_TextureBrowser);

	QList<int> sizes;
	sizes << 160 << 480;
	splitter->setSizes(sizes);

	QHBoxLayout *buttons = new QHBoxLayout(this);
	outer->addLayout(buttons);
	buttons->addStretch();

	QPushButton *select = new QPushButton("Select", this);
	buttons->addWidget(select);

	QPushButton *cancel = new QPushButton("Cancel", this);
	buttons->addWidget(cancel);

	connect(select, &QPushButton::clicked, this, &QDialog::accept);
	connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

	std::vector<std::string> paths;
	NLPIPELINE::CProjectConfig::getDatabaseTextureSearchPaths(paths);
	QString assetRoot = QString::fromUtf8(NLPIPELINE::CProjectConfig::getAssetRoot().c_str());

	QIcon folder(":/icons/folder-open-image.png");
	for (uint i = 0; i < paths.size(); ++i)
	{
		QString path = QString::fromUtf8(paths[i].c_str());
		if (path.startsWith(assetRoot))
			path = path.mid(assetRoot.size());
		folderList->addItem(new QListWidgetItem(folder, path));
	}

	auto textChanged = [this, assetRoot](const QString &text) -> void {
		if (text.isEmpty()) return;
		else if (QDir::isRelativePath(text)) m_TextureBrowser->setDirectory(assetRoot + text);
		else m_TextureBrowser->setDirectory(text);
	};

	if (folderList->count())
	{
		folderList->item(0)->setSelected(true);
		textChanged(folderList->item(0)->text());
	}

	connect(folderList, &QListWidget::currentTextChanged, this, textChanged);
}

CTextureSelectDialog::~CTextureSelectDialog()
{
	
}

/* end of file */
