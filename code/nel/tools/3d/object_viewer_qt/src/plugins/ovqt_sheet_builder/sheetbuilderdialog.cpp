// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "sheetbuilderdialog.h"
#include "sheetbuilder.h"
#include "sheetbuilderconfgdialog.h"
#include "../core/icore.h"
#include "../core/core_constants.h"

#include <QCheckBox>
#include <QPushButton>
#include <QLayout>
#include <QTextEdit>
#include <QSettings>
#include <QTreeWidget>
#include <QDebug>

SheetBuilderDialog::SheetBuilderDialog(QWidget *parent)
	: QDialog(parent)
{
	QPushButton *btnOk = new QPushButton(tr("Make sheet"));
	connect(btnOk, SIGNAL(clicked()), SLOT(buildSheet()));

	QPushButton *btnCancel = new QPushButton(tr("Close"));
	connect(btnCancel, SIGNAL(clicked()), SLOT(reject()));

	chckClean = new QCheckBox(tr("Clean unwanted types from input"));

	txtOutput = new QTextEdit;
	txtOutput->setMinimumHeight(300);
	txtOutput->setMinimumWidth(500);
	txtOutput->setReadOnly(true);
	txtOutput->setFontFamily("Monospace, Courier New, monospace");

	QPushButton *btnDetails = new QPushButton(tr("Show/Hide details..."));
	connect(btnDetails, SIGNAL(clicked()), SLOT(detailsShowHide()));

	QPushButton *btnConfig = new QPushButton(tr("Settings"));
	connect(btnConfig, SIGNAL(clicked()), SLOT(showConfig()));

	QHBoxLayout *ltButtons = new QHBoxLayout;
	ltButtons->addWidget(btnConfig);
	ltButtons->addWidget(btnDetails);
	ltButtons->addStretch(1);
	ltButtons->addWidget(btnOk);
	ltButtons->addWidget(btnCancel);

	QVBoxLayout *ltMain = new QVBoxLayout;
	ltMain->addWidget(chckClean);
	ltMain->addWidget(txtOutput, 1);
	ltMain->addLayout(ltButtons);
	ltMain->addStretch();

	txtOutput->hide();
	detailsVisible = false;

	setLayout(ltMain);
	defHeight = height();
	defWidth = 500;
	resize(defWidth, defHeight);
	setWindowTitle(tr("Sheet builder"));
}

void SheetBuilderDialog::showConfig()
{
	SheetBuilderConfigDialog dlg(this);
	dlg.exec();
}

void SheetBuilderDialog::detailsShowHide()
{
	if (!detailsVisible)
	{
		defHeight = height();
		defWidth = width();
	}

	detailsVisible = !detailsVisible;
	txtOutput->setVisible(detailsVisible);

	if (!detailsVisible)
	{
		adjustSize();
		resize(defWidth, defHeight);
	}
}

void SheetBuilderDialog::displayInfo(QString str)
{
	txtOutput->append(str);
}

void SheetBuilderDialog::buildSheet()
{
	QStringList paths;
	QString outputFile;
	QStringList extensions;

	// read settings
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("SheetBuilder");
	paths = settings->value("SheetPaths").toStringList();
	outputFile = settings->value("SheetOutputFile").toString();
	extensions = settings->value("ExtensionsAllowed").toStringList();
	settings->endGroup();

	bool clean = chckClean->isChecked();

	string outputFileName(outputFile.toUtf8());

	if (outputFileName.empty())
	{
		displayInfo("Error: Output file is not specified");
		return;
	}

	list<string> inputDirs;
	Q_FOREACH (QString str, paths)
	inputDirs.push_back(str.toUtf8().constData());

	Q_FOREACH (QString str, extensions)
	{
		ExtensionsAllowed.insert(str.toUtf8().constData());
	}

	// get the current associations (read the sheet_id and fill the working structures)
	readFormId( outputFileName );

	// output path
	sint lastSeparator = CFile::getLastSeparator(outputFileName);
	string outputPath;
	if( lastSeparator != -1 )
	{
		outputPath = outputFileName.substr(0,lastSeparator+1);
	}

	// erase the unwanted extensions from map (modify the map, save it, and quit)
	if( clean )
	{
		if( ExtensionsAllowed.empty() )
			displayInfo(tr("None extension list provided, the input will not be cleaned"));
		else
		{
			map<TFormId,string>::iterator itSheets;
			for( itSheets = IdToForm.begin(); itSheets != IdToForm.end();  )
			{
				string extStr = CFile::getExtension( (*itSheets).second );
				if( !extStr.empty() )
				{
					if( ExtensionsAllowed.find(extStr) == ExtensionsAllowed.end() )
					{
						map<TFormId,string>::iterator itDel = itSheets++;
						IdToForm.erase( itDel );
					}
					else
						++itSheets;
				}
			}
			COFile f( outputFileName );
			f.serialCont( IdToForm );
		}
		displayInfo("The file has been cleaned");
		return;
	}
	setCursor(Qt::WaitCursor);
	// make the ids
	makeId( inputDirs );
	setCursor(Qt::ArrowCursor);

	// save the new map
	COFile f( outputFileName );
	f.serialCont( IdToForm );

	string sheetListFileName = outputPath + "sheets.txt";
	COFile output;
	if( !output.open(sheetListFileName,false,true) )
	{
		displayInfo(tr("Can't open output file %1").arg(sheetListFileName.c_str()));
		return;
	}
	map<TFormId,string>::iterator it1;
	for( it1 = IdToForm.begin(); it1 != IdToForm.end(); ++it1 )
	{
		string outputLine = " id: " + toString((*it1).first.Id) + " file: " + (*it1).second +"\n";
		output.serialBuffer((uint8 *)(const_cast<char *>(outputLine.data())),(uint)outputLine.size());
	}

	displayInfo (tr("------------- results ----------------"));
	displayInfo (tr("%1 files added in '%2'").arg(NbFilesAdded).arg(outputFileName.c_str()));
	displayInfo (tr("%1 files discarded because they are empty, begin with .# _ and so on").arg(NbFilesDiscarded));
	displayInfo (tr("%1 files skipped because don't have extension").arg(NbFilesUnknownType));
	displayInfo (tr("%1 types added in '%1'").arg(NbTypesAdded).arg(outputFileName.c_str()));

	displayInfo (tr("%1 supported file types :").arg(FileTypeToId.size()));
	for ( map<string,uint8>::iterator it = FileTypeToId.begin(); it != FileTypeToId.end(); ++it )
	{
		displayInfo(QString("%1").arg((*it).first.c_str()));
	}
}
