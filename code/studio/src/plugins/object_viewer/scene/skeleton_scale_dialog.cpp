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
#include "skeleton_scale_dialog.h"

// Qt include
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

// NeL includes
#include <nel/misc/file.h>
#include <nel/3d/u_bone.h>
#include <nel/3d/bone.h>
#include <nel/3d/skeleton_shape.h>

// Project includes
#include "modules.h"
#include "skeleton_tree_model.h"

namespace NLQT
{

const int ssd_scale_precision = 1000;

CSkeletonScaleDialog::CSkeletonScaleDialog(CSkeletonTreeModel *model, QWidget *parent)
	: QDockWidget(parent), _Skeleton(NULL)
{
	_ui.setupUi(this);

	_SaveDirty= false;

	// avoid realloc
	_UndoQueue.resize(MaxUndoRedo);
	_RedoQueue.resize(MaxUndoRedo);
	_UndoQueue.clear();
	_RedoQueue.clear();

	_BoneBBoxNeedRecompute= false;

	_ui.treeView->setModel(model);

	connect(model, SIGNAL(modelReset()), this, SLOT(resetSkeleton()));

	connect(_ui.treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(setCurrentBone(QModelIndex)));
	connect(_ui.xBoneHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setBoneSliderX(int)));
	connect(_ui.yBoneHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setBoneSliderY(int)));
	connect(_ui.zBoneHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setBoneSliderZ(int)));
	connect(_ui.xSkinHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setSkinSliderX(int)));
	connect(_ui.ySkinHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setSkinSliderY(int)));
	connect(_ui.zSkinHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setSkinSliderZ(int)));

	connect(_ui.xBoneHorizontalSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
	connect(_ui.yBoneHorizontalSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
	connect(_ui.zBoneHorizontalSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
	connect(_ui.xSkinHorizontalSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
	connect(_ui.ySkinHorizontalSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
	connect(_ui.zSkinHorizontalSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));

	connect(_ui.undoPushButton, SIGNAL(clicked()), this, SLOT(clickUndo()));
	connect(_ui.redoPushButton, SIGNAL(clicked()), this, SLOT(clickRedo()));
	connect(_ui.mirrorPushButton, SIGNAL(clicked()), this, SLOT(clickMirrorSelected()));
	connect(_ui.saveAsSkelPushButton, SIGNAL(clicked()), this, SLOT(clickSaveAsSkel()));
	connect(_ui.saveSkelPushButton, SIGNAL(clicked()), this, SLOT(clickSaveSkel()));
	connect(_ui.saveScalePushButton, SIGNAL(clicked()), this, SLOT(clickSaveScale()));
	connect(_ui.loadScalePushButton, SIGNAL(clicked()), this, SLOT(clickLoadScale()));
}

CSkeletonScaleDialog::~CSkeletonScaleDialog()
{
}

void CSkeletonScaleDialog::setCurrentShape(const QString &name)
{
	_ui.treeView->expandAll();

	if (name.isEmpty())
		return;

	_Skeleton = Modules::objView().getEntity(name.toUtf8().constData()).getSkeleton();
	_SkeletonFileName = Modules::objView().getEntity(name.toUtf8().constData()).getFileNameSkeleton();

	// Setup Bone mirror
	_Bones.clear();
	if(!_Skeleton.empty())
	{
		_Bones.resize(_Skeleton.getNumBones());

		// copy from skel to mirror
		applySkeletonToMirror();

		_ui.boneGroupBox->setEnabled(true);
		_ui.skinGroupBox->setEnabled(true);
		_ui.buttonsGroupBox->setEnabled(true);
	}

	// reset bone bbox here
	_BoneBBoxes.clear();
	_BoneBBoxes.resize(_Bones.size());

	// delegate to drawSelection(), cause skins not still bound
	_BoneBBoxNeedRecompute= true;

	// clean undo/redo
	_UndoQueue.clear();
	_RedoQueue.clear();
	refreshUndoRedoView();

	// clear save button
	_SaveDirty= false;
	refreshSaveButton();
	_BkupBones = _Bones;
}

void CSkeletonScaleDialog::setCurrentBone(const QModelIndex &index)
{
	CSkeletonTreeItem *currentItem = static_cast<CSkeletonTreeItem *>(index.internalPointer());

	// bkup for undo
	static TBoneMirrorArray precState;
	precState = _Bones;

	// TODO: multiple selection
	for(size_t i = 0; i < _Bones.size(); i++)
		_Bones[i].Selected= false;

	_Bones[currentItem->getId()].Selected = true;

	// undo-redo
	// selection change => no need to dirt save
	pushUndoState(precState, false);

	// refresh text views
	updateBoneValues();
}

void CSkeletonScaleDialog::setBoneSliderX(int value)
{
	applyScaleSlider(value, SidBoneX);
}

void CSkeletonScaleDialog::setBoneSliderY(int value)
{
	applyScaleSlider(value, SidBoneY);
}

void CSkeletonScaleDialog::setBoneSliderZ(int value)
{
	applyScaleSlider(value, SidBoneZ);
}

void CSkeletonScaleDialog::setSkinSliderX(int value)
{
	applyScaleSlider(value, SidSkinX);
}

void CSkeletonScaleDialog::setSkinSliderY(int value)
{
	applyScaleSlider(value, SidSkinY);
}

void CSkeletonScaleDialog::setSkinSliderZ(int value)
{
	applyScaleSlider(value, SidSkinZ);
}

void CSkeletonScaleDialog::sliderReleased()
{
	// Bkup all scales (dont bother selected bones or which scale is edited...)
	_BkupBones = _Bones;

	// push an undo/redo only at release of slide. push the value of scale before slide
	// change => must save
	pushUndoState(_BkupBones, true);
	_SaveDirty = true;
	refreshSaveButton();

	_ui.xBoneHorizontalSlider->setValue(100);
	_ui.yBoneHorizontalSlider->setValue(100);
	_ui.zBoneHorizontalSlider->setValue(100);
	_ui.xSkinHorizontalSlider->setValue(100);
	_ui.ySkinHorizontalSlider->setValue(100);
	_ui.zSkinHorizontalSlider->setValue(100);
}

void CSkeletonScaleDialog::clickMirrorSelected()
{
	// bkup for undo
	static TBoneMirrorArray precState;
	precState = _Bones;
	nlassert(!_Skeleton.empty());

	// for each bone selected
	bool	change= false;
	for(uint i=0; i < _Bones.size(); ++i)
	{
		CBoneMirror &bone= _Bones[i];
		if(bone.Selected)
		{
			// get the bone side and mirrored name
			std::string mirrorName;
			sint side = getBoneForMirror(i, mirrorName);
			// if not a "centered" bone
			if(side!=0)
			{
				// get the bone with mirrored name
				sint mirrorId = _Skeleton.getBoneIdByName(mirrorName);
				if(mirrorId<0)
				{
					nlinfo("MirrorScale: Didn't found %s", mirrorName.c_str());
				}
				else
				{
					// copy setup from the dest bone
					nlassert(mirrorId<(sint)_Bones.size());
					_Bones[mirrorId].BoneScale= bone.BoneScale;
					_Bones[mirrorId].SkinScale= bone.SkinScale;
				}
			}
		}
	}

	// refresh display
	applyMirrorToSkeleton();
	updateBoneValues();

	// if some change, bkup for undo/redo
	pushUndoState(precState, true);
}

void CSkeletonScaleDialog::clickUndo()
{
	undo();
}

void CSkeletonScaleDialog::clickRedo()
{
	redo();
}

void CSkeletonScaleDialog::clickSaveSkel()
{
	// if no skeleton edited, quit
	if(_Skeleton.empty())
		return;

	// save the file
	NLMISC::COFile f;
	if( f.open(_SkeletonFileName) )
	{
		if(saveCurrentInStream(f))
		{
			// no more need to save (done)
			_SaveDirty= false;
			refreshSaveButton();
		}
	}
	else
	{
		QMessageBox::critical(this, tr("Skeleton scale editor"), tr("Failed to open file for write!"), QMessageBox::Ok);
	}
}

void CSkeletonScaleDialog::clickSaveAsSkel()
{
	// if no skeleton edited, quit
	if(_Skeleton.empty())
		return;

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save As Skeleton File"),
					   ".",
					   tr("Skeleton files (*.skel);;"));
	if (!fileName.isEmpty())
	{
		NLMISC::COFile f;

		if( f.open(fileName.toUtf8().constData()) )
		{
			if(saveCurrentInStream(f))
			{
				// no more need to save (done)
				_SaveDirty= false;
				refreshSaveButton();
			}

			// bkup the valid fileName (new file edited)
			_SkeletonFileName = fileName.toUtf8().constData();
		}
		else
		{
			QMessageBox::critical(this, tr("Skeleton scale editor"), tr("Failed to open file for write!"), QMessageBox::Ok);
		}
	}
}

void CSkeletonScaleDialog::clickLoadScale()
{
	// if no skeleton edited, quit
	if(_Skeleton.empty())
		return;

	// choose the file
	QString fileName = QFileDialog::getOpenFileName(this,
					   tr("Open Skeleton Scale File"), ".",
					   tr("SkelScale files (*.scale);;"));

	setCursor(Qt::WaitCursor);
	if (!fileName.isEmpty())
	{
		NLMISC::CIFile f;
		if( f.open(fileName.toUtf8().constData()) )
			loadSkelScaleFromStream(f);
		else
			QMessageBox::critical(this, tr("Skeleton scale editor"), tr("Failed to open file for read!"), QMessageBox::Ok);
	}
	setCursor(Qt::ArrowCursor);
}

void CSkeletonScaleDialog::clickSaveScale()
{
	// if no skeleton edited, quit
	if(_Skeleton.empty())
		return;

	// choose the file
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save As Skeleton Scale File"),
					   ".",
					   tr("SkelScale files (*.scale);;"));
	if (!fileName.isEmpty())
	{
		NLMISC::COFile f;
		if( f.open(fileName.toUtf8().constData()) )
			saveSkelScaleInStream(f);
		else
			QMessageBox::critical(this, tr("Skeleton scale editor"), tr("Failed to open file for write!"), QMessageBox::Ok);
	}
}

void CSkeletonScaleDialog::resetSkeleton()
{
	_ui.boneGroupBox->setEnabled(false);
	_ui.skinGroupBox->setEnabled(false);
	_ui.buttonsGroupBox->setEnabled(false);
	_Skeleton = NULL;
}

void CSkeletonScaleDialog::updateBoneValues()
{
	// 1.f for each component if multiple selection is different, else 0.f
	NLMISC::CVector boneScaleDiff = NLMISC::CVector::Null;
	NLMISC::CVector skinScaleDiff = NLMISC::CVector::Null;

	// valid if scale of each bone component is same
	NLMISC::CVector boneScaleAll = NLMISC::CVector::Null;
	NLMISC::CVector skinScaleAll = NLMISC::CVector::Null;
	bool someSelected = false;

	// For all bones selected
	for(uint i = 0; i < _Bones.size(); i++)
	{
		CBoneMirror &bone= _Bones[i];
		if(bone.Selected)
		{
			if(!someSelected)
			{
				someSelected= true;
				// just bkup
				boneScaleAll= bone.BoneScale;
				skinScaleAll= bone.SkinScale;
			}
			else
			{
				// compare each component, if different, flag
				if(boneScaleAll.x!= bone.BoneScale.x) boneScaleDiff.x= 1.f;
				if(boneScaleAll.y!= bone.BoneScale.y) boneScaleDiff.y= 1.f;
				if(boneScaleAll.z!= bone.BoneScale.z) boneScaleDiff.z= 1.f;
				if(skinScaleAll.x!= bone.SkinScale.x) skinScaleDiff.x= 1.f;
				if(skinScaleAll.y!= bone.SkinScale.y) skinScaleDiff.y= 1.f;
				if(skinScaleAll.z!= bone.SkinScale.z) skinScaleDiff.z= 1.f;
			}
		}
	}

	// if none selected, force empty text
	if(!someSelected)
	{
		boneScaleDiff.set(1.f,1.f,1.f);
		skinScaleDiff.set(1.f,1.f,1.f);
	}

	// All component refresh or only one refresh?
	refreshTextViewWithScale(_ui.xBoneDoubleSpinBox, boneScaleAll.x, boneScaleDiff.x);
	refreshTextViewWithScale(_ui.yBoneDoubleSpinBox, boneScaleAll.y, boneScaleDiff.y);
	refreshTextViewWithScale(_ui.zBoneDoubleSpinBox, boneScaleAll.z, boneScaleDiff.z);
	refreshTextViewWithScale(_ui.xSkinDoubleSpinBox, skinScaleAll.x, skinScaleDiff.x);
	refreshTextViewWithScale(_ui.ySkinDoubleSpinBox, skinScaleAll.y, skinScaleDiff.y);
	refreshTextViewWithScale(_ui.zSkinDoubleSpinBox, skinScaleAll.z, skinScaleDiff.z);
}

void CSkeletonScaleDialog::refreshTextViewWithScale(QDoubleSpinBox *spinBox, float scale, float diff)
{
	// if different values selected, diff
	if(diff)
		spinBox->setValue(0);
	// else display text
	else
		spinBox->setValue(scale * 100 / ssd_scale_precision);
}

void CSkeletonScaleDialog::applyScaleSlider(int scale, int idSelect)
{
	float factor = scale / 100.0;
	// Apply the noise to each selected bones
	for(uint i = 0; i < _Bones.size(); i++)
	{
		if( _Bones[i].Selected)
		{
			switch(idSelect)
			{
			case SidBoneX:
				_Bones[i].BoneScale.x = _BkupBones[i].BoneScale.x * factor;
				break;
			case SidBoneY:
				_Bones[i].BoneScale.y = _BkupBones[i].BoneScale.y * factor;
				break;
			case SidBoneZ:
				_Bones[i].BoneScale.z = _BkupBones[i].BoneScale.z * factor;
				break;
			case SidSkinX:
				_Bones[i].SkinScale.x = _BkupBones[i].SkinScale.x * factor;
				break;
			case SidSkinY:
				_Bones[i].SkinScale.y = _BkupBones[i].SkinScale.y * factor;
				break;
			case SidSkinZ:
				_Bones[i].SkinScale.z = _BkupBones[i].SkinScale.z * factor;
				break;
			};
			roundClampScale(_Bones[i].BoneScale);
			roundClampScale(_Bones[i].SkinScale);
		}
	}
	// update the skeleton view
	applyMirrorToSkeleton();

	// refresh text views
	updateBoneValues();
}

void CSkeletonScaleDialog::applyMirrorToSkeleton()
{
	if(!_Skeleton.empty())
	{
		nlassert(_Skeleton.getNumBones() == _Bones.size());
		for(uint i = 0; i < _Bones.size(); ++i)
		{
			// unmul from precision
			NLMISC::CVector boneScale = _Bones[i].BoneScale / ssd_scale_precision;
			NLMISC::CVector skinScale = _Bones[i].SkinScale / ssd_scale_precision;
			_Skeleton.getBone(i).setScale(boneScale);
			_Skeleton.getBone(i).setSkinScale(skinScale);
		}
	}
}

void CSkeletonScaleDialog::applySkeletonToMirror()
{
	if(!_Skeleton.empty())
	{
		nlassert(_Skeleton.getNumBones() == _Bones.size());
		for(uint i = 0; i < _Skeleton.getNumBones(); ++i)
		{
			// mul by precision, and round
			_Bones[i].SkinScale = _Skeleton.getBone(i).getSkinScale() * ssd_scale_precision;
			_Bones[i].BoneScale = _Skeleton.getBone(i).getScale() * ssd_scale_precision;
			roundClampScale(_Bones[i].SkinScale);
			roundClampScale(_Bones[i].BoneScale);
		}
	}
}

void CSkeletonScaleDialog::roundClampScale(NLMISC::CVector &v) const
{
	v.x+= 0.5f;
	v.y+= 0.5f;
	v.z+= 0.5f;
	v.x= (float)floor(v.x);
	v.y= (float)floor(v.y);
	v.z= (float)floor(v.z);
	// Minimum is 1 (avoid 0 scale)
	v.maxof(v, NLMISC::CVector(1.f,1.f,1.f));
}

void CSkeletonScaleDialog::refreshUndoRedoView()
{
	_ui.undoPushButton->setEnabled(!_UndoQueue.empty());
	_ui.redoPushButton->setEnabled(!_RedoQueue.empty());
}

void CSkeletonScaleDialog::applySelectionToView()
{
	_ui.treeView->blockSignals(true);
	CSkeletonTreeModel *model = Modules::mainWin().getSkeletonModel();
	for(uint i = 0; i < _Bones.size(); ++i)
	{
		if (_Bones[i].Selected)
			_ui.treeView->setCurrentIndex(model->getIndexFromId(i, model->index(0, 0)));
	}
	_ui.treeView->blockSignals(false);
}

sint CSkeletonScaleDialog::getBoneForMirror(uint boneId, std::string &mirrorName)
{
	sint side= 0;
	std::string::size_type pos;
	nlassert(!_Skeleton.empty() && (boneId < _Skeleton.getNumBones()));
	mirrorName= _Skeleton.getBone(boneId).getObjectPtr()->getBoneName();

	if((pos= mirrorName.find(" R "))!=std::string::npos)
	{
		side= 1;
		mirrorName[pos+1]= 'L';
	}
	else if((pos= mirrorName.find(" L "))!=std::string::npos)
	{
		side= -1;
		mirrorName[pos+1]= 'R';
	}

	return side;
}

void CSkeletonScaleDialog::refreshSaveButton()
{
	// SaveAs is always available
	_ui.saveSkelPushButton->setEnabled(_SaveDirty);
}

bool CSkeletonScaleDialog::saveCurrentInStream(NLMISC::IStream &f)
{
	try
	{
		nlassert(!_Skeleton.empty());
		nlassert(_Skeleton.getObjectPtr()->Shape);

		// Retrieve boneBase definition from the current skeleton
		std::vector<NL3D::CBoneBase> boneBases;
		(NLMISC::safe_cast<NL3D::CSkeletonShape *>((NL3D::IShape *)_Skeleton.getObjectPtr()->Shape))->retrieve(boneBases);

		// Copies bone scales from the model
		nlassert(boneBases.size() == _Skeleton.getNumBones());
		for(uint i = 0; i < _Skeleton.getNumBones(); i++)
		{
			NL3D::UBone bone = _Skeleton.getBone(i);
			NL3D::CBoneBase &boneBase = boneBases[i];

			boneBase.SkinScale = bone.getSkinScale();
			boneBase.DefaultScale = bone.getScale();
		}

		// build a new Skeleton shape
		NL3D::CSkeletonShape *skelShape= new NL3D::CSkeletonShape;
		skelShape->build(boneBases);

		// save
		NL3D::CShapeStream ss;
		ss.setShapePointer(skelShape);
		ss.serial(f);
		delete skelShape;
	}
	catch(NLMISC::EStream &)
	{
		QMessageBox::critical(this, tr("Skeleton scale editor"), tr("Failed to save file!"), QMessageBox::Ok);
		return false;
	}

	return true;
}

struct CBoneScaleInfo
{
	std::string Name;
	NLMISC::CVector Scale;
	NLMISC::CVector SkinScale;

	void	serial(NLMISC::IStream &f)
	{
		sint32 ver= f.serialVersion(0);
		f.serial(Name, Scale, SkinScale);
	}
};

bool CSkeletonScaleDialog::saveSkelScaleInStream(NLMISC::IStream &f)
{
	try
	{
		nlassert(!_Skeleton.empty());

		// Copies bone scales from the model
		std::vector<CBoneScaleInfo> boneScales;
		boneScales.resize(_Skeleton.getNumBones());
		for(uint i = 0; i < boneScales.size(); ++i)
		{
			NL3D::CBone *bone= _Skeleton.getBone(i).getObjectPtr();
			CBoneScaleInfo &boneScale= boneScales[i];

			// get scale info from current edited skeleton
			boneScale.Name = bone->getBoneName();
			boneScale.Scale = bone->getScale();
			boneScale.SkinScale = bone->getSkinScale();
		}

		// save the file
		sint32 ver= f.serialVersion(0);
		f.serialCont(boneScales);
	}
	catch(NLMISC::EStream &)
	{
		QMessageBox::critical(this, tr("Skeleton scale editor"), tr("Failed to save file!"), QMessageBox::Ok);
		return false;
	}

	return true;
}

bool CSkeletonScaleDialog::loadSkelScaleFromStream(NLMISC::IStream &f)
{
	try
	{
		nlassert(!_Skeleton.empty());

		// load the file
		sint32	ver= f.serialVersion(0);
		std::vector<CBoneScaleInfo> boneScales;
		f.serialCont(boneScales);

		// apply to the current skeleton
		for(uint i = 0; i < boneScales.size(); ++i)
		{
			sint32 boneId = _Skeleton.getBoneIdByName(boneScales[i].Name);
			if(boneId >= 0 && boneId < (sint32)_Skeleton.getNumBones())
			{
				CBoneScaleInfo	&boneScale= boneScales[i];
				_Skeleton.getBone(boneId).setScale(boneScale.Scale);
				_Skeleton.getBone(boneId).setSkinScale(boneScale.SkinScale);
			}
		}

		// Bkup _Bones, for undo
		static TBoneMirrorArray precState;
		precState = _Bones;

		// Then reapply to the mirror
		applySkeletonToMirror();

		// change => must save
		pushUndoState(precState, true);

		// and update display
		updateBoneValues();
	}
	catch(NLMISC::EStream &)
	{
		QMessageBox::critical(this, tr("Skeleton scale editor"), tr("Failed to load file!"), QMessageBox::Ok);
		return false;
	}

	return true;
}

void CSkeletonScaleDialog::pushUndoState(const TBoneMirrorArray &precState, bool dirtSave)
{
	// test if real change
	nlassert(precState.size() == _Bones.size());
	bool change = false;
	for(uint i = 0; i < _Bones.size(); ++i)
	{
		if( _Bones[i].BoneScale!=precState[i].BoneScale ||
				_Bones[i].SkinScale!=precState[i].SkinScale ||
				_Bones[i].Selected!=precState[i].Selected)
		{
			change= true;
			break;
		}
	}
	// no change? no op
	if(!change)
		return;

	// then bkup for undo
	// change => the redo list is lost
	_RedoQueue.clear();

	// if not enough space, the last undo is lost
	if(_UndoQueue.size() == size_t(MaxUndoRedo))
		_UndoQueue.pop_front();

	// add the precedent state to the undo queue
	_UndoQueue.push_back(precState);

	// refresh buttons
	refreshUndoRedoView();

	// refresh save button
	if(dirtSave && !_SaveDirty)
	{
		_SaveDirty = true;
		refreshSaveButton();
	}
}

void CSkeletonScaleDialog::undo()
{
	nlassert(_UndoQueue.size() + _RedoQueue.size() <= size_t(MaxUndoRedo));

	// is some undoable
	if(_UndoQueue.size())
	{
		// current goes into the redo queue
		_RedoQueue.push_front(_Bones);
		// restore
		_Bones= _UndoQueue.back();
		// pop
		_UndoQueue.pop_back();

		// refresh display
		applyMirrorToSkeleton();
		updateBoneValues();
		applySelectionToView();

		// refresh buttons
		refreshUndoRedoView();

		// change => must save
		_SaveDirty= true;
		refreshSaveButton();
	}
}

void CSkeletonScaleDialog::redo()
{
	nlassert(_UndoQueue.size() + _RedoQueue.size() <= size_t(MaxUndoRedo));

	// is some redoable
	if(_RedoQueue.size())
	{
		// current goes into the undo queue
		_UndoQueue.push_back(_Bones);
		// restore
		_Bones= _RedoQueue.front();
		// pop
		_RedoQueue.pop_front();

		// refresh display
		applyMirrorToSkeleton();
		updateBoneValues();
		applySelectionToView();

		// refresh buttons
		refreshUndoRedoView();

		// change => must save
		_SaveDirty= true;
		refreshSaveButton();
	}
}

} /* namespace NLQT */