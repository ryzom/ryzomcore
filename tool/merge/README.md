# core4 / atys merge playbook

How to bring the next batch of changes from `core4` or from the atys branches
into the shared merge branch without losing the clean diff again.

Everything here was used to build `feature/core4-atys` (atys-live + core4,
October 2026) and `feature/core4-gingo` (the same plus `main/gingo-test`).
The scripts need Python 3 only; `gen_db.py` additionally needs `lxml`.

## 1. The rules the branch is built on

* **core4 is authoritative for the shape of the code.** Formatting, file
  layout, refactorings, the CMake pipeline and CI, generated-file generators,
  tool names. A core4 change to a shared file must apply with a plain
  three-way merge, so the merge branch never carries another formatting of a
  core4 file.
* **atys is authoritative for what the game is.** Protocol, database layout,
  item and inventory formats, sheet formats, UI XML and Lua content, server
  gameplay, `web/`, `ryzom/server/www`, `README.md`. Those stay atys even when
  core4 has something newer, until atys decides to take the protocol step.
* **Data files take atys content in core4 formatting.** UI XML, Lua, config
  and translation-like files: the lines are what atys ships, the indentation,
  wrapping and line endings are core4's.
* **No autoformatting, ever.** Not on the branch, not on either parent. Hand
  alignment of comments and declarations is meaningful in this tree.
  Formatting is only ever applied to scratch copies as a measuring
  instrument (see `tokdiff.py`).
* **Generated files are regenerated, not merged.** CDB bank accessors
  (`database_plr/guild/outpost.{h,cpp}`) come from `gen_db.py`, module
  interfaces (`*_itf.{h,cpp}`) from `generate_module_interface.xslt`. A merge
  conflict in one of them means: merge the XML, regenerate, re-apply
  `NL_OVERRIDE`/`nullptr` with `tokapply.retidy()` (see `merge_step.py`).

### Things core4 has that are deliberately not in the branch

| core4 change | why it stays out | where the line is drawn |
|---|---|---|
| CDB nullable leaves and PACKED-from-4-bits (`6dfff8358`) | database stream format | `cdb_leaf`, `database.xml`, generated banks stay atys |
| client-tick position checks, rubber banding fix (`4407fc297`) | `POSITION` message format | `msg.xml`, `msg_client_server.h`, GPMS check stay atys |
| `MAX_OTHER_PET = 0`, pet inventory transactions (`4924b3563`) | inventory layout | `inventories.h`, `item_type`, pet DB branches stay atys |
| `CHARAC_BUFFS`, `ACCESS` inventory fields, `_DisplayItemQuality` (`24ceb5df0`, `d512d10f1`) | inventory sync format | atys inventory sync, guild chests, exchange slots |
| `LOCK`/`WRITE` message removal, exchange `ADD` format | client messages | `msg.xml` is atys |
| Lua ucstring API removal (`d11936a03`) | atys Lua scripts still use it | `RYZOM_LUA_UCSTRING` stays defined; its ucstring objects hold UTF-8, so the switch converts nothing |
| `web/` rewrite, core4 `README.md`, BuildInfo | atys-owned, expected to diverge | whole trees |

### Things atys has that core4 does not want upstream

PACS stairs trigger hack, `UInstance` null guards and the other crash
mitigations around Lua-placed objects, memcached command channel
(`WITH_MEMCACHED`), Breakpad handler (`WITH_BREAKPAD`), the `logs/`,
`docs/`, `macosx-build/`, `win-build/` directories. They live on the branch
because atys needs them; they are not candidates for `core4`.

## 2. Lineage you need before every step

The branch alternates checkpoint merges from each side. Every step needs the
**previous checkpoint of the same side**, because that is the base against
which conflicts are judged (what did *this side* actually change since it was
last merged). Find them with:

```sh
git log --first-parent --merges --format='%h %ad %s' --date=short feature/core4-atys | head
```

The commit subjects are `Merge core4 <sha> (<date>) into feature/core4-atys`
and `Merge atys main/<branch> <sha> (<date>) into ...`, so the last merged
checkpoint of each side can be read straight off. Keep that convention.

History facts that stay fixed: the last commit atys-live shares with core4
is `d8deff3f2` (October 2021). atys branch roles (their `BuildInfo.cmake`):
`main/atys-live` = live, fed by `main/rendor-staging`; `main/gingo-test` =
test; `main/yubo-dev` = dev, which also contains the hurried
`feature/atys-core-merge` attempt that must **not** be merged (take its
content through gingo/live instead, or cherry-pick).

## 3. One step, from either side

Work in a dedicated worktree so the main checkout and its build dir stay
usable:

```sh
git worktree add ../wt_merge feature/core4-atys
T=tool/merge          # scripts, from any checkout of core4 or the branch
```

### 3a. Merging a core4 checkpoint

Pick a checkpoint: a tagged release, a PR merge, or the tip. Big mechanical
passes (NL_OVERRIDE, clang-tidy, header updates) get a step of their own.

```sh
python $T/merge_step.py ../wt_merge <core4-commit> --side=core4 \
       --base=<previous core4 checkpoint> --tag=stepN
```

`--side=core4` runs `tokapply.py --verbatim`: wherever the branch did not
change the conflicted spot since the previous core4 checkpoint, the core4
text is inserted as-is (formatting included). Where atys changed the same
spot you get a `SKIP both-changed` and resolve by hand, keeping core4's
shape and atys's behaviour.

Policy for core4 steps:

* take core4's build files, CI, generators and tool layout outright
  (`git checkout --theirs` is usually right for `CMakeLists.txt`,
  `CMakeModules/`, `.github/`, `tool/`);
* keep `web/`, `README.md` ours (`git checkout --ours`);
* for the protocol table above, keep ours and re-check that the core4
  change did not sneak in through an auto-merged neighbour file;
* if core4 regenerated `database_*.{h,cpp}` or an `_itf` pair, rerun
  `gen_db.py` / the XSLT on the merged XML instead of merging the output;
* after an NL_OVERRIDE-style pass, `override_skips.py ../wt_merge stepN.apply
  --apply` annotates the atys-only declarations the same way so the next
  core4 step does not conflict on them again.

### 3b. Merging an atys checkpoint

Pick the atys commit (a live deploy tag, or the tip of `main/atys-live`,
`main/gingo-test`). `--base` is the previous atys checkpoint already in the
branch, `--ref` the current core4 checkpoint.

```sh
python $T/merge_step.py ../wt_merge origin/main/atys-live --side=atys \
       --base=<previous atys checkpoint> --ref=core4 --tag=stepN --restore-ws
```

`--side=atys` runs `tokapply.py --mech`: the atys change is applied on top
of the branch's text, re-wrapped into the branch's formatting, with
`NL_OVERRIDE` and `nullptr` differences folded (atys code never has them,
the branch always does). `--restore-ws` then undoes what the plain git
auto-merge took from atys that is whitespace only: files token-identical to
HEAD go back to HEAD, pure-whitespace hunks in mixed files are dropped.

Policy for atys steps:

* `web/`, `ryzom/server/www`, `README.md`, `logs/`, `docs/`: theirs;
* build files: if atys added a source file or a link dependency, add it in
  core4's CMake style (target-based, `NL_TARGET_*`); never take atys's
  `WITH_RYZOM_GAMESHARE` / `BuildInfo` / `WITH_SERVER_TOOLS` plumbing or
  the `user_agent.cpp` build-info line;
* if atys edited a generated file by hand, port the edit to the XML and
  regenerate;
* if atys copied a partial core4 feature by hand, the branch keeps the real
  core4 version; `tokapply --ref` marks such clusters `REF-HAS`;
* if atys reformatted a core4 file (clang-format, re-indent, EOL), nothing
  of that is taken; the content part, if any, comes through the applier;
* new atys directories are carried as-is and listed in the commit message.

### 3c. Finishing the step

1. Open `stepN.apply`. Every `HAND` file: look at the `SKIP` lines
   (`A a1-a2 B b1-b2` are line ranges in stage 2 and stage 3), edit the
   worktree file, `git add`. One-sided conflicts (`DU`/`UD`): decide whether
   the deletion or the edit wins by the rules above.
2. Go through the `POLICY-SENSITIVE` list even though git merged them
   cleanly.
3. `git add` the files the applier resolved, `git diff --cached --stat`
   against HEAD and sanity-check the areas touched.
4. Build. The branch is kept building on MSVC 2022 with
   `-DWITH_MONGODB=OFF -DWITH_PATCHMAN_SERVICE=OFF`; atys Linux server
   builds additionally use `-DWITH_BREAKPAD=ON -DWITH_MEMCACHED=ON`.
   Fix build breaks in a follow-up commit, not inside the merge commit.
5. Commit with the subject convention of section 2, listing the hand
   decisions in the body.
6. Verify the invariant: `git diff core4 feature/core4-atys --stat` must
   look like "atys content on core4", not like a reformat. The report
   tooling below makes this reviewable.

## 4. Checking the result

```sh
python $T/report/diffreport.py . core4 feature/core4-atys core4.json --jobs=6
python $T/report/render_report.py core4.json my_annot.py out_core4 --no-addrem
python $T/report/diffreport.py . origin/main/atys-live feature/core4-atys atys.json --jobs=6
python $T/report/render_report.py atys.json my_annot.py out_atys --compact
```

`diffreport.py` diffs two revisions on tokens: whitespace, wrapping and line
endings vanish, hunks that are only `NL_OVERRIDE`, `NULL/0 -> nullptr`,
copyright years or integer casts are counted per file instead of shown, and
whole-file adds/removes are one line each. `render_report.py` turns that into
a browsable HTML page; `--compact` hides every formatting-only or
mechanical-only file. `annot_example.py` is the annotation module of the
October 2026 atys report; copy it and rewrite `TITLE`, `SUBTITLE`, `INTRO`
and `NOTES`. Each takes about ten minutes on the full tree.

Single files: `tokdiff.py A B [--ref=C]` prints the content hunks with the
original line ranges of both files, so a hunk can be hand-applied.

## 5. Script reference

| script | job |
|---|---|
| `merge_step.py` | one checkpoint merge: git merge, conflict triage, applier over all conflicts, policy list, whitespace clean-up |
| `tokapply.py A B --base=BASE [--ref=REF] --out=F [--mech\|--verbatim]` | three-way, formatting-immune application of B's changes onto A; the core of every step |
| `tokdiff.py A B` | content diff on tokens, with original line numbers |
| `ws_restore.py WT [--lines] --apply files` | undo whitespace-only hunks the index took over HEAD |
| `remerge_files.py WT BASE THEIRS files` | reset files to git's plain three-way result (after a wrong hand edit) |
| `override_skips.py WT report --apply` | add `NL_OVERRIDE` to atys-only declarations after a core4 override pass |
| `gen_db.py REPO OUTDIR` | regenerate the server CDB bank accessors from `database.xml` |
| `report/diffreport.py`, `report/render_report.py` | the review reports |

`tokapply` prints one summary line (`clusters= apply= keep= skipped=
unexpected-residual=`) and exits non-zero when anything was skipped.
`apply` = only B changed that spot, B's change applied; `keep` = only A
changed it (or B merely reformatted it), A kept; `SKIP both-changed` = hand
work; `UNEXPECTED` = after applying, A still differs from B somewhere outside
the kept clusters, usually a token the tokenizer treats differently
(string literal spanning lines, `#include` reorder); look at it.

## 6. After atys has merged the branch

Once `feature/core4-atys` is in `main/atys-live`, atys becomes a plain
downstream: a core4 change merges into atys with `git merge core4` and no
applier, as long as the rules of section 1 keep holding on the atys side.
From that point the atys-side steps of this playbook are only needed if
atys reformats something again, and the core4-side steps become ordinary
merges. The protocol items of the table in section 1 can then be taken one
by one, each as a deliberate atys protocol step.
