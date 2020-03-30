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

#ifndef SKELETON_SCALE_DIALOG_H
#define SKELETON_SCALE_DIALOG_H

#include "ui_skeleton_scale_form.h"

// STL includes

// NeL includes
#include "nel/misc/smart_ptr.h"
#include "nel/misc/stream.h"
#include "nel/misc/vector.h"
#include "nel/misc/aabbox.h"
#include <nel/3d/u_skeleton.h>

// Project includes
#include "skeleton_tree_model.h"

namespace NLQT
{

/// A mirror to the list of bone
struct CBoneMirror
{
	CBoneMirror()
	{
		SkinScale= BoneScale= NLMISC::CVector(1.f,1.f,1.f);
		Selected= false;
	}

	/// Current Scale * ssd_scale_precision, and rounded
	NLMISC::CVector SkinScale;
	NLMISC::CVector BoneScale;

	/// If the bone is selected in the multi List
	bool		Selected;
};

typedef std::vector<CBoneMirror>	TBoneMirrorArray;

/**
@class CSkeletonScaleDialog
@brief Dialog to edit the skeleton.
*/
class CSkeletonScaleDialog: public QDockWidget
{
	Q_OBJECT

public:
	CSkeletonScaleDialog(CSkeletonTreeModel *model, QWidget *parent = 0);
	~CSkeletonScaleDialog();

	/// call each frame to display scaled bboxes around selected bones
	void drawSelection();

public Q_SLOTS:
	/// needs called when the changes current edited skeleton
	void setCurrentShape(const QString &name);

private Q_SLOTS:
	void setCurrentBone(const QModelIndex &index);
	void setBoneSliderX(int value);
	void setBoneSliderY(int value);
	void setBoneSliderZ(int value);
	void setSkinSliderX(int value);
	void setSkinSliderY(int value);
	void setSkinSliderZ(int value);
	void sliderReleased();
	void clickMirrorSelected();
	void clickUndo();
	void clickRedo();
	void clickSaveSkel();
	void clickSaveAsSkel();
	void clickLoadScale();
	void clickSaveScale();
	void resetSkeleton();

private:
	void updateBoneValues();
	void refreshTextViewWithScale(QDoubleSpinBox *spinBox, float scale, float diff);
	void applyScaleSlider(int scale, int idSelect);
	void applyMirrorToSkeleton();
	void applySkeletonToMirror();
	void roundClampScale(NLMISC::CVector &v) const;
	void refreshUndoRedoView();
	void applySelectionToView();
	sint getBoneForMirror(uint boneId, std::string &mirrorName);
	bool saveCurrentInStream(NLMISC::IStream &f);
	void refreshSaveButton();
	bool saveSkelScaleInStream(NLMISC::IStream &f);
	bool loadSkelScaleFromStream(NLMISC::IStream &f);

	/// bkup the current _Bones in the undo queue, and clear the redo. dirtSave indicate the skel need saving
	/// NB: compare precState with _Bones. if same, then no-op!
	void pushUndoState(const TBoneMirrorArray &precState, bool dirtSave);

	/// undo the last change, and store it in the redo queue
	void undo();

	/// redo the last undoed change, and restore it in the undo queue
	void redo();

	enum	TScaleId
	{
		SidBoneX = 0,
		SidBoneY,
		SidBoneZ,
		SidSkinX,
		SidSkinY,
		SidSkinZ,
		SidCount,
		SidNone = SidCount
	};

	NL3D::USkeleton _Skeleton;

	std::string _SkeletonFileName;

	// The current bones
	TBoneMirrorArray _Bones;

	// Backup of bones when slider start moving
	TBoneMirrorArray _BkupBones;

	bool _SaveDirty;

	// For selection drawing, the local bbox
	std::vector<NLMISC::CAABBox> _BoneBBoxes;
	bool _BoneBBoxNeedRecompute;

	std::deque<TBoneMirrorArray> _UndoQueue;
	std::deque<TBoneMirrorArray> _RedoQueue;

	static const int MaxUndoRedo = 100;

	Ui::CSkeletonScaleDialog _ui;

}; /* class CSkeletonScaleDialog */

} /* namespace NLQT */

#endif // SKELETON_SCALE_DIALOG_H
