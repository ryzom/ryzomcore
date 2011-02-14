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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

// STL includes

// Qt includes
#include <QtGui/QMainWindow>
#include <QtGui/QLabel>

// NeL includes
#include <nel/misc/config_file.h>

// Project includes

namespace NLMISC
{
class CConfigFile;
}

namespace NLQT
{

class CGraphicsViewport;
class CAnimationDialog;
class CAnimationSetDialog;
class CSlotManagerDialog;
class CParticleControlDialog;
class CParticleWorkspaceDialog;
class CSetupFog;
class CSkeletonScaleDialog;
class CSkeletonTreeModel;
class CWaterPoolDialog;
class CVegetableDialog;
class CGlobalWindDialog;
class CDayNightDialog;
class CSunColorDialog;
class CTuneMRMDialog;
class CTuneTimerDialog;

class CCameraControl;

class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CMainWindow(QWidget *parent = 0);
	~CMainWindow();

	virtual void setVisible(bool visible);

	int getFrameRate();
	CSkeletonTreeModel *getSkeletonModel() const
	{
		return _SkeletonTreeModel;
	}
	QPalette getOriginalPalette() const
	{
		return _originalPalette;
	}

private Q_SLOTS:
	void open();
	void resetScene();
	void reloadTextures();
	void settings();
	void about();
	void updateStatusBar();
	void updateRender();
	void setInterval(int value);

private:
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void createDialogs();

	bool loadFile(const QString &fileName, const QString &skelName);

	void cfcbQtStyle(NLMISC::CConfigFile::CVar &var);
	void cfcbQtPalette(NLMISC::CConfigFile::CVar &var);
	void cfcbSoundEnabled(NLMISC::CConfigFile::CVar &var);

	bool _isGraphicsInitialized, _isGraphicsEnabled;
	bool _isSoundInitialized, _isSoundEnabled;

	CGraphicsViewport *_GraphicsViewport;
	CAnimationDialog *_AnimationDialog;
	CAnimationSetDialog *_AnimationSetDialog;
	CSlotManagerDialog *_SlotManagerDialog;
	CParticleControlDialog *_ParticleControlDialog;
	CParticleWorkspaceDialog *_ParticleWorkspaceDialog;
	CVegetableDialog *_VegetableDialog;
	CWaterPoolDialog *_WaterPoolDialog;
	CGlobalWindDialog *_GlobalWindDialog;
	CSetupFog *_SetupFog;
	CSkeletonScaleDialog *_SkeletonScaleDialog;
	CDayNightDialog *_DayNightDialog;
	CSunColorDialog *_SunColorDialog;
	CTuneMRMDialog *_TuneMRMDialog;
	CSkeletonTreeModel *_SkeletonTreeModel;
	CTuneTimerDialog *_TuneTimerDialog;

	CCameraControl *_cameraControl;

	QPalette _originalPalette;
	QString _lastDir;

	QTimer *_mainTimer;
	QTimer *_statusBarTimer;

	QMenu *_fileMenu;
	QMenu *_viewMenu;
	QMenu *_sceneMenu;
	QMenu *_toolsMenu;
	QMenu *_helpMenu;
	QToolBar *_fileToolBar;
	QToolBar *_editToolBar;
	QToolBar *_toolsBar;
	QAction *_openAction;
	QAction *_exitAction;
	QAction *_setBackColorAction;
	QAction *_renderModeAction;
	QAction *_frameDelayAction;
	QAction *_lightGroupAction;
	QAction *_reloadTexturesAction;
	QAction *_resetCameraAction;
	QAction *_resetSceneAction;
	QAction *_saveScreenshotAction;
	QAction *_settingsAction;
	QAction *_aboutAction;
	QAction *_aboutQtAction;

	QLabel *_statusInfo;

	float _fps;
	uint _numTri;
	float _texMem;
	sint _mouseMode;
};/* class CMainWindow */

} /* namespace NLQT */

#endif // MAIN_WINDOW_H
