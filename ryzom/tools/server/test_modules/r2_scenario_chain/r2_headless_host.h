// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef R2_HEADLESS_HOST_H
#define R2_HEADLESS_HOST_H

#include "nel/misc/types_nl.h"
#include "game_share/object.h"
#include "game_share/scenario.h"

#include <map>
#include <string>
#include <vector>

struct lua_State;

namespace NLGEORGES {
class UFormLoader;
}

// A host function and its body: the body returns -1 with a message on the stack to raise a Lua error
#define R2H_DECLARE(name)              \
	static int name(lua_State *state); \
	static int name##Body(lua_State *state);

namespace R2 {

/**
 * Host for the Ring editor's Lua (ryzom/common/data_common/r2) without the client.
 *
 * The client runs these scripts on CEditor, CDynamicMapClient and the interface bindings. This class
 * provides the part of that host the scripts need to load a scenario, run the version update on it and
 * translate it to the run-time form:
 * - the scenario tree as projected objects (the userdata with the __index, __newindex, __next and __eq
 *   metamethods that CEditor::initObjectProjectionMetatable gives them);
 * - the node requests, which go through a model of the client's action historic (CActionHistoric): a
 *   request on a ghost node is applied at once, any other is recorded in the action in progress, sent
 *   on when that action ends and applied at the end of the frame, or dropped when the historic is
 *   cleared;
 * - the scenario the server has: the one it sent last, with every request that was sent on applied to
 *   it. A request on a ghost node does not reach it. It is what the editor gets when the scenario
 *   comes back from the server, after a test or a reconnection;
 * - the component factory, the palette and the property lookup through the palette and the class defaults.
 *
 * The Lua state has the libraries the release client gives its scripts (NLGUI::CLuaState) and no others.
 *
 * What the client offers beyond that is either missing, so that a call fails, or a stub. A stub that
 * gets called records its name and the script line of the first call, and so does a function that
 * answers with a value the client would compute from data the host does not have.
 */
class CHeadlessHost
{
public:
	struct CLogLine
	{
		std::string Phase;
		std::string Kind; // debugInfo, messageBox, systemInfo, print, luaError, loadError, stub, host
		std::string Text;
	};

	/// Name to the script line of the first use
	typedef std::map<std::string, std::string> TUseMap;

	CHeadlessHost();
	~CHeadlessHost();

	/**
	 * Creates the Lua state, runs the driver script (the host's own Lua, which keeps the debug functions
	 * it needs), reduces the libraries, registers the host functions and runs r2_core.lua and the
	 * interface scripts the translator needs from r2Dir. Returns false when any script fails.
	 */
	bool init(const std::string &r2Dir, const std::string &driverFile, std::string &error);

	/// Name of the step in progress, attached to the log lines and to the recorded uses
	void setPhase(const std::string &phase) { m_Phase = phase; }

	/// Prints the log lines to stdout as they come
	void setVerbose(bool verbose) { m_Verbose = verbose; }

	/// Loads the translation of the interface strings (en.uxt, r2_en.uxt) for i18n.get
	void loadTranslation(const std::string &directory, const std::string &language);

	/// Gives the leveldesign directory the creature sheets are read from (DFN and game_elem below it)
	void setSheetDirectory(const std::string &leveldesign);

	/**
	 * The start of CEditor::scenarioUpdated for a new tree, as it comes back from an upload
	 * (CClientEditionModule::onScenarioUploaded): the numbering of the factory starts again, the tree
	 * becomes the edited scenario and the factory gets the ids in use. A tree that comes from the server
	 * is also what the server has from then on; the scenario a test reads back from its file is not.
	 */
	void beginScenarioUpdated(CObject *highLevel, bool fromServer);
	/// The end of CEditor::scenarioUpdated: the action historic is cleared and takes the scenario
	void endScenarioUpdated();
	/// The end of a frame: the requests of the actions that ended are applied
	void endFrame();
	/// The scenario as r2.requestUploadCurrentScenario or r2.requestCreateScenario sent it, which comes back
	/// as an update. False when no upload was asked for since the last call.
	bool takeUploadedScenario(CObject::TSmartPtr &scenario);
	/**
	 * What the client does with a scenario it has read from a file, before it uploads or shows it: the
	 * version update on the scenario as a table (r2.Version.updateLoadedScenario). place is "File" for a
	 * file that is loaded and "Test" for the scenario a test reads back. scenario becomes the scenario
	 * the function returned, when it returned one. untouched tells whether the table gave the scenario
	 * as it was read after the function. Returns false when the update failed or raised an error.
	 */
	bool updateLoadedScenario(CObject::TSmartPtr &scenario, const std::string &place, bool &untouched, std::string &error);
	/**
	 * The author adds a component to the features of an act, as the palette does: one action with one
	 * insert request (r2Headless.addFeature). The caller ends the frame.
	 */
	bool addFeature(const CObject *component, std::string &error);
	/**
	 * Forgets the requests that are still recorded when a step of the run ends, and records them as late:
	 * the client starts a new action every frame, which ends the one in progress, so its requests are
	 * applied a frame later, after what the script was doing.
	 */
	void endStepWithLateRequests();
	CObject *getHighLevel() const;
	/// The scenario as the server has it, nullptr before the first scenario came from it
	CObject *getServerHighLevel() const;
	std::string getScenarioInstanceId() const;

	/// Calls r2Headless.<function> with string arguments. Results stay on the Lua stack when it returns true.
	bool callDriver(const std::string &function, const std::vector<std::string> &arguments, int resultCount, std::string &error);

	lua_State *getLua() const { return m_Lua; }

	/// Conversion of a Lua value to an object tree, as CComLuaModule::getObjectFromLua in the client
	static CObject::TSmartPtr getObjectFromLua(lua_State *state, int idx);

	const std::vector<CLogLine> &getLog() const { return m_Log; }
	void log(const std::string &kind, const std::string &text);

	/// Stubs called and values substituted, by phase
	const std::map<std::string, TUseMap> &getCalledStubs() const { return m_CalledStubs; }
	/// Global names and r2 fields read while undefined, by phase
	const std::map<std::string, TUseMap> &getUndefinedReads() const { return m_UndefinedReads; }
	/// Calls of the host functions that have an implementation, by phase and name
	const std::map<std::string, std::map<std::string, uint32>> &getHostCalls() const { return m_HostCalls; }
	/// Node requests by what became of them ("insertNode applied at once", "setNode dropped" and so on)
	const std::map<std::string, uint32> &getRequestCounts() const { return m_RequestCounts; }
	/// Requests that were recorded and dropped with the historic, by phase: "setNode Class.Attribute" to the script line
	const std::map<std::string, TUseMap> &getDroppedRequests() const { return m_DroppedRequests; }
	/// Requests that were recorded and that the client applies a frame later, by phase
	const std::map<std::string, TUseMap> &getLateRequests() const { return m_LateRequests; }

	uint32 getStartingActIndex() const { return m_StartingActIndex; }

	/// Restarts the numbering of the run-time ids, as the client does before each translation
	void resetRtIds();

private:
	// One node request of the scripts, as CActionHistoric::CRequestBase
	struct CNodeRequest
	{
		enum TKind
		{
			Insert,
			Set,
			Erase,
			Move
		};
		TKind Kind;
		std::string InstanceId;
		std::string AttrName;
		sint32 Position;
		std::string Key;
		CObject::TSmartPtr Value;
		std::string DestInstanceId;
		std::string DestAttrName;
		sint32 DestPosition;
		std::string Where; // script line of the request
		bool GhostCall; // asked for through requestInsertGhostNode or requestSetGhostNode

		CNodeRequest()
		    : Kind(Set)
		    , Position(-1)
		    , DestPosition(-1)
		    , GhostCall(false)
		{
		}
	};

	// projected objects
	R2H_DECLARE(luaProjectionIndex)
	R2H_DECLARE(luaProjectionNewIndex)
	R2H_DECLARE(luaProjectionNext)
	static int luaProjectionEqual(lua_State *state);
	static int luaProjectionGc(lua_State *state);
	static int luaProjectionToString(lua_State *state);
	void pushProjection(lua_State *state, const CObject *object);
	void pushProjectedValue(lua_State *state, const CObject *object);
	static void pushProjectionKey(lua_State *state, const CObject *obj, sint32 index);
	const CObject *getPropertyValue(const CObject *component, const std::string &attrName) const;

	// r2 functions with an implementation
	R2H_DECLARE(luaDoFile)
	R2H_DECLARE(luaRegisterGenerator)
	R2H_DECLARE(luaNewComponent)
	R2H_DECLARE(luaAddPaletteElement)
	R2H_DECLARE(luaGetPaletteElement)
	R2H_DECLARE(luaIsInPalette)
	R2H_DECLARE(luaGetPropertyValue)
	R2H_DECLARE(luaRequestInsertNode)
	R2H_DECLARE(luaRequestInsertGhostNode)
	R2H_DECLARE(luaRequestSetNode)
	R2H_DECLARE(luaRequestSetGhostNode)
	R2H_DECLARE(luaRequestEraseNode)
	R2H_DECLARE(luaRequestMoveNode)
	static int luaRequestNewAction(lua_State *state);
	static int luaRequestNewPendingAction(lua_State *state);
	R2H_DECLARE(luaRequestNewMultiAction)
	R2H_DECLARE(luaRequestNewPendingMultiAction)
	static int luaRequestEndAction(lua_State *state);
	static int luaRequestForceEndMultiAction(lua_State *state);
	static int luaRequestCancelAction(lua_State *state);
	static int luaClearActionHistoric(lua_State *state);
	static int luaRequestUploadCurrentScenario(lua_State *state);
	R2H_DECLARE(luaRequestCreateScenario)
	R2H_DECLARE(luaGetScenarioObj)
	R2H_DECLARE(luaRequestSetScenarioVersions)
	R2H_DECLARE(luaGetInstanceFromId)
	R2H_DECLARE(luaGenInstanceName)
	static int luaGetNamespace(lua_State *state);
	static int luaSetStartingActIndex(lua_State *state);
	R2H_DECLARE(luaGetSheetIdName)
	R2H_DECLARE(luaPrint)
	R2H_DECLARE(luaGetEmoteBehaviorFromEmoteId)
	static int luaSnapPosToGround(lua_State *state);

	// globals with an implementation
	R2H_DECLARE(luaDebugInfo)
	R2H_DECLARE(luaMessageBox)
	R2H_DECLARE(luaDisplaySystemInfo)
	R2H_DECLARE(luaDumpCallStack)
	R2H_DECLARE(luaI18nGet)
	R2H_DECLARE(luaI18nHasTranslation)
	R2H_DECLARE(luaGetCharacterSheetSkel)
	R2H_DECLARE(luaGetSheetId)
	R2H_DECLARE(luaGetR2PlayerRace)
	R2H_DECLARE(luaIsR2PlayerMale)
	R2H_DECLARE(luaFileLookup)
	R2H_DECLARE(luaGetDefine)
	R2H_DECLARE(luaGetClientCfgVar)
	R2H_DECLARE(luaGetCompleteIslands)
	static int luaColor(lua_State *state);
	static int luaTimeMs(lua_State *state);
	R2H_DECLARE(luaSafeLoad)
	R2H_DECLARE(luaSafeLoadFile)
	static int luaTraceback(lua_State *state);
	R2H_DECLARE(luaRecordSwallowedError)
	static int luaPanic(lua_State *state);

	// stubs, tracking and probes
	static int luaStub(lua_State *state);
	static int luaTracked(lua_State *state);
	static int luaUndefinedRead(lua_State *state);
	static void pushTracked(lua_State *state, const char *name, int (*function)(lua_State *));
	static void setProbe(lua_State *state, const char *prefix);
	void recordStubCall(lua_State *state, const std::string &name);

	// the action historic
	static int requestInsertNode(lua_State *state, bool ghost);
	static int requestSetNode(lua_State *state, bool ghost);
	static int requestNewAction(lua_State *state, bool pending, uint32 count);
	void submitRequest(lua_State *state, const CNodeRequest &request, bool direct);
	void applyRequest(CScenario *scenario, const CNodeRequest &request, const char *whose);
	void endAction();
	void flushOpenAction();
	void dropOpenAction(const char *reason, bool late);

	void openLibraries();
	void reduceLibraries();
	void registerFunctions();
	bool readScript(const std::string &filename, std::string &chunk, std::string &error) const;
	bool doFile(const std::string &filename, std::string &error);
	bool runChunk(const std::string &code, const std::string &chunkName, int resultCount, std::string &error);
	bool getCreatureSkeleton(const std::string &sheet, std::string &skeleton);
	bool getCreatureField(const std::string &sheet, const char *field, std::string &value);

private:
	lua_State *m_Lua;
	std::string m_R2Dir;
	std::string m_Phase;
	bool m_Verbose;
	bool m_TranslationLoaded;
	NLGEORGES::UFormLoader *m_FormLoader;
	std::map<std::string, std::string> m_SkeletonCache;

	CObjectFactory *m_Factory;
	CScenario *m_Scenario;
	CScenario *m_ServerScenario;
	std::string m_Eid;
	std::map<std::string, CObject::TSmartPtr> m_Palette;
	uint32 m_StartingActIndex;

	// CActionHistoric: the action being recorded, and whether the historic holds a scenario (it tells a
	// ghost node from another only when it does)
	std::vector<CNodeRequest> m_OpenAction;
	bool m_HasOpenAction;
	bool m_OpenActionIsPending;
	uint32 m_SubActionCount;
	bool m_HistoricHasScenario;
	// requests of the actions that ended, waiting for the end of the frame
	std::vector<CNodeRequest> m_FrameRequests;
	CObject::TSmartPtr m_UploadedScenario;

	std::vector<CLogLine> m_Log;
	std::map<std::string, TUseMap> m_CalledStubs;
	std::map<std::string, TUseMap> m_UndefinedReads;
	std::map<std::string, uint32> m_RequestCounts;
	std::map<std::string, TUseMap> m_DroppedRequests;
	std::map<std::string, TUseMap> m_LateRequests;
	std::map<std::string, std::map<std::string, uint32>> m_HostCalls;
};

} // namespace R2

#endif // R2_HEADLESS_HOST_H
