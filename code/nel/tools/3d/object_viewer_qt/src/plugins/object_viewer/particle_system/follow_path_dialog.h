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

#ifndef FOLLOW_PATH_DIALOG_H
#define FOLLOW_PATH_DIALOG_H

#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>

namespace NL3D
{
class CPSPlaneBasisFollowSpeed;
}

namespace NLQT
{

class CWorkspaceNode;

class CFollowPathDialog : public  QDialog
{
	Q_OBJECT

public:
	CFollowPathDialog(NL3D::CPSPlaneBasisFollowSpeed *pbfs, CWorkspaceNode *ownerNode, QWidget *parent = 0);
	~CFollowPathDialog();

private Q_SLOTS:
	void setProjectionMode(int index);

protected:
	QGridLayout *gridLayout;
	QLabel *label;
	QComboBox *comboBox;
	QSpacerItem *horizontalSpacer;
	QPushButton *pushButton;

	NL3D::CPSPlaneBasisFollowSpeed *_FollowPath;
	CWorkspaceNode *_Node;
}; /* class CFollowPathDialog */

} /* namespace NLQT */

#endif // FOLLOW_PATH_DIALOG_H