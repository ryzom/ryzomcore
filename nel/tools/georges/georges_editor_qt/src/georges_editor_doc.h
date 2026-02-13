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

#ifndef GEORGES_EDITOR_DOC_H
#define GEORGES_EDITOR_DOC_H

#include <string>
#include <vector>

#include <QString>

#include <nel/misc/types_nl.h>
#include <nel/misc/smart_ptr.h>
#include <nel/georges/type.h>
#include <nel/georges/form.h>
#include <nel/georges/form_dfn.h>
#include <nel/georges/form_loader.h>

class GeorgesEditorDoc;

/**
 * Sub object in the left view tree, ported from CGeorgesEditDocSub.
 */
class CGeorgesEditDocSub
{
public:
	enum TSub
	{
		Null,
		Header,
		Type,
		Dfn,
		Form,
	};

	CGeorgesEditDocSub();
	~CGeorgesEditDocSub();

	bool isEditable() const;
	TSub getType() const;
	uint getIdInParent() const;
	const std::string &getName() const;
	const std::string &getFormName() const;
	uint getChildrenCount() const;
	void removeChildren(uint child);
	CGeorgesEditDocSub *getChild(uint child);
	CGeorgesEditDocSub *getParent();
	uint getSlot() const;

	void create(TSub type, const char *name, uint structId, const char *formName, uint slot);
	CGeorgesEditDocSub *add(TSub type, const char *name, uint structId, const char *formName, uint slot);
	void clean();

	int getItemImage(GeorgesEditorDoc *doc) const;

private:
	uint _StructId;
	std::string _Name;
	std::string _FormName;
	TSub _Type;
	std::vector<CGeorgesEditDocSub *> _Children;
	CGeorgesEditDocSub *_Parent;
	uint _Slot;
};

/**
 * Document class managing one open Georges file, ported from CGeorgesEditDoc.
 */
class GeorgesEditorDoc
{
public:
	enum TDocType
	{
		TypeDoc,
		DfnDoc,
		FormDoc,
	};

	GeorgesEditorDoc();
	~GeorgesEditorDoc();

	bool open(const QString &path);
	bool save(const QString &path = QString());
	bool isModified() const;
	void setModified(bool modified);

	TDocType getDocType() const { return _docType; }
	bool isType() const { return _docType == TypeDoc; }
	bool isDfn() const { return _docType == DfnDoc; }
	bool isForm() const { return _docType == FormDoc; }

	NLGEORGES::CType *getTypePtr() { return _type; }
	NLGEORGES::CFormDfn *getDfnPtr() { return _dfn; }
	NLGEORGES::CForm *getFormPtr() { return dynamic_cast<NLGEORGES::CForm *>((NLGEORGES::UForm *)_form); }
	const NLGEORGES::CFileHeader *getHeaderPtr() const;
	NLGEORGES::CFileHeader *getHeaderPtr();

	QString getFilePath() const { return _filePath; }
	QString getFileName() const;

	CGeorgesEditDocSub &getDocTree() { return _docTree; }

	void updateDocumentStructure();

	// Undo/redo
	bool canUndo() const;
	bool canRedo() const;
	void undo();
	void redo();

private:
	void buildTypeTree();
	void buildDfnTree();
	void buildFormTree();

	TDocType _docType;
	bool _modified;
	QString _filePath;

	NLMISC::CSmartPtr<NLGEORGES::CType> _type;
	NLMISC::CSmartPtr<NLGEORGES::CFormDfn> _dfn;
	NLMISC::CSmartPtr<NLGEORGES::UForm> _form;

	NLGEORGES::CFormLoader _loader;

	CGeorgesEditDocSub _docTree;
};

#endif // GEORGES_EDITOR_DOC_H
