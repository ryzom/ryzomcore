# pynel roadmap — landscape/collision tooling

Working notes on the broader goal behind the PACS work in `pacs_format.md`:
a Python, dependency-free partial replacement for Ryzom Studio, covering just
what's actually used day-to-day — the landscape editor and primitives — not
a full Studio reimplementation (Studio's C++/Qt stack is slow to iterate on).

## Where things stand

- `pynel` already reads/writes `.ig`, `.shape`, `.bnp` — round-trip tested
  bit-perfect.
- `docs/pacs_format.md` documents the full PACS picture: `.rbank`/`.lr`/`.gr`
  (the walkable-surface graph and its build pipeline), border-chain linking
  (and why it's geometric/local, not continent-global — relevant to any
  future incremental rebuild), `CMoveContainer`/`.pacs_prim` (the separate,
  already-live obstacle-primitive system), and `.cmb` (the indoor collision
  interchange format, including the corrected `Visibility` semantics sourced
  from the community tutorial at
  [wiki.ryzom.dev/landscape/collisions_create](https://wiki.ryzom.dev/landscape/collisions_create)).

## Done — `.cmb` read/write in pynel

`nel/tools/pynel/pynel/collision_mesh_build.py` reads and writes `.cmb`
(round-trip tested against synthetic data, and validated against all 8 real
`.cmb` files found in `ryzom-data` — parses cleanly, no errors). CLI:
`ryzom-cmb dump <file>`. This is step 1 of the intended 3-step separation for
this whole area of work (per Nuno: keeps things flexible/incremental):

1. **pynel reads/writes the raw Ryzom file formats** (done for `.cmb`).
2. pynel exports to another format, if/when needed.
3. A 3D viewer tool integrates the pynel lib to actually render something.

Step 3 (an actual 3D viewer/engine integration) is explicitly **out of scope
for pynel itself** — it'll happen as part of a separate, bigger project later,
not here. Don't start building a viewer inside `pynel` unless that's revisited.
See "Ryzom Forgery" below for where step 3 actually lives.

## Done — `.zone`/`.zonew`/`.zonel` + `.land` read/write, native tool orchestration

`nel/tools/pynel/pynel/ryzom_zone.py` reads and writes `CZone`
(`.zone`/`.zonew`/`.zonel` -- same binary format at three different pipeline
stages, see `docs/zone_format.md`) round-trip. `ryzom_land.py` does the same
for `.land` (`CZoneRegion`, XML on disk -- see `docs/land_format.md`).

Since welding (`.zone`→`.zonew`) and lighting (`.zonew`→`.zonel`) are real
geometric processing (tangent smoothing between neighbor zones, lumel/point
light baking) already implemented in C++, pynel doesn't reimplement them --
it drives the existing native tools (`zone_welder`, `zone_lighter`,
`land_export`) as subprocesses instead. `ryzom_zone_tools.py`
(`run_zone_welder`/`run_zone_lighter`) and `ryzom_land_tools.py`
(`run_land_export` + `.cfg` generation from a dataclass) are thin wrappers
around this -- no automatic binary discovery, no parsing of the (mostly
free-text) tool output; the caller validates the result via
`ryzom_zone.load_zone()`/`ryzom_land.load_land()`. Full CLI contracts (args,
exit-code quirks, `.cfg`/`.depend` file formats) documented in
`docs/zone_tools.md`.

Validated end-to-end against real `ryzom-data`: `land_export` on the real
`nexus.land` (jungle ecosystem) produces 151 `.zonenh`, all re-parsed
successfully; `zone_welder` + `zone_lighter` on a real `.zone` produce a
valid `.zonew` then `.zonel`, both re-parsed successfully. Two native Linux
bugs were found and fixed along the way (hardcoded `\` path separators and
`world_editor` being Windows-gated in `land_export`'s CMake; a stack
corruption + an uninitialized main-thread handle in `zone_lighter`'s CPU
affinity code) -- see `logs/ryzom-core.md` in `ryzom-core` for the second
one, both fixes are in `ryzom-core` itself, not in pynel.

Zone welding is single-pass per the real pipeline's own convention (one
alphabetical pass over `zone_exported/*.zone`, writing into the same output
directory each call) -- orchestrating a full-grid, multi-zone build (calling
`zone_welder`/`zone_lighter` in the right order across many zones, plus
`zone_elevation`/`zone_dependencies`/`zone_ig_lighter`) is tracked
separately in `project-todos/pynel/land_pipeline.md`, not done here.

## Ryzom Forgery — the step-3 viewer/tool suite (design settled, not started)

**Goal**: cover step 3 above (and future editor-style tooling beyond just
viewing) without reproducing Ryzom Studio's problem — a single monolithic
C++/Qt app that's slow to iterate on. Instead: a **suite of separate, small
apps**, each built on `pynel` (file I/O) + Panda3D (rendering) + Dear ImGui
(tool UI), launched contextually by `ryztart` as its own subprocess — the
same pattern `ryztart` already uses to launch the Ryzom game client itself.
One app per tool, not one app with many modules.

**Decisions locked in**:
- **Engine**: Panda3D. Rejected PyOpenGL/moderngl (too low-level, would mean
  building an engine from scratch). Rejected UPBGE: it would tie the tool to
  running inside Blender specifically, but the `.dae` authoring tool is meant
  to be the graphic artist's free choice (Blender, Cinema4D, whatever) —
  export happens *through* the new tool, not by requiring one particular
  editor to host it.
- **Process model**: each tool is a standalone subprocess, launched by
  `ryztart` (mirrors how it already launches the Ryzom client via
  `subprocess.run` in `ryzom_installer/installer.py`). Rejected embedding
  Panda3D inside `ryztart`'s pywebview window or running it in-process
  alongside pywebview's event loop — pywebview only renders web content
  (can't host a native Panda3D GL context at all), and even an in-process
  approach would mean two frameworks fighting over the main thread/event
  loop.
- **Tool UI**: Dear ImGui (via the `panda3d-imgui` binding), rendered as an
  overlay directly in Panda3D's own GL context — no separate window, no
  second toolkit to embed. Rejected Qt (`WindowProperties.setParentWindow`
  embedding): Studio already being Qt doesn't save real work here, since none
  of Studio's C++ widget code is reusable from Python — every widget gets
  written from scratch either way, so Qt would only add a heavy dependency
  and window-embedding plumbing for no code-reuse benefit. ImGui covers the
  needed widgets natively, including trees (`TreeNode`) for nested primitive
  hierarchies.

**First tool: the object viewer** (`.shape` files). Scope: list of shapes,
3D display, properties panel, texture selection (when a shape has more than
one), pivot point editing. Primitive hierarchy shown as an ImGui tree.

**Repo/package layout**: `nel/tools/forgery/` holds `ryzom_forgery`, the
shared package with common Panda3D+ImGui scaffolding (window/camera/input,
and the standard sysinfo/explorer/panel layout) reused across the separate
tool apps. It depends on `pynel` (the standard explorer browses `.bnp`
contents), installed as an editable sibling checkout via `dev.sh` since
`pynel` isn't published to PyPI.

## Backlog chantier 1 — `.dae → .cmb` exporter (not started)

**Goal**: author indoor collision (apartments, stairs, and outdoor structures
that need real indoor-style retrievers like pontoons) in Blender (priority)
or Cinema4D, instead of requiring 3ds Max.

**Reference to mirror** (algorithm only — it's 3dsMax-specific, can't be
reused directly): `nel/tools/3d/pipeline_max_export_cmb/main.cpp`. Its header
comment is a precise spec of the merge/weld/validate pipeline a `.dae`
importer would need to replicate: cross-node vertex welding (quadgrid, 1m
cells, 5mm threshold), degenerate-face dropping after welding, and running
the real `CCollisionMeshBuild::link()` edge-consistency check before writing.

**Design decisions already locked in**:
- **Grouping / `COLLISION` / `COLLISION_EXTERIOR` metadata**: encoded via a
  **naming convention** on mesh/node names — not COLLADA `<extra>` custom
  properties (fragile across exporters), not a sidecar file. Chosen for
  portability between Blender and Cinema4D.
- **Opening marking** (which boundary edges are passable vs. solid wall —
  `CCollisionFace::Visibility`, see `pacs_format.md` §8.2b/c): a dedicated
  material, e.g. `pacs_opening`, assigned to the faces touching a door,
  stairwell, or ramp base. Everything else defaults to a wall (visible,
  never-linked edge). The opening must be marked on **both** the interior and
  exterior mesh, at positionally-snapped vertices — that's what the engine's
  build actually keys its stitching on.

**Still open, to validate when this chantier starts** (never decide
unilaterally — confirm with Nuno first):
- How to derive per-room `Surface` grouping. The original Max pipeline
  overloads material-id for two unrelated things at once: "which room" and
  "the real floor material / footstep sound". For a Blender-first workflow,
  decoupling these (e.g. one mesh object = one room, a separate material for
  the real floor sound) was floated as more natural — not decided.
- Whether `.cmb`/interior-retriever is even the right target for pontoons
  specifically, given the tutorial's documented limitation: **no true
  bridges** — one exterior mesh can't represent "walkable on top AND
  underneath" the same structure. `CMoveContainer`/`.pacs_prim` box
  primitives (§9 of `pacs_format.md`) support that natively via
  `Height`/`Position.z`, so a pontoon with an underside might belong there
  instead.

## Backlog idea 2 — live decor collision via an in-game scene tool

Separate from the `.cmb` work, and already confirmed *architecturally
feasible without any engine change*: an in-game 3D scene-placement tool
(driven from Lua) that exports a list of shapes + pos/rot/size + a collision
modifier could create real server-side collision **at runtime**, no patch or
rebuild:

- `UMoveContainer::addCollisionnablePrimitiveBlock()` /
  `addCollisionablePrimitive()` / `removePrimitive()` — confirmed live-safe
  because `gpm_service` already calls these on **every entity spawn/despawn**
  (`ryzom/server/src/gpm_service/world_entity.cpp`), not just at continent
  load. Full details in `pacs_format.md` §9.
- Shapes with a pre-authored `.pacs_prim` (precise footprint, e.g. narrower
  than the visual mesh so you can walk under a tree) get it automatically
  today when placed via a landscape `.ig`, keyed by shape name
  (`CContinentContainer::_PacsPrimMap`, see §9.2). For shapes without one, a
  box `CPrimitiveDesc` can be synthesized from the bbox instead, no
  `.pacs_prim` file needed.

**Open question, not yet investigated**: what RPC/service entry point (if
any) already exists to reach a running `gpm_service`'s `MoveContainer` from
outside the process — the calls above are all internal same-process C++
today, nothing observed yet driving them from a network message.
