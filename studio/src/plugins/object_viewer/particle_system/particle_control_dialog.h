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

#ifndef PARTICLE_CONTROL_DIALOG_H
#define PARTICLE_CONTROL_DIALOG_H

#include <nel/misc/types_nl.h>
#include "ui_particle_control_form.h"

// Project includes

namespace NLQT
{

class CSkeletonTreeModel;
class CParticleLinkDialog;

class CParticleControlDialog: public QDockWidget
{
	Q_OBJECT
public:
	CParticleControlDialog(CSkeletonTreeModel *model, QWidget *parent = 0);
	~CParticleControlDialog();

Q_SIGNALS:
	void changeState();
	void changeCount();
	void changeAutoCount(bool state);

public Q_SLOTS:
	void updateActiveNode();

private Q_SLOTS:
	void play();
	void stop();
	void sliderMoved(int value);
	void displayHelpers(bool state);
	void displayBBox(bool state);
	void autoRepeat(bool state);
	void setEnabledAutoCount(bool state);
	void resetAutoCount();
	void updateCount();

	void linkSceleton();
	void unlink();
	void setAnim();
	void clearAnim();
	void restickObjects();

	void setGeneralWidget();
	void setAddtitionalWidget();

private:

	QTimer *_timer;

	CParticleLinkDialog *_particleLinkDialog;

	Ui::CParticleControlDialog _ui;
}; /* class CParticleControlDialog */

} /* namespace NLQT */

#endif // PARTICLE_CONTROL_DIALOG_H
