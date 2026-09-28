// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "georges_editor_doc.h"

#include <QFileInfo>

#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/o_xml.h>
#include <nel/georges/form_loader.h>

using namespace NLGEORGES;
using namespace NLMISC;
using namespace std;

// ---- CGeorgesEditDocSub ----

CGeorgesEditDocSub::CGeorgesEditDocSub()
	: _StructId(0), _Type(Null), _Parent(nullptr), _Slot(0)
{
}

CGeorgesEditDocSub::~CGeorgesEditDocSub()
{
	clean();
}

bool CGeorgesEditDocSub::isEditable() const
{
	return _Type != Null;
}

CGeorgesEditDocSub::TSub CGeorgesEditDocSub::getType() const
{
	return _Type;
}

uint CGeorgesEditDocSub::getIdInParent() const
{
	if (_Parent)
	{
		for (uint i = 0; i < _Parent->_Children.size(); i++)
		{
			if (_Parent->_Children[i] == this)
				return i;
		}
	}
	return 0;
}

const std::string &CGeorgesEditDocSub::getName() const
{
	return _Name;
}

const std::string &CGeorgesEditDocSub::getFormName() const
{
	return _FormName;
}

uint CGeorgesEditDocSub::getChildrenCount() const
{
	return (uint)_Children.size();
}

void CGeorgesEditDocSub::removeChildren(uint child)
{
	if (child < _Children.size())
	{
		delete _Children[child];
		_Children.erase(_Children.begin() + child);
	}
}

CGeorgesEditDocSub *CGeorgesEditDocSub::getChild(uint child)
{
	if (child < _Children.size())
		return _Children[child];
	return nullptr;
}

CGeorgesEditDocSub *CGeorgesEditDocSub::getParent()
{
	return _Parent;
}

uint CGeorgesEditDocSub::getSlot() const
{
	return _Slot;
}

void CGeorgesEditDocSub::create(TSub type, const char *name, uint structId, const char *formName, uint slot)
{
	_Type = type;
	_Name = name;
	_StructId = structId;
	_FormName = formName;
	_Slot = slot;
}

CGeorgesEditDocSub *CGeorgesEditDocSub::add(TSub type, const char *name, uint structId, const char *formName, uint slot)
{
	CGeorgesEditDocSub *sub = new CGeorgesEditDocSub();
	sub->create(type, name, structId, formName, slot);
	sub->_Parent = this;
	_Children.push_back(sub);
	return sub;
}

void CGeorgesEditDocSub::clean()
{
	for (uint i = 0; i < _Children.size(); i++)
		delete _Children[i];
	_Children.clear();
}

int CGeorgesEditDocSub::getItemImage(GeorgesEditorDoc *doc) const
{
	Q_UNUSED(doc);
	switch (_Type)
	{
	case Header:
		return 0; // header icon
	case Type:
		return 1; // struct icon
	case Dfn:
		return 2; // vstruct icon
	case Form:
		return 3; // form/struct icon
	default:
		return 4; // root icon
	}
}

// ---- GeorgesEditorDoc ----

GeorgesEditorDoc::GeorgesEditorDoc()
	: _docType(TypeDoc), _modified(false)
{
}

GeorgesEditorDoc::~GeorgesEditorDoc()
{
}

bool GeorgesEditorDoc::open(const QString &path)
{
	_filePath = path;
	QFileInfo fi(path);
	QString ext = fi.suffix().toLower();

	std::string stdPath = path.toUtf8().constData();

	try
	{
		if (ext == "typ")
		{
			_docType = TypeDoc;
			_type = _loader.loadType(stdPath.c_str());
			if (!_type)
				return false;
		}
		else if (ext == "dfn")
		{
			_docType = DfnDoc;
			_dfn = _loader.loadFormDfn(stdPath.c_str(), false);
			if (!_dfn)
				return false;
		}
		else
		{
			_docType = FormDoc;
			UForm *uform = _loader.loadForm(stdPath.c_str());
			_form = uform;
			if (!_form)
				return false;
		}
	}
	catch (const Exception &e)
	{
		nlwarning("Error loading file: %s", e.what());
		return false;
	}

	updateDocumentStructure();
	return true;
}

bool GeorgesEditorDoc::save(const QString &path)
{
	QString savePath = path.isEmpty() ? _filePath : path;
	if (savePath.isEmpty())
		return false;

	if (!path.isEmpty())
		_filePath = path;

	std::string stdPath = savePath.toUtf8().constData();

	try
	{
		COFile file;
		if (!file.open(stdPath))
			return false;

		COXml xml;
		xml.init(&file);

		if (_docType == TypeDoc && _type)
		{
			_type->write(xml.getDocument());
		}
		else if (_docType == DfnDoc && _dfn)
		{
			_dfn->write(xml.getDocument(), stdPath);
		}
		else if (_docType == FormDoc && _form)
		{
			CForm *form = dynamic_cast<CForm *>((UForm *)_form);
			if (form)
				form->write(xml.getDocument(), stdPath);
		}

		xml.flush();
		file.close();
		_modified = false;
		return true;
	}
	catch (const Exception &e)
	{
		nlwarning("Error saving: %s", e.what());
		return false;
	}
}

bool GeorgesEditorDoc::isModified() const
{
	return _modified;
}

void GeorgesEditorDoc::setModified(bool modified)
{
	_modified = modified;
}

QString GeorgesEditorDoc::getFileName() const
{
	if (_filePath.isEmpty())
		return "Untitled";
	QFileInfo fi(_filePath);
	return fi.fileName();
}

const CFileHeader *GeorgesEditorDoc::getHeaderPtr() const
{
	if (_docType == TypeDoc && _type)
		return &_type->Header;
	if (_docType == DfnDoc && _dfn)
		return &_dfn->Header;
	if (_docType == FormDoc && _form)
	{
		const CForm *form = dynamic_cast<const CForm *>((const UForm *)_form);
		if (form)
			return &form->Header;
	}
	return nullptr;
}

CFileHeader *GeorgesEditorDoc::getHeaderPtr()
{
	if (_docType == TypeDoc && _type)
		return &_type->Header;
	if (_docType == DfnDoc && _dfn)
		return &_dfn->Header;
	if (_docType == FormDoc && _form)
	{
		CForm *form = dynamic_cast<CForm *>((UForm *)_form);
		if (form)
			return &form->Header;
	}
	return nullptr;
}

void GeorgesEditorDoc::updateDocumentStructure()
{
	_docTree.clean();

	if (_docType == TypeDoc)
		buildTypeTree();
	else if (_docType == DfnDoc)
		buildDfnTree();
	else if (_docType == FormDoc)
		buildFormTree();
}

void GeorgesEditorDoc::buildTypeTree()
{
	_docTree.create(CGeorgesEditDocSub::Null, getFileName().toUtf8().constData(), 0, "", 0);
	_docTree.add(CGeorgesEditDocSub::Header, "Header", 0, "", 0);
	_docTree.add(CGeorgesEditDocSub::Type, "Type", 0, "", 0);
}

void GeorgesEditorDoc::buildDfnTree()
{
	_docTree.create(CGeorgesEditDocSub::Null, getFileName().toUtf8().constData(), 0, "", 0);
	_docTree.add(CGeorgesEditDocSub::Header, "Header", 0, "", 0);
	_docTree.add(CGeorgesEditDocSub::Dfn, "DFN", 0, "", 0);
}

void GeorgesEditorDoc::buildFormTree()
{
	_docTree.create(CGeorgesEditDocSub::Null, getFileName().toUtf8().constData(), 0, "", 0);
	_docTree.add(CGeorgesEditDocSub::Header, "Header", 0, "", 0);

	if (_form)
	{
		_docTree.add(CGeorgesEditDocSub::Form, "Content", 0, "", 0);
	}
}

bool GeorgesEditorDoc::canUndo() const
{
	// Stub: undo system not yet implemented
	return false;
}

bool GeorgesEditorDoc::canRedo() const
{
	// Stub: redo system not yet implemented
	return false;
}

void GeorgesEditorDoc::undo()
{
	// Stub
}

void GeorgesEditorDoc::redo()
{
	// Stub
}
