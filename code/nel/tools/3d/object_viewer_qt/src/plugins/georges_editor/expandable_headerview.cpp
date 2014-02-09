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

#include "stdpch.h"
// Project includes
#include "expandable_headerview.h"

// Qt includes
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

namespace GeorgesQt
{

	ExpandableHeaderView::ExpandableHeaderView(Qt::Orientation orientation, QWidget * parent) 
		: QHeaderView(orientation, parent),
		m_expanded(true),
		m_inDecoration(false)
	{
	}

	void ExpandableHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
	{
			painter->save();
			QHeaderView::paintSection(painter, rect, logicalIndex);
			painter->restore();

			if (logicalIndex == 0)
			{
				QRect sectionRect = this->orientation() == Qt::Horizontal ? 
					QRect(this->sectionPosition(logicalIndex), 0, 
					this->sectionSize(logicalIndex), this->height()):
				QRect(0, this->sectionPosition(logicalIndex), 
					this->width(), this->sectionSize(logicalIndex));

				QStyleOptionHeader opt;
				initStyleOption(&opt);
				opt.iconAlignment = Qt::AlignVCenter;

				QVariant variant = this->model()->headerData(logicalIndex, this->orientation(),
					Qt::DecorationRole);
				opt.icon = qvariant_cast<QIcon>(variant);
				if (opt.icon.isNull())
				{
					opt.icon = qvariant_cast<QPixmap>(variant);
				}
				QRect headerLabelRect = this->style()->subElementRect(QStyle::SE_HeaderLabel, &opt, this);

				QPixmap pixmap
					= opt.icon.pixmap(this->style()->pixelMetric(QStyle::PM_SmallIconSize), 
					(opt.state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled);
				QRect aligned = this->style()->alignedRect(opt.direction, QFlag(opt.iconAlignment), 
					pixmap.size(), headerLabelRect);
				QRect inter = aligned.intersected(headerLabelRect);

				QStyleOption option;
				option.rect = QRect(inter.x()-2,inter.y(),inter.width(),inter.height());
				if (m_expanded)
					option.state = QStyle::State_Children | QStyle::State_Open;
				else
					option.state = QStyle::State_Children;
				if (m_inDecoration)
					option.state |= QStyle::State_MouseOver;
				QApplication::style()->drawPrimitive(QStyle::PE_IndicatorBranch, &option, painter);
			}
	}

	void ExpandableHeaderView::mousePressEvent(QMouseEvent *e)
	{
		int section = logicalIndexAt(e->x());

		if (section == 0 && m_inDecoration) {
			if (m_expanded)
				m_expanded = false;
			else
				m_expanded = true;
			this->QHeaderView::mousePressEvent(e);
			Q_EMIT headerClicked(section);
		}
	}

	void ExpandableHeaderView::mouseMoveEvent(QMouseEvent *e)
	{
		int section = this->logicalIndexAt(e->x());

		if (section != 0)
			return;

		bool tmp = m_inDecoration;
		if (isPointInDecoration(section, e->pos()))
			m_inDecoration = true;
		else
			m_inDecoration = false;

		if (m_inDecoration != tmp)
			updateSection(0);
	}

	bool ExpandableHeaderView::isPointInDecoration(int section, QPoint pos)const
	{
		QRect sectionRect = this->orientation() == Qt::Horizontal ? 
			QRect(this->sectionPosition(section), 0, 
			this->sectionSize(section), this->height()):
		QRect(0, this->sectionPosition(section), 
			this->width(), this->sectionSize(section));
		QStyleOptionHeader opt;
		this->initStyleOption(&opt);
		opt.iconAlignment = Qt::AlignVCenter;
		QVariant variant = this->model()->headerData(section, this->orientation(),
			Qt::DecorationRole);
		opt.icon = qvariant_cast<QIcon>(variant);
		if (opt.icon.isNull())
		{
			opt.icon = qvariant_cast<QPixmap>(variant);
		}
		QRect headerLabelRect = this->style()->subElementRect(QStyle::SE_HeaderLabel, &opt, this);
		// from qcommonstyle.cpp
		if (opt.icon.isNull()) 
		{
			return false;
		}
		QPixmap pixmap
			= opt.icon.pixmap(this->style()->pixelMetric(QStyle::PM_SmallIconSize), 
			(opt.state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled);
		QRect aligned = this->style()->alignedRect(opt.direction, QFlag(opt.iconAlignment), 
			pixmap.size(), headerLabelRect);
		QRect inter = aligned.intersected(headerLabelRect);
		return inter.contains(pos);
	}
}
