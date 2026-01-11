/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

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

#include "stdpch.h"
#include "color_edit_widget.h"

// Qt includes
#include <QtGui/QColorDialog>
#include <QtGui/QColor>
#include <QtGui/QPainter>

// Nel includes
#include <nel/misc/rgba.h>

namespace NLQT
{

CColorEditWidget::CColorEditWidget(QWidget *parent)
	: QWidget(parent),
	  _Wrapper(NULL),
	  _emit(true)
{
	_ui.setupUi(this);

	_ui.graphicsWidget->installEventFilter(this);

	connect(_ui.rSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setRed(int)));
	connect(_ui.gSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setGreen(int)));
	connect(_ui.bSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setBlue(int)));
	connect(_ui.aSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setAlpha(int)));
	connect(_ui.browsePushButton, SIGNAL(clicked()), this, SLOT(browseColor()));

	setColor(QColor(255, 255, 255, 255));
}

CColorEditWidget::~CColorEditWidget()
{
}

void CColorEditWidget::setWrapper(IPSWrapperRGBA *wrapper)
{
	_Wrapper = wrapper;
}

void CColorEditWidget::setColor(const NLMISC::CRGBA &color, bool emit)
{
	_emit = false;
	_ui.rSpinBox->setValue(color.R);
	_ui.gSpinBox->setValue(color.G);
	_ui.bSpinBox->setValue(color.B);
	_ui.aSpinBox->setValue(color.A);
	_emit = true;
	if (emit)
		Q_EMIT colorChanged(color);

	_ui.graphicsWidget->repaint();
}

void CColorEditWidget::setColor(const QColor &color, bool emit)
{
	setColor(NLMISC::CRGBA(color.red(), color.green(), color.blue(), color.alpha()), emit);
}

void CColorEditWidget::updateUi()
{
	if (_Wrapper == NULL) return;
	setColor(_Wrapper->get());
}

void CColorEditWidget::setRed(int r)
{
	if (_emit)
		Q_EMIT colorChanged(NLMISC::CRGBA(r, _ui.gSpinBox->value(),
										  _ui.bSpinBox->value(),
										  _ui.aSpinBox->value()));

	_ui.graphicsWidget->repaint();
	if (_Wrapper == NULL)
		return;

	NLMISC::CRGBA color = _Wrapper->get();

	if (r == color.R)
		return;

	color.R = r;
	_Wrapper->setAndUpdateModifiedFlag(color);

}

void CColorEditWidget::setGreen(int g)
{
	if (_emit)
		Q_EMIT colorChanged(NLMISC::CRGBA(_ui.rSpinBox->value(),
										  g,
										  _ui.bSpinBox->value(),
										  _ui.aSpinBox->value()));
	_ui.graphicsWidget->repaint();
	if (_Wrapper == NULL) return;

	NLMISC::CRGBA color = _Wrapper->get();

	if (g == color.G)
		return;

	color.G = g;
	_Wrapper->setAndUpdateModifiedFlag(color);
}

void CColorEditWidget::setBlue(int b)
{
	if (_emit)
		Q_EMIT colorChanged(NLMISC::CRGBA(_ui.rSpinBox->value(),
										  _ui.gSpinBox->value(),
										  b,
										  _ui.aSpinBox->value()));
	_ui.graphicsWidget->repaint();
	if (_Wrapper == NULL) return;

	NLMISC::CRGBA color = _Wrapper->get();

	if (b == color.B)
		return;

	color.B = b;
	_Wrapper->setAndUpdateModifiedFlag(color);
}

void CColorEditWidget::setAlpha(int a)
{
	if (_emit)
		Q_EMIT colorChanged(NLMISC::CRGBA(_ui.rSpinBox->value(),
										  _ui.gSpinBox->value(),
										  _ui.bSpinBox->value(),
										  a));
	_ui.graphicsWidget->repaint();

	if (_Wrapper == NULL) return;

	NLMISC::CRGBA color = _Wrapper->get();

	if (a == color.A)
		return;

	color.A = a;
	_Wrapper->setAndUpdateModifiedFlag(color);
}

void CColorEditWidget::browseColor()
{
	QColor color = QColorDialog::getColor(QColor(_ui.rSpinBox->value(),
										  _ui.gSpinBox->value(),
										  _ui.bSpinBox->value(),
										  _ui.aSpinBox->value()));
	if (!color.isValid()) return;

	setColor(color);
}

bool CColorEditWidget::eventFilter(QObject *object, QEvent *event)
{
	if( event->type() == QEvent::Paint )
	{
		_color = QColor(_ui.rSpinBox->value(),
						_ui.gSpinBox->value(),
						_ui.bSpinBox->value(),
						_ui.aSpinBox->value());

		QPainter painter(_ui.graphicsWidget);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setBrush(QBrush(_color));
		painter.setPen(QPen(Qt::black, 2, Qt::SolidLine));
		painter.drawRoundedRect(QRect(3, 3, _ui.graphicsWidget->width() - 6, _ui.graphicsWidget->height() - 6), 3.0, 3.0);
	}
	return QWidget::eventFilter(object, event);
}

} /* namespace NLQT */