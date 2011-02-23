#include "nel/misc/types_nl.h"
#include <stdio.h>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QByteArray>
#include <QVector>
#include "nel/misc/path.h"
#include "nel/misc/sheet_id.h"
#include <vector>
#include "nel/misc/types_nl.h"

using	namespace std;
using	namespace NLMISC;

class BinReader
{
public:
	BinReader(QString dir);
	int count();
	void pushToTable(QTableWidget*& table);
	std::vector<CSheetId> getVector() const;
private:
	std::vector<CSheetId>	SheetList;
};