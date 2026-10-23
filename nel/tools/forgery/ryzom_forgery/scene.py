"""Patina scene model: the shapes shown together in the viewer (the 3 scale
references plus any imported .shape), each with its own display state and
bind, plus the one shared creature and an opaque `ui` dict (camera, open
panels, lights, wind...) -- serialized as JSON in a workspace's `scenes/`
folder, extension `.scene` (see project-todos/forgery/
patina_multi_shapes_scenes.md). No UI, no Panda3D here: ObjectEditorApp
reads/writes this model, this module only owns the data and its JSON.

An entry's source is stored as one of:
- reference: {"type": "reference", "label": <reference shape label>}
- workspace: {"type": "workspace", "path": <relative to the workspace>}
- file:      {"type": "file", "path": <absolute path>}
- bnp:       {"type": "bnp", "bnp": <absolute .bnp path>, "name": <entry name>}
Entries whose source no longer exists are silently dropped on load.
"""

import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Callable, Dict, List, Optional

SCENE_EXTENSION = ".scene"
SCENES_DIR_NAME = "scenes"
DEFAULT_SCENE_NAME = "default" + SCENE_EXTENSION
SCENE_VERSION = 1

SOURCE_REFERENCE = "reference"
SOURCE_WORKSPACE = "workspace"
SOURCE_FILE = "file"
SOURCE_BNP = "bnp"


@dataclass
class SceneEntry:
	id: str
	source: Dict[str, str]
	visible: bool = True
	transparent: bool = False
	placement: str = "auto"  # "auto" (side by side) | "origin" | "pivot"
	bind_slot: str = ""  # creature_ref.BODY_SLOTS entry or "hair" (skinned); "" = none
	bind_attach_point: str = ""  # bone name (rigid); "" = none
	kept: bool = True  # False only for the entry a load from the Explorer created, until the user adds it with [+]: the next Explorer load replaces it. Not saved -- entries read from a scene are all kept

	@property
	def editable(self) -> bool:
		return self.source.get("type") != SOURCE_REFERENCE


@dataclass
class Scene:
	entries: List[SceneEntry] = field(default_factory=list)
	edited_id: str = ""  # id of the entry being edited, "" = none
	solo_id: str = ""  # id of the only entry shown, "" = solo off
	creature: str = ""  # the shared creature's name, "" = none
	ui: Dict[str, Any] = field(default_factory=dict)  # opaque: camera, panels, lights, wind...

	def entry(self, entry_id: str) -> Optional[SceneEntry]:
		return next((e for e in self.entries if e.id == entry_id), None)

	def add(self, entry: SceneEntry) -> None:
		if self.entry(entry.id) is not None:
			raise ValueError(f"duplicate scene entry id {entry.id!r}")
		self.entries.append(entry)

	def remove(self, entry_id: str) -> None:
		self.entries = [e for e in self.entries if e.id != entry_id]
		if self.edited_id == entry_id:
			self.edited_id = ""
		if self.solo_id == entry_id:
			self.solo_id = ""

	def set_solo(self, entry_id: str) -> None:
		"""Clicking the already-solo entry turns solo back off. Only an
		imported (editable) entry can be solo."""
		if entry_id == self.solo_id:
			self.solo_id = ""
			return
		target = self.entry(entry_id)
		if target is not None and target.editable:
			self.solo_id = entry_id

	def is_shown(self, entry: SceneEntry) -> bool:
		return entry.visible and (not self.solo_id or entry.id == self.solo_id)


def source_for_path(path: Path, workspace_dir: Optional[Path]) -> Dict[str, str]:
	"""Workspace-relative if `path` is inside the workspace, absolute otherwise."""
	if workspace_dir is not None:
		try:
			return {"type": SOURCE_WORKSPACE, "path": path.resolve().relative_to(workspace_dir.resolve()).as_posix()}
		except ValueError:
			pass
	return {"type": SOURCE_FILE, "path": str(path)}


def source_for_bnp(bnp_path: Path, name: str) -> Dict[str, str]:
	return {"type": SOURCE_BNP, "bnp": str(bnp_path), "name": name}


def resolve_source(source: Dict[str, str], workspace_dir: Optional[Path],
                   bnp_has: Optional[Callable[[Path, str], bool]] = None) -> Optional[Path]:
	"""The on-disk file (for a bnp source: the .bnp itself) the source points
	at, or None if it no longer exists. A reference source always resolves to
	Path("") -- the references live in the app's own examples folder, resolved
	by reference_shapes.py. `bnp_has(bnp_path, name)` checks the entry exists
	inside the .bnp; without it only the .bnp file itself is checked."""
	kind = source.get("type")
	if kind == SOURCE_REFERENCE:
		return Path("")
	if kind == SOURCE_WORKSPACE:
		if workspace_dir is None:
			return None
		path = workspace_dir / source.get("path", "")
		return path if path.is_file() else None
	if kind == SOURCE_FILE:
		path = Path(source.get("path", ""))
		return path if path.is_file() else None
	if kind == SOURCE_BNP:
		bnp_path = Path(source.get("bnp", ""))
		if not bnp_path.is_file():
			return None
		if bnp_has is not None and not bnp_has(bnp_path, source.get("name", "")):
			return None
		return bnp_path
	return None


def to_dict(scene: Scene) -> Dict[str, Any]:
	return {
		"version": SCENE_VERSION,
		"creature": scene.creature,
		"edited": scene.edited_id,
		"solo": scene.solo_id,
		"entries": [
			{"id": e.id, "source": e.source, "visible": e.visible, "transparent": e.transparent,
			 "placement": e.placement, "bind_slot": e.bind_slot, "bind_attach_point": e.bind_attach_point}
			for e in scene.entries
		],
		"ui": scene.ui,
	}


def from_dict(data: Dict[str, Any], workspace_dir: Optional[Path],
              bnp_has: Optional[Callable[[Path, str], bool]] = None) -> Scene:
	"""Rebuilds a Scene, dropping every entry whose source can't be resolved
	(or that is malformed) -- and clearing edited/solo if they pointed at one."""
	scene = Scene(creature=str(data.get("creature", "")), ui=dict(data.get("ui", {})))
	for raw in data.get("entries", []):
		try:
			entry = SceneEntry(
				id=str(raw["id"]), source=dict(raw["source"]), visible=bool(raw.get("visible", True)),
				transparent=bool(raw.get("transparent", False)), placement=str(raw.get("placement", "auto")),
				bind_slot=str(raw.get("bind_slot", "")), bind_attach_point=str(raw.get("bind_attach_point", "")))
		except (KeyError, TypeError, ValueError):
			continue
		if resolve_source(entry.source, workspace_dir, bnp_has) is None or scene.entry(entry.id) is not None:
			continue
		scene.entries.append(entry)
	edited, solo = str(data.get("edited", "")), str(data.get("solo", ""))
	scene.edited_id = edited if (scene.entry(edited) is not None and scene.entry(edited).editable) else ""
	scene.solo_id = solo if (scene.entry(solo) is not None and scene.entry(solo).editable) else ""
	return scene


def scenes_dir(workspace_dir: Path) -> Path:
	return workspace_dir / SCENES_DIR_NAME


def save_scene(scene: Scene, workspace_dir: Path, file_name: str) -> Path:
	"""Writes `scenes/<file_name>` (the folder is created on demand, also in
	an external workspace) and returns its path."""
	target = scenes_dir(workspace_dir) / file_name
	target.parent.mkdir(parents=True, exist_ok=True)
	target.write_text(json.dumps(to_dict(scene), indent="\t", ensure_ascii=False), encoding="utf-8")
	return target


def load_scene(path: Path, workspace_dir: Optional[Path],
               bnp_has: Optional[Callable[[Path, str], bool]] = None) -> Optional[Scene]:
	"""None if the file is missing or isn't valid scene JSON."""
	try:
		data = json.loads(path.read_text(encoding="utf-8"))
	except (OSError, ValueError):
		return None
	if not isinstance(data, dict):
		return None
	return from_dict(data, workspace_dir, bnp_has)
