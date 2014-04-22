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

#ifndef FORMDELEGATE_H
#define FORMDELEGATE_H

#include <QStyledItemDelegate>

#if 0 

namespace GeorgesQt 
{

	class FormDelegate : public QStyledItemDelegate
	{

	public:
		FormDelegate(QObject *parent = 0);

		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
			const QModelIndex &index) const;
		void setEditorData(QWidget *editor, const QModelIndex &index) const;
		void setModelData(QWidget *editor, QAbstractItemModel *model,
			const QModelIndex &index) const;
		void updateEditorGeometry(QWidget *editor,
			const QStyleOptionViewItem &option, const QModelIndex &index) const;
	};

}
#endif // 0

#endif // FORMDELEGATE_H
