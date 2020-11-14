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

#ifndef NL_PARTICLE_WORKSPACE_H
#define NL_PARTICLE_WORKSPACE_H

#include "nel/misc/smart_ptr.h"
#include "nel/misc/stream.h"
#include "nel/3d/skeleton_model.h"
//
#include "ps_initial_pos.h"

namespace NL3D
{
	class CParticleSystem;
	class CParticleSystemModel;
	class CshapeBank;
	class CFontManager;
	class CFontGenerator;
}


class CObjectViewer;





/** A workspace containing several fxs    
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2004
  */
class CParticleWorkspace
{
public:		
	// A node in the workspace
	// Contains a particle system and its relative path 
	class CNode : public NLMISC::CRefCount
	{
	public:		
		void					   init(CParticleWorkspace *ws);
		void					   setRelativePath(const std::string &relativePath);
		const std::string		   &getRelativePath() const { return _RelativePath; }
		std::string				   getFullPath() const;
		std::string				   getFilename() const { return NLMISC::CFile::getFilename(_RelativePath); }
		// Serial node information into workspace stream. This does not save the particle system shape, only a reference to its file
		void					   serial(NLMISC::IStream &f);
		// Save the particle system target file
		void					   savePS();
		// Save particle system with an arbitrary filename
		void					   savePSAs(const std::string &fullPath);
		// put back in the unloaded state
		void					   unload();
		/** Load the particle system target file
		  * \return true if loading succeed (false means that loading was ok, but this is not a particle system). Other cases throw an exception.
		  */
		bool					   loadPS();
		// create an empty particle system
		void					   createEmptyPS();
		// helper flag to know if a ps has been modified
		bool					   isModified() const { return _Modified; }
		void					   setModified(bool modified);
		NL3D::CParticleSystem	   *getPSPointer() const { return _PS; }
		NL3D::CParticleSystemModel *getPSModel() const { return _PSM; }
		// See if this node ps has been loaded
		bool					   isLoaded() const { return _PS != NULL; }
		// Get the workspace in which this node is inserted
		CParticleWorkspace         *getWorkspace() const { return _WS; }		
		// Memorize current position of object in the system. Useful to play the system because instances can be created / deleted
		void					   memorizeState();
		// Restore state previously memorize. Is usually called when the user stops a particle system
		void					   restoreState();
		// Test if state is currenlty memorized
		bool					   isStateMemorized() const;
		// For edition : If the state of the system has been memorized, keep it on par with the system when it is modified
		void					   removeLocated(NL3D::CPSLocated *loc);		
		// For edition : If the state of the system has been memorized, keep it on par with the system when it is modified	
		void					   removeLocatedBindable(NL3D::CPSLocatedBindable *lb);		
		// Returns the skeleton to which the ps is currently sticked
		NL3D::CSkeletonModel	  *getParentSkel() const { return _ParentSkel; }
		const std::string		   &getParentSkelName() const { return _ParentSkelName; }
		const std::string		   &getParentBoneName() const { return _ParentBoneName; }
		//
		std::string				   getTriggerAnim() { return _TriggerAnim; }
		void					   setTriggerAnim(const std::string &anim);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	private:
		std::string					 _TriggerAnim;
		NL3D::CParticleSystem		*_PS;
		NL3D::CParticleSystemModel	*_PSM;
		NL3D::CShapeBank			*_ShapeBank;   // Keep a shape bank per node because we want the whole path to identify the ps, not just its filename
		                                           // (shape bank keeps the filename only)
		std::string					_RelativePath; // relative path from which the ps was inserted
		                                           // relative path is also a unique identifier for this ps in the workspace												  
		CPSInitialPos				_InitialPos;   // initial pos of system. Allow to restore the initial instances of the system  when doing start / stop
		bool						_Modified;
		CParticleWorkspace			*_WS;		
		NLMISC::CRefPtr<NL3D::CSkeletonModel> _ParentSkel;
		bool						_ResetAutoCount;
		//
		std::string					_ParentSkelName;
		std::string					_ParentBoneName;
	private:
		void setup(NL3D::CParticleSystemModel &psm);
	public:
		bool						getResetAutoCountFlag() const { return _ResetAutoCount; }
		void						setResetAutoCountFlag(bool reset) { _ResetAutoCount = reset; }
		// stick to a skeleton
		void stickPSToSkeleton(NL3D::CSkeletonModel *skel,
							   uint bone,
							   const std::string &parentSkelName, // for callback after loading
							   const std::string &parentBoneName 
							  );
		void unstickPSFromSkeleton();
	private:
		friend class CParticleWorkspace;		
		// Ctor
		CNode();		
	public:
		// DTor
		~CNode();
	};
	// callback to know when a workspace node has been modified
	struct IModificationCallback
	{
		virtual void workspaceModifiedFlagChanged(CParticleWorkspace &pw) = 0;
		virtual void nodeModifiedFlagChanged(CNode &node) = 0;
		virtual void nodeSkelParentChanged(CNode &node) = 0; // called when fx has been linked / unlinked from a skeleton parent
	};
	// sorting predicate
	struct ISort
	{
		virtual bool less(const CNode &lhs, const CNode &rhs) const = 0;
	};
	// ctor
	CParticleWorkspace();
	// dtor
	~CParticleWorkspace();

	// Init the workspace for the given object viewer
	// must be called prior to other methods	
	void init(CObjectViewer			*ov,
			  const std::string		&filename,
			  NL3D::CFontManager	*fontManager, 
			  NL3D::CFontGenerator	*fontGenerator 
			 );
	// Set a new name for the workspace (not its filename)
	void setName(const std::string &name);
	std::string getName() const { return _Name; }	
	// Get the object viewer instance
	CObjectViewer *getObjectViewer() const { return _OV; }
	// Get the path in which workpsace is located with a trailing slash
	std::string getPath() const;	
	std::string getFilename() const { return _Filename; }	
	// Get Number of nodes in the workspace
	uint		getNumNode() const { return (uint)_Nodes.size(); }
	/** Get a node in workspace
	  * Can keep pointer safely as long as the node is not deleted
	  */
	CNode		*getNode(uint index) const { return _Nodes[index]; }
	// Get a node from a pointer on a particle system
	CNode		*getNodeFromPS(NL3D::CParticleSystem *ps) const;
	/** Test if the workspace already contains a node with the given filename name
	  * NB : 2 node with the same name re not allowed, even if their path is different
	  */
	bool		containsFile(std::string filename) const;

	/** Add a node in the workspace. Will succeed only if fx filename does not already exist in the workspace.	 
	  * The node is in the 'unloaded' state, so caller must load it afterward.	              
	  * NB : no lookup is done, full path must be provided.
	  * 
	  * \return pointer to new node, or NULL if already inserted	  
	  */
	CNode		*addNode(const std::string &filenameWithFullPath);	
	// remove a node by its index
	void		removeNode(uint index);
	// remove a node by its pointer
	void		removeNode(CNode *ptr);
	// Get index of a node from its pointer, or -1 if not found
	sint		getIndexFromNode(CNode *node) const;
	/** Save the workspace structure. The target file is the one given when this object was created	  
	  * NB : ps shape are not saved, only the structure is. To save the shapes, call CNode::save()
	  */
	void		save();
	/** Load the workspace structure. The target file is the one given when this object was created
	  * All nodes are in the 'unloaded" state, so it is to the caller to load them by calling load() on their node
	  */
	void		load();
	// Test whether the structure of the workspace has been modified (does not test if ps inside the workspace have been modified)
	bool		isModified() const { return _Modified; }
	// Test whether the content of the workspace has ben modified
	bool		isContentModified() const;
	void		touch() { setModifiedFlag(true); }
	void		clearModifiedFlag() { setModifiedFlag(false); }
	// set a callback to know when a node is modified
	void					setModificationCallback(IModificationCallback *cb) { _ModificationCallback = cb; }		
	IModificationCallback  *getModificationCallback() const { return _ModificationCallback; }	
	// Sort the workspace. The node pointer remains valid and unchanged
	void		sort(ISort &predicate);	
	// get font manager / font generator
	NL3D::CFontGenerator			*getFontGenerator() const { return _FontGenerator; }
	NL3D::CFontManager				*getFontManager() const { return _FontManager; }
	// restick all objects, useful after loading
	void		restickAllObjects(CObjectViewer *ov);
private:		
	typedef std::vector<NLMISC::CSmartPtr<CNode> > TNodeVect;
	TNodeVect						_Nodes;		// use smart ptr to avoir prb wih resize
	std::string						_Filename;  // path + name of workspace
	CObjectViewer					*_OV;	
	bool							 _Modified; // 
	NL3D::CFontManager				*_FontManager;
	NL3D::CFontGenerator			*_FontGenerator;
	IModificationCallback			*_ModificationCallback;
	std::string						_Name;		// workspace user name
private:
	// serial the object
	void serial(NLMISC::IStream &f);	
	// set the 'modified flag' and call the callback
	void setModifiedFlag(bool modified);
public:		
	void nodeModified(CNode &node);
};









#endif