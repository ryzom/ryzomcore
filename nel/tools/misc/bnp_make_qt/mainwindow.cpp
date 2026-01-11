#include "mainwindow.h"

MainWindow::MainWindow(QWidget * parent) :
  QMainWindow(parent),
  uiMainWindow_(new Ui::MainWindow),
  processList_(new QProcess(this)),
  processSearch_(new QProcess(this)),
  processPack_(new QProcess(this)),
  processUnpack_(new QProcess(this))
{
  uiMainWindow_->setupUi(this);

  settings_ = new QSettings("bnp_make_frontend.cfg", QSettings::IniFormat);
  bnpMakeBinary_ = settings_->value("BnpMakeBinary").toString();
  dataPath_      = settings_->value("DataPath").toString();

  uiMainWindow_->lineEditSearchPath->setText(dataPath_);
  uiMainWindow_->lineEditBnpMake->setText(bnpMakeBinary_);
  uiMainWindow_->lineEditDataPath->setText(dataPath_);

  connect(uiMainWindow_->pushButtonListBrowse, SIGNAL(clicked()), this, SLOT(onButtonListBrowseClicked()));
  connect(uiMainWindow_->pushButtonList, SIGNAL(clicked()), this, SLOT(onButtonListClicked()));
  connect(processList_, SIGNAL(finished(int)), this, SLOT(onProcessListComplete()));

  connect(uiMainWindow_->pushButtonSearchBrowse, SIGNAL(clicked()), this, SLOT(onButtonSearchBrowseClicked()));
  connect(uiMainWindow_->pushButtonSearch, SIGNAL(clicked()), this, SLOT(onButtonSearchClicked()));
  connect(processSearch_, SIGNAL(finished(int)), this, SLOT(onProcessSearchComplete()));

  connect(uiMainWindow_->pushButtonPackBrowse, SIGNAL(clicked()), this, SLOT(onButtonPackBrowseClicked()));
  connect(uiMainWindow_->pushButtonPack, SIGNAL(clicked()), this, SLOT(onButtonPackClicked()));
  connect(processPack_, SIGNAL(finished(int)), this, SLOT(onProcessPackComplete()));

  connect(uiMainWindow_->pushButtonUnpackBrowse, SIGNAL(clicked()), this, SLOT(onButtonUnpackBrowseClicked()));
  connect(uiMainWindow_->pushButtonUnpack, SIGNAL(clicked()), this, SLOT(onButtonUnpackClicked()));
  connect(processUnpack_, SIGNAL(finished(int)), this, SLOT(onProcessUnpackComplete()));

  connect(uiMainWindow_->pushButtonBnpMakeBrowse, SIGNAL(clicked()), this, SLOT(onButtonBnpMakeBrowseClicked()));
  connect(uiMainWindow_->pushButtonDataPathBrowse, SIGNAL(clicked()), this, SLOT(onButtonDataPathBrowseClicked()));
}

MainWindow::~MainWindow()
{
  delete uiMainWindow_;
}

void MainWindow::onButtonListBrowseClicked()
{
  QString fileName;

  fileName = QFileDialog::getOpenFileName(this, "Choose a BNP file", dataPath_, "BNP file (*.bnp)");
  uiMainWindow_->lineEditList->setText(fileName);
}

void MainWindow::onButtonListClicked()
{
  QStringList arguments;

  uiMainWindow_->textEditList->clear();

  if (bnpMakeBinary_ != "")
    if (uiMainWindow_->lineEditList->text() != "")
    {
      arguments << "/l" << uiMainWindow_->lineEditList->text();
      processList_->start(bnpMakeBinary_, arguments);
    }
    else
      uiMainWindow_->textEditList->append("Choose a BNP file.");
  else
    uiMainWindow_->textEditList->append("Check bnp_make path.");
}

void MainWindow::onProcessListComplete()
{
  QString output;

  output = processList_->readAllStandardOutput();
  uiMainWindow_->textEditList->append(output);
  uiMainWindow_->textEditList->append("List complete.");
}

void MainWindow::onButtonSearchBrowseClicked()
{
  QString directory;

  directory = QFileDialog::getExistingDirectory(this, tr("Choose a directory"), dataPath_, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  uiMainWindow_->lineEditSearchPath->setText(directory);
}

void MainWindow::onButtonSearchClicked()
{
  QDir          dir;
  QStringList   nameFilters;

  uiMainWindow_->textEditSearch->clear();

  if (bnpMakeBinary_ != "")
    if (uiMainWindow_->lineEditSearchPath->text() != "")
    {
      dir.cd(uiMainWindow_->lineEditSearchPath->text());

      nameFilters << "*.bnp";
      dir.setNameFilters(nameFilters);
      dir.setFilter(QDir::Files);

      fileInfoList_ = dir.entryInfoList();
      fileInfoListIndex_ = 0;

      onProcessSearchComplete();
    }
    else
      uiMainWindow_->textEditSearch->append("Choose a directory.");
  else
    uiMainWindow_->textEditSearch->append("Check bnp_make path.");
}

void MainWindow::onProcessSearchComplete()
{
  QFileInfo   fileInfo;
  QStringList arguments;
  QString     output;
  QStringList outputList;
  QString     line;

  if (fileInfoListIndex_ > 0)
  {
    output = processSearch_->readAllStandardOutput();

    if (uiMainWindow_->lineEditSearchString->text() == "")
      uiMainWindow_->textEditSearch->append(output);
    else
    {
      outputList = output.split("\n");

      foreach (line, outputList)
        if (line.contains(uiMainWindow_->lineEditSearchString->text()))
          uiMainWindow_->textEditSearch->append(line.trimmed());
    }
  }

  if (fileInfoListIndex_ < fileInfoList_.count())
  {
    fileInfo = fileInfoList_.at(fileInfoListIndex_++);
    uiMainWindow_->textEditSearch->append("===> " + uiMainWindow_->lineEditSearchPath->text() + "/" + fileInfo.fileName() + ":");
    arguments << "/l" << uiMainWindow_->lineEditSearchPath->text() + "/" + fileInfo.fileName();
    processSearch_->start(bnpMakeBinary_, arguments);
  }
  else
  {
    uiMainWindow_->textEditSearch->append("Search complete.");
  }
}

void MainWindow::onButtonPackBrowseClicked()
{
  QString directory;

  directory = QFileDialog::getExistingDirectory(this, tr("Choose the source directory"), dataPath_, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  uiMainWindow_->lineEditPack->setText(directory);
}

void MainWindow::onButtonPackClicked()
{
  QStringList arguments;

  uiMainWindow_->textEditPack->clear();

  if (bnpMakeBinary_ != "")
    if (uiMainWindow_->lineEditPack->text() != "")
    {
      uiMainWindow_->textEditPack->append("Pack in progress...");

      arguments << "/p" << uiMainWindow_->lineEditPack->text();
      processPack_->start(bnpMakeBinary_, arguments);
    }
    else
      uiMainWindow_->textEditPack->append("Choose the source directory.");
  else
    uiMainWindow_->textEditPack->append("Check bnp_make path.");
}

void MainWindow::onProcessPackComplete()
{
  QString output;

  output = processPack_->readAllStandardOutput();
  uiMainWindow_->textEditPack->append(output);
  uiMainWindow_->textEditPack->append("Pack complete.");
}

void MainWindow::onButtonUnpackBrowseClicked()
{
  QString fileName;

  fileName = QFileDialog::getOpenFileName(this, "Choose a BNP file", dataPath_, "BNP file (*.bnp)");
  uiMainWindow_->lineEditUnpack->setText(fileName);
}

void MainWindow::onButtonUnpackClicked()
{
  QStringList arguments;

  uiMainWindow_->textEditUnpack->clear();

  if (bnpMakeBinary_ != "")
    if (uiMainWindow_->lineEditUnpack->text() != "")
    {
      uiMainWindow_->textEditUnpack->append("Unpack in progress...");

      arguments << "/u" << uiMainWindow_->lineEditUnpack->text();
      processUnpack_->start(bnpMakeBinary_, arguments);
    }
    else
      uiMainWindow_->textEditUnpack->append("Choose a BNP file.");
  else
    uiMainWindow_->textEditUnpack->append("Check bnp_make path.");
}

void MainWindow::onProcessUnpackComplete()
{
  uiMainWindow_->textEditUnpack->append("Unpack complete.");
}

void MainWindow::onButtonBnpMakeBrowseClicked()
{
  QString fileName;

  fileName = QFileDialog::getOpenFileName(this, "Locate the bnp_make binary", dataPath_);
  uiMainWindow_->lineEditBnpMake->setText(fileName);

  bnpMakeBinary_ = fileName;
  settings_->setValue("BnpMakeBinary", fileName);

  uiMainWindow_->textEditSettings->clear();
  if (fileName != "")
    uiMainWindow_->textEditSettings->append("bnp_make path changed.");
  else
    uiMainWindow_->textEditSettings->append("Locate the bnp_make binary.");
}

void MainWindow::onButtonDataPathBrowseClicked()
{
  QString directory;

  directory = QFileDialog::getExistingDirectory(this, tr("Choose a directory"), dataPath_, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  uiMainWindow_->lineEditDataPath->setText(directory);

  dataPath_ = directory;
  settings_->setValue("DataPath", directory);
  uiMainWindow_->lineEditSearchPath->setText(directory);

  uiMainWindow_->textEditSettings->clear();
  uiMainWindow_->textEditSettings->append("Data path changed.");
}
