#!/usr/bin/env python3
"""Where a core4 step changed what the merge branch differs from core4 in.

Usage: residual_diff.py WT BASE TIP PRE OUTDIR

WT      worktree on the merge branch with the step staged or committed (the index is read)
BASE    the previous core4 checkpoint already in the branch
TIP     the core4 commit merged in this step
PRE     the branch commit before the step
OUTDIR  one .interdiff file per file whose residual changed

For every file core4 changed, the branch's residual against core4 is the set of +/- lines of `git diff core4 branch`
for that file. The invariant of the branch is that a core4 step leaves the residual alone: the same atys content on
the new core4. This compares the residual before the step (BASE vs PRE) with the one after (TIP vs index) and
writes the difference, which is exactly the hand resolution of the step plus whatever the merge got wrong. Lines
that merely moved show up as a removal and an addition of the same text; everything else is to be read.
"""
import difflib
import os
import subprocess
import sys

if len(sys.argv) != 6:
    print(__doc__)
    sys.exit(2)
WT, BASE, TIP, PRE, OUT = sys.argv[1:6]
os.makedirs(OUT, exist_ok=True)


def git(*a):
    r = subprocess.run(['git', '-c', 'core.autocrlf=false'] + list(a), cwd=WT, capture_output=True)
    return r.stdout.decode('utf-8', 'replace').replace('\r', '')


def residual(diff):
    return [l for l in diff.split('\n') if l.startswith(('+', '-')) and not l.startswith(('+++', '---'))]


summary = []
files = git('diff', '--name-only', BASE, TIP).split()
for f in files:
    before = residual(git('diff', '-U0', BASE, PRE, '--', f))
    after = residual(git('diff', '-U0', '--cached', TIP, '--', f))
    d = list(difflib.unified_diff(before, after, 'residual before', 'residual after', lineterm='', n=0))
    if d:
        name = f.replace('/', '_') + '.interdiff'
        with open(os.path.join(OUT, name), 'w', encoding='utf-8') as out:
            out.write('\n'.join(d) + '\n')
        summary.append((len(d), f, len(before), len(after)))
for n, f, b, a in sorted(summary, reverse=True):
    print('%5d lines  residual before %4d after %4d  %s' % (n, b, a, f))
print('files whose residual changed: %d of %d' % (len(summary), len(files)))
