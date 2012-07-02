// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// graph_plugin.cpp: implementation of the CGraphPlugin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "nel/misc/types_nl.h"
#include "nel/misc/bitmap.h"
#include "graph_plugin.h"
#include <string>
#include <list>
#include "nel/ligo/primitive_utils.h"
#include "../../../leveldesign/mission_compiler_lib/mission_compiler.h"
#include "nel/misc/i18n.h"
#include <stdlib.h>
#include <stdio.h>

using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;
using namespace std;

class CDatabaseLocator;

CFileDisplayer		*GraphPluginLogDisplayer= NULL;

// vl: important to add the next line or AfxGetApp() will returns 0 after AFX_MANAGE_STATE(AfxGetStaticModuleState());
CWinApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern "C"
{
	void *createPlugin()
	{
		return new CGraphPlugin();
	}
}

CGraphPlugin::CGraphPlugin()
{
	NLMISC::createDebug();

	GraphPluginLogDisplayer= new CFileDisplayer("world_editor_graph_plugin.log", true, "WORLD_EDITOR_GRAPH_PLUGIN.LOG");
	DebugLog->addDisplayer (GraphPluginLogDisplayer);
	InfoLog->addDisplayer (GraphPluginLogDisplayer);
	WarningLog->addDisplayer (GraphPluginLogDisplayer);
	ErrorLog->addDisplayer (GraphPluginLogDisplayer);
	AssertLog->addDisplayer (GraphPluginLogDisplayer);

	nlinfo("Starting graph plugin...");
	GraphDlg=new CWorldEditorGraphPluginDlg();
}

CGraphPlugin::~CGraphPlugin()
{
	delete [] GraphDlg;
}

void CGraphPlugin::init(IPluginAccess *pluginAccess)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	AfxEnableControlContainer();

	_PluginAccess = pluginAccess;

	_Name="mission_graph";

	GraphDlg->Create(IDD_WORLDEDITORGRAPHPLUGIN_DIALOG, CWnd::FromHandle(_PluginAccess->getMainWindow()->m_hWnd));
	GraphDlg->ShowWindow(TRUE);
	GraphDlg->init(this);
	_PluginActive=true;

	// Retreive the dot exe path
	CConfigFile::CVar *dotPath = pluginAccess->getConfigFile().getVarPtr("DotPath");
	nlinfo("%s", dotPath->asString().c_str());
	if (dotPath)
	{
		_DotPath = CPath::getFullPath(dotPath->asString(), false);
		nlinfo("%s", _DotPath.c_str());
	}
	else
	{
		// use default configuration
		nlwarning("graph_plugin : Can't find configuration variable 'DotPath', using 'dot/dot.exe' instead");
		_DotPath = CPath::getFullPath("dot/dot.exe", false);
	}
}


/// The current region has changed.
//void CSoundPlugin::primRegionChanged(const std::vector<NLLIGO::CPrimRegion*> &regions)
void CGraphPlugin::primitiveChanged(const NLLIGO::IPrimitive *root)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}


void CGraphPlugin::positionMoved(const NLMISC::CVector &position)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}

void CGraphPlugin::lostPositionControl()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}


void CGraphPlugin::onIdle()
{
}

void CGraphPlugin::refreshPrimitives()
{
	// get the current selection
	const list<IPrimitive*> &selection = _PluginAccess->getCurrentSelection();

	if (selection.empty())
		return;

	TPrimitiveClassPredicate pred("mission_tree");
	// for each selected node, look for an interesting class
	list<IPrimitive*>::const_iterator first(selection.begin()), last(selection.end()),curIte;
	IPrimitive*	missionTreeRoot=(*first);
	CPrimitiveSet<TPrimitiveClassPredicate> offsprings;
	TPrimitiveSet resltOffspring;
	offsprings.buildSet(missionTreeRoot,pred,resltOffspring);

	vector<IPrimitive*> vectSel;//vector of faulty mission element (sent by the mission compiler)

	if ((pred(missionTreeRoot))||
		((missionTreeRoot=getPrimitiveParent((*first),pred))!=NULL)||
		((!resltOffspring.empty())&&(missionTreeRoot=*resltOffspring.begin())))
	{
		// ok, we find a good node
		try
		{
			

			IPrimitive *rootNode = missionTreeRoot;
			while (rootNode->getParent())
				rootNode = rootNode->getParent();
			string fileName = _PluginAccess->getRootFileName(rootNode);
			
			CMissionCompiler mc;
			mc.compileMission(missionTreeRoot, NLMISC::CFile::getFilename(fileName));
			TMissionDataPtr md = mc.getMission(0);
			string dot = md->generateDotScript();

			string tmpPath = string(::getenv("TEMP"));
			FILE *fp = fopen ((tmpPath+"/lang.dot").c_str(), "wt");
			if (!fp)
			{
				string msg("Can't write script file '");
				msg += tmpPath+"/lang.dot'";
				AfxMessageBox(msg.c_str());
			}
			else
			{
				fwrite(dot.data(), dot.size(), 1, fp);
				fclose(fp);
//				NLMISC::CI18N::writeTextFile(tmpPath+"/lang.dot", dot, true);
				NLMISC::CFile::deleteFile(tmpPath+"/output.png");
				NLMISC::CFile::deleteFile(tmpPath+"/output.imap");
				system((_DotPath+" "+tmpPath+"/lang.dot -Tpng -o "+tmpPath+"/output.png").c_str());
				system((_DotPath+" "+tmpPath+"/lang.dot -Timap -o "+tmpPath+"/output.imap").c_str());

				//currently using output.png as the input file...
				if(!createBitmap(tmpPath))
					AfxMessageBox("BEWARE: the image couldn't be loaded.");
			}

			while (missionTreeRoot->getParent()!=NULL) 
			{
				missionTreeRoot=missionTreeRoot->getParent();
			}
			_rootFileName=_PluginAccess->getRootFileName(missionTreeRoot);
		}
		catch(EParseException &e)
		{
			string err = e.Why;
			if (e.Primitive != NULL)
			{
				string primName;
				
				vectSel.push_back(e.Primitive);
				e.Primitive->getPropertyByName("name", primName);
				
				err = toString("%s : %s", primName.c_str(), e.Why.c_str());
			}
			AfxMessageBox(err.c_str());
		}
		catch (exception e) //catch a possible exception from getRootFileName
		{
			AfxMessageBox(e.what());
		}
	}
	else
	{
		refreshMachine();		
	}

	if(vectSel.size()>0)
		_PluginAccess->setCurrentSelection(vectSel);
	GraphDlg->Invalidate();
}

void CGraphPlugin::refreshMachine()
{
	// get the current selection
	const list<IPrimitive*> &selection = _PluginAccess->getCurrentSelection();

	if (selection.empty())
		return;

	TPrimitiveClassPredicate predFolder("npc_folder");
	TPrimitiveClassPredicate predManager("npc_manager");
	// for each selected node, look for an interesting class
	list<IPrimitive*>::const_iterator first(selection.begin()), last(selection.end()),curIte;
	IPrimitive*	missionTreeRoot=(*first);
	CPrimitiveSet<TPrimitiveClassPredicate> offsprings;
	TPrimitiveSet resltOffspring;
	offsprings.buildSet(missionTreeRoot,predFolder,resltOffspring);

	if ((predFolder(missionTreeRoot))||
		((missionTreeRoot=getPrimitiveParent((*first),predFolder))!=NULL))
	{
		
	
		string dot = generateDotScript(missionTreeRoot);

		string tmpPath = string(::getenv("TEMP"));
		FILE *fp = fopen ((tmpPath+"/lang.dot").c_str(), "wt");
		if (!fp)
		{
			string msg("Can't write script file '");
			msg += tmpPath+"/lang.dot'";
			AfxMessageBox(msg.c_str());
		}
		else
		{
			fwrite(dot.data(), dot.size(), 1, fp);
			fclose(fp);
	//				NLMISC::CI18N::writeTextFile(tmpPath+"/lang.dot", dot, true);
			NLMISC::CFile::deleteFile(tmpPath+"/output.png");
			NLMISC::CFile::deleteFile(tmpPath+"/output.imap");
			system((_DotPath+" "+tmpPath+"/lang.dot -Tpng -o "+tmpPath+"/output.png").c_str());
			system((_DotPath+" "+tmpPath+"/lang.dot -Timap -o "+tmpPath+"/output.imap").c_str());

			//currently using output.png as the input file...
			if(!createBitmap(tmpPath))
				AfxMessageBox("BEWARE: the image couldn't be loaded.");
		}
		
			while (missionTreeRoot->getParent()!=NULL) 
			{
				missionTreeRoot=missionTreeRoot->getParent();
			}
			_rootFileName=_PluginAccess->getRootFileName(missionTreeRoot);

		

	}
	else 
	{
		list<IPrimitive*>::const_iterator first(selection.begin()), last(selection.end()),curIte;
		IPrimitive*	missionTreeRoot=(*first);
		offsprings.buildSet(missionTreeRoot,predManager,resltOffspring);
		if ((predManager(missionTreeRoot))||
		((missionTreeRoot=getPrimitiveParent((*first),predManager))!=NULL))
		{
			
		
			string dot = generateDotScript(missionTreeRoot);

			string tmpPath = string(::getenv("TEMP"));
			FILE *fp = fopen ((tmpPath+"/lang.dot").c_str(), "wt");
			if (!fp)
			{
				string msg("Can't write script file '");
				msg += tmpPath+"/lang.dot'";
				AfxMessageBox(msg.c_str());
			}
			else
			{
				fwrite(dot.data(), dot.size(), 1, fp);
				fclose(fp);
//				NLMISC::CI18N::writeTextFile(tmpPath+"/lang.dot", dot, true);
				NLMISC::CFile::deleteFile(tmpPath+"/output.png");
				NLMISC::CFile::deleteFile(tmpPath+"/output.imap");
				system((_DotPath+" "+tmpPath+"/lang.dot -Tpng -o "+tmpPath+"/output.png").c_str());
				system((_DotPath+" "+tmpPath+"/lang.dot -Timap -o "+tmpPath+"/output.imap").c_str());

				//currently using output.png as the input file...
				if(!createBitmap(tmpPath))
					AfxMessageBox("BEWARE: the image couldn't be loaded.");
			}	

			while (missionTreeRoot->getParent()!=NULL) 
			{
				missionTreeRoot=missionTreeRoot->getParent();
			}
			_rootFileName=_PluginAccess->getRootFileName(missionTreeRoot);
		}
		else
		{
			AfxMessageBox("The selected node could not be processed.");
		}

	}
	


	
}

//////////////////////////////////////////////////////////////////////////

string CGraphPlugin::spaceTo_(string strInput)
{
	string ret;
	vector<string>	strTmp;
	explode(strInput,string(" "),strTmp);
	if (strTmp.size()>1)
	{
	
		for(uint i=0;i<strTmp.size();i++)
		{
			vector<string>	strTmpTmp;
			explode(strTmp[i],string(":"),strTmpTmp);
			for(uint j=0;j<strTmpTmp.size();j++)
			{
				ret += strTmpTmp[j].c_str();
				ret +="_";
			}
		}
		return ret;
	}
	return strInput;
}

//////////////////////////////////////////////////////////////////////////

string		CGraphPlugin::createNode(IPrimitive* managerNode,string strPredicate,uint& numClusters,string strShape)
{
	string ret;
	TPrimitiveClassPredicate predicate(strPredicate.c_str());
	TPrimitiveSet resSet;
	CPrimitiveSet<TPrimitiveClassPredicate> offsprings;
	offsprings.buildSet(managerNode,predicate,resSet);

	for (uint i=0; i<resSet.size(); ++i)
	{
		string strName;
		resSet[i]->getPropertyByName("name",strName);
		ret += "subgraph ";
		ret += "cluster_";
		char* res=new char[100];
		_itoa(i+numClusters,res,10) ;
		ret += res;
		delete []res;
		ret +="{";
		ret +=NL;
		ret += "label = " ;
		ret +='"';
		ret += spaceTo_(strName) ;
		ret +='"';
		ret +=";";
		ret += NL ;
		ret += spaceTo_(strName) ;
		ret += " [URL=\""+buildPrimPath(resSet[i])+"\"]";
		ret += "[shape=";
		ret += strShape.c_str();
		ret += "];" ;
		ret += NL ;
		ret	+= "}" ;
		ret	+= NL;
		
	}

	
	numClusters+=resSet.size();
	return ret;
}

//////////////////////////////////////////////////////////////////////////

string		CGraphPlugin::createParsedNode(IPrimitive* managerNode,string strPredicate,uint& numClusters,string strShape)
{
	string ret;
	TPrimitiveClassPredicate predicate(strPredicate.c_str());
	TPrimitiveSet resSet;
	CPrimitiveSet<TPrimitiveClassPredicate> offsprings;
	offsprings.buildSet(managerNode,predicate,resSet);

	for (uint i=0; i<resSet.size(); ++i)
	{
		string strName;
		resSet[i]->getPropertyByName("name",strName);
		ret += "subgraph ";
		ret += "cluster_";
		char* res=new char[100];
		_itoa(i+numClusters,res,10) ;
		ret += res;
		delete []res;
		ret +="{";
		ret +=NL;
		ret += "label = " ;
		ret +='"';
		ret += spaceTo_(strName) ;
		ret +='"';
		ret +=";";
		ret += NL ;
		ret += parseStateMachine(resSet[i],spaceTo_(strName),string(""),string("_")+spaceTo_(strName));
		ret	+= "}" ;
		ret	+= NL;
		
	}
	for (uint i=0; i<resSet.size(); ++i)
	{
		string strName;
		resSet[i]->getPropertyByName("name",strName);
		
		TPrimitiveClassPredicate predJump("npc_event_handler_action");
		TPrimitiveSet resJump;
		offsprings.buildSet(resSet[i],predJump,resJump);
		//if there is a change state action
		for(uint j=0;j<resJump.size();j++)
		{
			string strAction;
			string labelTail;
			string strTmpName;
			TPrimitiveClassPredicate predTail("npc_state_event_handler");
			IPrimitive* primTail=getPrimitiveParent(resJump[j],predTail);
			if(primTail)
				primTail->getPropertyByName("name", labelTail);
			if (labelTail.size()==0)
			{
				TPrimitiveClassPredicate predTail("npc_group_event_handler");
				primTail=getPrimitiveParent(resJump[j],predTail);
				if(primTail)
					primTail->getPropertyByName("name", labelTail);
			}
			
			vector<string> *jumpDest;
			resJump[j]->getPropertyByName("action",strAction);
			if (strAction.compare(string("begin_state"))==0)
			{
				ret += spaceTo_(strName) ;
				resJump[j]->getPropertyByName("parameters",jumpDest);
				ret += "->";
				ret +=spaceTo_(jumpDest->at(0));
				ret +="[label=";
				ret +=spaceTo_(labelTail);
				ret +="]"; 
				ret += ";" ;
				ret += NL;
			}
			/*if (strAction.compare(string("random_select_state"))==0)
			{
				vector<string>* jumpDest;
				string strTmpTmpName;
				
				resJump[j]->getPropertyByName("parameters",jumpDest);
				resJump[j]->getPropertyByName("name",strTmpTmpName);

				ret += spaceTo_(strName) ;
				ret +="->";
				ret +=strTmpTmpName;
				ret +="_";
				ret += spaceTo_(strName) ;
				ret += "[color=pink];";
				ret += NL;
				for(uint k=0;k<jumpDest->size();k++)
				{
					ret +=strTmpTmpName;
					ret +="_";
					ret += spaceTo_(strName) ;
					ret +="->";
					
					vector<string> resJump;
					explode(jumpDest->at(k)," ",resJump);
					ret +=resJump[1];
					ret+= ";";
					ret+=NL;
					
				}
			}*/
			if (strAction.compare(string("punctual_state"))==0)
			{
				ret += spaceTo_(strName) ;
				resJump[j]->getPropertyByName("parameters",jumpDest);
				ret += "->";
				ret +=spaceTo_(jumpDest->at(0));
				ret +="[label=";
				ret +=spaceTo_(labelTail);
				ret +="]"; 
				ret += ";" ;
				ret += NL;
				ret +=spaceTo_(jumpDest->at(0));
				ret += "->";
				ret += strName ;
				ret +="[label=";
				ret +=spaceTo_(labelTail);
				ret +="]"; 
				ret += ";" ;
				ret += NL;
			}
		}
		
	}
	numClusters+=resSet.size();
	return ret;
}

//////////////////////////////////////////////////////////////////////////


string CGraphPlugin::generateDotScript(IPrimitive* managerNode)
{
	string resName;
		managerNode->getPropertyByName("name",resName);
	string ret = "digraph " + spaceTo_(resName) + NL;
	ret += "{" + NL;

	uint numNode=0;

	ret+=createNode(managerNode,string("npc_zone"),numNode,string("Mrecord"));

	ret+=createNode(managerNode,string("npc_route"),numNode,string("Mrecord"));

	ret+=createNode(managerNode,string("npc_punctual_state"),numNode,string("Mrecord"));

	numNode=0;

	ret+=createParsedNode(managerNode,string("npc_zone"),numNode,string("Mrecord"));

	ret+=createParsedNode(managerNode,string("npc_route"),numNode,string("Mrecord"));

	ret+=createParsedNode(managerNode,string("npc_punctual_state"),numNode,string("Mrecord"));

	ret += "}" + NL;

	return ret;
}

string	CGraphPlugin::parseStateMachine(NLLIGO::IPrimitive* currentNode,string emiterNode,string tailLabel,string tag)
{
	string ret,strName,strClassName,strAction,strCurName,strCurClassName,strCurAction,strTail;
	currentNode->getPropertyByName("class",strCurClassName);
	currentNode->getPropertyByName("name",strCurName);
	currentNode->getPropertyByName("action",strCurAction);

	uint numChild=currentNode->getNumChildren();
	for(uint i=0;i<numChild;i++)
	{
		
		IPrimitive* child;
		currentNode->getChild(child,i);
		child->getPropertyByName("class",strClassName);
		child->getPropertyByName("name",strName);
		child->getPropertyByName("action",strAction);
		
		if (strClassName.compare(string("npc_group"))==0)
		{
			child->getPropertyByName("name",strName);
			
			ret+=spaceTo_(strName);
			ret+=tag;
			ret += " [URL=\""+buildPrimPath(child)+"\"];"+NL;
			ret+=spaceTo_(strName);
			ret+=tag;
			ret+="->";
			ret+=emiterNode;
			ret+="[arrowhead=none]";
			if(!tailLabel.empty())
			{
				ret+="[label=";
				ret+=tailLabel;
				ret+="]";
			}
			ret+=";";
			ret+=NL;
			ret+=parseStateMachine(child,spaceTo_(strName+tag),spaceTo_(tailLabel),tag);
		}
		else if (strClassName.compare(string("npc_state_event_handler"))==0)
		{
			child->getPropertyByName("name",strTail);
			ret+=parseStateMachine(child,emiterNode,spaceTo_(strTail+"][color=cyan"),tag);
		}
		else if (strClassName.compare(string("npc_group_event_handler"))==0)
		{
			child->getPropertyByName("name",strTail);
			ret+=parseStateMachine(child,emiterNode,spaceTo_(strTail+"][color=cyan"),tag);
		}
		else if ((strAction.compare(string("multi_actions"))==0)
				||	(strAction.compare(string("random_select"))==0))
		{
			child->getPropertyByName("name",strName);
			ret+=spaceTo_(strName);
			ret+=tag;
			ret += " [URL=\""+buildPrimPath(child)+"\"];"+NL;
			if(strAction.compare(string("random_select"))==0)
			{
				ret+=spaceTo_(strName);
				ret+=tag;
				ret+="_";
				ret+="random_select";
				ret += " [URL=\""+buildPrimPath(child)+"\"];"+NL;
				ret+=spaceTo_(strName);
				ret+=tag;
				ret+="->";
				ret+=spaceTo_(strName);
				ret+=tag;
				ret+="_random_select;"+NL;
				ret+=spaceTo_(strName);
				ret+=tag;
				ret+="_";
				ret+="random_select";
			}
			else
			{
				ret+=spaceTo_(strName);
				ret+=tag;
			}
			ret+="[shape=record]";
			if(child->getNumChildren()>0)
			{
				bool first=false;
				ret+="[label=";
				ret+='"';
				ret+="{";
				for (uint l=0;l<child->getNumChildren();l++ )
				{
					string tmpStr;
					IPrimitive* tmpChild;
					child->getChild(tmpChild,l);
					tmpChild->getPropertyByName("name",tmpStr);
					tmpChild->getPropertyByName("class",strClassName);
					tmpChild->getPropertyByName("action",strAction);
					if ((!tmpStr.empty())
						&&	(strClassName.compare("npc_event_handler_action")==0)
						&&	(!strAction.compare("begin_state")==0)
						&&  (!strAction.compare("random_select")==0))
					{
						if (first)
						{
							ret+="|";
						}
					ret+=strAction;
					ret+=tag;
					first=true;
					}
					
				}
				ret+="}";
				ret+='"';
				ret+="]";
			}
			ret+=";";
			ret+=NL;
			ret+=emiterNode;
			ret+="->";
			ret+=spaceTo_(strName);
			ret+=tag;
			if(!tailLabel.empty())
			{
				ret+="[label=";
				ret+=tailLabel;
				ret+="]";
			}
			ret+=";";
			ret+=NL;
			if(child->getNumChildren()>0)
			{
				for (uint l=0;l<child->getNumChildren();l++ )
				{
					string tmpStr;
					IPrimitive* tmpChild;
					child->getChild(tmpChild,l);
					tmpChild->getPropertyByName("name",tmpStr);
					tmpChild->getPropertyByName("class",strClassName);
					tmpChild->getPropertyByName("action",strAction);
					if ((!tmpStr.empty())
						&&	(strClassName.compare("random_select_state")==0))
					{
						vector<string>* jumpDest;
						child->getPropertyByName("parameters",jumpDest);						
						for(uint k=0;k<jumpDest->size();k++)
						{
							ret+=spaceTo_(strName);
							ret+=tag;
							ret+="->";
							ret +=spaceTo_(jumpDest->at(k));
							ret+= ";";
							ret+=NL;							
						}
					}
					
				}
				ret+=NL;
			}
		/*	ret+=spaceTo_(strName);
			ret+=tag;
			ret+="->";
			ret+=spaceTo_(strName);
			ret+=tag;
			ret+="_";
			ret+="random_select";
			ret+="[arrowhead=none];";
			ret+=NL;*/
			for (uint l=0;l<child->getNumChildren();l++ )
			{
				IPrimitive* tmpChild;
				child->getChild(tmpChild,l);
				for (uint m=0;m<tmpChild->getNumChildren();m++ )
				{
					IPrimitive* tmpTmpChild;
					child->getChild(tmpTmpChild,m);
					ret+=parseStateMachine(tmpTmpChild,spaceTo_(strName+tag),string(""),tag);
				}
			}
		}
		else if (strAction.compare(string("random_select_state"))==0)
		{
			vector<string>* jumpDest;
			bool first=true;
			child->getPropertyByName("parameters",jumpDest);
			
			ret += spaceTo_(strName) ;
			ret +="_";
			ret += emiterNode;
			ret += " [URL=\""+buildPrimPath(child)+"\"];"+NL;
			ret += spaceTo_(strName) ;
			ret +="_";
			ret += emiterNode;
			ret +="->";
			for(uint k=0;k<jumpDest->size();k++)
			{
				if (!first)
				{
					ret += spaceTo_(strName) ;
					ret +="_";
					ret += emiterNode;
					ret +="->";
				}
				first=false;
				vector<string> resJump;
				explode(jumpDest->at(k),string(" "),resJump);
				ret +=resJump[1];
				ret+= ";";
				ret+=NL;
				
			}
			ret+=emiterNode;
			ret += "->";
			ret += spaceTo_(strName) ;
			ret +="_";
			ret += emiterNode;
			ret +=";";
			ret +=NL;
		}
		else
		{
			ret+=parseStateMachine(child,emiterNode,tailLabel,tag);
			
		}
		
		
	}	
	currentNode->getPropertyByName("class",strClassName);
	currentNode->getPropertyByName("name",strName);
	currentNode->getPropertyByName("action",strAction);
	if (	(numChild==0)
		&&	(!strName.empty())
		&&	(!strAction.compare("begin_state")==0)
		&&	(!strAction.compare("random_select_state")==0)
		&&	(!strAction.compare("random_select")==0)
		&&	(strClassName.compare("npc_event_handler_action")==0))
	{
		ret+=spaceTo_(strAction);
		ret+=tag;
		ret+="[color=green]";
		ret += " [URL=\""+buildPrimPath(currentNode)+"\"];";
		ret+=NL;
		ret+=emiterNode;
		ret+="->";
		ret+=spaceTo_(strAction);
		ret+=tag;
		if(!tailLabel.empty())
		{
			ret+="[label=";
			ret+=tailLabel;
			ret+="]";
		}
		ret+=";";
		ret+=NL;
	}
	return ret;
	
}

bool CGraphPlugin::createBitmap (const string &tmpPath)
{
	string fileName(tmpPath+"/output.png");
	NLMISC::CBitmap bitmap;
	NLMISC::CIFile pngLocation;
	if(!pngLocation.open(fileName))
	{
		nlwarning("Can't open file '%s'", fileName.c_str());
		return false;
	}

	WORD retour;
 
	if(retour=bitmap.load(pngLocation))	
	{	
		
		HBITMAP &_Hbmp=GraphDlg->_Hbmp;

		HBITMAP &_Hdib=GraphDlg->_Hdib;

		BITMAPINFO &_DibBitmapInfo=GraphDlg->_DibBitmapInfo;;

		uint8* &_DibBits=GraphDlg->_DibBits;
		
		_DibBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
		_DibBitmapInfo.bmiHeader.biWidth = bitmap.getWidth();
		_DibBitmapInfo.bmiHeader.biHeight = bitmap.getHeight();
		_DibBitmapInfo.bmiHeader.biPlanes = 1;
		_DibBitmapInfo.bmiHeader.biBitCount = retour;
		_DibBitmapInfo.bmiHeader.biCompression  = BI_RGB;
		_DibBitmapInfo.bmiHeader.biSizeImage = bitmap.getSize();
		_DibBitmapInfo.bmiHeader.biXPelsPerMeter = 0;
		_DibBitmapInfo.bmiHeader.biYPelsPerMeter = 0;
		_DibBitmapInfo.bmiHeader.biClrUsed = 0;
		_DibBitmapInfo.bmiHeader.biClrImportant = 0;

		// Create the bitmap
		HWND desktop = ::GetDesktopWindow();
		HDC dc = ::GetDC (desktop);	
		
		nlverify(_Hdib = CreateDIBSection(dc, &_DibBitmapInfo, DIB_RGB_COLORS,
		(void**)&_DibBits, NULL, 0));

		bitmap.getDibData(_DibBits);

		nlverify (_Hbmp = CreateDIBitmap(dc, &(_DibBitmapInfo.bmiHeader), CBM_INIT,
		(void*)_DibBits, &_DibBitmapInfo, DIB_RGB_COLORS));
		::ReleaseDC (desktop, dc);
		pngLocation.close();
		pngLocation.flush();
		return true;
	}
	else
	{
		nlwarning("Can't load file '%s'", fileName.c_str());
	}
	pngLocation.close();
	pngLocation.flush();
	return false;
}


bool CGraphPlugin::activatePlugin()
{
	if(!_PluginActive)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		AfxEnableControlContainer();
		GraphDlg->ShowWindow(TRUE);
		_PluginActive=true;
		return true;
	}
	return false;
}

bool CGraphPlugin::closePlugin()
{
	if (_PluginActive)
	{
		GraphDlg->ShowWindow(FALSE);
		_PluginActive=false;
		return true;
	}
	return false;
}

std::string&	CGraphPlugin::getName()
{
	return _Name;
}

bool CGraphPlugin::isActive()
{
	return _PluginActive;
}

void CGraphPlugin::unsetDlgGraph()
{
	_PluginActive=false;
}


void CGraphPlugin::doSelection(const string& primPath)
{
	IPrimitive	*rootNode;

	TPrimitiveClassPredicate pred("root");
	TPrimitiveSet resSet;

	if(_PluginAccess->getCurrentSelection().size()>0)
	{
		
		try
		{
			rootNode=(IPrimitive*)_PluginAccess->getRootNode(_rootFileName);

			selectPrimByPath(rootNode,primPath,resSet);

			_PluginAccess->setCurrentSelection(resSet);

		}catch(exception e){
			GraphDlg->MessageBox(e.what());
		}
	}
}