# Changelog

## 2026-08-27 — ✨ Auto-export imports/ -> shapes/ Step 4+5: mismatch backup, sysbar status

Finished the "Auto-export imports/ -> shapes/" chantier:

**Step 4** (material-count mismatch): design changed from the original plan (a viewport-takeover
popup reusing `_draw_replace_match_popup()`) to fully automatic instead, per the user, avoiding
the discard-unsaved-work risk a viewport takeover would carry on whatever else is open. New
`ImportWatcher._backup_and_reexport()` in `import_watcher.py`: on `MaterialCountMismatch`, renames
the existing target to `<stem>_backup_<YYYYMMDD_HHMMSS><suffix>` (same convention as the manual
Save-as-backup flow) and re-exports the newly imported mesh fresh under the real target name. No
popup, no viewport takeover, no `object_editor.py` wiring needed for this part.

**Step 5** (status messages): new `on_status(message, is_error)` hook on `ImportWatcher`, via a
`_report()` helper that both prints and (if given) calls the hook, for every outcome (export,
update, backup-and-reexport, or any failure). `object_editor.py` wires it to
`_on_import_status()`/`_flush_pending_import_status()` -- same cross-thread queue-then-drain-once-
per-frame pattern as the existing import-conflict popup, since the hook fires off ImportWatcher's
background thread -- surfacing the message on the sysinfo status bar (red for a failure, green for
success, new `_IMPORT_STATUS_ERROR_COLOR`/`_IMPORT_STATUS_SUCCESS_COLOR`).

Also unified the "target is the shape currently open in the viewport" conflict-resolution path
(`_apply_import_conflict_update()`): it used to call `update_existing_shape()` directly and just
report a mismatch as a failure via a separate `self._save_status` mechanism, without ever falling
back to Step 4's backup-and-reexport. Discovered live: a `.obj` mismatch on an open shape reported
under the panel's Save/Quit row instead of the sysbar, and failed instead of backing up. Now routes
through the same `ImportWatcher._update_existing_target()` (made to return True/False so the
caller knows whether to refresh the viewport) as the automatic path -- single outcome-reporting
story regardless of whether the target was open.

`test.sh` extended to 6 tests (was 5): new test confirms a material-count mismatch gets backed up
and the target re-exported under its original name. All 6 pass on the real machine.

## 2026-08-27 — 🐛 Fix Patina's restart-loop bug and add a 5s splash self-destruct safety net

Finished a UI font/size Settings feature (Settings > Tools: font picker, size drag, "Restart now"
button) that had been left uncommitted, but its `ForgeryApp.restart()` method silently shadowed
Panda3D's own `ShowBase.restart()` -- called internally by `ShowBase.__init__()` to start the
IGLOOP render/input task -- so on every single launch, mid-`__init__`, *our* `restart()` fired
instead and re-exec'd (`os.execv`) the whole process, forever, before a window ever fully opened.
Fixed by renaming to `relaunch()`, which doesn't collide with anything in the `ShowBase` API.

Root-caused by bisecting with `sys.exit()` calls dropped at increasing depth through `__init__`
until the exact call site surfaced in the stack -- much faster than the several turns spent
guessing beforehand (an unrelated native `import imgui_bundle` crash turned out to be a red
herring from a different, coincidental issue on the same machine).

Independently of that root cause, `_splash_process.py` (the separate Tk process showing the
splash image while the main app starts up) had no way to notice its parent had died -- a crash
anywhere before `Splash.close()` runs left it orphaned forever, and enough failed launches in a
row could pile up unkillable-feeling process litter. Added `root.after(5000, root.destroy)`: a
self-destruct that fires regardless of what happens to the parent, bounding the damage from any
future crash (of any kind) to a single lingering window for at most 5s, never an accumulation.

Also: moved the font Settings' "Restart now" button to the front of its line (was after the
explanatory text, now before it), and colored the bottom bar's Save (pastel green) and Quit
(pink) buttons via the existing `_colored_button()` helper, matching the color-coding convention
already used for the import-conflict popup's own buttons.

## 2026-08-26 — 🔖 Bump Forgery version to 0.1.21

Routine version bump covering the import-conflict popup polish, app-wide ImGui style fixes, and
the reference-examples packaging fix -- required for ryztart's auto-update to fire.

## 2026-08-26 — 🐛 Ship Patina's reference example shapes in the pip wheel

Patina's 3 built-in reference shapes ("Cube (1x1x1)", "Smallest character", "Tallest character")
were failing to load on any real `pip install` of the Forgery wheel: `_REFERENCE_EXAMPLES_DIR`
already correctly pointed at `ryzom_forgery/examples/` (inside the installable package), but the
actual `.shape` files had only ever lived in a sibling `nel/tools/forgery/examples/` folder,
outside the package -- fine for a source checkout, silently missing from an installed wheel, since
setuptools only bundles files inside the package directory and only what
`[tool.setuptools.package-data]` explicitly lists. Moved just the 3 needed `.shape` files
(`ge_mission_1_caisse.shape`, `npc_dummy_short.shape`, `npc_dummy_tall.shape`, ~1.3MB total) into
`ryzom_forgery/examples/` and added `"examples/*.shape"` to `pyproject.toml`'s package-data --
no code change needed, the path formula was already correct for that location.

## 2026-08-26 — ✨ Import-conflict popup polish, app-wide ImGui style fixes

Follow-ups from live-testing the import-conflict popup (`_draw_import_conflict_popup()`, Step 3 of
the auto-export imports/ chantier):

- The popup used to open unconditionally whenever the target shape was the one open in the
  viewport, regardless of whether there was actually anything unsaved. New
  `_has_unsaved_changes_at()` serializes the in-memory shape to bytes (`save_shape()` into an
  `io.BytesIO()`, no disk write) and compares against a fresh read of the target file -- exact,
  without needing to track every individual edit throughout the editor. The popup now only opens
  on a real conflict; otherwise it auto-updates silently, same as when the shape isn't open at all.
- Reworded the 3 action buttons, color-coded them (orange/pink/green, new `_colored_button()`
  helper with derived hover/active shades) and centered them (new `_center_next_widget()` helper).
- App-wide ImGui style fixes in `ForgeryApp.__init__` (`app.py`, shared by every Forgery tool app):
  rounded corners (`frame_rounding`), forced pure white text and fully opaque window/popup
  backgrounds (both were left at Dear ImGui's slightly-translucent stock defaults, never touched
  before, so every popup in every Forgery app always looked a bit hazy). Chasing a pushed
  pure-black button text color rendering as `0x0A` instead of `0x00` led to the real culprit:
  `ModalWindowDimBg` (meant to only dim whatever's *behind* a modal popup) was blending over the
  modal's own content too in this rendering pipeline -- confirmed by the arithmetic matching
  exactly (`0.4 * 0.1 + 0.6 * 0 = 0.04` -> `0x0A`). Disabled entirely rather than toned down, so
  popup colors/text now render exactly as set; the trade-off is no more dimming of whatever's
  behind a modal popup.

## 2026-08-26 — 🔖 Bump Forgery version to 0.1.20

Routine version bump covering this batch of Patina changes (Panoply masks/always-show/add-mask,
scan performance work, splash timing fix) -- required for ryztart's auto-update to fire.

## 2026-08-26 — 🐛 Keep startup splash open until the first real UI frame

`Splash.close()` (`splash.py`) used to be called at the end of `ForgeryApp.__init__` -- before
`ShowBase`'s own `run()` loop actually fires its first frame, so the splash could disappear a
moment before anything was really on screen. Now `ForgeryApp` keeps the `Splash` instance open
(`self._splash`) and only closes it on the very first `draw_ui()` call instead, so its `min_duration`
floor (1.2s by default) is measured against real elapsed time up to the first visible frame, not
just up to the end of Python-side construction. `close()` itself is unchanged: it only sleeps for
whatever's left of `min_duration`, so a slow-enough startup (already past that floor on its own)
still closes immediately with no extra wait.

## 2026-08-26 — ⚡ Skip external rescan on workspace change, persist scan cache, incremental scan

Three related performance fixes to `search_paths_dialog.py`'s asset scanning, all found and
root-caused through real-machine profiling:

- A texture/mask copy into the active workspace was triggering a full external-search-paths
  rescan, even though only the workspace (already covered by its own watchdog observer) had
  changed. Split scanning into two independent halves (`_ScanResult` for `_external_result`/
  `_workspace_result`, merged via `_merge_and_publish()`) so a workspace-only change
  (`_reload_workspace_only()`) never re-walks the external paths.
- The external scan's dominant cost is the directory walk itself, re-paid on every app startup.
  The scan result is now persisted to disk (`external_scan_index_cache.json`) and loaded
  synchronously at startup for an instant (if possibly slightly stale) index, while a real full
  scan always still runs in the background afterwards to self-correct.
- Converting that background scan to a per-frame time-sliced generator (to avoid GIL contention
  with the render thread -- see below) initially made things *worse*: `.anim` parse failures for
  several `CTrackKeyFramer*` classes `pynel.ryzom_animation.parse_animation()` doesn't support
  were never cached, so every scan re-attempted and re-failed the same expensive parses forever.
  Fixed by caching failures too, not just successes.
- The scan itself was moved from a background thread to a generator advanced a small
  (~2ms) time-bounded slice per frame on the main thread -- a background thread doing this much
  CPU-bound dict/object-building work contends for the GIL with the render thread badly enough to
  drag FPS well under 30 for the scan's whole duration. The incremental version trades wall-clock
  scan time for never blocking the UI thread.
- A further ~1.2s freeze was found right at the *end* of a full scan: `_save_external_index_cache()`
  (JSON-serializing the ~10^5-entry index and writing it to disk) ran synchronously in a single
  frame. Moved that save (plus the smaller `save_scan_cache()`/`save_bnp_table_cache()`) to a
  background thread -- safe here (unlike the scan itself) since it's a one-shot ~1s burst once per
  finished scan, not sustained multi-second background work.

## 2026-08-26 — ✨ Panoply masks/ folder, always-show section, add-mask button

Several Panoply-related UI improvements to Patina's Textures/Materials tabs:

- The "Panoply" section in `_draw_global_panoply_section()` used to draw nothing at all when none
  of a shape's textures had a detected variant, giving no indication Panoply was even checked --
  it's now always shown, with a disabled explanatory message when nothing matches.
- New `masks` workspace subfolder (`workspaces.SUBDIRS`). `_draw_panoply_masks_for()` now shows a
  copy/edit button next to each resolved mask thumbnail (reusing the same
  copy-to-workspace/edit-in-image-editor mechanism as a real texture, generalized with a `subdir`
  parameter), and a "+" button offering to create a mask (solid black, same pixel size as the base
  texture, written straight to the workspace's `masks/` folder) for any Panoply axis that doesn't
  have a resolved mask file yet -- independent of whether `panoply_files.txt` already declares
  that axis, since turning a plain texture into a panoplied one from scratch is a normal workflow
  here.
- The same green "already in the workspace" border used on texture reference text fields is now
  also drawn around the thumbnail itself (`_draw_preview_workspace_border()`), for the simple
  material row, Multi Bitmap slots, and the new Panoply masks row alike.

## 2026-08-26 — ✨ Wexplorer: browsable workspace folders in the Explorer sidebar

Added the "Wexplorer": each active-workspace folder (`tex`/`shapes`/`anims`/`skels`/`exports`/
`imports`) now shows as its own expandable tree section in the Explorer sidebar, always
reachable regardless of where the Explorer is currently browsing. New `Explorer.pinned_folders`
(list of `(label, Path)`) + `_draw_pinned_folders()`/`_draw_tree_children()` in `explorer.py`:
unlike the main view's flat click-to-navigate `_draw_dir_contents()`, sub-folders expand in
place; file entries reuse the existing `_draw_leaf()`, so double-click-to-load and the
right-click command menu (Load in viewer/as bone-preview skeleton/animation...) work exactly
like the main view. `object_editor.py`'s `_on_active_workspace_changed()` now sets
`self.explorer.pinned_folders` to the active workspace's own subfolders.

Every folder in the Wexplorer shows its own visible-entry count (e.g. "shapes (12)") --
factored the already-duplicated directory-listing/caching logic in `explorer.py` into a shared
`_list_dir()` used by every listing method. The Wexplorer shows every file regardless of the
shared search/extension-filter state (that filter stays scoped to the main arborescence only);
texture files (`.tga`/`.dds`/`.png`/`.jpg`/`.jpeg`/`.bmp`) get a real thumbnail instead of a
generic icon.

Also moved `WorkspaceSetupDialog.draw_active_workspace_row()` (the workspace combo + reveal-in-
file-manager button) from the bottom of the right panel to the very top of the Explorer window,
via a new generic `Explorer.extra_header` hook (mirrors the existing `extra_toolbar` hook, keeps
Explorer itself workspace-agnostic). Final top-to-bottom order in the Explorer window: Current
Workspace row -> Wexplorer -> Refresh/Import toolbar -> search+filter+path bar -> Favorites ->
the main arborescence.

The Wexplorer auto-refreshes when the workspace changes on disk (this app's own writes
included), piggybacking on `search_paths_dialog.py`'s existing workspace-wide watcher instead of
adding a third `Observer`: new generic `on_workspace_changed` callback hook, fired alongside its
own `reload()` once the watch settles, wired to `self.explorer.refresh`.

Backfilled a gap where a workspace missing some `SUBDIRS` (e.g. a folder introduced in a later
Forgery version) only got them created when the workspace was re-picked interactively --
`_on_active_workspace_changed()` now also calls `ensure_structure()` itself, covering every path
into it (including `__init__` resuming a workspace already active from a previous session).

Two performance bugs found and fixed while testing this feature:
- `search_paths_dialog.py`'s workspace watcher reacted to *every* watchdog event type, including
  `'opened'`/`'closed_no_write'` -- pure read access (e.g. from `_load_shape()`'s own
  `item.read_bytes()` on a shape living under the watched workspace), not an actual content
  change. This pre-existing bug (predates this session) triggered a full search-path rescan on
  every single shape load, visibly slowing the UI -- only surfaced now because the Wexplorer
  makes loading many shapes in a row far more common. Fixed with an event-type filter (same
  approach as the Auto-export watcher's own handler, see below).
- Expanding a `tex/` folder decoded every texture thumbnail synchronously on the main/render
  thread the first time -- fine for many small files, but a real 4096x4096 Ryzom texture took
  over a second to decode, visibly stalling the UI (raw PNG decode cost, unlike the `.dds`
  format real shape textures normally use, which loads near-instantly). Fixed by moving thumbnail
  decoding to a background thread per texture (own `_decode_thumbnail_worker()`, same pattern as
  `search_paths_dialog.py`'s own background scan), downscaling to 32px before the now-cheap GPU
  upload on the main thread.

## 2026-08-26 — ✨ Auto-export imports/ -> shapes/: headless mesh-import engine for Forgery workspaces

New `ryzom_forgery/import_watcher.py`: watches a workspace's `imports/` folder for created/
modified `.obj`/`.dae`/`.fbx` files (debounced watchdog `Observer`, own per-file debounce so one
file's write burst doesn't reset another's pending timer) and keeps `<workspace>/shapes/` in
sync automatically. Source file names are sanitized to a safe `.shape` base name (every
character outside `[A-Za-z0-9_-]` becomes `_`, case kept). If the target shape doesn't exist
yet, it's a full headless import (`export_new_shape()`, same path as `apps/shape_importer.py`'s
CLI). If it already exists and both meshes have the same material count, `update_existing_shape()`
replaces the geometry and updates only each material's diffuse texture reference where it
changed (`_update_diffuse_texture()`) -- every other material edit made in Patina (blend,
alpha-test, 2-sided, further Multi Bitmap stages...) survives. A material-count mismatch raises
`MaterialCountMismatch`, left for a future Patina UI to resolve (not yet implemented).

Wired into `object_editor.py` via a new `ImportWatcher`, started/restarted whenever the active
workspace changes. Since Patina has no dirty-edit tracking, silently auto-updating the shape
currently open in the viewport risked discarding unsaved work -- when the auto-export target is
that exact shape, a new confirmation popup (`_draw_import_conflict_popup()`) instead offers
"Save then import" / "Import without saving" / "Save as a backup copy, then import"
(auto-named `<name>_backup_<timestamp>.shape`) / Cancel.

Also centralized the `IMPORTERS`/extension-dispatch dict (previously redefined identically in
`import_dialog.py` and `apps/shape_importer.py`) into `shape_import.py` itself as the single
source of truth, and fixed 3 stale `docs/log.md` references there (moved to
`logs/forgery-object-editor.md` a while ago).

## 2026-08-26 — ✨ Reveal texture in file manager; edit-in-workspace image editor button

Two related additions to the Textures tab:

- `_draw_texture_preview_static()`'s thumbnail button was always force-disabled
  (`clicking it wouldn't do anything`); now clickable whenever the texture actually
  resolves to a real file on disk (`self._resolve_texture(name).fs_path is not
  None` -- still disabled for an unresolvable reference or one living inside a
  `.bnp`, which has no file of its own to show), and reveals it in the OS's file
  manager on click. New `workspaces.py` `reveal_in_system_file_manager()`: Windows
  (`explorer /select,`) and macOS (`open -R`) both select the file itself; Linux has
  no cross-desktop-environment equivalent, so falls back to
  `open_in_system_file_manager()` on the parent folder there instead.
- The per-texture "copy to workspace" button (added earlier this session) now turns
  into an "Edit" button once that texture already lives in the workspace -- copying
  it again would be a no-op, so launching an external editor on it is the only
  useful action left. `Settings.image_editor_path` (new field) holds the
  configured editor executable, picked via a new "Tools" section in the Settings
  tab (`_draw_image_editor_settings()`, same `portable_file_dialogs.open_file()`
  poll pattern as the Skinning preview's `.skel`/`.anim` pickers); the Edit button
  stays disabled with an explanatory tooltip until one is set.

## 2026-08-26 — ✨ Overlay the material index on its texture preview instead of a separate label

In the Textures tab, each material row showed a separate `#N` label next to its
color/texture swatch, taking up horizontal width this (already narrow) panel doesn't
have to spare -- same issue for the Multi Bitmap editor's own per-material rows, which
also had an `imgui.indent()` around them eating further left margin for no real
navigational benefit (there's nothing else at that indent level to distinguish it
from).

Removed both labels-as-separate-text and the Multi Bitmap indent; added
`_draw_preview_badge()`, which overlays the index directly on the swatch/thumbnail
that was just drawn (bottom-right corner, at 70% of the normal font size, 1px black
shadow for legibility over any thumbnail color) via `ImDrawList.add_text()`'s
explicit font+size overload -- draws at any size regardless of the currently pushed
font, while `calc_text_size()` only ever measures at the current (full) one, so the
small size's own footprint is derived by scaling that measurement down (exact, not an
approximation: glyph metrics scale linearly with size in ImGui's font atlas).

## 2026-08-26 — ✨ Restore last folder/workspace/shape on next launch

`Settings` gained `last_folder`/`last_bnp`/`last_shape_path`/`last_shape_bnp`/
`last_shape_name` (`settings.py`) -- `active_workspace` didn't need anything new, it's
already persisted live on every change. `_save_session_state()` writes these from the
Explorer's current `root`/`_current_bnp` and the loaded shape's `_shape_source_path`/
`_shape_source_bnp_path` (new field, tracks `ExplorerItem.bnp_path` alongside the
existing `_shape_source_path`, which is deliberately `None` for a `.bnp`-loaded shape
-- without also keeping `bnp_path` there was no way to reconstruct that case on
restore). `_restore_session_state()`, called once at the end of `ObjectEditorApp.__init__()`,
reverses this: navigates the Explorer back, re-enters the `.bnp` if one was open, and
reloads the shape via a reconstructed `ExplorerItem`. Best-effort throughout -- a
moved/deleted folder or shape is silently skipped, not an error.

Wiring this to actually run on every exit path took a second pass: the in-app Quit
button's own `self._save_session_state()` call (right before `self.userExit()`) never
fired when the OS's own window-close control (X button/Alt+F4) was used instead,
because `ShowBase.windowEvent()` (Panda3D's own base implementation, called via
`super().windowEvent(win)` in `app.py`'s override) already calls `self.userExit()`
itself the instant it sees the window's `getOpen()` go `False` -- and `userExit()`'s
`sys.exit()` unwinds straight out of that `super()` call, skipping everything placed
after it in the override, including a `getOpen()` check that was tried first and never
actually reached. Replaced with Panda3D's own `exitFunc` hook (`self.exitFunc =
self._on_exit` in `ForgeryApp.__init__`) -- the one thing `userExit()`/`finalizeExit()`
is guaranteed to call before actually exiting, regardless of which of the two paths
triggered it. `ObjectEditorApp._on_exit()` overrides it to call
`_save_session_state()`; the Quit button now just calls `self.userExit()` directly,
same as it always did.

## 2026-08-26 — 🐛 Run the startup splashscreen as a separate process, not a thread

The splashscreen (added 2026-08-21 below) ran Tk/Tcl on a background thread inside the
main Forgery process. This reliably crashed (`Tcl_AsyncDelete: async handler deleted by
the wrong thread`, `abort (core dumped)`) once this same process started spawning more
of its own background threads elsewhere (the workspace filesystem watcher and its
debounced reload worker, added the same session -- see below): Tcl/Tk is not
thread-safe at all, and while `splash.py`'s own thread created and destroyed its `Tk()`
root on the same thread throughout, the `tkinter` module's own `_default_root` global
(and/or delayed cyclic-GC of lingering Tk-wrapped Python objects) could still end up
finalized from whichever *other* thread happened to trigger garbage collection at the
wrong moment. Tried clearing `_default_root`/forcing `gc.collect()` and blocking
`close()`'s thread-join without a timeout first -- reduced how often it happened but
didn't eliminate it.

Settled on the actually-robust fix: moved the whole splashscreen into its own OS
process. New `ryzom_forgery/_splash_process.py` (a standalone script, no threading of
its own at all -- just `tk.Tk()` + `mainloop()` on that process's own main thread) is
launched via `subprocess.Popen([sys.executable, "-m", "ryzom_forgery._splash_process",
...])` from `splash.py`'s `Splash.__init__()`; `close()` now just waits out
`min_duration` then `terminate()`s + `wait()`s on that process. No IPC of any kind
needed -- the child has no self-close timer, the parent simply kills it when ready.
This also happens to be the only fix that would ever have worked on macOS, where a GUI
toolkit is restricted to a process's own *main* thread outright (a background thread
could never satisfy that constraint, regardless of any Tcl-specific fix) -- a separate
process sidesteps both constraints for the same reason: there's no way for this
process's other threads to interact with a different process's Tcl interpreter at all.

## 2026-08-26 — ✨ Per-texture "copy to workspace" button; keep valid absolute texture paths on import

Two related changes, both about a texture reference not necessarily living in the
active workspace's own `tex/` folder:

- `shape_import.py`: an imported `.obj`/`.dae`/`.fbx`'s texture reference used to
  always collapse to just its bare file name (`_texture_base_name()`), discarding a
  relative/absolute path even when it resolves to a real file right now -- meaning the
  shape would render untextured immediately after import unless that exact bare name
  also happened to exist somewhere in the configured search paths. Now resolves the
  reference against the source file's own folder (`base_dir`, threaded through
  `build_mesh()`/`_build_material_from_assimp_material()`) if it isn't already
  absolute, and keeps the full resolved path as `Texture.file_name` when it points at
  a real file, falling back to the bare name only when there's nothing on disk to
  preserve. `shape_geometry.resolve_texture_ref()` gained matching support for
  resolving `name` as an absolute path directly (bypassing the by-name search
  entirely), with a stale/moved absolute reference falling back to a by-name search on
  its own basename rather than failing outright.
- `object_editor.py`'s Textures tab: rather than auto-copying every referenced texture
  into the workspace on Save (tried first, reverted -- would silently clutter
  `tex/` with textures nobody actually chose to bring in for editing), added an
  explicit per-texture "copy to workspace" icon button (simple textures and each of
  the 3 Multi Bitmap masks alike), disabled once that texture already lives there. The
  texture reference's own text field is colored green when it currently resolves
  inside the active workspace's `tex/`, the normal color otherwise.

## 2026-08-26 — ✨ Save now always targets the active workspace, even for a .bnp-loaded shape

`_write_shape()` used to overwrite `_shape_source_path` -- wherever the shape was
*opened* from on disk -- contradicting `_draw_bottom_bar()`'s own docstring, which
already claimed Save "always targets the active workspace" (leftover from an earlier,
never-finished pass at the Workspaces chantier). A shape loaded from inside a `.bnp`
had Save disabled outright, since there was no real on-disk path to overwrite.

Added `_workspace_shape_save_path()` (`<active workspace>/shapes/<name>.shape`, `None`
without an active workspace or a known shape name) and switched the Save
button/overwrite-confirmation flow to use it instead of `_shape_source_path` --
Save now works for a `.bnp`-loaded shape too, always writing a fresh copy into the
workspace rather than touching the original source.

## 2026-08-26 — ✨ Watch the active workspace folder for external changes

Added `watchdog` as a dependency; `search_paths_dialog.py`'s `set_workspace_dir()` now
(re)starts an `Observer` on the new active workspace folder (native OS APIs --
inotify/FSEvents/ReadDirectoryChangesW, the most robust cross-platform option),
stopping any previous one first. A new `_DebouncedReloadHandler` coalesces a burst of
filesystem events (extracting a `.bnp`, a git checkout, a texture batch export from
another tool...) into a single `reload()` call 0.5s after the last event, instead of
one rescan per individual file event. A watch failure (e.g. an exotic filesystem
`inotify` can't watch) is logged and otherwise ignored -- external changes just won't
auto-refresh then, no worse than before this existed.

## 2026-08-26 — ✨ Add `imports/` to a workspace's standard subfolders

`workspaces.py`'s `SUBDIRS` gained `imports` alongside the existing
`tex`/`shapes`/`anims`/`skels`/`exports`. Since `ensure_structure()` (which creates
every `SUBDIRS` entry) was previously only ever called from `create_workspace()`, a
workspace created before this change would never get the new folder -- 
`workspace_setup_dialog.py`'s `set_active_workspace()` now also calls
`ensure_structure()` on every activation (idempotent), so switching back into an
older workspace backfills any folder introduced since it was first created.

## 2026-08-26 — ✨ Show version and authors in the status bar; don't reset locked transform axes

Two small, independent additions:

- `sysinfo.py`'s `SysInfoBar` now shows `"Ryzom Forgery v{version} -- Ulukyn,
  Claude@anthropic"` right-aligned in the bottom status bar, `version` read once via
  `importlib.metadata.version("ryzom_forgery")` (falls back to `"dev"` if not
  installed as a package).
- `object_editor.py`'s transform reset (the gizmo's Home button, and each panel row's
  own Reset button) used to always reset position/rotation/scale to their defaults
  regardless of any per-axis lock (`self.transform_locks`) -- the same lock
  `_set_transform_axis()` already respects for manual edits and `camera.py`'s
  Ctrl-drag. Added `_reset_node_transform()`/`_node_transform_values()`/
  `_set_node_transform()`/`_quat_to_prh()` helpers so a reset now only overwrites
  axes that aren't currently locked, leaving locked ones at their existing value.

## 2026-08-21 — ✨ Add app icon and startup splashscreen to ForgeryApp

Added to `ryzom_forgery/app.py`, the shared `ForgeryApp` base class every tool app
(including Patina/`object_editor.py`) inherits from, so both apply automatically
everywhere:

- Window icon: `forgery.png` (128x128, at the `nel/tools/forgery/` root) set via
  `WindowProperties.setIconFilename()` when building the window's initial properties.
- Startup splashscreen: new `ryzom_forgery/splash.py` module, `Splash` class. Shows
  `splashscreen.png` (512x512, same root) in a borderless, always-on-top Tkinter
  window, running on its own thread with its own Tk mainloop so it keeps repainting
  while the caller thread does the actual (blocking) Panda3D startup work. Centered on
  the app's last known window position (`center_on`, from the geometry already saved
  per-app in `~/.ryzom_forgery/*.json`) rather than on `winfo_screenwidth/height()`,
  which reports the whole virtual desktop (not a single monitor) on multi-monitor X11
  setups and would otherwise center the splash right at the seam between two screens.

Tried and discarded two approaches to also hide the real Panda3D window itself while it
loads underneath the splash: `WindowProperties.setOpen(False)` (only honored if set at
window *creation*, and even then some window managers flash it mapped for a frame
before the hide request lands) and creating it at an off-screen origin/minimum size
(window managers are free to ignore position/size requests entirely -- e.g. tiling WMs
enforce their own layout). Since neither is reliably honored across window managers,
settled for the simplest option instead: `ShowBase.__init__()` (which creates the real
window) is delayed by a flat 1-second `time.sleep()` after the splash is already up, so
the splash has already fully rendered before the real window even exists, whatever the
window manager then does with it. `Splash.close()` still enforces its own 1.2s minimum
display duration on top of that, mainly to cover the rest of a tool app's own init work
(explorer, lights, icon font, etc).

## 2026-08-19 — ✨ Auto-detect edited Panoply sources instead of a Reload button

Phase A Step 5 of the "génération live des textures Panoply" chantier (see
`.todo/forgery-object-editor.md`): decided with the user against adding a manual
"Reload" action -- a real auto-detection makes one redundant, so instead made the
mtime-based freshness check (Step 3/4) actually run continuously instead of only once
at first resolution. New periodic task `_update_panoply_freshness()` (registered
alongside `_update_wind()`/`_update_skin_preview()`, throttled to about once a second
via a `task.time` gate, same `taskMgr.add()` pattern already used for those): re-
resolves every Panoply-affected texture name currently in play
(`self._panoply_texture_sources`, newly recorded by `_resolve_panoply_texture_name()`
on every call) against its current on-disk sources, compares that to the signature
recorded when it was last resolved (new `_panoply_freshness_signature()` -- base/mask/
baked mtimes, distinct from `panoply_live.is_baked_stale()`'s "is this good enough"
check, since a live-but-not-stale combination must NOT be re-evicted every tick just
because it still has no baked file backing it), and on a mismatch evicts it from
`self._texture_cache` and forces every material to re-apply. `_ensure_live_panoply_texture()`
and the new resolution-refs logic were factored out into a shared
`_resolve_panoply_refs()` so both the initial resolution and the periodic re-check
resolve names identically.

While building the standalone test harness for this (calling the real
`ObjectEditorApp` methods against a fake `self` + real copied `ryzom-data` files,
without booting the whole ShowBase/imgui app), caught and fixed a real bug the
earlier, narrower Step 4 tests hadn't exercised: `_ensure_live_panoply_texture()` was
passing `_panoply_selection`'s raw dims values (a bare int for `user`/`hair`/`eyes`,
e.g. `7`, and a lowercase race code for `skin`, e.g. `"zo"`) straight to
`panoply_config.get_color_params()`, which actually expects the same `"U7"`/`"ZO"`-style
`color_id` strings the real `.cfg`/`panoply_colors.toml` use -- silently returning
`None` and aborting every live compute before it started. Fixed by extracting
`panoply.py`'s existing (but private) axis-letter-prepending logic out of
`variant_file_name()` into a new public `panoply.color_id_for(axis, value)`, used by
both `variant_file_name()` (unchanged behavior) and `_ensure_live_panoply_texture()`
(the fix). Validated end-to-end on the real machine: a first resolution live-computes
and caches a texture with no baked file backing it; an unrelated freshness tick
doesn't evict it; editing a copy of a real mask file (content + mtime) gets detected
on the next tick, evicting the cache entry and triggering exactly one reapply; re-
resolving afterwards recomputes a texture that's actually pixel-different from the
first (confirming the edit was picked up, not just cache-missed); a further tick with
nothing new changed doesn't reapply again.

## 2026-08-19 — ✨ Wire live Panoply recoloring into texture resolution

Phase A Step 4 (final step of Phase A) of the "génération live des textures Panoply"
chantier (see `.todo/forgery-object-editor.md`): object_editor.py can now render a
Panoply variant it never found baked on disk, or that's older than the base texture or
mask it should have been built from, by recomputing it live in memory. New
`ryzom_forgery/panoply_texture.py`: Panda3D glue decoding a resolved texture reference
into the HxWx4 uint8 RGBA array `panoply_colorize.py` expects (`ref_to_rgba_array()`,
via a throwaway `Texture` + `get_ram_image_as("RGBA")`) and building a `Texture` back
from a recolored result (`rgba_array_to_texture()`, `set_ram_image_as(..., "RGBA")`) --
validated on the real machine that the two agree with each other and round-trip
losslessly (both against synthetic data and a real texture file), even though the row
order they use internally turned out to be vertically flipped relative to `PNMImage`'s
own top-down one -- never an issue since every `panoply_colorize` operation is
per-pixel, not spatial. `panoply_colorize.py` gained `colorize()`, chaining
`convert_bitmap()` once per selected axis the same way `panoply_maker.cpp` applies
successive masks in place. `shape_geometry.py`'s `load_panda_texture()` had its
name-resolution step (search_dirs then `finder`) extracted into a new
`resolve_texture_ref()`, reused by the new code without duplicating that logic or
reaching into a private helper.

The actual wiring lives in `object_editor.py`: `_resolve_panoply_texture_name()` now
also calls a new `_ensure_live_panoply_texture()`, which -- only when the resolved
variant is missing/stale (`panoply_live.is_baked_stale()`) and every mask the current
axis selection needs is actually available -- recolors the base texture live
(`panoply_config.py` for the real target hue/lightness/saturation/luminosity/contrast,
keyed by the base texture's 2-letter race prefix for hair/eyes) and pre-inserts the
result into `self._texture_cache` under the resolved name itself. Every existing
caller of `load_panda_texture(resolved_name, cache=self._texture_cache, ...)` -- 3D
render, thumbnails, color-picker popups alike -- then picks it up transparently
through that cache's own name lookup, with no special case of their own; a no-op
(falls through to whatever's on disk) whenever a needed source can't be resolved.
Never writes to disk. Raw recolored arrays are memoized in the (Step 3)
`LiveColorizeCache` so a shape reload doesn't force a recompute for an unchanged
combination. Validated on the real machine: the full chain (decode real base + skin +
user masks from `ryzom-data`, colorize with real `panoply_config.py` parameters,
rebuild into a `Texture`, store back to a `PNMImage`) run end-to-end -- alpha
preserved exactly, a real, plausible color shift where the masks are strong, no shift
where they're weak, and the final stored image spot-checked pixel-for-pixel against
the in-memory result with zero mismatches.

## 2026-08-19 — ✨ Add freshness check and cache for live Panoply recoloring

Phase A Step 3 of the "génération live des textures Panoply" chantier (see
`.todo/forgery-object-editor.md`): new `ryzom_forgery/panoply_live.py`, pure logic with
no disk I/O of its own -- callers (Step 4's wiring) pass already-resolved
`search_paths.FoundEntry`-like objects for the baked variant (if any), the base
texture, and each relevant mask. `is_baked_stale()` says whether a live recompute is
needed at all: true if nothing baked resolved on disk, or its mtime is older than the
base texture's or any relevant mask's (`cache_stat()`, the same signal
`search_paths.py`'s own scan cache already relies on to detect changed files).
`LiveColorizeCache` memoizes one computed image per (base texture, axis selection,
source mtimes) key, so an edited base/mask file naturally misses the cache instead of
serving a stale result, without needing an explicit invalidation call -- and a session
only ever accumulates one entry per distinct combination actually viewed, small enough
in practice that no eviction was added. Validated on the real machine via the
`.agentcom` bridge with fake ref objects covering: no baked file, baked older/newer
than base, baked newer than base but older than a mask, baked mtime exactly equal to
base's (treated as fresh), and cache key stability/change across repeated vs. edited
vs. differently-selected combinations.

## 2026-08-19 — ✨ Bundle Panoply's real color palette into Forgery

Phase A Step 2 of the "génération live des textures Panoply" chantier (see
`.todo/forgery-object-editor.md`), revised from the original plan of parsing
`panoply_common.cfg`/`panoply_<race>.cfg` live from the user's search paths: instead,
consolidated the real production palette (read from
`ryzom-data/leveldesign/workspace/common/characters_maps_hr/panoply_common.cfg` +
`panoply_fyros/matis/tryker/zorai/generique.cfg`) once into a new bundled
`ryzom_forgery/panoply_colors.toml`, so the feature doesn't need the user to add
`ryzom-data`'s leveldesign tree to their search paths just to read these constants.
`skin`/`user` (common to every texture) live at the TOML's top level; `hair`/`eyes`
(race-specific -- zorai has no `eyes` axis, `generique` has neither) live under a
per-race table (`fyros`/`matis`/`tryker`/`zorai`), matching the real
`panoply_maker.cpp` pipeline's own common+race `.cfg` pairing
(`ryzom-data/leveldesign/workspace/common/characters_maps_hr/directories.py`'s
`MapPanoplySourceDirectories`). New `ryzom_forgery/panoply_config.py` loads it
(`tomlkit`, matching `settings.py`'s existing TOML usage) and exposes
`get_color_params(axis, color_id, race=None)` / `available_color_ids(axis, race=None)`,
plus `RACE_PREFIX_TO_TABLE` mapping a base texture's 2-letter file name prefix
(`fy_`/`ma_`/`tr_`/`zo_`/`ge_`) to the matching race table. Validated on the real
machine via the `.agentcom` bridge: all 60 values across every axis/race/color_id
compared byte-for-byte against a from-scratch regex parse of the 5 real source `.cfg`
files (0 mismatches), plus confirmed zorai has no `eyes` entries and `generique` has
neither `hair` nor `eyes`.

## 2026-08-19 — ✨ Add pure NumPy port of Panoply's color-shift algorithm

New `ryzom_forgery/panoply_colorize.py` (Phase A Step 1 of the "génération live des
textures Panoply" chantier, see `.todo/forgery-object-editor.md`): a pure, I/O-free
NumPy port of the color-shift algorithm real Panoply variants are baked with --
`nel/tools/3d/panoply_maker/color_modifier.cpp`'s `CColorModifier::evalBitmapStats()`/
`convertBitmap()`, plus the underlying HLS conversions from
`nel/src/misc/rgba.cpp` (`CRGBA::convertToHLS`/`buildFromHLS`, `CBGRA::blendFromui`).
Takes a base texture + one grayscale mask + the 5 target parameters (hue/lightness/
saturation/luminosity/contrast) and returns the recolored image, matching how
`panoply_maker.cpp` chains masks in place across axes
(`cm.convertBitmap(resultBitmap, resultBitmap, mask, ...)`). One deliberate departure
from the C++: `evalBitmapStats`'s hue average uses a standard intensity-weighted
circular mean (sin/cos vector average) instead of the original's order-dependent
running-unwrap trick, since the vectorized form needs an order-independent formula --
this is the textbook way to average circular quantities, not an approximation of the
original. Not aiming for bit-exact parity (accepted with the user, see the chantier
notes) -- validated instead with targeted sanity checks run on the real machine via
the `.agentcom` bridge: mask=0 is a no-op (exact pixel identity), mask=255 with
current-stats parameters (no actual shift) only introduces ~1/255 truncation noise,
RGB->HLS->RGB roundtrips similarly within ~1/255, and pure red/green/blue map to the
expected 0°/120°/240° hues.

## 2026-08-19 — ✨ Show Panoply mask thumbnails under panoplied textures

Added `_draw_panoply_masks_for()`: below a panoplied texture's own row in the Materials
tab, shows thumbnails of the grayscale masks (mask weight in the red channel)
`panoply_maker` actually baked this texture's variants from -- e.g. for
`tr_hom_armor00_epaule_c1.tga`, the `tr_hom_armor00_epaule_c1_skin.png`/`..._user.png`
masks sitting in a sibling `mask/` folder next to the base texture, resolved through
the same search-paths lookup as any other texture. Only axes the texture actually has
Panoply variants for are checked (`search_paths_dialog.panoply_variants_for()`); if
none of their mask files resolve under the current search paths, nothing is drawn --
purely an informational aid for understanding how a variant was built, no functional
effect on rendering or editing either way. Wired into both the main per-slot texture
panel and the "simple" texture picker used by non-slot material fields.

## 2026-08-18 — ✨ Generalize Panoply to all 4 axes and fix small-shape framing

Generalized Panoply (`ryzom_forgery/panoply.py`) from the race+user-color pair alone to
all 4 axes real production data actually uses (found by reading
`nel/tools/3d/panoply_maker/panoply_maker.cpp` and the real `.cfg` files in
`ryzom-data/leveldesign/workspace/common/characters_maps_hr/`): `skin` (race skin
tone, renamed from the earlier "race" to match the real `skin_color_id` config key --
FY/MA/TR/ZO), `user` (item craft color, U1..U8), `hair` (H1..H6), `eyes` (E1..E8). Each
axis is independent -- an armor texture only ever carries skin+user masks, a hairstyle
carries skin+hair, a face texture carries skin+eyes; `hair`/`eyes` never apply to armor
and vice versa, purely because that's which mask files existed for a given base texture
at build time, not something Forgery has to special-case. Rewrote
`parse_panoply_files()` to strip whichever `_<color_id>` suffix tokens are actually
present, generically, instead of only recognizing the fixed `_<race>_u<N>` pattern --
this alone found panoply variants for 1013 base textures instead of 379 (every
hairstyle/face texture that has no race+user combo was previously invisible to the
tool entirely).

Selection UX: only `skin` can be deselected (falling back to the shape's true base
texture, clearing every other axis too since none of them resolve to a real file
without a skin picked); `user`/`hair`/`eyes`, once picked, can only be switched to a
different value, never turned off -- there's no real "..._U1.tga" file on disk without
a skin paired with it. The first axis picked from an empty selection now auto-fills
every other available axis with its first value, instead of resolving to a texture that
was never actually generated (e.g. picking only a skin used to try loading
"..._FY.tga", which doesn't exist -- only "..._FY_U1.tga" does -- and rendered blank).

Fixed hairstyle shapes rendering invisible: Panda3D's default lens near plane (1.0
world unit) clipped them out entirely once `OrbitCamera.frame()`'s distance (bbox
radius, floored at 0.1, times 5.65) landed under 1.0 for a hairstyle's thin bbox --
the whole mesh sat behind the near plane. `ForgeryApp.__init__` (`ryzom_forgery/app.py`)
now sets `camLens.set_near_far(0.02, 20000.0)` explicitly, with real headroom for
Ryzom's actual asset scale range (tiny props to whole ecosystems) instead of relying on
Panda3D's one-size-fits-all default.

## 2026-08-18 — 🐛 Fix texture preview corruption and make Panoply shape-wide

Fixed `_get_preview_texture_ref()` (thumbnails/tooltips in the Textures and Materials
tabs): it dropped alpha via `Texture.set_format(F_rgb)` on an already-decoded texture,
which only relabels the stored ram image's component count without repacking its bytes
from 4 to 3 per pixel -- every pixel after the first then read shifted by one channel,
producing a scrambled checkerboard look for any texture with an actual alpha channel
(found on `tr_hom_armor00_epaule_c1.png`, a plain valid RGBA PNG). Replaced with a
`PNMImage` round-trip (`store()` -> `remove_alpha()` -> `load()`), which repacks the
pixel data correctly and also works for a compressed source texture.

Reworked the Panoply feature (added earlier this session) after realizing its design was
wrong: race and user-color are not a per-material choice, they're shape-wide -- race is a
skin-tone difference (Fyros tanned, Matis pale, Tryker in between, Zorai blue) and user
color is the item's craft color, both meant to be uniform across a whole equipped piece.
Replaced the per-`(material_id, slot)` `_panoply_selection` dict with a single shape-wide
`(race, user_color)` tuple (or `None`), and `_resolve_panoply_texture_name()` now just
checks whether the specific texture being resolved has a variant for that pick, falling
back to the base name untouched otherwise (expected for parts that don't change color,
e.g. a buckle or a weapon). The picker itself (`_draw_global_panoply_section()`) is now
drawn once, at the top of both the Textures and Materials tabs, instead of repeated under
every material row -- the previous per-material placement had also silently never
included Multi Bitmap materials (armor pieces, almost always Multi Bitmap, never got a
Panoply section at all). Race buttons offered are the union of every race with a variant
across every texture the shape currently uses; user-color buttons for the selected race
are now on their own line below the race buttons rather than trailing on the same line.
Selecting a race/user-color now re-applies every material (`_reapply_all_materials()`,
new), not just one, since the shape-wide change can affect any of the shape's textures at
once.

## 2026-08-18 — ✨ Add Panoply support and unify Forgery settings into one TOML file

Unified the three separate JSON config files under `~/.config/ryzom_forgery/`
(`export_settings.json`, `explorer_settings.json`, `search_paths_settings.json`) into a
single `ryzom_forgery/settings.py` module (`Settings`/`ExportSettings`/`SearchPathDir`
dataclasses) persisted as one hand-editable `settings.toml`, via `tomlkit` (new
dependency, chosen over `tomli-w` for its comment/formatting-preserving round-trip). No
migration path from the old JSON files -- explicitly a one-time, single-machine
transition; they're left untouched on disk but never read again. Every settings-owning
component (`ExportDialog`, `Explorer`, `SearchPathsDialog`) reloads the full `Settings`
fresh before writing its own section back, so concurrent edits to different sections
never clobber each other.

Removed the separate `AssetIndex`/"data root" texture-resolution mechanism
(`ryzom_forgery/asset_index.py`, deleted) entirely, merging it into the existing
priority-ordered "Paths" search-path list -- a folder with "recursive" on covers the same
case a dedicated data root did. `load_panda_texture()`, `shape_export.py`,
`shape_exporter.py` (CLI) and `export_dialog.py` now take a generic `texture_finder`
callable instead of an `AssetIndex`. Added up/down reorder buttons per folder in Settings
> Paths, since priority order now determines which folder's copy of a texture wins.

Added Ryzom's "panoply" armor-texture-variant system (`ryzom_forgery/panoply.py`): a
material texture like `tr_hof_armor00_handupside_c1.tga` can have real per-race
(`fy`/`ma`/`tr`/`zo`) x per-user-color (`u1..uN`) on-disk variants
(`<base>_<race>_u<N>.tga`), listed in `characters_maps_hr.bnp@panoply_files.txt`. When
that list is reachable via the search paths and the current material's texture has
variants in it, a "Panoply" section (race buttons, then user-color buttons for the
selected race) now appears after the texture in both the Textures and Materials tabs.
Selecting a variant is a pure render-time override, tracked per `(material_id, slot)` --
it never touches the shape's own material data, but does affect both the live 3D
viewport and the Materials tab's preview thumbnail.

Search-path scan performance: added a scanned-file count + elapsed time to the Paths
status text, and a `.bnp` table-listing cache (`bnp_table_cache.json`) alongside the
existing `.skel`/`.anim` bone-name cache -- measured to not be the bottleneck on a real
~161k-entry tree (4.12s vs 5.13s baseline). The real cost was `pathlib.Path.is_dir()`'s
per-child `stat()` syscall and raw Python object overhead for the ~10^5 `FoundEntry`
instances built per scan; fixed by switching the directory walk to `os.scandir()`
(reuses `readdir()`'s cached file type) with an iterative stack instead of a
self-recursive generator, and making `FoundEntry` a `slots=True` dataclass (5.13s ->
3.6s).

Settings tab UI polish: sections are now collapsible (closed by default), export settings
save immediately on every field change instead of via a separate "Save" button, missing
icons added ("Add folder", "Choose folder...", relocated next to "Output folder:"), and
the object_editor Save button is now always visible but grayed out (with an explanatory
tooltip) when the loaded shape came from inside a `.bnp` archive, instead of being hidden
behind a "Save unavailable" text label.

## 2026-08-18 — ✨ Add skinning preview and generalized search paths

Two chantiers' worth of work, validated end to end against real creature data over the
`.agentcom` bridge (`tr_mo_zerx.shape`/`.skel`/`_baillement.anim`).

**Rigid rendering + compatible-skeleton detection**: a `CMeshMRMSkinned` no longer needs a
`.skel` loaded to render at all -- `shape_geometry.py` gained
`_passes_from_mrm_skinned_geom_rigid()`, a fallback that renders the mesh's raw bind-pose
local vertices (decompacted, unskinned), matching the real client's own fallback for an
unskinned `CMeshMRMSkinned` instance (`CMeshMRMSkinnedGeom::render()`'s "no skeleton" path,
`mesh_mrm_skinned.cpp:589-801`).

**Generalized, `.bnp`-aware search paths** (`ryzom_forgery/search_paths.py` +
`search_paths_dialog.py`, replacing the earlier `.skel`-only `skel_search_config.py`/
`skel_search_dialog.py`): user-configured folders (recursive or not; a `.bnp` sitting in a
scanned folder is always descended into, recursive or not, since it's a single filesystem
node) used to find `.skel`/`.anim` files compatible with the loaded shape/skeleton, and as an
extra fallback for texture resolution (`shape_geometry.py`'s `load_panda_texture()`). Scanning
runs on a background thread (`_reload_worker()`, kicked off automatically the first time a
shape is displayed) with a persistent on-disk cache
(`~/.cache/ryzom_forgery/skel_anim_scan_cache.json` on Linux, `cache_dir.py`) keyed by each
file's `(mtime, size)` -- an unchanged file is never re-parsed. Only each `.skel`/`.anim`'s
bone names are kept in memory for the compatibility lists; the full parse only happens once,
on demand, when the user actually picks one.

**"Skinning preview" window** (renamed from "Bone attach preview"): always visible once a
shape is loaded, with `Skeleton`/`Animation` combos listing every scanned candidate
(compatible ones sorted first and highlighted green), a "load from disk" icon button, and a
"Reload" icon button. The bone-attach mechanism itself (`_update_bone_preview`, the "Bone"
combo, `_SUGGESTED_ATTACH_BONES`) was removed entirely: its design didn't make sense (an
object attaches to a bone of an *external* character's skeleton, never its own) --
`.todo/forgery-object-editor.md` keeps a note on what a real redesign would need.

**Settings tab** reorganized into "Export"/"Paths" sections. **Bottom bar**: Save/Save As
(unchanged logic, just relocated), a new Export button (format picker, then the existing
`ExportDialog` flow -- now exports the shape's live in-memory state, edits included, not a
disk/bnp re-read; `export_shape()`/`ExportDialog.export()` dropped their now-redundant
`ExplorerItem`-based path once the Explorer's own "Export to .xxx" commands were removed), and
Quit (`base.userExit()`) flush right. The panel's window title shows the loaded shape's name
instead of the generic "Panel" (`ForgeryApp.panel_title()`, overridable per app).

Smaller fixes along the way: a real perf issue (5-30+ fps lost while an animation played,
traced to `evaluate_bone_world_matrix()` being called once per bone every frame -- fixed on
the pynel side, see `logs/pynel.md`), a duplicate ImGui id between the two "load from disk"
icon buttons, and the Explorer's double-click now runs whatever the right-click menu's first
command would (`CommandRegistry.commands_for_selection()` already preserves registration
order).

## 2026-08-18 — 🐛 Fix CMeshMRMSkinned rendering, blank textures, and Explorer UX (Step 4)

Real visual validation of the new CMeshMRMSkinned pipeline (`fo_carnitree.shape`+`.skel`,
tested live by the user on their own machine), which surfaced two real bugs:

- `_rebuild_geometry()` crashed on any skinned shape:
  `MeshMRMSkinnedGeom.vertex_program` doesn't exist (only `MeshGeom`/`MeshMRMGeom` have it --
  no wind animation support for skinned characters in NeL). Switched to `getattr(...,
  "vertex_program", None)`.
- The shape rendered fully white -- its `CTextureMultiFile` materials all have slot 0 (the
  stored "current" selection) empty; `pynel.ryzom_shape` faithfully preserves that as-is, so
  it's not a parsing bug, just real content whose default slot happens to be unpopulated (the
  real client picks its Multi Bitmap slot dynamically at runtime -- quality/ecosystem/season
  -- rather than trusting whatever was last saved as "current"). Added
  `_auto_select_multi_bitmap_slot()`, called once when a shape loads: if every material's
  currently-selected slot is empty, switches to the first slot that actually has a texture in
  at least one material, so the shape doesn't render blank by default.

Also fixed, from separate user feedback while testing:

- Explorer click behavior was inconsistent: a `.shape` auto-loaded on a plain single click,
  while `.skel`/`.anim` always needed a right-click command. Removed the single-click
  auto-load for `.shape` (`on_selection_changed()`) -- right-click -> "Load in
  viewer"/"Load as bone-preview skeleton"/"...animation" is now the one consistent way to
  load any of the three.
- Added file-type icons to the Explorer's leaf rows (`explorer.py`'s `_LEAF_ICONS`) --
  previously only folders/.bnp archives had one. First attempt used `ICON_FA_BONE`/
  `ICON_FA_WALKING` for `.skel`/`.anim`, silently invisible: those are Font Awesome 5
  additions, and the actual font file loaded here (`app.py`'s `_ICON_FONT_PATH`,
  `fontawesome-webfont.ttf`) is genuine FA 4.7, confirmed by parsing its `cmap` table
  directly (no `fontTools` available on the target machine to just ask a library) -- neither
  codepoint is present at all. Replaced with `ICON_FA_MALE`/`ICON_FA_FILM`, confirmed present
  in the same file the same way.
- Added a "Settings" tab (always visible, even with no shape loaded) holding the export
  settings that used to live behind a right-click "Export settings..." command -- global
  app preferences reachable via a right-click on a file read as out of place, since every
  other command there acts on the selected file. `ExportDialog._draw_settings_window()`
  (its own floating window, `open()`-triggered) became `draw_settings_content()`, embedded
  directly in the new tab instead.

## 2026-08-18 — ✨ Render and animate CMeshMRMSkinned shapes in object_editor (Step 3)

The main loaded shape can now be a skinned character/creature, not just weapons/props: when
it's `CMeshMRMSkinned`, `_rebuild_geometry()` reuses the bone-attach preview's own loaded
skeleton/animation state (no separate "load a skeleton for the main shape" flow -- one
skeleton context serves both features) to skin it via `_build_skin_state()` +
`_update_skin_preview()`. Without a skeleton loaded yet, a clear status message tells the
user to load one via the Explorer's right-click command, instead of the generic "no
renderable geometry" message.

`_update_skin_preview()` recomputes the skin every frame with a fully vectorized numpy path
(gather each vertex's up to 4 influence matrices, batched `einsum` for position and normal,
weighted sum, renormalize) rather than calling pynel's own `ryzom_skin.skin_vertex()` in a
per-vertex Python loop -- the same performance tradeoff already hit and documented for the
wind preview (`_update_wind()`'s own note: a real prop-sized mesh dropped 60fps to ~38fps
even with a *much* cheaper per-vertex formula than 4-bone blend skinning). Results are
written into the shared vertex buffer with the same buffer-protocol/modification-stamp
technique `_update_wind()` already established.

`_update_bone_preview()`'s time advancement was decoupled from having an attach bone chosen
-- a loaded skeleton + animation is now enough to drive the skin preview's playback, even
without also picking a bone for the separate weapon-attach use case.

Validated with a direct numerical cross-check against pynel's own (already-validated)
`ryzom_skin.skin_mesh()` on `fo_carnitree`: max deviation ~4e-7 (float32 noise), 0 vertices
out of tolerance. The first comparison attempt showed a ~2.9m discrepancy -- traced to a bad
test reference (comparing the finest lod's *resolved* vertices against the *unresolved*
`packed_vertices` list; this particular model's finest lod turned out to have 88 geomorph
placeholders, not zero as assumed), not an actual bug in the vectorized skinning.

## 2026-08-18 — ✨ Extend shape_geometry.py for CMeshMRMSkinned (Step 2)

`iter_render_passes()` gained optional `skeleton`/`bone_world_matrices` parameters, used only
for `CMeshMRMSkinned` (`_passes_from_mrm_skinned_geom()`, new): resolves the finest LOD's
geomorph placeholders (same idea as the existing `_resolve_lod_geomorphs()`, adapted to a
`PackedVertex` list instead of a plain channels dict), then skins via pynel's new
`ryzom_skin` module (`bone_skin_matrices_for_mesh()` once, `skin_vertex()` per vertex),
producing a regular `VertexBuffer` (Position/Normal/TexCoord0 channels) so callers don't need
to special-case skinned shapes downstream. Without a skeleton (not yet loaded by the caller),
a skinned shape simply yields no passes -- same as any other shape type with nothing
renderable, rather than raising. `shape_geom()`/`shape_bbox()` also extended (trivial: the
geom/bbox are already fully known without any skinning).

Validated via the bridge on the same `fo_carnitree.shape`/`.skel` pair used for
`ryzom_skin.py`'s own validation: correct bbox, 5 material passes, the expected channel set,
and confirmed empty when no skeleton is supplied.

## 2026-08-18 — ✨ Add a bone-attach animation preview (Steps 1-2)

New independent state (not tied to the loaded `.shape`): a `.skel` and `.anim` can be loaded
via new right-click Explorer commands (`.skel`/`.anim` added to `explorer.py`'s
`FILTER_PRESETS`), then a bone picked from a combo (known attach dummies -- `box_arme`,
`box_arme_gauche`, `Box_bouclier`, `stick_1` -- flagged with a `*` when present, but any bone
is selectable). A new floating "Bone attach preview" window (same positioning pattern as
`_draw_wind_controls()`, stacked below it) shows what's loaded plus Play/Pause and a time
scrubber once an animation is loaded.

Each frame (`_update_bone_preview()`, a `_update_wind()`-style task), the chosen bone's world
matrix is evaluated via pynel's new `evaluate_bone_world_matrix()` and applied to
`_object_pivot` with `NodePath.set_mat()` -- mirroring the engine's own
`CSkeletonModel::stickObject()` (the loaded object rigidly follows the bone, no full skeleton
displayed, per the earlier scoping decision). With no animation loaded, this just holds the
`.skel`'s static bind pose.

Converting pynel's row-major, column-vector matrix convention (`NLMISC::CMatrix`'s own
layout) to Panda3D's row-vector convention needed a transpose of the rotation block (rows
become columns) with the translation moved from the last column to the last row -- get this
backwards and the result looks plausible (loads, doesn't crash) but is subtly wrong, so it's
called out explicitly in `_update_bone_preview()`'s `Mat4(...)` construction rather than left
implicit.

Conflict with the existing Position/Rotation/Scale panel: whichever row currently targets
`_object_pivot` (see the panel's own pivot-lock) is disabled (grayed out, but still
live-reading current values) while the bone preview drives it, since editing it by hand would
either get silently overwritten next frame or fight with playback while paused.

Step 3 (visual validation via the bridge) is still open -- turned out to need a rendered
body for a meaningful test, which object_editor can't do yet (see the new "rendu
CMeshMRMSkinned" chantier). `evaluate_bone_world_matrix()` itself was already validated
structurally in `logs/pynel.md`.

## 2026-08-17 — 🐛 Fix the transform panel's row Reset and vertical position

Two follow-ups to the Position/Rotation/Scale panel added earlier the same session:

- Its per-row Reset button (`_icon_button(ICON_FA_UNDO)`) reset BOTH `_object_pivot` and
  `model_root` unconditionally, regardless of that row's pivot-lock state -- so resetting
  while the pivot was locked (editing `model_root`) also silently wiped out the pivot's own
  transform, and vice versa. Added `_reset_transform_row(prop)`, which only resets whichever
  node `_transform_node()` currently owns, leaving the other reference frame untouched.
  `_reset_transform()` itself is unchanged and kept for `reset_object_transform()` (the
  gizmo's Home button), which is deliberately a total reset of both frames at once regardless
  of any row's current lock state.
- `_draw_transform_panel()` was vertically centered on the navcube gizmo's own pixel rect;
  moved to align its bottom edge with `_draw_viewport_toggles()`'s bottom-left icon bar
  instead (same `win_h - sysinfo_height - margin - own_height` formula), so the two floating
  panels read as a matched pair along the bottom of the viewport.

## 2026-08-17 — ✨ Add a Position/Rotation/Scale panel to object_editor

New floating window in the viewport, flush against the left edge of the navcube gizmo's own
pixel rect, vertically centered on it (`ObjectEditorApp._draw_transform_panel()`, called from
`draw_panel()`, hidden until a shape is loaded). One row each for Pos/Rot/Scl, live real-time
values:

- A pivot-lock toggle (anchor icon): unlocked (default) edits `_object_pivot` itself, moving
  the object along with it -- the existing Ctrl+drag behavior. Locked routes edits to
  `model_root`'s own local transform instead, so the object moves within a pivot that stays
  put (`_transform_node()` picks which node a row currently owns).
- X/Y/Z lock toggles per row: a locked axis never changes, from the field or a Ctrl+drag.
  Locking scale's X, for example, leaves Y/Z free to resize while X stays fixed. Lock buttons
  now render in grey (`_LOCKED_COLOR`, a new `active_color` param on `_icon_button()`, default
  unchanged blue) instead of the same blue as every other toggle button, and a locked axis's
  input field is wrapped in `imgui.begin_disabled()` so it visibly greys out and stops
  accepting input rather than just silently ignoring edits.
- X/Y/Z value fields (`imgui.input_float`, `_get_transform_values()`/`_set_transform_axis()`):
  position in meters (3 decimals), rotation in degrees (`NodePath.get_hpr()`/`set_hpr()`,
  X/Y/Z = pitch/roll/heading; 2 decimals), scale as a multiplier (3 decimals) -- checked NeL's
  actual float precision (`CVector`/`CQuat` are both plain 32-bit `float`, ~7 significant
  digits) to ground these, rather than guessing a decimal count.
- A per-row reset button, resetting both `_object_pivot` and `model_root` regardless of which
  one is currently locked, so the result always reads as a clean reset. The navcube gizmo's
  Home button, while the object is targeted, now calls the same reset for all 3 rows at once
  (`reset_object_transform()`, replacing the old rotation-only `reset_object_rotation()`).

`ObjectManipulator` (`camera.py`)'s Ctrl+drag `_move()`/`_rotate()`/`_scale()` enforce the same
per-row pivot/axis locks (`_target_node()` mirrors `_transform_node()`'s own lookup) so dragging
respects them exactly like the text fields do:
- `_move()` converts the drag's necessarily-world-space delta into the target node's own local
  frame (`parent.get_relative_vector(render, ...)`) before zeroing locked axes and applying it,
  so "locked" consistently means the same local X/Y/Z the fields show, whether editing the
  pivot (parent = render, local already = world) or `model_root` (parent = the pivot, which may
  itself be rotated).
- `_rotate()` applies the drag's world-frame delta via `get_quat(render)`/`set_quat(render,
  ...)` (handles the pivot-vs-model_root parent-relative conversion automatically either way),
  then restores any locked axis's pre-drag local HPR component afterwards -- not a true
  constrained rotation, but matches what the per-axis fields already mean.
- `_scale()` now applies its drag factor per-axis, skipping locked ones, instead of always
  scaling all three uniformly.

## 2026-08-17 — ✨ Add drag-mode status/force icons to the navcube pad

Two new small icons in `NavigationCube.draw_controls()`'s directional pad, bottom-left and
bottom-right:

- `_draw_status_icon()` (bottom-right) shows which mouse-drag action is currently active --
  a pointer by default, or rotate/move/scale while the matching mouse button is held (left/
  middle/right, in either the camera or the object context). Clicking it cycles
  `app.forced_drag_mode` through pointer -> move -> rotate -> scale -> pointer; while forced
  (icon turns orange), a plain left-click-drag alone performs that action, without needing to
  hold the button that normally triggers it.
- `_draw_mode_icon()` (bottom-left) shows/controls `app.target_mode`: whether a drag targets
  the camera or the object. Clicking it cycles Ctrl-decides (the normal behavior, camera/object
  glyph reflecting Ctrl's live state, white) -> camera (orange, ignores Ctrl) -> object (orange)
  -> Ctrl-decides again.

Both `OrbitCamera`/`ObjectManipulator` (`camera.py`) read these each frame via a new shared
`object_targeted(app)` helper (replacing their previous hardcoded `KeyboardButton.control()`
checks) and per-button forcing logic in their own `_update()`, so `app.target_mode`/
`forced_drag_mode` transparently override the normal Ctrl/left/middle/right split wherever it
was previously checked -- including the gizmo's own inner/outer cube coloring and the Home
button's reset-view-vs-reset-object choice, which already depended on the same Ctrl check.

The "scale" status icon initially showed as a "?" tofu glyph: `ICON_FA_EXPAND_ARROWS_ALT`'s
codepoint exists in the Python FA4 icon-name bindings but not in the actual bundled
`fontawesome-webfont.ttf` -- swapped for `ICON_FA_EXPAND`, which does render.

## 2026-08-17 — ✨ Add right-mouse zoom/scale and a navcube directional pad

Right mouse button was unused by both `OrbitCamera` and `ObjectManipulator`
(`ryzom_forgery/camera.py`) -- both only ever responded to left (orbit/rotate) and middle
(pan/move) drag. It's now wired up as a smooth, continuous alternative to wheel zoom:

- `OrbitCamera`: plain right-mouse drag zooms (`_zoom_drag()`), same direction convention as
  wheel zoom (dragging up zooms in) but continuous instead of discrete notches, via the same
  exponential falloff as `_zoom()`'s own `zoom_speed`, scaled by a new `zoom_drag_speed`.
- `ObjectManipulator`: Ctrl + right-mouse drag scales the object uniformly (`_scale()`,
  clamped to `min_scale`/`max_scale`) -- dragging up grows it, matching the same "up" feel as
  the camera's own zoom-in, so both gestures make the object read bigger on screen. Applied to
  the shared `_object_pivot`, the same node Ctrl+left/middle already rotate/move, so it
  persists across shape rebuilds the same way.

Separately, `NavigationCube.draw_controls()` (`ryzom_forgery/navcube.py`) replaces its old
rotate-left/Reset/rotate-right single-row bar (hand-drawn circular-arrow icons rolling the
view around its own forward axis) with a 3x3 pad:

```
  ccw  ^   cw
  <  HOME  >
       v
```

The 4 arrows (`OrbitCamera.step_to_face()`) step the view by a clean 90 degrees to the
adjacent axis-aligned face in that screen direction; the 2 top corners (`roll_step()`) instead
roll the current face view by 90 degrees without changing which face is shown. The center
button is a Home icon instead of a "Reset" text label, keeping its previous behavior (reset
view, or Ctrl+click to reset the object's rotation instead).

Both are a single explicit rotation of the current offset (position - target) and `up_hint` by
a fixed axis-angle (new `OrbitCamera._start_axis_anim()`/`_advance_anim()`, animating the
rotation ANGLE itself and re-deriving heading/pitch from the rotated offset each frame, so the
view traces the true circular arc of the rotation) -- not a jump to a value pulled from a
heading/pitch lookup table, which is what the existing `_start_anim()` (independent
heading/pitch/up_hint lerps, kept for `snap_to_axis()`'s arbitrary-retarget case, e.g. clicking
a gizmo face directly) does. This distinction mattered in practice: `OrbitCamera` parameterizes
the view as heading/pitch/up_hint, and heading becomes a meaningless `atan2(0, 0)`-style
artifact exactly at a pole (+z/-z, where forward is +/-Z) -- re-deriving a rotation axis from it
rotated a further step around the wrong axis, and hardcoding `up_hint` back to world Z on every
step was degenerate for `lookAt()` right at a pole (parallel to forward), leaving Panda's
fallback "up" resolution to pick an uncontrolled axis. `step_to_face()`'s up/down instead reads
the rotation axis live from the camera's actual current right vector
(`self.app.camera.getQuat().getRight()`, immune to the singularity since it reflects whatever's
actually rendered), and left/right rotates around `self.up_hint` itself instead of resetting it
to world Z, so a previous up/down step's roll carries through a later left/right step instead
of being silently corrected away. `setFromAxisAngle()`'s unit-length assertion on the rotation
axis is guarded against too, since chained rotations could drift `up_hint` off unit length over
many steps without an explicit re-normalize.

Holding a button polls `imgui.is_item_active()` every frame instead of relying on ImGui's
`button_repeat` flag, and `step_to_face()`/`roll_step()` no-op while a step animation is
already in flight -- button_repeat's own fixed timer, independent of how long a step animation
takes, left either a stutter (repeat fires before the previous step's animation naturally
clears) or an interrupted, half-finished rotation if the button was released mid-step. Polling
every frame starts the next step on the exact frame the previous one completes, and lets the
in-flight step always finish once started. The "axis" animation kind also uses a constant
angular velocity rather than smoothstep easing, since smoothstep's zero velocity at both ends
of every chained step read as a stutter at each 90-degree boundary when several play back to
back from a held button.

## 2026-08-17 — ✨ Add per-reference-shape placement and transparency controls to object_editor

The top-left viewport toggles for the 3 scale-reference shapes (Cube / smallest character /
tallest character, `_draw_reference_shapes_toggles()`) previously only had one behavior once
activated: line up next to the main shape's bbox, side by side. Now, while a reference shape
is active, 3 more square icon buttons appear stacked vertically right below its toggle
button (each shape's own `imgui.begin_group()` column, so columns stay independently aligned
regardless of how many buttons are shown):

- Place at 0,0,0 -- moves the reference shape to the world origin.
- Place on the object's pivot -- moves it to `self._object_pivot`'s current world position
  (which can differ from the origin once the user has Ctrl-dragged the main object around
  via `ObjectManipulator`).
- 50% transparent -- independent toggle, same `TransparencyAttrib.M_alpha` +
  `set_color_scale` approach as the main object's own transparency toggle.

The two placement buttons are a persistent, mutually exclusive mode per shape (clicking the
active one turns it back off, back to the default side-by-side layout) rather than a
one-shot move, re-applied every time `_rebuild_reference_shapes()` runs (shape change,
another reference toggled, etc.) via two new per-label state dicts:
`self._reference_placement` and `self._reference_transparent`.

Also fixed a pre-existing crash in `_build_reference_geometry()` (`TypeError: Geom.Geom()
argument 0 must be panda3d.core.GeomVertexData, not VertexBuffer`), hit as soon as any
reference shape was toggled on, which made testing the above impossible until fixed: it
passed the raw `vertex_buffer` from `iter_render_passes()` straight to `_build_geom()`
instead of first building a `GeomVertexData` via `_build_vertex_data()`, unlike the main
shape's own geometry-building code path.

## 2026-08-17 — 💄 Move object_editor's wind preview controls to a floating panel

The wind preview controls (`_draw_wind_controls()`: Animate/Strength/Direction) were drawn
inline at the top of the right-hand "Panel" window, ahead of the Textures/Materials/All
Properties tabs. Since wind preview isn't a shape property -- it's a viewer-only playback
control -- it's now a separate floating ImGui window, anchored top-right of the 3D viewport
flush against the left edge of the side panel, always visible regardless of which panel tab
is active (same auto-size tracking pattern used by `_draw_viewport_toggles()`'s floating
bottom-left icon bar). Only shown when the loaded shape actually has wind data
(`self._wind_state is not None`), same as before.

A separate attempt in the same session to make the app window start maximized was reverted:
Panda3D's `WindowProperties` has no native "maximize" call, and Ryzom Forgery is meant to be
shared across Windows/Linux (X11 and Wayland)/macOS users, so OS/display-server-specific
hacks (`xdotool`/`wmctrl`, which are X11-only) were ruled out. A `screeninfo`-based
approximation was also ruled out since it only reports full monitor resolution, not the
work area excluding taskbars/panels, so it wouldn't be a faithful "maximize" either.

Instead, `ForgeryApp` (`ryzom_forgery/app.py`) now starts with a bigger-than-default window
(1600x900, vs. Panda3D's own ~800x600 default) the first time an app is launched, and from
then on remembers the window's last position/size across launches, persisted to
`~/.ryzom_forgery/<slugified-app-title>.json` (one file per tool app, e.g. `object_editor.py`'s
"Ryzom Forgery - Object Editor" title). Geometry is captured by overriding `windowEvent()`
(saving on every resize/move, which also covers the final geometry right before the window
closes) -- a plain cross-platform approach with no extra dependency and no OS-specific code.

## 2026-08-17 — ✨ Add an expandable Transparency editor to object_editor's Materials tab

First step of a broader "material editor: full parameter coverage" chantier (`__TODO__.md`)
-- the Materials tab only ever exposed double-sided; everything else (blend, alpha test,
colors, lighting, shader type...) was read from the shape and used for rendering, but never
editable.

Each material row now gets an expand chevron (same pattern as the Multi Bitmap slot rows),
revealing one or more collapsible category sections -- only "Transparency" exists yet, via a
new generic `_draw_material_section(material_id, key, label, draw_fn, material)` helper so
later steps (base colors, lighting, shader type, textures, lightmaps, depth, other) just add
sibling sections. `_draw_material_transparency_section()` exposes:

- `flags & BLEND` toggle, with "Alpha Blend"/"Additive" preset buttons (pre-filling
  `src_blend`/`dst_blend` with the most common values -- srcalpha/invsrcalpha and one/one
  respectively, per `docs/material_options.md`'s existing "Mélange" section) plus the raw
  Src/Dst Blend combos (`_TBLEND_NAMES`, `CMaterial::TBlend`'s own member names) for full
  control.
- `flags & ALPHA_TEST` toggle + `alpha_test_threshold` slider.
- Diffuse alpha ("Opacity") slider -- only shown when Blend or Alpha Test is actually on,
  since it has no visible effect otherwise (confirmed the material renders fully opaque
  regardless of diffuse alpha with neither flag set).

Each control shows its docs/material_options.md explanation in the status bar on hover (same
mechanism the Multi Bitmap editor already uses, factored into a new `_doc_hint_if_hovered()`
helper) -- added a new `{#blend-factors}` doc section explaining what each raw Src/Dst Blend
value (`one`/`zero`/`srcalpha`/`invsrcalpha`/`srccolor`/`invsrccolor`/`blendConstant*`)
actually multiplies, since the existing `{#blend}` section only covered the two presets.

Hit and fixed an ImGui "2 visible items with same ID" warning while testing: the material
row's own expand chevron and `_draw_material_section()`'s chevron use the exact same icon
glyph as their button ID, both under the same `push_id(f"mat-row-{material_id}")` scope with
no further distinction -- `_draw_material_section()` now wraps itself in `push_id(key)`.

## 2026-08-17 — 🐛 Apply BLEND and ALPHA_TEST material flags independently

`_apply_material_texture()` used `if flags & BLEND: ... elif flags & ALPHA_TEST: ...`,
silently dropping the alpha-test cutout whenever blend was also enabled on the same
material. Checked the real engine (`driver_opengl_material.cpp:473-493`): `enableBlend()`
and `enableAlphaTest()` are two entirely separate `if` blocks, each driven only by its own
flag -- both GL states can be active together (alpha test discards fully-transparent texels
outright first, blend still applies a soft fade to whatever's left). Changed the `elif` to a
second independent `if`.

## 2026-08-17 — ⚡ Fix wind preview dropping from 60 to ~38fps on real prop-sized meshes

`_update_wind()`'s per-frame vertex update used a `GeomVertexRewriter` loop, one Python-level
`set_row()`/`set_data3()` call pair per vertex -- fine for a handful of vertices, but a real
prop (`ooc_summer_raceline.shape`, 28k verts) dropped 60fps to ~38fps purely from Python call
overhead, not actual work.

Replaced it with a single vectorized write into a numpy view onto the vertex array's raw
buffer (`GeomVertexArrayData` supports the buffer protocol directly, confirmed writable via
`modify_array()` and round-tripped through a `GeomVertexReader` to verify the write actually
lands). First attempt cached that numpy view across frames and only wrote into the same
buffer each time -- silently stopped animating anything past the first frame, since
`modify_array()` (not writing into the buffer it returns) is what bumps the array's
modification stamp Panda3D's GSG checks to decide the GPU-side vertex buffer needs
re-uploading. Fixed by calling `modify_array()` fresh every frame (itself cheap, no data
copy) and only caching the row layout (stride/vertex-column offset), which can't change for a
given vdata's lifetime.

## 2026-08-17 — 🐛 Cap the wind preview's Strength slider to the engine's real range

`_draw_wind_controls()`'s Strength slider used an arbitrary `0.0-3.0` range. Checked what the
real client/engine ever actually passes to `CScene::setGlobalWindPower()`: the weather system
(`ryzom/client/src/weather_manager_client.cpp`) computes it as `0.1 + 0.9 * windIntensity`
(or the equivalent sheet-driven `TreeMinWindIntensity`/`TreeMaxWindIntensity`, both defaulting
to 0.1/1.0 in `weather_function_sheet.cpp`), the static `client.cfg` fallback defaults to
`0.10` (`client_cfg.cpp`), and the engine's own pre-client default is `0.2`
(`nel/src/3d/scene.cpp`) -- consistently `[0.1, 1.0]` in practice, matching the range NeL's
own `object_viewer` debug tool (`global_wind_dlg.cpp`) already uses for the same slider.
Changed the max to `1.0` to match.

## 2026-08-17 — 🐛 Fix texture orientation, wrap-mode, and caching bugs across imports/saves/reloads

A long live-testing session on `fy_acc_civbanner.shape` ("texture upside down in Forgery,
correct in-game") and freshly-imported `.fbx` shapes ("all textures shifted") surfaced
several distinct, compounding bugs:

- **`p3dimgui`'s `loadTexture()` mutates its argument in-place**: `_get_preview_texture_ref()`
  (Materials tab thumbnails) was passing the exact same cached `Texture` object also applied
  to the live 3D material -- `p3dimgui/backend.py`'s `loadTexture()` calls `pnm.flip(False,
  True, False)` on it for ImGui's own row-order convention, silently flipping the
  3D-rendered texture too the first time a shape's thumbnail was ever drawn. Fixed by passing
  `panda_texture.make_copy()` instead of the shared cached object.
- **UV wrap mode was never set at all** (Panda3D's own default, `WM_clamp`, is correct for a
  single non-tiling texture per material but wrong for shapes that deliberately tile UVs past
  `[0, 1]`, e.g. `ooc_summer_raceline.shape`'s V up to 2.0). Tried reducing UVs modulo 1 first
  -- reverted immediately, since it breaks any triangle whose UVs straddle an integer
  boundary (routine for a wrapped cylindrical texture), stretching the whole triangle's image.
  Real fix: `_uvs_need_repeat()` auto-detects per-shape (margin of 0.05, to not misfire on an
  imported mesh's float noise around 1.0) whether the whole `TexCoord0` channel relies on
  tiling, and `load_panda_texture()`'s new `repeat` param sets `WM_repeat` only then.
- **Imported meshes' UVs were never converted to NeL's own V-origin convention**: verified
  against Assimp's own FBX/OBJ reader source that neither flips V on read (both already match
  OpenGL/Panda3D's own convention, the opposite of a `.shape` file's). A first attempt fixed
  this at *display* time only (a session flag disabling `object_editor.py`'s own V-flip for
  imported shapes) -- wrong on two counts: the flag was never reset between shape loads (so
  importing anything once silently broke every subsequently loaded real `.shape` for the rest
  of the session), and more fundamentally, it only affected the live Panda3D vertex data, not
  what actually gets written to disk -- a freshly imported-then-saved `.shape` round-tripped
  wrong once reloaded, since nothing in the file itself encoded which convention its UVs were
  in. Real fix: the V-flip now happens once, at import time, in `shape_import.py`'s
  `_assemble_mesh()` (shared by every importer), so the *saved* `.shape` data is correct on
  its own; `object_editor.py`'s own flip in `_build_vertex_data()` is back to being simple and
  unconditional for every shape alike.
- **`self._texture_cache`/`self._preview_texture_refs` were never cleared between shape
  loads**: both are keyed by texture name only, so loading a shape whose textures need
  `WM_repeat` and then a different shape sharing a same-named texture (but needing `WM_clamp`,
  or resolving to a different file via different `_texture_search_dirs`) silently reused the
  first shape's cached `Texture` object and wrap mode. Both caches are now reset in
  `_reset_shape_state()`.
- Texture preview thumbnails/tooltips also gained an opaque black backdrop
  (`_draw_image_opaque_bg()`, `_PREVIEW_BG_COLOR`, `image_button(..., bg_col=...)`) and drop
  their alpha channel entirely on the throwaway preview copy (`set_format(F_rgb)`) -- most of
  these textures are mostly transparent (e.g. a shape cut from a square atlas), which made the
  real color underneath unreadable regardless of backdrop; safe to do only on the copy, now
  that it's confirmed independent of the shared 3D-material texture (see the `loadTexture()`
  fix above).

## 2026-08-17 — ✨ Add a live wind-sway preview to object_editor for WindTree shapes

object_editor could already display a shape's `WindTreeParams` (read-only, via the generic
"All Properties" tab), but had no way to actually see the wind animation it describes --
scoped down from an initial "editable WindTree UI" idea to preview-only first, since editing
parameters blind isn't useful without seeing what they actually do.

Added a viewer-only "Wind preview" panel (`_draw_wind_controls()`: Animate toggle, Strength
0-3 slider, Direction 0-360° slider -- shown only when the loaded shape has wind data,
mirrors the engine's own `CScene::setGlobalWindPower/Direction` scene setting rather than
anything saved to the shape) and a per-frame animation task (`_update_wind()`), ported from
`nel/src/3d/meshvp_wind_tree.cpp` and its `wind_tree_vp.glsl` vertex decode: each vertex's
`PrimaryColor` channel encodes its level-0/1/2 wind blend weight (R) and which of 4
level-1/level-2 phase branches it swings with (G/B), combined with per-level
frequency/power/bias oscillation (`cos(2*pi*t)`) and the global strength/direction from the
new controls (`_build_wind_state()` precomputes the per-vertex decode once in
`numpy` arrays; `_update_wind()` recomputes positions every frame and writes them back via a
`GeomVertexRewriter`).

Needed splitting `_build_geom()` into `_build_vertex_data()` (builds the shared
`GeomVertexData` once per `_rebuild_geometry()` call, now `Geom.UH_dynamic` for shapes with
wind data so Panda3D doesn't assume the position buffer is upload-once) and a smaller
`_build_geom()` (just the per-pass triangle indices) -- every render pass already indexed
into the exact same vertex array, so this also means the vertex data (and its per-frame
wind update) is built/updated once per shape instead of once per material pass.

## 2026-08-17 — ✨ Resolve imported meshes' textures from their own folder as a fallback

An `.obj`/`.dae`/`.fbx`'s own texture references routinely point next to the source file
(or an `.fbx`'s own `<name>.fbm` sibling folder, or a conventional `tex`/`textures`/`data`
subfolder) rather than anywhere inside the Ryzom asset root object_editor otherwise searches
via `AssetIndex` -- previously the only way to see such a texture at all was to also have it
somewhere in the indexed data tree.

Added `shape_import.texture_search_dirs_for(path)` (the imported file's own folder, plus its
`.fbm` sibling for `.fbx`) and `shape_geometry._find_local_texture_ref()` (checked as a
fallback by `load_panda_texture()`'s new `search_dirs` param only once the `AssetIndex`
itself comes up empty, same case-insensitive/extension-fallback matching as
`AssetIndex.find_texture()`). `object_editor.py` now tracks `self._texture_search_dirs` per
loaded shape: the file's own folder for a `.shape` opened from the Explorer,
`texture_search_dirs_for()` for a freshly imported mesh (both the "import as new shape" and
"replace geometry" flows -- `ImportDialog.on_replace` now threads the source path through for
this).

## 2026-08-17 — 🐛 Fix FBX/DAE import axis conversion and null-channel crashes

Two robustness bugs in `shape_import.py`'s `_import_via_assimp()` (used by both `.fbx` and
`.dae`), found while validating the material-defaults fix above against a real Cinema4D->
`.fbx`->3dsMax->`.shape` export: the shape's bbox orientation didn't match the same `.fbx`
imported directly through Forgery. Root cause: `_iter_mesh_instances()` was seeded with a
plain identity matrix, silently assuming the source file's axes already matched Ryzom's
Z-up -- but Assimp's own FBX and Collada loaders both always normalize whatever up axis a
source file declares into Assimp's own canonical Y-up first (`FBXConverter.cpp`'s
`correctRootTransform()`, `ColladaLoader.cpp`'s `ConvertScene()`), so 3dsMax's own
Z-up-targeting FBX importer and Forgery's Y-up-targeting one were disagreeing on a
mid-import convention neither had actually been told about. Replaced the identity seed with
`_YUP_TO_ZUP_MATRIX`, the exact inverse of Assimp's own Y-up normalization.

Also fixed two related crashes on meshes assimp-py returns as having no data for a given
channel at all: `assimp_py.Mesh.normals`/`.texcoords` come back as `None` (not an empty
list/memoryview) in that case, which `_mesh_normals()` and `_import_via_assimp()`'s
`has_normals`/`has_uvs` detection didn't guard against before calling `len()` on it.

## 2026-08-17 — ✨ Match 3dsMax's material export defaults when importing meshes

`shape_import.py`'s importers used to build each material straight from the source
`.obj`/`.dae`/`.fbx`'s own diffuse/ambient/specular/shininess/opacity data -- which turns
out not to be how real Ryzom content's materials actually come about. Reading
`nel/tools/3d/plugin_max/nel_mesh_lib/export_material.cpp` and `nel/src/3d/material.cpp`
showed that 3dsMax's exporter always reads these values off the NeL-material-plugin
instance an artist assigns in 3dsMax itself, completely independent of whatever the
original imported source file's own material data said -- confirmed by comparing a real
artist's Cinema4D->.fbx->3dsMax->.shape export against the same .fbx imported directly
through here.

`_build_material()` now always builds a "blank NeL material" (ambient/diffuse = 3dsMax's
own Standard-material gray `Rgba(150,150,150,255)`, specular = `Rgba(0,0,0,0)`,
shininess = 8.0 matching 3dsMax's default 10% Glossiness, `_MODULATE_TEX_ENV` matching
`CMaterial::setShader()`'s documented stage-0 default once a texture is assigned), keeping
only the diffuse texture reference from the source file -- plus `double_sided`, the one
property honored from the source material after all, since it's a real geometric necessity
(thin panels/foliage with no back faces) rather than an artistic choice that gets manually
redone in 3dsMax anyway. Read from Assimp's own `TWOSIDED` material property (present only
when the source file set it explicitly), defaulting off otherwise.

## 2026-08-17 — 🐛 Fix icon button tooltips rendering blank under the large icon font

`_icon_button()`'s callers that use the 1.5x large icon font (`_draw_viewport_toggles()`,
`_draw_reference_shapes_toggles()`) pushed that font around the *entire* toggle bar,
including each button's hover tooltip -- but the bundled Font Awesome `.ttf` only has icon
glyphs, no letters, so a tooltip rendered under it came out blank instead of the intended
text. `_icon_button()` now takes an optional `large_font` (ImFont, size) pair and pushes it
only around the button glyph itself, popping it before drawing the tooltip -- both callers
now pass `large_font` per-button instead of wrapping the whole bar in `push_font()`/
`pop_font()`.

## 2026-08-16 — 🎨 Move scale-reference toggles to the viewport, size icons consistently

Moved the "Cube (1x1x1)" / "Smallest character" / "Tallest character" scale-reference
toggles (`apps/object_editor.py`) from a text-button row inside the right-hand panel to a
top-left viewport bar, matching the bottom-left `_draw_viewport_toggles()` bar's
positioning pattern -- icon buttons (`ICON_FA_CUBE`/`ICON_FA_CHILD`/`ICON_FA_MALE`) instead
of text labels.

Both this new bar and the existing bottom-left one now render at 1.5x the normal UI icon
size (`ForgeryApp.large_icon_font`, a second Font Awesome font loaded standalone -- this
imgui_bundle version has no per-window font-scale API to reach for instead) and use a
square `_icon_button(..., square=True)` size (`imgui.get_frame_height()` on both axes)
instead of ImGui's default auto-size, which otherwise made each button only as wide as its
own glyph -- visibly inconsistent between different Font Awesome icons.

## 2026-08-16 — ✨ Add .fbx import and move .dae import to assimp

Added `.fbx` import support to Forgery (`ryzom_forgery/shape_import.py`, wired into both
the object_editor's import dialog and the `shape_importer.py` CLI), via the `assimp-py`
PyPI package -- picked over `pyassimp` (ctypes wrapper needing a system-installed
`libassimp.so`, not portable across a shared Forgery install) since it ships self-contained
precompiled wheels for Linux/macOS/Windows. Those wheels only go up to Python 3.13 though
(no cp314 yet, and no cp38 either, ruling out staying aligned with ryztart's older Python),
so `dev.sh` now creates the `.venv` with `python3.12`/`python3.13` if available, falling
back to whatever `python3` resolves to only if neither is found.

`.dae` import was then also switched from the hand-rolled `pycollada`-based parser to the
same assimp path (`_import_via_assimp()`, shared with `.fbx` -- Assimp auto-detects format
from content, so there's nothing format-specific left once node-transform handling is
generic). Verified assimp doesn't auto-rotate based on a `.dae`'s `<up_axis>` tag before
switching (Forgery's own .dae exporter writes a `Y_UP` tag via pycollada's own default
while leaving vertex data in Ryzom's actual Z-up coordinates untouched -- an auto-rotating
importer would have silently broken re-importing Forgery's own exports). `pycollada`
itself stays a dependency regardless, since `shape_export.py`'s .dae *exporter* still uses
it directly (assimp-py only wraps Assimp's import side, not export).

Two real bugs found while validating end-to-end through the bridge on a Blender-exported
test .fbx and the repo's own `test.dae`: `Process_GlobalScale` was needed (an FBX authored
in centimeters, e.g. Blender's default export, otherwise comes back 100x too large --
Assimp leaves that unit conversion sitting on the root node's scale unless asked to apply
it), and Assimp's generated normals (`Process_GenNormals`) come back at the pre-GlobalScale
magnitude rather than unit length, so they're now explicitly renormalized after each node
transform is applied. Also: COLLADA colors come back as RGBA (4 components) from assimp
where FBX/obj ones are plain RGB (3) -- `_build_material_from_assimp_material()` now slices
to the first 3 either way.

## 2026-08-16 — ✨ Add viewport helper toggles to Forgery object_editor

Added a small icon-button bar bottom-left of the 3D viewport (`_draw_viewport_toggles()`
in `apps/object_editor.py`), four toggles off by default:

- Floor grid (`ICON_FA_TABLE`): 1x1m squares on the world XY plane, fixed at world Z=0 so
  it reads as an actual ground reference (whether the object floats above or sinks below
  it is exactly what this is meant to show -- deliberately NOT anchored to the object's own
  bbox bottom, per explicit feedback after the first version did that).
- World axes (`ICON_FA_COMPASS`) and object-pivot axes (`ICON_FA_CROSSHAIRS`, attached to
  `_object_pivot` so they rotate/move with the object): colored X/Y/Z lines through the
  origin, two different color palettes (red/green/blue vs magenta/yellow/cyan) so both can
  be shown together without confusing one for the other.
- 50% object transparency (`ICON_FA_ADJUST`): `TransparencyAttrib.M_alpha` +
  `set_color_scale(1,1,1,0.5)` on `model_root`, re-applied after every `_rebuild_geometry()`
  call since `model_root` itself gets torn down and recreated then.

Both the grid's footprint (squares count, X/Y center) and the axes' length are re-derived
from the loaded shape's own bounding box each time geometry is rebuilt
(`_rebuild_viewport_helpers()`), so they scale to cover whatever object is on screen instead
of a fixed size that's tiny for a building and huge for a trinket.

## 2026-08-16 — ✨ Add Ctrl+drag object manipulation to Forgery

Comparing a loaded shape against reference objects previously required orbiting the whole
scene, which swung the reference objects around too. Reworked `object_editor.py`'s
viewport controls in `ryzom_forgery/camera.py` and `ryzom_forgery/navcube.py`:

- `OrbitCamera`: plain left-click-drag now orbits the camera (was middle-click), plain
  middle-click-drag pans it (shift no longer required). Both are now gated off while Ctrl
  is held, so they don't fight the new object controls below.
- New `ObjectManipulator` class: Ctrl+left-click-drag rotates the shape itself around its
  own pivot, Ctrl+middle-click-drag moves it -- both camera-relative (the drag direction
  follows the current view angle at any camera orientation), independent of the camera
  controls. The rotation composition initially used the wrong Panda3D quaternion
  multiplication order (`delta * pivot.getQuat()`), which silently applied the delta in
  the object's own already-rotated local frame instead of the camera's fixed world frame
  -- fixed to `pivot.getQuat() * delta` (Panda composes left-operand-first).
- The object's pivot is seeded from the shape's own `default_rot_quat` on load (full
  quaternion, since real shapes can have roll/tilt, not just heading/pitch -- confirmed
  2506/3997 shapes in the live data have a non-identity `default_rot_quat`). This is a
  pure viewer-side transform: Ctrl+drag never writes back to `default_rot_euler`/
  `default_rot_quat` in the shape file.
- The gizmo's cube grew a second, 2x smaller inner cube that live-mirrors the object's
  current rotation, while the outer cube (still representing the camera/scene) became
  semi-transparent so both stay visible together. Held Ctrl swaps which of the two reads
  as "active" (orange vs. blue), showing which one a drag is about to affect.
- The gizmo's "Reset" button now resets the object's rotation instead of the camera's view
  while Ctrl is held. `object_editor.py` tracks a baseline quat for this (the loaded
  `default_rot_quat`, or whatever rotation was in effect at the last successful save --
  saving never writes the rotation to the file, but does move the in-session baseline).

## 2026-08-16 — ✨ Split object_editor's Materials tab into Textures + Materials

The old single "Materials" tab conflated two different concerns -- editing texture
references and editing material properties -- under a UI that was really only about
textures. Split into two tabs (order: Textures, Materials, All Properties):

- **Textures** (renamed from "Materials"): unchanged texture-editing rows (text field +
  browse icon, Multi Bitmap set editor), now prefixed with each row's material index
  (`#N`) and showing an actual texture thumbnail instead of a flat color swatch.
- **Materials** (new): one row per material -- index, a Simple bitmap/Multi Bitmap/Color
  type badge, a "Double-sided" checkbox (toggles `CMaterial::flags & IDRV_MAT_DOUBLE_SIDED`
  live, hidden for Color materials), and conversion actions: Simple/Color -> Multi Bitmap
  (existing), and the reverse -- Multi Bitmap -> Simple bitmap (only offered when slot 0 /
  Low Quality is the only populated set) or -> Color (only when no set has a texture --
  `_multi_bitmap_populated_slots()`), both lossless by construction since they're only
  shown when nothing would be discarded.

Thumbnails: `_get_preview_texture_ref()` resolves a texture by name via the existing
`load_panda_texture()`/`_texture_cache` and wraps it with `self.imgui.loadTexture()`
(`p3dimgui`'s backend, exposed as `ObjectEditorApp.imgui` since `ShowBase` sets the global
`base` to the app instance) into an `imgui.ImTextureRef`, cached by name -- new territory
for this codebase, no prior `imgui.image()`/`image_button()` usage. Hovering a thumbnail
shows a bigger version in a tooltip. An untextured material's own diffuse color reuses the
exact same `image_button` widget rather than a hand-drawn look-alike: a hand-drawn border
around a plain `color_button` (tried first, positioned via `get_item_rect_min/max()` +
`ImDrawList.add_rect()`) never looked quite like a real thumbnail's border/hover
highlight, so `shape_geometry.solid_color_texture()` now builds a 1x1 texture filled with
the color instead, going through the identical rendering path
(`_get_color_texture_ref()`/`_draw_color_preview_button()`).

Textures tab semantics: for an untextured material, that same color swatch
(`_draw_texture_color_button()`) now edits `CMaterial::diffuse` directly and for real --
saved with the shape -- unlike the Materials tab's own color button
(`_draw_material_color_button()`), which stays a temporary, per-session visualization
override (`_material_override_colors`, never written to the shape) for spotting which
faces use a given material. Once a texture is set, editing stops being offered there at
all (the diffuse color becomes moot) -- the swatch turns into a plain preview. That preview
first used a bare `imgui.image()`, which rendered visibly smaller than a real thumbnail
button (no button frame padding); switched to a *disabled* `image_button` instead so the
size matches exactly, which took `imgui.is_item_hovered(HoveredFlags_.allow_when_disabled)`
to keep the hover-zoom tooltip working (disabled items don't report hovered by default).
The same disabled-preview treatment now applies to Multi Bitmap's per-slot rows too: those
never supported real color editing to begin with (a texture-variant slot isn't "a color"),
so they never got the Materials-tab override button either -- just a static preview
(or an empty placeholder for an unpopulated slot).

Fixed one crash hit while testing: `_set_material_diffuse_color()` initially sliced
`color[:3]` off the `color_picker4()` return value, which is an `ImVec4`, not a plain
tuple -- `ImVec4.__getitem__` only accepts a single int index, not a slice. Reads `.x/.y/.z/.w`
explicitly instead.

## 2026-08-16 — ✨ Wire up .obj/.dae import as a new shape or a geometry replace

Finished the import flow started in "Rework the Forgery Explorer's navigation and add an
import entry point": `ImportDialog`'s two active modes now actually do something instead
of printing a TODO placeholder.

Split `_load_shape()`'s body into `_reset_shape_state()` (editing state that should be
cleared -- Multi Bitmap expand state, material color overrides, save path/name...) and
`_display_shape()`/`_rebuild_geometry()` (deriving 3D render passes + camera framing from
whatever `self.shape_file` currently is), so both a `.shape` opened from the Explorer and a
freshly imported mesh go through the same rendering path, and a geometry *replace* can
reuse `_rebuild_geometry()` alone without wiping the material edits it's meant to survive.

"Import as new shape" (`_on_import_new_shape()`) wraps the parsed `Mesh` into a
`ShapeFile(type_name="Mesh", ...)` and displays it fresh, pre-filling
`_shape_source_name` from the source file's own name (`<stem>.shape`) so "Save As"
defaults sensibly for it too (see the entry below).

"Replace in current shape" (`_on_import_replace()`/`_replace_geometry()`) only swaps
`self.shape_file.value.geom` for the imported mesh's -- `self.shape_file.value.materials`
(and any blend/alpha-test/2-sided/Multi Bitmap edits already made to them) is left
completely untouched, which is the entire point of this mode over a plain re-import.
Restricted to `type_name == "Mesh"`: `MeshMRM`/`MeshMRMSkinned`/`MeshMultiLod` shapes parse
their geometry into real Python structures, but `dumps()`
(`pynel/ryzom_shape.py:2538`) writes those three types' geometry back out as `_raw_geom` --
the *original* captured bytes, verbatim, never reconstructed from the parsed fields -- so
mutating their in-memory geometry would render correctly in the viewer while silently
saving the old, untouched geometry to disk. Only `CMesh` has a real writer
(`_write_mesh()`) that serializes from the live dataclass fields.

When the imported mesh's material count doesn't match the current shape's, a "Match
materials" modal lists each imported material (by texture name) with a combo to map it
onto an existing material index or "Add as new material"; confirming remaps the imported
geometry's per-render-pass `material_id`s through that mapping (appending any "new" picks
to the current material list) before calling `_replace_geometry()`. Hit and fixed a real
bug here during testing (`anlor_stick.shape`, mismatched counts): the match popup's
`imgui.open_popup()` was called synchronously from inside `_on_import_replace()`, itself
called while still nested inside `ImportDialog`'s own "Import mesh" popup (not yet
`end_popup()`-ed for the rest of that frame) -- opening a popup from inside another
not-yet-closed one silently failed to register at all ("rien du tout": no popup, no
error, no visible change). Fixed by not calling `open_popup()` there at all -- just
recording the pending mesh/mapping -- and having `_draw_replace_match_popup()` open it
itself on the next frame, from `draw_panel()`'s own top-level call, outside any nested
popup context.

## 2026-08-16 — 🐛 Default Save As's folder/name to the Explorer's current location

`_draw_save_buttons()`'s "Save As..." defaulted to `self._shape_source_path` when set, but
fell back to an empty string otherwise -- notably for a shape loaded from inside a `.bnp`
(which has no real on-disk path of its own), opening the native save dialog with no folder
*or* file name pre-filled at all. Added `self._shape_source_name` (the original file name,
set in `_load_shape()` alongside `_shape_source_path` but kept even when that one is
`None`) and now fall back to `self.explorer.root / self._shape_source_name` -- the
Explorer's current folder, with the shape's own name pre-filled.

## 2026-08-16 — ✨ Rework the Forgery Explorer's navigation and add an import entry point

Reworked `ryzom_forgery/explorer.py`'s browsing UX end to end, driven by live feedback
over several rounds via the .agentcom bridge:

- Search input and extension-filter combo are now side by side (`imgui.same_line()`),
  both with hidden labels (`"##search"`/`"##filter"`) instead of visible "Search"/"Filter"
  text taking up their own rows.
- The root path is now an editable text field (`"##path"`) instead of plain `imgui.text()`,
  with folder-name autocomplete: a bordered box lists matching subdirectories, navigable
  with Up/Down and Enter, or clicked directly. Landed on an inline child (not a
  freestanding overlay `imgui.begin()`/`end()` window, tried first) after the overlay
  version's clicks stopped registering -- moving keyboard focus off the path input on the
  same click that hit a suggestion apparently broke that click's hit-testing; the fix also
  needed decoupling "is the box open" from `is_item_active()` alone (checking whether the
  box itself is hovered/focused too), since focus already leaves the input by the time its
  own click is processed. Picking a suggestion now also appends a trailing `/`, refocuses
  the path input and re-shows suggestions for the new directory immediately -- typing a
  full path can flow continuously, one folder at a time, without touching the mouse.
  Suggestions are uncapped with the box scrolling past 8 rows instead.
- Favorite folders: a star button toggles the current folder in/out of a persisted list
  (`ryzom_forgery/explorer_config.py`, JSON via a new shared `ryzom_forgery/config_dir.py`
  extracted from `export_config.py`'s `_config_dir()`), browsable from a "Favorites" combo
  dropdown (switched from a row of buttons after that overflowed with more than a few
  favorites) -- each entry has its own star to remove it, same toggle metaphor as the
  main button.
- Replaced the recursive expand/collapse tree with flat, single-click navigation: clicking
  a sub-folder or `.bnp` navigates straight into it (a new `_current_bnp` state represents
  "browsing this archive's flat entry list"), with `..` to go back up; the path bar and
  favorites star now track wherever navigation actually is.
- Dotfiles/dotfolders are hidden by default (`_is_hidden()`, applied to both the file list
  and path autocomplete), with a "Show hidden files/folders" toggle on the list's own
  right-click context menu (`begin_popup_context_window(..., no_open_over_items)`, so it
  doesn't fight with `_draw_leaf`'s per-item command menu).

Also added `ryzom_forgery/import_dialog.py` (`ImportDialog`, mirroring `ExportDialog`'s
shape): picks a `.obj`/`.dae` via a native file dialog, then a mode popup -- "Import as new
shape" / "Replace in current shape" / "Add to current shape" (disabled, future work) -- the
point being that replacing geometry in an already-open shape can keep its materials (and
whatever editing was done to them: blend/alpha-test/2-sided/Multi Bitmap...) intact, unlike
a fresh `.obj` -> `.shape` import which can't carry any of that. Wired an upload-icon
toolbar button into the Explorer (`Explorer.extra_toolbar`, a new optional per-app hook
next to Refresh) and `ImportDialog.draw()` into `object_editor.py`'s panel; the two actual
import modes themselves are still `# TODO` stubs (next plan steps), the toolbar button
currently only proves the file-pick -> mode-popup flow reaches `object_editor.py` with a
parsed `Mesh`.

The Refresh button also became an icon-only button (`ICON_FA_SYNC`) to match, freeing up
space for the upload button next to it. An icon initially added for the ".." row turned out
not to render (this bundled Font Awesome 4 `.ttf` predates several FA5-renamed icon
aliases despite the generated Python bindings still defining constants for them --
`ICON_FA_LEVEL_UP_ALT` was one of these, same class of issue as the missing `fa-images`
noted in the "Let object_editor convert a simple material to Multi Bitmap" entry further
down); reverted that row to plain text rather than chase down another working glyph name.

## 2026-08-16 — 🐛 Respect CMaterial::ZWRITE for blended materials in object_editor

Follow-up to the transparency work below: `apt_snowglobe.shape`'s glass sphere
(material 5, `flags=0x90` -- BLEND|LIGHTING, no ZWRITE bit) intermittently hid geometry
behind it (its base pedestal, and the grass patch under the reindeer) depending on draw
order, reproducing across app restarts but not always within a session -- confusing to
track down since toggling the material color override (which forces a full
`_reapply_material()`) sometimes "fixed" it, and it always looked fine from inside the
globe (camera past the glass, so it's simply not in the view frustum).

Root cause: `_apply_material()` never touched depth-write state, so every material wrote
depth (Panda3D's default), including ones the shape data explicitly marks as not writing
it (`CMaterial::flags & IDRV_MAT_ZWRITE`, checked in
`driver_opengl_material.cpp:497`'s `enableZWrite()`) -- exactly the ones meant to be
translucent and therefore not occlude what's behind them. Whichever object happened to
z-sort before the glass in a given run got its depth buffer values overwritten by the
(otherwise correctly alpha-blended) glass sphere, so anything drawn after failed the
depth test and never appeared. Ruled out an alpha-channel decoding bug first (checked
`apt_snowglobe_alpha.tga` -- actually a `.dds` via the extension fallback again -- alpha
sampled uniformly ~0.15, consistent with a deliberately subtle glass tint, not a decode
failure) before landing on this.

Added `_IDRV_MAT_ZWRITE` and `node_path.set_depth_write(bool(material.flags &
_IDRV_MAT_ZWRITE))` (default `True` when there's no material), matching `_two_sided`'s
existing pattern of following the shape's own flag instead of a fixed default.

## 2026-08-16 — 🐛 Render material transparency (blend/alpha-test) in object_editor

`_apply_material()` (`apps/object_editor.py`) only ever set up an opaque `Material`
(diffuse/ambient/emissive/specular) and a texture -- `CMaterial::flags` (BLEND,
ALPHA_TEST, DOUBLE_SIDED) were parsed by pynel but completely ignored by the viewer, so
every shape rendered fully opaque and double-sided regardless of what the material
actually specified. Found via `12thanniv_flacon.shape` (glass, should be translucent) and
`ge_mission_xmass_tree.shape` (foliage, should be alpha-tested cutout).

Added `_IDRV_MAT_BLEND`/`_IDRV_MAT_DOUBLE_SIDED`/`_IDRV_MAT_ALPHA_TEST` flag constants and
a `_TBLEND_TO_PANDA_OPERAND` table mapping `CMaterial::TBlend` (material.h) to
`panda3d.core.ColorBlendAttrib.Operand`, in the enum's own declaration order. `flags &
BLEND` now sets an explicit `ColorBlendAttrib` built from the material's actual
`src_blend`/`dst_blend` (not always the ordinary alpha srcalpha/invsrcalpha case -- e.g.
`12thanniv_flacon_body.tga`'s glass turned out to use `one/one`, i.e. genuine additive
blending, confirmed against the shape's own parsed data via the .agentcom bridge); `flags &
ALPHA_TEST` sets an `AlphaTestAttrib(M_greater, alpha_test_threshold)`, matching the
engine's own `glAlphaFunc(GL_GREATER, threshold)` (`driver_opengl_material.cpp:456`).
`set_two_sided()` now also follows `flags & DOUBLE_SIDED` instead of always being forced
on -- forcing it made additive-blend materials double their contribution from unwanted
backface overdraw, visibly whitening `12thanniv_flacon_body`. An initial version also
forced `TransparencyAttrib.M_alpha` alongside the explicit `ColorBlendAttrib`; removed
after user request, since it's redundant (the explicit attrib already wins) and not
correct for non-alpha blend funcs like the flacon's.

Also fixed two adjacent bugs hit while testing: `load_panda_texture()`
(`ryzom_forgery/shape_geometry.py`) crashed the whole app with an uncaught `AssertionError`
(`linear_size == header.pitch`, `panda/src/gobj/texture.cxx:8921`) on a malformed DDS
header (`ge_mission_xmass_tree_star.tga`, which -- like several textures here -- actually
resolves to a `.dds` file via `AssetIndex.find_texture()`'s extension fallback); now caught
and treated as a normal decode failure. And the DDS-loading path had no equivalent of the
PNM path's `image.flip(False, True, False)` (NeL vs Panda3D V-origin), so any texture that
happened to resolve to `.dds` rendered upside-down (confirmed on the xmas tree's foliage,
tip pointing the wrong way) while `.tga`/`.png`-backed ones didn't; the flip is now done
once, uniformly, on the UV coordinates in `_build_geom()` instead of per-format at texture
load time.

## 2026-08-16 — 🐛 Fix object_editor's default camera distance being too close

`ObjectEditorApp._load_shape()` framed a newly loaded shape's camera at `radius * 3.0`,
which the user found consistently too close (had to manually zoom out ~6 mouse-wheel
notches on every load to reach a comfortable distance). Since each wheel notch scales
distance by `1/zoom_speed` (`camera.py`'s `_zoom()`, `zoom_speed=0.9`), 6 notches is a
`(1/0.9)^6 ≈ 1.88x` factor; changed the framing multiplier to `radius * 5.65` (`3.0 *
1.88`, rounded) to match that as the new default.

## 2026-08-16 — ✨ Let object_editor convert a simple material to Multi Bitmap

A material imported from `.obj`/`.dae` (via the new `shape_importer.py`) always lands as
a plain single-texture material -- there's no way for those formats to express a Multi
Bitmap variant set. Added a "convert" icon button (fa-clone; fa-images, tried first,
turned out to be missing from imgui_bundle's bundled `fontawesome-webfont.ttf` -- added
in FA 4.7, this font predates it, confirmed by hand-parsing the ttf's `cmap` table since
`fontTools` wasn't installed) next to each simple-material row in the Materials tab:
`_convert_to_multi_bitmap()` swaps slot 0's `CTextureFile` for a `CTextureMultiFile`
seeded with a single set (index 0 = the current texture), so the material immediately
moves into the Multi Bitmap section, ready for more slots to be filled in.

Confirmed this is safe to save with only one slot filled: `CTextureMultiFile::getTexIndex()`
(`nel/src/3d/texture_multi_file.cpp:55-67`) already falls back to index 0 whenever the
engine asks for a season/quality index past the end of `_FileNames` (`selectTexture()`
itself never validates the index, just stores it) -- so a partially-filled Multi Bitmap
material just always shows its slot 0 texture until more slots are added, no crash or
out-of-bounds read regardless of what the game requests.

## 2026-08-16 — ✨ Add .obj/.dae -> .shape import to Forgery

Added `ryzom_forgery/shape_import.py` and `apps/shape_importer.py` (CLI:
`shape_importer.py INPUT.{obj,dae} OUTPUT.shape`), the reverse of the existing
`shape_exporter.py`/`shape_export.py`. Only `CMesh` can be produced this way -- unlike
`CMeshMRM`, pynel's `dumps()` writes `CMesh` geometry field-by-field rather than copying
bytes from an already-parsed file, so it's the only shape type buildable from scratch.
That means no LOD levels on an imported shape; investigated whether an existing tool
could add them afterward (`CMRMBuilder`, `nel/include/nel/3d/mrm_builder.h`): it operates
on the engine-native `CMesh::CMeshBuild` struct, not anything 3dsMax-specific, so it's
architecturally reusable, but no standalone CLI wrapping it exists today (the only
current caller is the 3dsMax-plugin-only `pipeline_max_export_shape`) -- writing one is a
separate, C++-only chantier (`CMRMBuilder` has no Python binding), noted for later.

`.obj`/`.mtl` are hand-parsed (same reasoning as the hand-written `.obj` exporter: simple
dependency-free text format) -- vertices/normals/texcoords, faces grouped by `usemtl`
(fan-triangulated if >3 verts per face, negative/relative indices resolved), materials
read from the referenced `mtllib` (Kd/Ka/Ks/Ns/d/Tr/map_Kd). `.dae` goes through
`pycollada` (already a dependency for `.dae` export) -- iterates
`doc.scene.objects("geometry")`'s bound `BoundTriangleSet`s (skipping non-triangle
primitives), reading each `Triangle`'s already-dereferenced vertex/normal/texcoord
values and its bound `Material.effect` (diffuse/ambient/specular/shininess/transparency,
falling back to a neutral default when a property is a texture `Map` rather than a plain
color). Both formats share an extracted `_assemble_mesh()` (dedup vertices into one
shared buffer, build the single matrix block/materials/bbox/MeshBase/MeshGeom) and a
generalized `_build_material()`.

Material defaults for properties the source format doesn't specify are grounded in
`nel/include/nel/3d/material.h`'s own documented default-construction values (line 273):
shader=Normal(0), src/dst blend=srcalpha(2)/invsrcalpha(3), z_function=lessequal(5),
flags=ZWRITE|LIGHTING|DOUBLE_SIDED.

Validated via the bridge with a full round-trip per format: export a real `.shape`
(`zo_paneau_armure.shape`) to `.obj`/`.dae`, import it back with the new importer, save
as a new `.shape`, re-parse. Both formats landed on identical numbers (5 materials, 66
verts, 38 tris, all 5 texture names correctly recovered), matching the original. Found
and fixed two bugs in the process: a generator-exhaustion bug in the bbox min/max
computation (`xs`/`ys`/`zs` consumed twice), and `.dae`'s `Triangle.material` being a
`collada.material.Material` rather than directly an `Effect` (needed `.effect`). User
confirmed both imported shapes render correctly and are editable in object_editor.py's
Materials tab like any other shape.

## 2026-08-16 — 🐛 Fix CMeshMRM rendering the wrong (coarsest) LOD, and resolve geomorph placeholder wedges

Found while investigating Forgery's object_editor showing too few materials/an oddly
shaped flag on `zo_paneau_armure.shape`: `ryzom_forgery/shape_geometry.py`'s
`_passes_from_mrm_geom` read `geom.lods[0]`, assuming it was the finest (most detailed)
LOD. Verified against `nel/src/3d/mesh_mrm.cpp` (`CMeshMRMGeom::chooseLod`'s alphaMRM
math, and the "just first lod is loaded" progressive-streaming comment) and pynel's own
`MeshMRMGeom.num_triangles` property (already using `lods[-1]`) that `lods[0]` is
actually the *coarsest* LOD. Fixed to `lods[-1]`. Confirmed empirically: 18 of 41 real
multi-LOD `.shape` files sampled have a different material set between `lods[0]` and
`lods[-1]`, always fewer materials/triangles in `lods[0]` (e.g. `zo_paneau_armure.shape`:
3 materials/6 tris vs. 5 materials/38 tris).

Switching to the finer LOD surfaced a second, deeper bug: some of its vertices rendered
collapsed at the origin. Root-caused to `CMeshMRM`'s geomorph mechanism (see
`mrm_builder.cpp`'s wedge-decal step and `CMeshMRMGeom::applyGeomorph` in
`mesh_mrm.cpp`): the progressive mesh reserves a block of empty placeholder wedges
(position/normal/uv all zero, a default `CWedge()`) shared for smooth blending between
adjacent LODs, which pynel was silently discarding instead of parsing (fixed on the
`ryzom/pynel` branch: new `MrmLod.geomorphs`). `shape_geometry.py` gained
`_resolve_lod_geomorphs()`, which substitutes each placeholder wedge's channels with its
geomorph "end" wedge (the correct static resolution, matching `applyGeomorph`'s blend at
alphaLod=0) before yielding render passes for a LOD. Verified via the bridge: the
previously-collapsed flag vertices on `zo_paneau_armure.shape` now resolve to real,
coherent positions matching the rest of the geometry.

## 2026-08-16 — ✨ Add a Multi Bitmap material editor to Forgery's object_editor

Added editing of a `.shape`'s materials to `object_editor.py`, focused on the "Multi
Bitmap" mechanism (`CTextureMultiFile`, see `docs/material_options.md`'s dedicated
section): a per-material texture slot can hold up to 8 alternate images, of which one is
selected at runtime based on context (season, item quality, creature ecosystem...).

The panel gained a "Materials" / "All Properties" tab bar (the latter is the pre-existing
generic `draw_properties()` tree, unchanged). The new Materials tab lists every material,
grouped simple-texture materials first, then Multi Bitmap ones:
- Multi Bitmap materials are shown per *slot index* (0-7) rather than per material --
  picking a slot is a whole-shape appearance choice, so its "Select" button (green when
  active) switches every Multi Bitmap material to that index at once. Each slot's label
  shows all three known Georges/engine numbering conventions for that index side by side
  (quality tiers from `item_map.typ`, creature ecosystem from `_creature_texture.typ`,
  and season from `EGSPD::CSeason` in `ryzom/common/src/game_share/season.h` -- which one
  actually applies depends on the shape). An "Expand" toggle reveals a per-material row
  to hand-edit that slot's texture (editable filename + a native file-browse button).
- Simple-texture materials get the same editable filename + browse row.
- Every material row has a color-swatch button (opens a picker, plus a "No color"
  option) that swaps that material's 3D geometry to a flat color -- lighting/material are
  explicitly disabled for that override (`set_material_off`/`set_light_off`), since a lit
  NodePath shades from its attached Material rather than `set_color()`. This is a
  viewer-only visualization, never written to the `.shape`.
- Icon-only buttons (Font Awesome 4, the only icon font `imgui_bundle` ships a `.ttf`
  for -- merged into the default font in the shared `ryzom_forgery/app.py`, so any
  Forgery tool app can use `icons_fontawesome_4` glyphs) replace the earlier text
  buttons, each with a hover tooltip for discoverability.
- The doc-derived Multi Bitmap explanation (from `material_options.md`, via a new
  `ryzom_forgery/material_docs.py` that splits the doc on its `## Title {#key}` headers)
  shows in the status bar in orange on hover, rather than as an ImGui tooltip (too wide
  for that use).
- "Save" (asks for a one-time per-session overwrite confirmation, then writes back to
  the loaded `.shape`'s path) and "Save As..." (always prompts for a new path) persist
  edits via `pynel.ryzom_shape.save_shape` -- confirmed via the bridge that an edited
  Multi Bitmap selection survives a save + re-parse round-trip.
- Every typed or browsed texture file name is lower-cased at the point of entry too
  (on top of pynel's own read/write normalization, from the `ryzom/pynel` branch), so
  mixed-case names from real data don't linger once touched.

## 2026-08-15 — 📝 Add player-friendly material options doc, rename object_viewer to object_editor

Added `nel/tools/forgery/docs/material_options.md`, a plain-language (non-technical,
written in French for the target audience) explanation of every material option exposed
by the 3dsMax "NelMaterial" editor (`nel/tools/3d/plugin_max/scripts/startup/nel_material.ms`)
and stored in a compiled `.shape`'s `CMaterial`: base colors, opacity, specular/glossiness,
self-illumination, two-sided, lighting modes, shader type (Normal/UserColor/LightMap/
Specular/Water/PerPixelLighting...), blend (alpha blend vs. additive), alpha test, z-bias,
z-write, the 4 usable texture slots and their per-shader meaning, texture coordinate
generation, multi-texturing (operation/arguments/constant color), water-specific settings,
user color, and the texture matrix export flag.

Catalogued by reading `nel_material.ms` (the MaxScript UI), `export_material.cpp` (how
each UI parameter is serialized into `CMaterial`), and `nel/include/nel/3d/material.h`
(the runtime enums) -- confirmed empirically against 600 real `.shape` files from
`ryzom-data` via pynel that a material never carries more than 4 textures in practice
(`IDRV_MAT_MAXTEXTURES = 4`), even though the 3dsMax editor exposes 8 texture slots in its
UI (slots 5-8 exist only for the Water shader's editor-only settings, never written to
`CMaterial`).

Also documented the "Nel Multi Bitmap" texture type (`nel_multi_set.ms`,
`export_material.cpp:1217-1256`): not an extra texture slot, but a per-slot feature
letting a single texture channel hold up to 8 alternate images, runtime-selected via
`CTextureMultiFile::selectTexture()` (`texture_multi_file.h`) based on context (season --
`continent.cpp` -- or item quality/variant -- `character_3d.cpp`/`player_cl.cpp`). Already
parseable by pynel (`_parse_texture_multi_file` in `ryzom_shape.py`, pre-existing).

Grounded that section with real Georges sheet fields, cross-referenced in
`ryzom-data/leveldesign/DFN`: creature sheets' `Texture` field
(`_creature_texture_equipment.typ`, -1 = "Season" special value falls back to
season-based selection instead of a fixed index), items' `map_variant` field
(`item_map.typ`, the 8 quality tiers Low/Medium/High/Super/XL/Suprem/Divine/Obiwan
Quality, matching `player_cl.cpp`'s `selectTextureSet((uint)item->MapVariant)` call), and
`_creature_texture.typ` (none/Lacustre/Desert/Jungle/Primr/goo, a creature's
ecosystem-dependent appearance).

Renamed `apps/object_viewer.py` to `apps/object_editor.py` (class `ObjectViewerApp` ->
`ObjectEditorApp`, window title, debug log prefixes, and every doc/comment mention) ahead
of adding material editing to it -- "viewer" no longer described its role once it becomes
able to edit a `.shape`'s material, not just inspect it.

## 2026-08-15 — ✨ Add shape_exporter CLI app to Forgery

Added `nel/tools/forgery/apps/shape_exporter.py`, a command-line counterpart to the
object viewer's "Export to..." commands: `shape_exporter.py INPUT.shape OUTPUT.ext`
converts a single `.shape` to `.obj`/`.dae`/`.stl`/`.gltf`/`.glb`, the output format
picked from `OUTPUT`'s extension. Reuses `ryzom_forgery/shape_export.py`'s
`EXPORT_FORMATS` list and per-format writers directly, no GUI/Panda3D window involved.

Texture handling mirrors the GUI's two modes (`ryzom_forgery/export_config.py`), picked
via an optional `--data-root` (a Ryzom data tree, indexed with `AssetIndex` to resolve a
`.shape` material's texture file name to an actual file): without it, textures are left
as a reference to their original file name (`reference_only`); with it, defaults to
copying a decoded `.png` next to the export (`copy_png`), overridable with
`--texture-mode`.

## 2026-08-15 — 🐛 Support legacy (pre-1) CIndexBuffer format in pynel's .shape parser

`_parse_index_buffer` (`nel/tools/pynel/pynel/ryzom_shape.py`) previously raised
`ShapeParseError` on any `.shape` whose `CIndexBuffer` predates version 1 (NeL's
"primitive block" format, replaced by the flat index-buffer format in version 1) --
found via Forgery's new `shape_exporter.py` failing on a real production asset
(`sfx/mp_ressources_gen.shape`).

Added support for that `ver < 1` case, mirroring the exact byte layout of
`CIndexBuffer::serial`'s `ver < 1` branch in `nel/src/3d/index_buffer.cpp`: three
length-prefixed sections (lines, triangles, quads), each a `(count, capacity)` header
followed by its index vector. Only the triangle section carries renderable indices
(`_NbIndexes = triangle_count * 3`); the line and quad sections are read and discarded
to stay positioned correctly in the stream.
