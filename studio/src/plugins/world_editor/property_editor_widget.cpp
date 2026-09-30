// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011-2012  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

// Project includes
#include "property_editor_widget.h"
#include "world_editor_misc.h"
#include "world_editor_constants.h"

// NeL includes
#include <nel/misc/debug.h>

// STL includes
#include <vector>
#include <string>

// Qt includes
#include <QtCore/QModelIndex>
#include <QApplication>
#include <QClipboard>
#include <QMap>
#include <QMenu>
#include <QTreeWidget>

#include "const_string_array_property.h"

namespace WorldEditor
{

struct PropertyEditorWidgetPrivate
{
	/// One row can stand for several primitives at once - editing it has to reach every
	/// one of them, which is why this is a list and not a single pointer.
	QMap< QtProperty*, QList< NLLIGO::IPrimitive* > > propToPrims;

	void clearPrimitives()
	{
		propToPrims.clear();
	}

	void addPrimitive( QtProperty *p, NLLIGO::IPrimitive *prim )
	{
		QList< NLLIGO::IPrimitive* > &prims = propToPrims[ p ];
		if( !prims.contains( prim ) )
			prims.append( prim );
	}

	QList< NLLIGO::IPrimitive* > getPrimitives( QtProperty *p )
	{
		return propToPrims.value( p );
	}
};

PropertyEditorWidget::PropertyEditorWidget(QWidget *parent)
	: QWidget(parent)
{
	m_ui.setupUi(this);

	m_stringManager = new QtStringPropertyManager(this);
	m_boolManager = new QtBoolPropertyManager(this);
	m_enumManager = new QtEnumPropertyManager(this);
	m_stringArrayManager = new QtTextPropertyManager(this);

	m_constStrArrPropMgr = new ConstStrArrPropMgr(this);
	m_constStrArrEditorFactory = new ConstStrArrEditorFactory(this);

	QtLineEditFactory *lineEditFactory = new QtLineEditFactory(this);
	QtCheckBoxFactory *boolFactory = new QtCheckBoxFactory(this);
	QtEnumEditorFactory *enumFactory = new QtEnumEditorFactory(this);
	// The default one line editor is no way to work on a block of script; this one shows
	// a summary and opens a proper window.
	m_scriptEditorFactory = new ScriptEditorFactory(this);

	m_ui.treePropertyBrowser->setFactoryForManager(m_stringManager, lineEditFactory);
	m_ui.treePropertyBrowser->setFactoryForManager(m_boolManager, boolFactory);
	m_ui.treePropertyBrowser->setFactoryForManager(m_enumManager, enumFactory);
	m_ui.treePropertyBrowser->setFactoryForManager(m_stringArrayManager, m_scriptEditorFactory);
	m_ui.treePropertyBrowser->setFactoryForManager(m_constStrArrPropMgr, m_constStrArrEditorFactory);

	m_groupManager = new QtGroupPropertyManager(this);

	// The browser is readable enough on its own, it was just never set up: no header, no
	// room for the values, every row the same colour.
	m_ui.treePropertyBrowser->setAlternatingRowColors(true);
	m_ui.treePropertyBrowser->setHeaderVisible(true);
	m_ui.treePropertyBrowser->setRootIsDecorated(false);
	m_ui.treePropertyBrowser->setPropertiesWithoutValueMarked(true);
	m_ui.treePropertyBrowser->setResizeMode(QtTreePropertyBrowser::Interactive);
	m_ui.treePropertyBrowser->setSplitterPosition(180);

	// Values that cannot be selected cannot be copied either, and a property editor whose
	// values you have to retype by hand is not much of one. The browser keeps its tree
	// private, so reach it once and hang a context menu and Ctrl+C on it.
	m_browserTree = m_ui.treePropertyBrowser->findChild<QTreeWidget *>();
	if (m_browserTree != 0)
	{
		m_browserTree->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_browserTree, SIGNAL(customContextMenuRequested(QPoint)),
				this, SLOT(showContextMenu(QPoint)));

		QAction *copyAction = new QAction(tr("Copy Value"), m_browserTree);
		copyAction->setShortcut(QKeySequence::Copy);
		copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		connect(copyAction, SIGNAL(triggered()), this, SLOT(copyValue()));
		m_browserTree->addAction(copyAction);
	}
	else
	{
		nlwarning("World Editor: no tree inside the property browser, "
				  "copying values will not work.");
	}

	d_ptr = new PropertyEditorWidgetPrivate();

	connect(m_stringManager, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	connect(m_boolManager, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	connect(m_enumManager, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	connect(m_stringArrayManager, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	connect(m_constStrArrPropMgr, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));

	connect(m_boolManager, SIGNAL(resetProperty(QtProperty *)), this, SLOT(resetProperty(QtProperty *)));
	connect(m_stringManager, SIGNAL(resetProperty(QtProperty *)), this, SLOT(resetProperty(QtProperty *)));
	connect(m_enumManager, SIGNAL(resetProperty(QtProperty *)), this, SLOT(resetProperty(QtProperty *)));
	connect(m_stringArrayManager, SIGNAL(resetProperty(QtProperty *)), this, SLOT(resetProperty(QtProperty *)));
}

PropertyEditorWidget::~PropertyEditorWidget()
{
	delete d_ptr;
	d_ptr = NULL;
}

void PropertyEditorWidget::clearProperties()
{
	d_ptr->clearPrimitives();
	m_ui.treePropertyBrowser->clear();
}

void PropertyEditorWidget::addFileProperties(Node *node)
{
	// The tree shows file names only - the paths are far too long for it - so the full
	// path is offered here, where there is room for it, and as a tooltip in the tree.
	QString fileName;
	switch (node->type())
	{
	case Node::WorldEditNodeType:
		fileName = static_cast<WorldEditNode *>(node)->fileName();
		break;
	case Node::LandscapeNodeType:
		fileName = static_cast<LandscapeNode *>(node)->fileName();
		break;
	case Node::RootPrimitiveNodeType:
		fileName = static_cast<RootPrimitiveNode *>(node)->fileName();
		break;
	default:
		return;
	}

	if (fileName.isEmpty())
		return;

	QtProperty *fileGroup = m_groupManager->addProperty(
			node->data(Qt::DisplayRole).toString());
	m_ui.treePropertyBrowser->addProperty(fileGroup);

	QtProperty *pathProperty = m_stringManager->addProperty(tr("path"));
	m_stringManager->setValue(pathProperty, fileName);
	pathProperty->setEnabled(false);
	fileGroup->addSubProperty(pathProperty);

	m_ui.treePropertyBrowser->setExpanded(
			m_ui.treePropertyBrowser->topLevelItem(fileGroup), true);
}

std::list<NLLIGO::CPrimitiveClass::CParameter> PropertyEditorWidget::commonParameters(
		const QList<const NLLIGO::IPrimitive *> &primitives,
		const NLLIGO::CPrimitiveClass *primitiveClass)
{
	std::list<NLLIGO::CPrimitiveClass::CParameter> parameterList;
	const NLLIGO::IPrimitive *primitive = primitives.first();

	// Use the class or not ?
	if (primitiveClass)
	{
		for (uint p = 0; p < primitiveClass->Parameters.size(); p++)
		{
			if (primitiveClass->Parameters[p].Visible)
			{
				if (primitiveClass->Parameters[p].Name == "name")
					parameterList.push_front(primitiveClass->Parameters[p]);
				else
					parameterList.push_back(primitiveClass->Parameters[p]);
			}
		}
	}
	else
	{
		uint numProp = primitive->getNumProperty();
		for (uint p = 0; p < numProp; p++)
		{
			std::string propertyName;
			const NLLIGO::IProperty *prop;
			nlverify(primitive->getProperty(p, propertyName, prop));

			NLLIGO::CPrimitiveClass::CParameter defProp(*prop, propertyName.c_str());

			if (defProp.Name == "name")
				parameterList.push_front(defProp);
			else
				parameterList.push_back(defProp);
		}
	}

	// "class" is what picked the parameter list in the first place, not a row of its own.
	std::list<NLLIGO::CPrimitiveClass::CParameter>::iterator ite = parameterList.begin();
	while (ite != parameterList.end())
	{
		std::list<NLLIGO::CPrimitiveClass::CParameter>::iterator next = ite;
		++next;

		if (ite->Name == "class")
			parameterList.erase(ite);

		ite = next;
	}

	return parameterList;
}

void PropertyEditorWidget::updateSelection(Node *node)
{
	clearProperties();

	if (node == 0)
		return;

	addFileProperties(node);

	if (node->type() != Node::PrimitiveNodeType)
		return;

	blockSignalsOfProperties(true);

	PrimitiveNode *primitiveNode = static_cast<PrimitiveNode *>(node);
	const NLLIGO::IPrimitive *primitive = primitiveNode->primitive();
	const NLLIGO::CPrimitiveClass *primClass = primitiveNode->primitiveClass();

	QList<const NLLIGO::IPrimitive *> primitives;
	primitives.append(primitive);

	std::list<NLLIGO::CPrimitiveClass::CParameter> parameterList =
			commonParameters(primitives, primClass);

	// primClass may be null here - commonParameters() catches that explicitly and builds
	// the parameters from the raw properties instead. Without this check, studio crashes
	// on clicking a primitive whose class is not listed in world_editor_classes.xml.
	const QString className = (primClass != 0)
			? QString::fromUtf8(primClass->Name.c_str())
			: tr("unknown class");

	const QString title =
			QString("%1(%2)").arg(node->data(Qt::DisplayRole).toString()).arg(className);

	QtProperty *groupNode = m_groupManager->addProperty(title);
	m_ui.treePropertyBrowser->addProperty(groupNode);

	std::list<NLLIGO::CPrimitiveClass::CParameter>::iterator ite = parameterList.begin();
	while (ite != parameterList.end())
	{
		NLLIGO::CPrimitiveClass::CParameter &parameter = (*ite);
		QtProperty *prop;
		NLLIGO::IProperty *ligoProperty = 0;
		primitive->getPropertyByName(parameter.Name.c_str(), ligoProperty);

		if (parameter.Type == NLLIGO::CPrimitiveClass::CParameter::ConstString)
			prop = addConstStringProperty(ligoProperty, parameter, primitive);
		else if (parameter.Type == NLLIGO::CPrimitiveClass::CParameter::String)
			prop = addStringProperty(ligoProperty, parameter, primitive);
		else if (parameter.Type == NLLIGO::CPrimitiveClass::CParameter::StringArray)
			prop = addStringArrayProperty(ligoProperty, parameter, primitive);
		else if (parameter.Type == NLLIGO::CPrimitiveClass::CParameter::ConstStringArray)
			prop = addConstStringArrayProperty(ligoProperty, parameter, primitive);
		else
			prop = addBoolProperty(ligoProperty, parameter, primitive);

		d_ptr->addPrimitive(prop, const_cast<NLLIGO::IPrimitive *>(primitive));

		// Default value ?
		if	((ligoProperty == NULL)	|| (ligoProperty->Default))
			prop->setModified(false);
		else
			prop->setModified(true);

		bool staticChildSelected = Utils::ligoConfig()->isStaticChild(*primitive);
		if (parameter.ReadOnly || (staticChildSelected && (parameter.Name == "name")))
			prop->setEnabled(false);

		groupNode->addSubProperty(prop);

		++ite;
	}

	m_ui.treePropertyBrowser->setExpanded(
			m_ui.treePropertyBrowser->topLevelItem(groupNode), true);

	blockSignalsOfProperties(false);
}

QString PropertyEditorWidget::currentRowText(int column) const
{
	if (m_browserTree == 0)
		return QString();

	const QTreeWidgetItem *item = m_browserTree->currentItem();
	if (item == 0)
		return QString();

	return item->text(column);
}

void PropertyEditorWidget::copyValue()
{
	const QString value = currentRowText(1);
	if (!value.isEmpty())
		QApplication::clipboard()->setText(value);
}

void PropertyEditorWidget::copyNameAndValue()
{
	const QString name = currentRowText(0);
	if (name.isEmpty())
		return;

	QApplication::clipboard()->setText(name + QLatin1String(" = ") + currentRowText(1));
}

void PropertyEditorWidget::showContextMenu(const QPoint &pos)
{
	if ((m_browserTree == 0) || (m_browserTree->itemAt(pos) == 0))
		return;

	QMenu menu(this);
	menu.addAction(tr("Copy Value"), this, SLOT(copyValue()), QKeySequence::Copy);
	menu.addAction(tr("Copy Name and Value"), this, SLOT(copyNameAndValue()));
	menu.exec(m_browserTree->viewport()->mapToGlobal(pos));
}

void PropertyEditorWidget::propertyChanged(QtProperty *p)
{
	nlinfo(QString("property %1 changed").arg(p->propertyName()).toUtf8().constData());
}

void PropertyEditorWidget::resetProperty(QtProperty *property)
{
	nlinfo(QString("property %1 reset").arg(property->propertyName()).toUtf8().constData());
}

NLLIGO::IProperty* PropertyEditorWidget::getLigoProperty( QtProperty *p )
{
	const QList< NLLIGO::IPrimitive* > prims = d_ptr->getPrimitives( p );
	if( prims.isEmpty() )
		return NULL;

	NLLIGO::IProperty *prop = NULL;
	prims.first()->getPropertyByName( p->propertyName().toUtf8().constData(), prop );

	return prop;
}

namespace
{
/// Write a single string into one primitive's property.
void setStringValue( NLLIGO::IPrimitive *primitive, const QString &name, const QString &value )
{
	NLLIGO::IProperty *prop = NULL;
	if( !primitive->getPropertyByName( name.toUtf8().constData(), prop ) || ( prop == NULL ) )
		return;

	NLLIGO::CPropertyString *pp = dynamic_cast< NLLIGO::CPropertyString* >( prop );
	if( pp == NULL )
		return;

	pp->String = value.toUtf8().constData();
	pp->Default = false;
}

/// Write a block of lines into one primitive's string array property.
void setStringArrayValue( NLLIGO::IPrimitive *primitive, const QString &name, const QString &value )
{
	NLLIGO::IProperty *prop = NULL;
	if( !primitive->getPropertyByName( name.toUtf8().constData(), prop ) || ( prop == NULL ) )
		return;

	NLLIGO::CPropertyStringArray *pp = dynamic_cast< NLLIGO::CPropertyStringArray* >( prop );
	if( pp == NULL )
		return;

	pp->StringArray.clear();

	const QStringList lines = value.split( QLatin1Char( '\n' ) );
	Q_FOREACH( const QString &line, lines )
		pp->StringArray.push_back( line.toUtf8().constData() );

	pp->Default = false;
}
}

void PropertyEditorWidget::onBoolValueChanged( QtProperty *p, bool v )
{
	const QString value = v ? QLatin1String( "true" ) : QLatin1String( "false" );
	Q_FOREACH( NLLIGO::IPrimitive *primitive, d_ptr->getPrimitives( p ) )
		setStringValue( primitive, p->propertyName(), value );
}

void PropertyEditorWidget::onStringValueChanged( QtProperty *p, const QString &v )
{
	// The marker is what the row shows while the selected primitives disagree. Writing it
	// back would set them all to the literal text "<different values>".
	if( v == QLatin1String( Constants::DIFFERENT_VALUE_STRING ) )
		return;

	Q_FOREACH( NLLIGO::IPrimitive *primitive, d_ptr->getPrimitives( p ) )
		setStringValue( primitive, p->propertyName(), v );
}

void PropertyEditorWidget::onEnumValueChanged( QtProperty *p, int v )
{
	Q_UNUSED( v );

	const QString value = p->valueText();
	if( value == QLatin1String( Constants::DIFFERENT_VALUE_STRING ) )
		return;

	Q_FOREACH( NLLIGO::IPrimitive *primitive, d_ptr->getPrimitives( p ) )
		setStringValue( primitive, p->propertyName(), value );
}

void PropertyEditorWidget::onStrArrValueChanged( QtProperty *p, const QString &v )
{
	if( v == QLatin1String( Constants::DIFFERENT_VALUE_STRING ) )
		return;

	Q_FOREACH( NLLIGO::IPrimitive *primitive, d_ptr->getPrimitives( p ) )
		setStringArrayValue( primitive, p->propertyName(), v );
}

void PropertyEditorWidget::onConstStrArrValueChanged( QtProperty *p, const QString &v )
{
	if( v == QLatin1String( Constants::DIFFERENT_VALUE_STRING ) )
		return;

	Q_FOREACH( NLLIGO::IPrimitive *primitive, d_ptr->getPrimitives( p ) )
		setStringArrayValue( primitive, p->propertyName(), v );
}


QtProperty *PropertyEditorWidget::addBoolProperty(const NLLIGO::IProperty *property,
		const NLLIGO::CPrimitiveClass::CParameter &parameter,
		const NLLIGO::IPrimitive *primitive)
{
	std::string value;
	std::string name = parameter.Name.c_str();
	primitive->getPropertyByName(name.c_str(), value);
	QtProperty *prop = m_boolManager->addProperty(name.c_str());
	// if (Default)
	{
		//DialogProperties->setDefaultValue (this, value);
		m_boolManager->setValue(prop, bool((value=="true")?1:0));
	}
	return prop;
}

QtProperty *PropertyEditorWidget::addConstStringProperty(const NLLIGO::IProperty *property,
		const NLLIGO::CPrimitiveClass::CParameter &parameter,
		const NLLIGO::IPrimitive *primitive)
{
	std::string value;
	std::string name = parameter.Name.c_str();

	// Get current value
	primitive->getPropertyByName(name.c_str(), value);

	// Create qt property
	QtProperty *prop = m_enumManager->addProperty(parameter.Name.c_str());

	QStringList listEnums = getComboValues(parameter);

	if (listEnums.isEmpty())
	{
		listEnums << QString(value.c_str()) + tr(" (WRN: Check leveldesign!)");
		m_enumManager->setEnumNames(prop, listEnums);
		m_enumManager->setValue(prop, 0);
		prop->setEnabled(false);
	}
	else
	{
		// TODO: check this logic
		if (parameter.DefaultValue.empty() || (parameter.DefaultValue[0].Name.empty()))
			listEnums.prepend("");

		// Fill qt property
		m_enumManager->setEnumNames(prop, listEnums);

		// Find index of current value
		for (int i = 0; i < listEnums.size(); i++)
		{
			if (value == std::string(listEnums[i].toUtf8().constData()))
			{
				m_enumManager->setValue(prop, i);
				break;
			}
		}
	}

	return prop;
}

QtProperty *PropertyEditorWidget::addStringProperty(const NLLIGO::IProperty *property,
		const NLLIGO::CPrimitiveClass::CParameter &parameter,
		const NLLIGO::IPrimitive *primitive)
{
	std::string value;
	std::string name = parameter.Name.c_str();
	primitive->getPropertyByName(name.c_str(), value);
	QtProperty *prop = m_stringManager->addProperty(parameter.Name.c_str());
	m_stringManager->setValue(prop, QString(value.c_str()));
	return prop;
}

QtProperty *PropertyEditorWidget::addStringArrayProperty(const NLLIGO::IProperty *property,
		const NLLIGO::CPrimitiveClass::CParameter &parameter,
		const NLLIGO::IPrimitive *primitive)
{
	std::string name = parameter.Name.c_str();
	QtProperty *prop = m_stringArrayManager->addProperty(parameter.Name.c_str());

	const NLLIGO::IProperty	*ligoProperty;
	std::vector<std::string> vectString;

	if	(primitive->getPropertyByName(parameter.Name.c_str (), ligoProperty))
	{
		const NLLIGO::CPropertyStringArray *const propStringArray = dynamic_cast<const NLLIGO::CPropertyStringArray *> (ligoProperty);
		if (propStringArray)
		{
			const std::vector<std::string> &vectString = propStringArray->StringArray;
			if (!vectString.empty())
			{
				std::string temp;
				for (size_t i = 0; i < vectString.size(); i++)
				{
					temp += vectString[i];
					if (i != (vectString.size() - 1))
						temp += '\n';
				}
				m_stringArrayManager->setValue(prop, temp.c_str());
				prop->setToolTip(temp.c_str());
			}
		}
		else
		{
			m_stringArrayManager->setValue(prop, "StringArray :(");
		}
	}

	// Create an "EDIT" button if the text is editable (FileExtension != "")
	if (parameter.FileExtension != "")
	{	
		// Create an edit box
		// TODO:
	}
	return prop;
}

QtProperty *PropertyEditorWidget::addConstStringArrayProperty(const NLLIGO::IProperty *property,
		const NLLIGO::CPrimitiveClass::CParameter &parameter,
		const NLLIGO::IPrimitive *primitive)
{
	std::string value;
	std::string name = parameter.Name.c_str();

	// Get current value
	primitive->getPropertyByName(name.c_str(), value);

	// Create qt property
	QtProperty *prop = m_constStrArrPropMgr->addProperty(parameter.Name.c_str());

	QStringList listEnums = getComboValues(parameter);

	if (listEnums.isEmpty())
	{
		prop->setEnabled(false);
	}
	else
	{
		// Fill qt property
		m_constStrArrPropMgr->setStrings(prop, listEnums);

		const NLLIGO::IProperty	*ligoProperty;
		std::vector<std::string> vectString;

		if	(primitive->getPropertyByName (parameter.Name.c_str(), ligoProperty))
		{
			const NLLIGO::CPropertyStringArray *const propStringArray = dynamic_cast<const NLLIGO::CPropertyStringArray *> (ligoProperty);
			if (propStringArray)
			{
				const std::vector<std::string> &vectString = propStringArray->StringArray;
				if (!vectString.empty())
				{
					std::string temp;
					for (size_t i = 0; i < vectString.size(); i++)
					{
						temp += vectString[i];
						if (i != (vectString.size() - 1))
							temp += '\n';
					}
					m_constStrArrPropMgr->setValue(prop, temp.c_str());
					prop->setToolTip(temp.c_str());
				}
			}
			else
			{
				m_constStrArrPropMgr->setValue(prop, "StringArray :(");
			}
		}

	}

	return prop;
}

QStringList PropertyEditorWidget::getComboValues(const NLLIGO::CPrimitiveClass::CParameter &parameter)
{
	// TODO: get context value from dialog
	std::string context("jungle");
	std::string defaultContext("default");

	std::vector<std::string> listContext;
	
	if (context != defaultContext)
		listContext.push_back(context);
	listContext.push_back(defaultContext);

	QStringList listEnums;

	// Correct fill properties with *both* contexts if the current context is not default and is valid.
	for (size_t j = 0; j < listContext.size(); j++)
	{
		std::map<std::string, NLLIGO::CPrimitiveClass::CParameter::CConstStringValue>::const_iterator ite = parameter.ComboValues.find(listContext[j].c_str());

		if (ite != parameter.ComboValues.end())
		{
			std::vector<std::string> pathList;

			// Fill pathList
			ite->second.appendFilePath(pathList);

			if (parameter.SortEntries)
				std::sort(pathList.begin(), pathList.end());

			for (size_t i = 0; i < pathList.size(); ++i)
				listEnums.append(pathList[i].c_str());
		}
	}

	return listEnums;
}

void PropertyEditorWidget::blockSignalsOfProperties(bool block)
{
	m_stringManager->blockSignals(block);
	m_boolManager->blockSignals(block);
	m_enumManager->blockSignals(block);
	m_stringArrayManager->blockSignals(block);
	
	if( block )
	{
		disconnect(m_constStrArrPropMgr, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));

		disconnect(m_boolManager, SIGNAL( valueChanged( QtProperty*, bool ) ),
			this, SLOT( onBoolValueChanged( QtProperty*, bool ) ) );
		
		disconnect(m_stringManager, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onStringValueChanged( QtProperty*, const QString& ) ) );

		disconnect(m_enumManager, SIGNAL( valueChanged( QtProperty*, int ) ),
			this, SLOT( onEnumValueChanged( QtProperty*, int ) ) );

		disconnect(m_stringArrayManager, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onStrArrValueChanged( QtProperty*, const QString& ) ) );

		disconnect(m_constStrArrPropMgr, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onConstStrArrValueChanged( QtProperty*, const QString& ) ) );
	}
	else
	{
		connect(m_constStrArrPropMgr, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));

		connect(m_boolManager, SIGNAL( valueChanged( QtProperty*, bool ) ),
			this, SLOT( onBoolValueChanged( QtProperty*, bool ) ) );

		connect(m_stringManager, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onStringValueChanged( QtProperty*, const QString& ) ) );

		connect(m_enumManager, SIGNAL( valueChanged( QtProperty*, int ) ),
			this, SLOT( onEnumValueChanged( QtProperty*, int ) ) );

		connect(m_stringArrayManager, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onStrArrValueChanged( QtProperty*, const QString& ) ) );

		connect(m_constStrArrPropMgr, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onConstStrArrValueChanged( QtProperty*, const QString& ) ) );
	}
}
} /* namespace WorldEditor */
