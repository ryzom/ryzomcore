# PACS — collision/navigation format reference

Source of truth: `nel/include/nel/pacs/` (headers) and `nel/src/pacs/` (implementation).
This document maps the on-disk formats produced by the PACS build pipeline
(`.rbank`, `.lr`, `.gr`) and the build tools that generate them, as groundwork
for a future incremental-regeneration library in `pynel`.

**PACS is actually two largely independent subsystems**, and it's easy to
conflate them:

- **§1-7: `CGlobalRetriever` / `CLocalRetriever`** (`.gr`/`.rbank`/`.lr`) — the
  static walkable-surface graph used for ground detection and pathfinding.
  Build-time only today; see §7 for why it *could* be made incremental.
- **§9: `CMoveContainer` / `UMovePrimitive`** (`.pacs_prim`) — lightweight
  box/cylinder obstacle primitives used for movement collision. This one is
  **already fully dynamic at runtime**, no rebuild needed — see §9.

If the goal is "block server-side movement through decor placed by an
in-game tool", §9 is very likely the right target, not §1-7.

## 1. What PACS does

PACS (Physical Action Collision System) is NeL's 2.5D navigation/collision
layer, separate from the 3D landscape mesh. It answers "what surface am I
standing on, can I walk from A to B, where's the nearest wall" without
touching the renderer. It works on two kinds of "retrievers":

- **Outdoor / Landscape retrievers**: one per zone tile (160×160m), built by
  tessellating the zone heightfield + collision primitives into 2D convex-ish
  walkable surfaces bounded by chains (borders).
- **Indoor / Interior retrievers**: one per building/interior mesh (`.cmb`,
  exported from 3ds Max collision meshes), representing rooms/floors as
  surfaces linked by an exterior boundary mesh.

A **local retriever** (`CLocalRetriever`, `.lr` file) is the self-contained
geometry+topology of a single tile or interior. A **global retriever**
(`CGlobalRetriever`, `.gr` file) places many local-retriever *instances* in
world space and links their borders together to form one continuous
continent-wide walkable graph. A **retriever bank** (`CRetrieverBank`,
`.rbank` file) is just the array of local retrievers referenced by instance
index, shared by the global retriever.

## 2. Build pipeline (`nel/tools/build_gamedata/processes/rbank`)

Orchestrated by 4 python stages, which shell out to C++ tools built from
`nel/tools/pacs/`:

| Stage | Script | What it does |
|---|---|---|
| 0 | `0_setup.py` | Creates working directories only. |
| 1 | `1_export.py` | Runs 3ds Max headless with `maxscript/cmb_export.ms` to export indoor collision meshes from `.max` source files to `.cmb`. |
| 2 | `2_build.py` | Runs the actual build (see below). |
| 3 | `3_install.py` | Copies final `.rbank`/`.gr`/`.lr` to the client install directory (`data/pacs` typically). |

`2_build.py` runs these executables, in order:

1. **`build_ig_boxes`** (`nel/tools/3d/build_ig_boxes` — not in `nel/tools/pacs`)
   Reads every `.ig` (instance group placements) and their referenced
   `.shape` files, outputs one `.bbox` file: a flat list of `(igName, CAABBox)`
   pairs (see `CIGBox::serial` in `build_rbank.cpp`). Used later to know
   which decorative instances overlap a given zone (for elevation/level
   computation of surfaces, e.g. bridges vs. ground).

2. **`build_rbank`** (`nel/tools/pacs/build_rbank`), invoked three times with
   different CLI flags, driven by a generated `build_rbank.cfg`:
   - `-C -p -g` → `CheckPrims` only: runs `CPrimChecker` (see §5) to
     validate/rasterize `.primitive` level-design files, no output geometry.
   - `-c -P -g` → per-zone pass, `processAllPasses(zoneName)` in
     `build_rbank.cpp`: for each `.zonew` (welded zone geometry) that changed
     (or has a changed neighbor — `get_neighbors` tool), tessellates the zone
     into a `CZoneTessellation`, builds a `CLocalRetriever`, and writes
     `<zone>.lr` into `RbankSmoothBuildDirectory`. **This is the per-tile,
     independently-regenerable step.**
   - `-c -P -G` → global pass, `processGlobalRetriever()` in
     `build_rbank.cpp`: loads *every* `<zone>.lr` in the configured
     `ZoneUL`..`ZoneDR` rectangle into a `CRetrieverBank`, places one instance
     per zone in a `CGlobalRetriever`, links all instance borders together
     (`makeAllLinks()`), and serializes `temp.gr` + `temp.rbank` (with LR data
     embedded inline — see §3). **This step currently always processes the
     whole configured rectangle — the real incremental-regen bottleneck.**

3. **`build_indoor_rbank`** (`nel/tools/pacs/build_indoor_rbank`): reads every
   `.cmb` (indoor collision mesh), builds one `CLocalRetriever` per interior
   (type `Interior`), and — because `Merge=1` / `AddToRetriever=1` in its
   config — merges them into the outdoor `temp.rbank`/`temp.gr` produced by
   step 2, adding new instances and re-linking. Output:
   `tempMerged.rbank`, `tempMerged.gr`, `tempMerged_<i>.lr`.

4. Final rename: `tempMerged.*` → `<RbankRbankName>.*` (e.g. `fyros.rbank`,
   `fyros.gr`, `fyros_<i>.lr` — one `.lr` per retriever-bank slot index,
   hence the ~1144 `fyros_*.lr` files for the desert continent).

`3_install.py` just copies these three sets of files as-is to the client/server
data directory.

### Incremental-update detection that already exists

`2_build.py` already skips per-zone `.lr` regeneration when neither the zone
nor any of its direct neighbors' `.zonew` changed (via `needUpdate` timestamp
checks + `get_neighbors`). **But** stage `-G` (global assembly) and
`build_indoor_rbank`'s merge always reprocess everything from scratch — there
is no existing incremental path for `.rbank`/`.gr` regeneration. That's the
gap to fill.

## 3. `.rbank` format — `CRetrieverBank::serial` (`retriever_bank.h`/`.cpp`)

```
version:  uint32 (current = 1)
if version > 0:
    lrPresent: bool          # _LrInRBank flag
if lrPresent:
    retrievers: vector<CLocalRetriever>   # full LR data serialized inline
else:
    count: uint32
    # LR data NOT in this file; loaded lazily on demand from
    # "<NamePrefix>_<index>.lr" via CPath::lookup
```

Two usage modes, both produced by the pipeline:
- **Build-time intermediate** (`temp.rbank`): `_LrInRBank = true` (ctor
  default) → the whole `CLocalRetriever` array is embedded in the file.
- **Final installed bank** (`fyros.rbank`, via `saveShortBank()`):
  `_LrInRBank = false` → the `.rbank` only stores the retriever *count*, and
  each retriever is written separately by `saveRetrievers()` as
  `<prefix>_<index>.lr`. This is the format actually shipped/loaded by the
  client and server.

**Critical detail for incremental regen**: the index `<i>` in
`<prefix>_<i>.lr` is the **array position** in `_Retrievers`, which is simply
insertion order during the build (`processGlobalRetriever()`'s raster loop
over `y0..y1, x0..x1`, skipping missing zones, then indoor retrievers
appended after). It is *not* a stable zone id. Adding or removing a zone/tile
shifts every subsequent index, forcing a full rename of all downstream `.lr`
files and re-validation of every `CRetrieverInstance.RetrieverId` in the
`.gr`. Any incremental scheme needs either: (a) a stable id-to-slot mapping
kept separately, or (b) always appending new/changed retrievers at the end
and tombstoning removed ones instead of compacting the array.

## 4. `.lr` format — `CLocalRetriever::serial` (`local_retriever.h`/`.cpp`, current version 4)

```
version: sint (4)
Chains:            vector<CChain>            # topology: which surfaces border which
OrderedChains:     vector<COrderedChain>     # geometry: actual 2D polylines (packed CVector2s, snapped to 1/1024m)
FullOrderedChains: vector<COrderedChain3f>   # same, but float 3D — build-time only, usually flushed (see CRetrieverBank::clean())
Surfaces:          vector<CRetrievableSurface>
__Tips:            vector<...>               # chain endpoint junctions
BorderChains:      vector<uint16>            # subset of Chains indices that touch the retriever's bbox edge (need linking to neighbors)
ChainQuad:         CChainQuad                # spatial index over chains, for fast point-in-surface queries
BBox:              CAABBox
Type:              enum { Landscape, Interior }
ExteriorMesh:      CExteriorMesh             # interior only: boundary mesh linking to outdoor
if version >= 1:
    InteriorVertices, InteriorFaces          # for interior ground-height snapping
if version >= 2:
    FaceGrid:      CFaceGrid                 # spatial index over InteriorFaces
if version >= 3:
    Id:            string                    # e.g. the .cmb basename, for interiors
```

### `CChain::serial` (`chain.h`/`.cpp`)

```
SubChains:  vector<...>   # indices into OrderedChains that concatenate to form this chain
Left, Right: sint32        # surface id on each side; Right < -1 (or CChain::getDummyBorderChainId()) means "outside the retriever, needs linking"
StartTip, StopTip: ...
Length: float
LeftLoop, LeftLoopIndex, RightLoop, RightLoopIndex
```

### `COrderedChain::serial`

```
Vertices: vector<CVector2s>   # packed 16-bit local-space vertices (snapped, see CRetrieverInstance::SnapPrecision = 1024)
Forward: bool
ParentId, IndexInParent
Length: float
Min, Max: CVector2s            # bbox of the chain, added in v1
```

### `CRetrievableSurface::serial` (`retrievable_surface.h`/`.cpp`, version 2)

```
NormalQuanta, OrientationQuanta: ...
Material: uint32                  # surface material id, drives footstep sound/decal etc.
Character: ...
Level: ...                        # vertical stacking level (bridges over ground, etc.)
Chains: vector<...>               # chain ids bounding this surface
Loops: vector<TLoop>
Center: CVector2f
IsFloor, IsCeiling: bool
Flags: uint32                     # includes IsUnderWaterBit
WaterHeight: float                # v1+
QuantHeight: ...                  # v2+, replaced the old HeightQuad
```

## 5. Where surface data comes from at tile-build time (`processAllPasses`, `build_rbank.cpp`)

For an outdoor zone:
1. `CZoneTessellation::setup/build/compile` tessellates the zone's heightfield
   (`.zonew`) into 2D surface polygons, honoring `CPrimChecker`-rasterized
   `.primitive` zones (see below) for water/include/exclude/cliff/cluster-hint
   flags.
2. `generateBorders()` produces the border chain vertex loops.
3. A `CLocalRetriever` is filled: `addSurface()` per tessellated surface
   (water flag, water height, cluster hint, quantized height...),
   `addChain()` per border segment (marking chains with `Right < -1` as
   needing cross-zone linking via `CChain::getDummyBorderChainId()`).
4. `computeLoopsAndTips()`, `findBorderChains()`, `updateChainIds()`,
   `computeTopologies()`, `computeCollisionChainQuad()` finalize internal
   indices and the spatial-query acceleration structure.
5. Serialized to `<zone>.lr`.

### `CPrimChecker` (`build_rbank/prim_checker.h`/`.cpp`)

Reads `.primitive` files (Ligo level-design primitives, `LevelDesignWorldPath`)
and rasterizes zone/water/exclude/cliff/cluster-hint primitives into a sparse
256×256-cell grid (`CGrid`, lazily allocated per 256×256 cell, ~65536×65536
addressable cells total). Results are cached to disk
(`build(...)`/`load(...)`, with a `forceRebuild` flag) — this cache is
already keyed on content, so re-running `CheckPrims` is comparatively cheap
and not the bottleneck; the tessellation pass that *reads* the grid per zone
is the meaningful per-tile granularity.

## 6. `.gr` format — `CGlobalRetriever::serial` (`global_retriever.h`/`.cpp`, version 0)

```
version: uint32 (0)
Instances: vector<CRetrieverInstance>
BBox: CAABBox
```

On load, `initAll(false)` is called: recomputes the spatial `_InstanceGrid`
(quadgrid, 128 cells, 160m each — one cell per zone dimension) and the
`_RetrieveTable` scratch buffer. It does **not** recompute instance data or
links — those are all serialized as-is inside each `CRetrieverInstance`.

### `CRetrieverInstance::serial` (`retriever_instance.h`/`.cpp`, version 1)

```
InstanceId: sint32
RetrieverId: sint32        # index into the CRetrieverBank's _Retrievers array (see §3 caveat)
Orientation: uint8         # 0..3, 90° rotation of the local retriever in world space
Origin: CVector            # world-space translation
Neighbors: vector<sint32>  # instance ids spatially touching this one
BorderChainLinks: vector<CLink>   # one entry per this-instance's CLocalRetriever::BorderChains slot
BBox: CAABBox
NodesCount: uint16          # A*-pathfinding node table size (rebuilt, not content-serialized beyond count)
if version >= 1:
    Type: enum
    ExteriorEdgeQuad: CEdgeQuad   # interior only
```

### `CRetrieverInstance::CLink::serial`

```
Instance: uint16       # neighbor instance id, 0xFFFF = unlinked
BorderChainId: uint16   # index into the neighbor's own BorderChains
ChainId: uint16         # the neighbor's actual chain id (into its Chains vector)
SurfaceId: uint16       # the neighbor's surface id on that chain's far side
```

## 7. How border-chain linking actually works — the key to incremental regen

`processGlobalRetriever()` builds `CGlobalRetriever` like this:

1. `makeInstance(retrieverId, orientation, origin)` per zone/interior — just
   stores geometry+bbox reference, no linking yet.
2. `makeAllLinks()`:
   ```
   resetAllLinks()                    # clears every instance's Neighbors/BorderChainLinks
   for each instance n:
       makeLinks(n)
   ```
   `makeLinks(n)`:
   - `selectInstances(instance.getBBox(), cst)` queries the **quadgrid**
     (`_InstanceGrid`, purely spatial, bbox-overlap test) for candidate
     neighbor instances — *not* based on any stored adjacency list.
   - For each candidate neighbor, calls `instance.link(neighbor, retrievers)`
     and `neighbor.link(instance, retrievers)`.
   - `CRetrieverInstance::link()`: for every `BorderChain` on `this` instance
     without a link yet, computes its world-space start/stop tip vectors
     (translated by `neighbor.Origin - this.Origin`), and finds the
     **closest-matching border chain tip pair** on the neighbor (distance
     threshold ~1.0m by default, tightened later). When found, fills in the
     `CLink` (`Instance`, `BorderChainId`, `ChainId`, `SurfaceId`) on both
     sides symmetrically.
3. Any chain that still can't find a geometric match after the pass is a
   "faulty link" — `fixFaultyLinks()` in `build_rbank.cpp` attempts to stitch
   together fragmented/mis-tessellated chain segments across instance
   boundaries by reconstructing full polylines and re-splitting them to match
   (this is a fallback for tessellation seams, not the common case).
4. `initQuadGrid()` (again, redundant with step in `makeAllLinks`'s
   `selectInstances`, but ensures the grid reflects final instance set).
5. Serialize `.gr` (instances + bbox) and `.rbank` (LR array, inline for the
   temp file).

**Implication**: linking is a pure function of local geometry (border chain
tip positions) + spatial proximity (quadgrid). It does **not** require
reprocessing the whole continent — only:
- The changed instance(s) themselves.
- Their spatial neighbors (bbox-overlap via quadgrid — in practice the 8
  adjacent zone tiles, since each instance is ~160×160m and the quadgrid cell
  size matches).

A targeted incremental update for "regenerate zone Z" would be:
1. Rebuild `<Z>.lr` (already isolated, existing per-zone build step).
2. In the loaded `.gr`+`.rbank`: replace the retriever slot content for Z's
   index (same slot if Z already existed — **no index shift**, since only
   content changed, not slot count).
3. `resetLinks(Z's instance id)` + `resetLinks()` on each of Z's current
   `Neighbors` (to let stale links be recomputed) — cheaper than
   `resetAllLinks()`.
4. Re-run `makeLinks(n)` only for Z's instance id and its neighbor instance
   ids.
5. Re-serialize only the `.gr` (small — it's just the instance array) and
   Z's own `.lr`. The `.rbank` itself only needs rewriting if using the
   "inline" LR-in-rbank mode; the shipped "short bank" mode (`.rbank` = count
   only, `.lr` external) doesn't even need touching if the retriever count is
   unchanged.

The only scenario requiring a **slot-index-changing, and therefore wider**
rewrite is adding/removing a tile that changes `_Retrievers.size()` in a way
that shifts other tiles' indices — which is avoidable by never compacting:
reuse freed slots or always append, and keep a persistent zone-name → slot-id
map (which doesn't currently exist in the pipeline; the build script derives
it implicitly from raster order every time).

## 8. `.cmb` format — `CCollisionMeshBuild::serial` (`collision_mesh_build.h`)

`.cmb` is **not** a final PACS artifact — it's the raw triangle-soup
interchange format exported from 3ds Max (stage 1, `1_export.py`/`cmb_export.ms`,
see §2) and consumed by `build_indoor_rbank` to *produce* an indoor `.lr`
(§4). One `.cmb` per indoor ig-group (building/interior), named `<igname>.cmb`.

### 8.1 On-disk format

Literally just `CCollisionMeshBuild::serial()` — no version, no magic check:

```
Vertices: vector<CVector>         # world-space (already baked with the node's object-to-world matrix)
Faces:    vector<CCollisionFace>
```

`CCollisionFace::serial`:
```
V[0..2]:          uint32   # indices into Vertices
Visibility[0..2]: bool     # per-edge, see §8.2b — true = hard boundary (never linked), false = opening (becomes a linkable border chain)
Surface:          sint32   # author-assigned grouping tag; -1 = CCollisionFace::ExteriorSurface (boundary mesh), >=0 = an arbitrary interior "room/floor" tag
Material:         sint32   # Max material id, carried straight through to CRetrievableSurface::Material — and load-bearing for exterior/opening faces, see §8.2b
```
Not serialized (computed on load by `CCollisionMeshBuild::link()`):
`Edge[0..2]` (adjacent face index per edge, -1 if none), `InternalSurface`
(flood-fill result, see below), `EdgeFlags[0..2]`.

### 8.2 How it's authored (3ds Max side)

Source (primary, now confirmed): the community tutorial
["How to create collisions in a landscape"](https://wiki.ryzom.dev/landscape/collisions_create)
(original: nevrax.org, archived
[here](https://web.archive.org/web/20030609155621/http://www.nevrax.org/docs/contrib/creating_landscape_collisions.html)).
It documents the authoring workflow directly, independent of (and more
authoritative than) reading the exporter code for *intent*.

Artists flag geometry nodes with `NEL3D_APPDATA_COLLISION` (interior
floor/wall collision) or `NEL3D_APPDATA_COLLISION_EXTERIOR` (the outer
boundary mesh that will later link the interior instance back to the outdoor
landscape retriever — becomes `CExteriorMesh`, all its faces get `Surface =
-1`). Nodes are grouped into one `.cmb` per `NEL3D_APPDATA_IGNAME` value
(`nel_mesh_lib/export_collision.cpp`'s `createCollisionMeshBuildList`).

**Every collision system is a *pair*: one interior mesh + one exterior mesh,
always both present even if the exterior mesh does nothing useful on its
own.** The interior mesh is what the player actually walks on (becomes the
indoor `CLocalRetriever`'s surfaces, §8.3). The exterior mesh is the collider
seen from the landscape side, *and* the bridge back onto it.

### 8.2b `Visibility` — the semantics, corrected

**This was gotten backwards in an earlier revision of this document — the
tutorial's images and the actual `build_indoor_rbank` code agree, and both
disagree with what was written before. Corrected here.**

An "open" edge is one that belongs to only one face (no triangle neighbor —
`Edge[edge] == -1`). Whether that open edge is *visible* or *invisible* in
3ds Max (the native Editable Mesh edge-visible/invisible toggle, repurposed)
decides what happens to it:

- **Visible edge** (`Visibility=true`) → a **hard boundary, never linked**.
  In code: `build_surfaces.cpp`'s `computeSurfaceBorders()` sets
  `border.Right = -1` (a literal sentinel, *not* converted to a border-chain
  id); `build_indoor.cpp`'s `buildExteriorMesh()` sets `link = -1` on the
  exterior mesh side, same sentinel. Nothing ever tries to match this edge to
  anything else. On the exterior mesh: this is what blocks a player walking
  in from the landscape. On the interior mesh: this is what stops a player
  from falling off the side of a ramp/platform.
- **Invisible edge** (`Visibility=false`) → **the connection itself.**
  `computeSurfaceBorders()` sets `border.Right = -2`, which
  `build_surfaces.cpp:316` then converts to a real border-chain id
  (`CChain::convertBorderChainId(numBorderChains++)`) — i.e. it becomes part
  of `_BorderChains` and is eligible for geometric matching (§7's linking
  mechanism, generalized to indoor). `buildExteriorMesh()` does the mirror
  operation on the exterior-mesh side (`link = numLink++`). The interior
  mesh's invisible edges and the exterior mesh's invisible edges must
  literally be superposed in space — matching pairs of invisible edges are
  what stitches the walkable interior surface onto the surrounding exterior
  collider/landscape.

Concretely, for a ramp rising from the ground: the exterior mesh's visible
edges block the player coming from the landscape; the interior mesh's
visible edges are the ramp's side rails (don't fall off); the *invisible*
edges of both meshes, superposed at the ramp's base, are what lets the player
actually step from the ground onto the ramp surface.

**A "true everywhere" default (what an earlier draft of this doc
recommended) is wrong for real geometry**: it would make every visible
boundary edge of a room — i.e. every real wall — into a hard, unlinked
sentinel, which happens to be *safe* (walls stay solid) but makes doorways
default to walls too, i.e. **nothing would be enterable**. A "false
everywhere" default is the genuinely dangerous one: every wall edge would
become a border chain the engine tries to link somewhere, which is the
actual footgun. Neither blanket default is usable for apartments/stairs —
door/opening edges must be explicitly marked invisible, walls explicitly
visible.

### 8.2c Author-time constraints (from the tutorial, "Objects informations and constraints")

These are validated by convention/discipline in 3ds Max, not enforced by the
file format itself — worth encoding as explicit checks in any pynel exporter:

- Interior and exterior mesh nodes must share the same non-empty
  `NEL3D_APPDATA_IGNAME` (this is the `.cmb` grouping key, §8.2).
- The interior mesh node has `Collision Mesh` checked; the exterior mesh node
  has **both** `Collision Mesh` and `Collision Mesh Exterior` checked.
- **Both meshes must have the exact same number of invisible open edges**,
  and the interior mesh's vertices along those edges must be positionally
  **snapped** (identical coordinates) to the exterior mesh's — this is what
  the matching pass actually keys on.
- **The exterior mesh's faces must all have Material ID 666 — as entered in
  3ds Max's (1-based) Material ID field.** The `.cmb` file's `Material` field
  is 0-based, so **the on-disk value is 665**, not 666. Confirmed against
  real production assets in `ryzom-data`: every `.cmb` there that has any
  `Surface == -1` face has `Material == 665` on all of them, consistently
  (`fy_cn_module_small_inner_empty_col.cmb` is the one exception in the small
  sample checked, using `Material == 0` on its exterior faces — status
  unclear, possibly a placeholder/dummy collision, not investigated further).
  Player-apartment `.cmb` files (`FY_appart_joueur.cmb`,
  `TR_appart.cmb`, `ZO_bt_Appart.cmb`, `MA_appart_joueur.cmb`) have **no**
  `Surface == -1` faces at all — confirmed intentional: these apartments are
  entered by teleport, stay sealed, and simply have no exterior/landscape
  connection to author.
- **On the interior mesh, every face touching a given opening (invisible
  edge) must share one single, unique material id.** The reference exporter
  splits faces by material id into separate root meshes/nodes at export time
  (see `export_collision.cpp` — one `SNodeMesh`/root per node, and separately
  each node's faces get a `Surface` derived from `totalSurfaces + matId`, so
  a mixed-material face group spanning an opening ends up split across two
  different `Surface` tags where it should have been one) — mixing materials
  across an opening silently generates extra, non-matching open edges instead
  of the intended single seam.
- **Known engine limitation: no true bridges.** The exterior mesh cannot be
  split into two disconnected pieces to represent "walkable on top AND
  underneath" the same structure (e.g. a foot-bridge you can also walk under).
  For your pontoons: if under/over passage matters, that's a strong signal to
  build that particular case as a `CMoveContainer`/`.pacs_prim` box obstacle
  (§9) rather than a `.cmb` interior retriever — the box-primitive system
  naturally supports "occupies this height range only" via `Height` +
  `Position.z`, which `.cmb`/interior retrievers structurally cannot express
  for a single connected structure.

**A from-scratch, non-3dsMax re-implementation of this exact export already
exists in this repo**: `nel/tools/3d/pipeline_max_export_cmb/main.cpp`. Its
header comment is a precise, field-by-field spec of the whole algorithm —
worth reading directly if pynel ever needs to go from arbitrary mesh data
(e.g. a `.dae` import) to a valid `.cmb`:
- per-face `Visibility[0..2]` bit remapping from Max's edge-visibility bits
  (index 0 ← Max's `EDGE_B`, 1 ← `EDGE_C`, 2 ← `EDGE_A`),
- `Material` = the face's Max material id; `Surface` = `-1` when the node's
  own `COLLISION_EXTERIOR` flag is set, else a running counter,
- cross-node vertex welding within a group via a real quadgrid (1m cells,
  5mm threshold; ascending-index/first-writer-wins), never welding within
  the same node,
- degenerate faces (two welded indices equal) dropped after welding,
- runs the real `CCollisionMeshBuild::link(false,...)`/`link(true,...)`
  edge-consistency validation before writing; a group that fails is skipped
  entirely (not the whole export).

### 8.3 How it's consumed — `build_indoor_rbank/build_surfaces.cpp`

`buildSurfaces(cmb, lr)` turns the flat triangle soup into a `CLocalRetriever`:

1. `floodFillSurfaces()`: for each face with `Surface != ExteriorSurface`,
   flood-fills across edges (`Edge[]` from `link()`) to every neighbor face
   sharing the **same `Surface` tag**, assigning a shared `InternalSurface`
   id — i.e. one connected component per author-defined room/floor becomes
   one `CInteriorSurface`. Exterior-tagged faces are excluded (they never
   become a walkable surface, only the boundary mesh).
2. `computeSurfaceBorders()`: for every face edge that either has no
   neighbor or crosses into a different `InternalSurface`, emits a border
   segment. `Right` is set to: the neighboring `InternalSurface` id if
   crossing to another indoor surface; otherwise (naked edge) `-1` if
   `Visibility[edge]` is true (hard boundary, never linked further — see
   §8.2b) or `-2` if `Visibility[edge]` is false (an opening — gets converted
   to a real border-chain id and later geometrically matched).
3. Each `CInteriorSurface` becomes one `CRetrievableSurface` via
   `lr.addSurface(0, 0, Material, 0, 0, false, 0.0f, false, Center, quad)` —
   `Material` is passed through unchanged from the `.cmb` face data (see §4's
   `CRetrievableSurface::Material` field).
4. The exterior-flagged faces build the instance's `CExteriorMesh`, used at
   global-retriever build time (`CRetrieverInstance::initEdgeQuad`/
   `linkEdgeQuad`, §7) to stitch the interior instance's open borders back
   onto the surrounding outdoor landscape instance.

## 9. Dynamic obstacle primitives — `CMoveContainer` / `.pacs_prim` (separate from `CGlobalRetriever`)

This is a **different, simpler, and already-dynamic** PACS subsystem, unrelated
to `.gr`/`.rbank`/`.lr`. It doesn't describe walkable surfaces or borders — it's
a flat collection of box/cylinder collision volumes checked against moving
entities.

### 9.1 Format — `CPrimitiveBlock` / `CPrimitiveDesc` (`primitive_block.h`)

A `.pacs_prim` file is a serialized `CPrimitiveBlock`: `std::vector<CPrimitiveDesc>`.
Each `CPrimitiveDesc`:

```
Length[2]:      float   # box: width, depth. cylinder: radius (first slot only)
Height:         float
Attenuation:    float
Type:           UMovePrimitive::TType        # box or cylinder
Reaction:       UMovePrimitive::TReaction
Trigger:        UMovePrimitive::TTrigger
Obstacle:       bool
OcclusionMask:  UMovePrimitive::TCollisionMask
CollisionMask:  UMovePrimitive::TCollisionMask
Position:       CVector                      # local to the block's own origin
Orientation:    float                        # free rotation, OZ, no quantization
UserData:       UMovePrimitive::TUserData
```

`.pacs_prim` files are produced today by hand in 3ds Max
(`nel/tools/build_gamedata/processes/pacs_prim`, artists place
`nel_pacs_box`/`nel_pacs_cylinder` helper objects, exported via
`NelExportPACSPrimitives`) — there is no tooling today that derives a
`.pacs_prim` automatically from a shape's mesh/bbox.

### 9.2 How it's loaded and instantiated — `CContinentContainer` (`ryzom/server/src/server_share/continent_container.cpp`)

At continent startup (`initPacsPrim()` / `loadPacsPrims()`):

1. Every `.pacs_prim` found is loaded once into `_PacsPrimMap`, keyed by
   **its filename without extension, lowercased** — which is expected to
   match a `.shape` name (e.g. `tree01.pacs_prim` ↔ `tree01.shape`).
2. For every instance in every landscape `.ig`, the shape name is looked up
   in `_PacsPrimMap`. On a match, the instance's real world matrix is decomposed
   (`UMoveContainer::getPACSCoordsFromMatrix(pos, angle, matrix)` — position +
   **free-float angle**, no 90°-quantization unlike `CRetrieverInstance`), and:
   ```cpp
   moveContainer->addCollisionnablePrimitiveBlock(pb, 0, 1, &insertedPrimitives, angle, pos, true);
   ```
   This instantiates a *copy* of every `CPrimitiveDesc` in the block, transformed
   to that instance's position/rotation, and returns the resulting
   `UMovePrimitive*` list.
3. A special-cased fallback also keys some blocks by *instance name* instead of
   shape name, used for gameplay triggers (`isTrigger` path, feeds `_TriggerMap`).

`addCollisionnablePrimitiveBlock` also accepts a `scale` parameter
(`const CVector &scale = CVector(1,1,1)`, see `u_move_container.h:123`) — so
position, rotation (free) and non-uniform scale are all supported natively at
instantiation time.

### 9.3 Confirmed: this is a genuinely live, per-instant runtime API

`UMoveContainer::addCollisionablePrimitive()` / `addNonCollisionablePrimitive()`
/ `addCollisionnablePrimitiveBlock()` and `removePrimitive()` are not
init-only calls guarded by some "world sealed after load" invariant — they're
exactly what the server calls **every time any entity spawns or despawns**:

`ryzom/server/src/gpm_service/world_entity.cpp:287-328` — every player, mob or
NPC entity creates its own collision primitive on spawn
(`pMoveContainer->addCollisionablePrimitive(...)` /
`pMoveContainer->addNonCollisionablePrimitive(...)`) and removes it on
despawn (`MoveContainer->removePrimitive(Primitive)`), continuously during
normal play — this is the `gpm_service`'s bread-and-butter operation, not an
edge case.

### 9.4 Implication for the in-game scene tool

For "spawn decor in a live scene and make server-side entities collide with
it", `CMoveContainer` is the right target, **not** `CGlobalRetriever`/`.gr`:

- No rebuild, no patch, no restart. A service holding the continent's
  `MoveContainer` (this is `gpm_service`, via `CWorldPositionManager` /
  `CContinentContainer` — same place that owns `_PacsPrimMap`) can add/remove
  primitives on demand.
- For shapes that already have a hand-authored `.pacs_prim` (precise
  footprint — e.g. a narrower cylinder than the canopy so you can walk under
  a tree, or a raised box so you can walk under a bridge): reuse
  `addCollisionnablePrimitiveBlock()` with the placement's position/rotation/scale
  from the scene JSON — exactly what already happens for landscape `.ig`
  instances at continent load, just triggered dynamically instead.
- For shapes without one (today: "just a quad from the bbox"): build a single
  box `CPrimitiveDesc` in-process from the shape's bbox (no `.pacs_prim` file
  needed at all — `addCollisionablePrimitive()` takes a primitive directly)
  and call the same API. This is strictly less precise than a hand-authored
  block (no "walk under" cutouts) but requires zero offline asset step.
- Keep the returned `UMovePrimitive*` (or an id derived from it) per placed
  scene object so it can be `removePrimitive()`'d if the scene is edited or
  torn down.
- Still to verify: what RPC/service entry point (if any) already exists to
  reach `gpm_service`'s `MoveContainer` from outside the service process, or
  whether a new message needs to be added for this (the existing dynamic
  add/remove calls above are all internal, same-process C++ calls — nothing
  observed yet driving them from a network message).

## 10. Open items / what pynel would still need to determine experimentally

- Exact byte layout of `NLMISC::IStream` primitives used throughout
  (`CVector2s` packing, `serialCont` container framing, `serialVersion`
  encoding, `serialCheck` magic numbers like `PCHK`) — needed to write a
  pure-Python binary reader/writer. Should already be partly known from
  pynel's existing `.ig`/`.shape`/`.bnp` work if it reuses the same NeL
  stream primitives (worth checking `nel_message.py`/`json_serde.py`).
- `CChainQuad`, `CFaceGrid`, `CExteriorMesh`, `CEdgeQuad` binary formats —
  not yet dumped field-by-field here (secondary, query-acceleration
  structures; only strictly needed if pynel wants to *read* geometry, not
  just patch/relink retrievers).
- Whether the live server actually needs `.gr`/`.rbank` reload without a
  restart, or whether "incremental" only needs to mean "the *build* is fast
  and produces byte-identical output to a full rebuild for unaffected tiles"
  — changes how much of this needs to be a runtime-safe hot-reload path vs.
  just a faster offline tool.
