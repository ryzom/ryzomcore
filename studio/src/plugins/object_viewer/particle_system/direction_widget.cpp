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
#include "direction_widget.h"

// Qt includes
#include <QtGui/QInputDialog>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

// NeL includes
#include <nel/misc/vector.h>

namespace NLQT
{
const int directionSize = 35;

CDirectionWidget::CDirectionWidget(QWidget *parent)
	: QWidget(parent), _globalName("")
{
	_ui.setupUi(this);

	_ui.xzWidget->installEventFilter(this);
	_ui.yzWidget->installEventFilter(this);
	_ui.xzWidget->setObjectName("XZ");
	_ui.yzWidget->setObjectName("YZ");
	_ui.globalPushButton->hide();

	connect(_ui.globalPushButton ,SIGNAL(clicked()), this, SLOT(setGlobalDirection()));
	connect(_ui.incVecIPushButton ,SIGNAL(clicked()), this, SLOT(incVecI()));
	connect(_ui.incVecJPushButton ,SIGNAL(clicked()), this, SLOT(incVecJ()));
	connect(_ui.incVecKPushButton ,SIGNAL(clicked()), this, SLOT(incVecK()));
	connect(_ui.decVecIPushButton ,SIGNAL(clicked()), this, SLOT(decVecI()));
	connect(_ui.decVecJPushButton ,SIGNAL(clicked()), this, SLOT(decVecJ()));
	connect(_ui.decVecKPushButton ,SIGNAL(clicked()), this, SLOT(decVecK()));

	// Set default value +K
	setValue(NLMISC::CVector::K);
}

CDirectionWidget::~CDirectionWidget()
{
}

void CDirectionWidget::enabledGlobalVariable(bool enabled)
{
	_ui.globalPushButton->setVisible(enabled);
	setGlobalName("", false);
}

void CDirectionWidget::setValue(const NLMISC::CVector &value, bool emit)
{
	_value = value;
	_ui.xzWidget->repaint();
	_ui.yzWidget->repaint();

	if (emit)
	{
		Q_EMIT valueChanged(_value);
	}
}

void CDirectionWidget::setGlobalName(const QString &globalName, bool emit)
{
	_globalName = globalName;

	_ui.xzWidget->setVisible(_globalName.isEmpty());
	_ui.yzWidget->setVisible(_globalName.isEmpty());

	_ui.incVecIPushButton->setEnabled(_globalName.isEmpty());
	_ui.incVecJPushButton->setEnabled(_globalName.isEmpty());
	_ui.incVecKPushButton->setEnabled(_globalName.isEmpty());
	_ui.decVecIPushButton->setEnabled(_globalName.isEmpty());
	_ui.decVecJPushButton->setEnabled(_globalName.isEmpty());
	_ui.decVecKPushButton->setEnabled(_globalName.isEmpty());

	if (emit)
		globalNameChanged(_globalName);
}

void CDirectionWidget::setGlobalDirection()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("Enter Name"),
										 "", QLineEdit::Normal,
										 QString(_globalName), &ok);

	if (ok)
		setGlobalName(text);
}

void CDirectionWidget::incVecI()
{
	setValue(NLMISC::CVector::I);
}

void CDirectionWidget::incVecJ()
{
	setValue(NLMISC::CVector::J);
}

void CDirectionWidget::incVecK()
{
	setValue(NLMISC::CVector::K);
}

void CDirectionWidget::decVecI()
{
	setValue( - NLMISC::CVector::I);
}

void CDirectionWidget::decVecJ()
{
	setValue( - NLMISC::CVector::J);
}

void CDirectionWidget::decVecK()
{
	setValue( - NLMISC::CVector::K);
}

void CDirectionWidget::setNewVecXZ(float x, float y)
{
	const float epsilon = 10E-3f;
	NLMISC::CVector v = _value;

	v.x = x;
	v.z = y;

	float d = v.x * v.x + v.z * v.z;
	float f;
	if (fabs(d) > epsilon)
		f = sqrt((1.f - v.y * v.y) / d);
	else
		f = 1;

	v.x *= f;
	v.z *= f;

	v.normalize();

	setValue(v);
}

void CDirectionWidget::setNewVecYZ(float x, float y)
{
	const float epsilon = 10E-3f;
	NLMISC::CVector v = _value;

	v.y = x;
	v.z = y;

	float d = v.y * v.y + v.z * v.z;
	float f;
	if (fabs(d) > epsilon)
		f = sqrt((1.f - v.x * v.x) / d);
	else
		f = 1;

	v.y *= f;
	v.z *= f;

	v.normalize();

	setValue(v);
}

bool CDirectionWidget::eventFilter(QObject *object, QEvent *event)
{
	QWidget *widget = qobject_cast<QWidget *>(object);
	switch (event->type())
	{
	case QEvent::Paint:
	{
		float x;
		if (widget->objectName() == "XZ")
			x = _value.x;
		else
			x = _value.y;
		QPainter painter(widget);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setBrush(QBrush(Qt::white));
		painter.setPen(QPen(Qt::black, 2, Qt::SolidLine));
		painter.drawRoundedRect(QRect(3, 3, widget->width() - 6, widget->height() - 6), 3.0, 3.0);
		painter.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
		painter.drawLine(widget->width() / 2, 4, widget->width() / 2, widget->height() - 4);
		painter.drawLine(4, widget->height() / 2, widget->width() - 4, widget->height() / 2);
		painter.drawText( 10, 15, widget->objectName());
		painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
		painter.drawLine(widget->width() / 2, widget->height() / 2,
						 int((widget->width() / 2) + x * 0.9f * directionSize), int((widget->height() / 2) - _value.z * 0.9f * directionSize));
		break;
	}
	case QEvent::MouseButtonDblClick:
	{
		QMouseEvent *mouseEvent = (QMouseEvent *) event;
		float vx = (mouseEvent->x() - (widget->width() / 2)) / 0.9f;
		float vy = ((widget->height() / 2) - mouseEvent->y()) / 0.9f;

		if (widget->objectName() == "XZ")
			setNewVecXZ(vx, vy);
		else
			setNewVecYZ(vx, vy);

		break;
	}
	}
	return QWidget::eventFilter(object, event);
}

} /* namespace NLQT */