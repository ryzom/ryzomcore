// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian Jaekel <aj at elane2k dot com>
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
	class UForm;
    class CFormElmStruct;
    class CFormDfn;
    class CFormElmArray;
}

namespace GeorgesQt 
{

	class CFormItem;

	class CGeorgesFormModel : public QAbstractItemModel 
	{
	public:
		CGeorgesFormModel(NLGEORGES::UForm *form, QMap< QString, QStringList> deps,
			QString comment, QStringList parents, bool* expanded, QObject *parent = 0);
		~CGeorgesFormModel();

		QVariant data(const QModelIndex &index, int role) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
		Qt::ItemFlags flags(const QModelIndex &index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex index(int row, int column, CFormItem *item) const;
		QModelIndex parent(const QModelIndex &index) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		CFormItem *getItem(const QModelIndex &index) const;
		CGeorgesFormModel *model() { return this; }
		bool showParents() { return m_showParents;}
		bool showDefaults() { return m_showDefaults;}
		void setShowParents( bool show );
		void setShowDefaults( bool show );
		void addParentForm(QString parentForm);
		void removeParentForm(QString parentForm);
        NLGEORGES::UFormElm *getRootForm() { return m_rootElm; }

        CFormItem *addStruct (CFormItem *parent, NLGEORGES::CFormElmStruct *_struct, NLGEORGES::CFormDfn *parentDfn,
                              const char *name, uint structId, const char *formName, uint slot, bool isVirtual = false );

        CFormItem *addArray(CFormItem *parent, NLGEORGES::CFormElmArray *array, NLGEORGES::CFormDfn *rootDfn,
                            const char *name, uint structId, const char *formName, uint slot);

		CFormItem *addAtom(CFormItem *parent, NLGEORGES::CFormElm *elm, NLGEORGES::CFormDfn *dfn, const char *name, uint id, const char *formName);

		void emitDataChanged(const QModelIndex &index)
		{ 
			Q_EMIT dataChanged(index, index); 
		}

		void arrayResized( const QString &name, int size );
		void appendArray( QModelIndex idx );
		void deleteArrayEntry( QModelIndex idx );
		void renameArrayEntry( QModelIndex idx, const QString &name );

	private:
		void setupModelData();
		void loadFormData(NLGEORGES::UFormElm *rootElm, CFormItem *parent);
		void loadFormHeader();

		NLGEORGES::UForm*		m_form;
		CFormItem*                  m_rootItem;
        NLGEORGES::UFormElm*        m_rootElm;
		QList<QVariant>				m_rootData;
		QMap< QString, QStringList> m_dependencies;
		QString                     m_comments;
		QStringList                 m_parents;
		QList<const QModelIndex*>*	m_parentRows;

		bool						m_showParents;
		bool						m_showDefaults;
		bool						*m_expanded;

	};/* class CGeorgesFormModel */

} /* namespace GeorgesQt */

#endif // GEORGESFORM_MODEL_H


