#include "nel/misc/types_nl.h"
#include <stdio.h>
#include "bin_reader.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QByteArray>
#include <QVector>
#include "nel/misc/path.h"
#include "nel/misc/sheet_id.h"
#include <vector>
#include "nel/misc/types_nl.h"

class	CPred
{
public:
	bool	operator()(const CSheetId &a, const CSheetId &b)
	{
		return a.toString()<b.toString();
	}
};

BinReader::BinReader(QString dir)
{
	QByteArray dirChar = dir.toLatin1();
	char* dirCharRef = dirChar.data();
// NLMISC::CApplicationContext appContext;
	CPath::addSearchPath(dirCharRef);
	CSheetId::init(false);
	CSheetId::buildIdVector(this->SheetList);
	CPred	Pred;
	sort(this->SheetList.begin(), this->SheetList.end(), Pred);
//   this->SheetList.fromStdVector(sheets);
}
int BinReader::count()
{
	return this->SheetList.size();
}
std::vector<CSheetId>  BinReader::getVector() const
{
	return this->SheetList;
}
void BinReader::pushToTable(QTableWidget*& table)
{
	table->clear();
	table->setRowCount(this->SheetList.size());
	table->setColumnCount(2);
	for (int i = 0; i < this->SheetList.size(); i++)
	{
		QTableWidgetItem* item1 = new QTableWidgetItem(QString(this->SheetList[i].toString().c_str()));
		QTableWidgetItem* item2 = new QTableWidgetItem(QString("%1").arg(this->SheetList[i].asInt()));
		table->setItem(i,1,item1);
		table->setItem(i,2,item2);
	}

}
