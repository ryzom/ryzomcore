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
#include "animation_dialog.h"

// Qt includes

// NeL includes

// Project includes
#include "modules.h"

using namespace NLMISC;

namespace NLQT
{

CAnimationDialog::CAnimationDialog(QWidget *parent)
	: QDockWidget(parent)
{
	_ui.setupUi(this);

	//setFixedHeight(sizeHint().height());

	connect(_ui.startPushButton, SIGNAL(clicked()), this, SLOT(start()));
	connect(_ui.playPushButton, SIGNAL(clicked()), this, SLOT(play()));
	connect(_ui.stopPushButton, SIGNAL(clicked()), this, SLOT(stop()));
	connect(_ui.endPushButton, SIGNAL(clicked()), this, SLOT(end()));
	connect(_ui.horizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(changeFrame(int)));
	connect(_ui.startSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeStartAnim(int)));
	connect(_ui.endSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeEndAnim(int)));
	connect(_ui.playlistToolButton, SIGNAL(toggled(bool)), this, SLOT(setModePlayList()));
	connect(_ui.mixerToolButton, SIGNAL(toggled(bool)), this, SLOT(setModeMixer()));

	// init QTimeLine
	_timeLine = new QTimeLine(_ui.endSpinBox->value() * _frameRate, this);
	_timeLine->setCurveShape(QTimeLine::LinearCurve);
	_timeLine->setUpdateInterval(25);
	_timeLine->setFrameRange(_ui.startSpinBox->value(), _ui.endSpinBox->value());

	connect(_timeLine, SIGNAL(frameChanged(int)), this, SLOT(updateAnim(int)));
	connect(_timeLine, SIGNAL(finished()), this, SLOT(finish()));

	connect(_ui.incPosCheckBox, SIGNAL(toggled(bool)), this, SLOT(setIncPos(bool)));
	connect(_ui.inPlaceCheckBox, SIGNAL(toggled(bool)), this, SLOT(setInPlace(bool)));

	// sync horizontalSlider with a timeLine
	_ui.endSpinBox->setValue(99);
}

CAnimationDialog::~CAnimationDialog()
{
}

NL3D::TAnimationTime CAnimationDialog::getTime ()
{
	return float(_timeLine->currentFrame()) / _frameRate;
}

void CAnimationDialog::changeAnimLength()
{
	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
		return;
	CEntity &entity = Modules::objView().getEntity(curObj);
	float animLength = entity.getPlayListLength();

	_ui.startSpinBox->setValue(0);
	_ui.endSpinBox->setValue(int(animLength * _frameRate));
}

void CAnimationDialog::setCurrentShape(const QString &name)
{
	if (name.isEmpty())
		return;
	CEntity &entity = Modules::objView().getEntity(name.toUtf8().constData());

	_ui.inPlaceCheckBox->setChecked(entity.getInPlace());
	_ui.incPosCheckBox->setChecked(entity.getIncPos());

	if (_ui.playlistToolButton->isChecked())
		entity.setMode(CEntity::Mode::PlayList);
	else
		entity.setMode(CEntity::Mode::Mixer);
}

void CAnimationDialog::start()
{
	_timeLine->setCurrentTime((float(_ui.startSpinBox->value()) / _frameRate) * 1000);
}

void CAnimationDialog::play()
{
	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
	{
		_ui.playPushButton->setChecked(false);
		return;
	}
	CEntity &entity = Modules::objView().getEntity(curObj);
	entity.playbackAnim(true);

	if (_timeLine->state() == QTimeLine::Running)
		_timeLine->setPaused(true);
	else if (_timeLine->currentFrame() == _timeLine->endFrame())
		_timeLine->start();
	else
		_timeLine->resume();
}

void CAnimationDialog::stop()
{
	_timeLine->stop();
	_timeLine->setCurrentTime(0);
	_ui.playPushButton->setChecked(false);

	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
		return;
	CEntity &entity = Modules::objView().getEntity(curObj);
	entity.playbackAnim(false);
}

void CAnimationDialog::end()
{
	_timeLine->setCurrentTime((float(_ui.endSpinBox->value()) / _frameRate) * 1000);
}

void CAnimationDialog::changeFrame(int frame)
{
	if (_timeLine->state() == QTimeLine::Running)
	{
		_timeLine->setPaused(true);
		_timeLine->setCurrentTime((float(frame) / _frameRate) * 1000);
		_timeLine->resume();
	}
	else _timeLine->setCurrentTime((float(frame) / _frameRate) * 1000);
}

void CAnimationDialog::changeStartAnim(int start)
{
	_timeLine->setDuration((float(start - _ui.startSpinBox->value()) / _frameRate) * 1000);
	_timeLine->setFrameRange(start, _ui.endSpinBox->value());
}

void CAnimationDialog::changeEndAnim(int end)
{
	_ui.horizontalSlider->setMaximum(end);
	_timeLine->setDuration((float(end - _ui.startSpinBox->value()) / _frameRate) * 1000);
	_timeLine->setFrameRange(_ui.startSpinBox->value(), end);
}

void CAnimationDialog::updateAnim(int frame)
{
	_ui.horizontalSlider->setSliderPosition(frame);
}

void CAnimationDialog::setInPlace(bool state)
{
	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
		return;
	CEntity &entity = Modules::objView().getEntity(curObj);
	entity.setInPlace(state);
}

void CAnimationDialog::setIncPos(bool state)
{
	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
		return;
	CEntity &entity = Modules::objView().getEntity(curObj);
	entity.setIncPos(state);
}

void CAnimationDialog::finish()
{
	if (_ui.loopCheckBox->isChecked())
		play();
	else
		_ui.playPushButton->setChecked(false);
}

void CAnimationDialog::setModePlayList()
{
	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
		return;
	CEntity	&entity = Modules::objView().getEntity(curObj);

	entity.setMode(CEntity::Mode::PlayList);
}

void CAnimationDialog::setModeMixer()
{
	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
		return;
	CEntity	&entity = Modules::objView().getEntity(curObj);

	entity.setMode(CEntity::Mode::Mixer);
}

} /* namespace NLQT */
