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

#ifndef PARTICLE_TEXTURE_WIDGET_H
#define PARTICLE_TEXTURE_WIDGET_H

#include "ui_particle_texture_form.h"

// STL includes

// Qt includes

// NeL includes
#include <nel/misc/smart_ptr.h>
#include <nel/3d/texture.h>
#include <nel/3d/ps_particle_basic.h>

// Project includes
#include "ps_wrapper.h"

namespace NLQT
{

class CParticleTextureWidget: public QWidget
{
	Q_OBJECT

public:
	CParticleTextureWidget(QWidget *parent = 0);
	~CParticleTextureWidget();

	void updateUi();

	// set a wrapper to get the datas, called before setCurrentTextureNoAnim
	void setWrapper(IPSWrapperTexture *wrapper)
	{
		_Wrapper = wrapper ;
	}

	void enableRemoveButton(bool enabled)
	{
		_ui.removePushButton->setVisible(enabled);
	}

Q_SIGNALS:
	void textureChanged(const QString &texName);

private Q_SLOTS:
	void chooseTexture();
	void removeTexture();

private:
	void updateTexture();

	IPSWrapperTexture *_Wrapper;

	// the current texture
	NLMISC::CSmartPtr<NL3D::ITexture> _Texture;

	Ui::CParticleTextureWidget _ui;
}; /* class CParticleTextureWidget */

} /* namespace NLQT */

#endif // PARTICLE_TEXTURE_WIDGET_H
