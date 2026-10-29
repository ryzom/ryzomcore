// Ryzom Core - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "process_row.h"

#include "icons.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QToolButton>

namespace {

const int s_RowHeight = 24;
const int s_LightSize = 11;
const char *const s_TitleStyle = "QPushButton { text-align: left; padding: 2px 8px; }";
const char *const s_SelectedTitleStyle = "QPushButton { text-align: left; padding: 2px 8px; background-color: palette(highlight); color: palette(highlighted-text); }";

/// The state lights of the original: grey new, orange launched, yellow captured, green
/// ready, blue stopping, red stopped.
QColor stateColor(CProcessEntry::TState state)
{
	switch (state)
	{
	case CProcessEntry::New: return QColor(0x9A, 0x9A, 0x9A);
	case CProcessEntry::Launched: return QColor(0xF0, 0x8C, 0x20);
	case CProcessEntry::Captured: return QColor(0xE8, 0xC8, 0x20);
	case CProcessEntry::Ready: return QColor(0x40, 0xB8, 0x40);
	case CProcessEntry::Stopping: return QColor(0x40, 0x78, 0xE8);
	case CProcessEntry::Stopped: return QColor(0xE0, 0x40, 0x40);
	}
	return QColor();
}

QString stateName(CProcessEntry::TState state)
{
	switch (state)
	{
	case CProcessEntry::New: return CProcessRow::tr("Not started");
	case CProcessEntry::Launched: return CProcessRow::tr("Starting");
	case CProcessEntry::Captured: return CProcessRow::tr("Started");
	case CProcessEntry::Ready: return CProcessRow::tr("Ready");
	case CProcessEntry::Stopping: return CProcessRow::tr("Stopping");
	case CProcessEntry::Stopped: return CProcessRow::tr("Stopped");
	}
	return QString();
}

} // namespace

CProcessRow::CProcessRow(CProcessEntry *entry, QWidget *parent)
    : QWidget(parent)
    , m_Entry(entry)
{
	// the row of the original: the light, the start/stop button, the title button; no margins,
	// so the rows line up with the two buttons above them
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(4);

	m_Light = new QLabel(this);
	m_Light->setFixedSize(s_LightSize + 2, s_RowHeight);
	m_Light->setAlignment(Qt::AlignCenter);
	layout->addWidget(m_Light, 0, Qt::AlignVCenter);

	m_StartStop = new QToolButton(this);
	m_StartStop->setFixedSize(s_RowHeight, s_RowHeight);
	m_StartStop->setIconSize(QSize(16, 16));
	connect(m_StartStop, &QToolButton::clicked, this, &CProcessRow::onStartStop);
	layout->addWidget(m_StartStop, 0, Qt::AlignVCenter);

	m_Title = new QPushButton(this);
	m_Title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_Title->setFixedHeight(s_RowHeight);
	m_Title->setMinimumWidth(40);
	m_Title->setStyleSheet(s_TitleStyle);
	connect(m_Title, &QPushButton::clicked, this, [this]() { emit selected(m_Entry); });
	layout->addWidget(m_Title, 1, Qt::AlignVCenter);

	connect(entry, &CProcessEntry::stateChanged, this, &CProcessRow::refresh);
	connect(entry, &CProcessEntry::titleChanged, this, &CProcessRow::refresh);
	connect(entry, &CProcessEntry::controlChanged, this, &CProcessRow::refresh);
	refresh();
}

void CProcessRow::setSelected(bool selected)
{
	// the selected title button takes the highlight colours
	m_Selected = selected;
	m_Title->setStyleSheet(selected ? s_SelectedTitleStyle : s_TitleStyle);
	update();
}

void CProcessRow::refresh()
{
	m_Light->setPixmap(Icons::light(stateColor(m_Entry->state()), s_LightSize));
	QString state = stateName(m_Entry->state());
	if (m_Entry->config().Control)
		state += m_Entry->controlConnected() ? tr(", console connected") : tr(", console not connected");
	m_Light->setToolTip(state);
	QString label = m_Entry->label();
	QFontMetrics metrics(m_Title->font());
	m_Title->setText(metrics.elidedText(label, Qt::ElideRight, qMax(40, m_Title->width() - 16)));
	m_Title->setToolTip(m_Entry->title());
	bool started = m_Entry->started();
	m_StartStop->setIcon(started ? Icons::stop() : Icons::play());
	m_StartStop->setToolTip(started ? (m_Entry->state() == CProcessEntry::Stopping ? tr("Kill") : tr("Stop")) : tr("Start"));
}

void CProcessRow::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	if (event->size().width() != event->oldSize().width())
		refresh();
}

void CProcessRow::onStartStop()
{
	emit selected(m_Entry);
	if (m_Entry->started())
		m_Entry->stopButton();
	else
		m_Entry->startButton();
}
