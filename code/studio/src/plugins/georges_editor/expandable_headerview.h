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

#ifndef EXPANDABLE_HEADERVIEW_H
#define EXPANDABLE_HEADERVIEW_H

// Qt includes
#include <QHeaderView>

namespace GeorgesQt
{
	class ExpandableHeaderView : public QHeaderView
	{
		Q_OBJECT
	public:
		ExpandableHeaderView(Qt::Orientation orientation, QWidget * parent = 0);

		bool* expanded() { return &m_expanded; }

	protected:
		void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
		bool isPointInDecoration(int section, QPoint pos)const;
		void mousePressEvent(QMouseEvent *event);
		void mouseMoveEvent(QMouseEvent *event);

	private:
		bool m_expanded;
		bool m_inDecoration;

Q_SIGNALS:
		void headerClicked(int);
	};

} /* namespace NLQT */

#endif // EXPANDABLE_HEADERVIEW_H
