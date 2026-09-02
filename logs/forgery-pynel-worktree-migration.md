# Changelog

## 2026-09-02 — 🔧 Resolve pynel path via sibling worktree in dev.sh, fall back to old layout

Forgery and pynel were split out of `ryzom-core` into their own sparse-checkout
git worktrees (`/home/ulukyn/repos/forgery`, branch `ryzom/forgery`, sparse
`nel/tools/forgery`; `/home/ulukyn/repos/pynel`, branch `ryzom/pynel`, sparse
`nel/tools/pynel`), sharing `ryzom-core`'s remote/history. `dev.sh`'s 3
references to pynel via the relative `../pynel` (i.e. `nel/tools/pynel` in the
same checkout) broke as a result -- that path no longer exists once Forgery's
own sparse-checkout stopped including `nel/tools/pynel`.

Fixed with an auto-detection loop, tried in order: `../../../../pynel/nel/tools/pynel`
(the sibling worktree layout) first, `../pynel` (the old mono-checkout layout)
as a fallback -- each candidate accepted only if it actually has a
`pyproject.toml` with `name = "pynel"`, so a stray directory of the same name
can't be picked up by accident. Exits with a clear error if neither resolves,
instead of the previous silent-if-missing relative path.
