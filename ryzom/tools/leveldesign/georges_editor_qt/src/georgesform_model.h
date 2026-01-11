/*
Georges Viewer Qt
Copyright (C) 2010 Adrian Jaekel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef GEORGESFORM_MODEL_H
#define GEORGESFORM_MODEL_H

// Qt includes
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QStringList>
#include <QVariant>

// project includes

namespace NLGEORGES {
	class UFormElm;
}

namespace NLQT 
{

	class CFormItem;

	class CGeorgesFormModel : public QAbstractItemModel 
	{
		
	public:
		CGeorgesFormModel(NLGEORGES::UFormElm *root, QMap< QString, QStringList> deps,
			QString comment, QStringList parents, QObject *parent = 0);
		~CGeorgesFormModel();

		QVariant data(const QModelIndex &index, int role) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
		Qt::ItemFlags flags(const QModelIndex &index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &index) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		CFormItem *getItem(const QModelIndex &index) const;
		CGeorgesFormModel *model() { return this; }
		bool showParents() { return _showParents;}
		bool showDefaults() { return _showDefaults;}
		void setShowParents( bool show );
		void setShowDefaults( bool show );

	private:
		void setupModelData();	
		void loadFormData(NLGEORGES::UFormElm *rootElm, CFormItem *parent);
		void loadFormHeader();

		CFormItem*                  _rootItem;
		NLGEORGES::UFormElm*        _rootElm;
		QMap< QString, QStringList> _dependencies;
		QString                     _comments;
		QStringList                 _parents;
		QList<const QModelIndex*>*	_parentRows;

		bool						_showParents;
		bool						_showDefaults;

	};/* class CGeorgesFormModel */

} /* namespace NLQT */

#endif // GEORGESFORM_MODEL_H
