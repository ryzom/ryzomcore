#!/usr/bin/env python3
"""Check a core4 step of the merge branch: every line core4 added in BASE..TIP must be in the merged tree, with
core4's own indentation.

Usage: check_core4_step.py WT BASE TIP

WT     worktree on the merge branch with the merge staged or committed (the index is read, not the working files)
BASE   the previous core4 checkpoint already in the branch (the merge base of the step)
TIP    the core4 commit being merged

For each file core4 changed, the report lists the added lines that are
  MISSING      not in the merged file at all: a hunk the merge dropped, or a feature the branch deliberately lacks
               (atys removed the function): judge each one, and name the deliberate ones in the commit message;
  MISINDENTED  in the merged file only modulo whitespace: a moved or re-wrapped block kept its old indentation,
               so the branch would carry another formatting of a core4 file; take core4's text for it.
Blank lines are ignored. Exit status 1 when anything is listed.
"""
import re
import subprocess
import sys

if len(sys.argv) != 4:
    print(__doc__)
    sys.exit(2)
WT, BASE, TIP = sys.argv[1:4]


def git(*a):
    r = subprocess.run(['git', '-c', 'core.autocrlf=false'] + list(a), cwd=WT, capture_output=True)
    return r.stdout.decode('utf-8', 'replace').replace('\r', '')


def squash(s):
    return re.sub(r'\s+', '', s)


def added_lines(diff):
    for line in diff.split('\n'):
        if line.startswith('+') and not line.startswith('+++'):
            l = line[1:]
            if l.strip():
                yield l


total = 0
for f in git('diff', '--name-only', BASE, TIP).split():
    if not git('ls-files', '--', f).strip():
        print('%-70s ABSENT in the merged tree' % f)
        total += 1
        continue
    exact = set(git('show', ':' + f).split('\n'))
    loose = set(squash(l) for l in exact)
    missing, misindented = [], []
    for l in added_lines(git('diff', BASE, TIP, '--', f)):
        if l in exact:
            continue
        if squash(l) in loose:
            misindented.append(l)
        else:
            missing.append(l)
    if not missing and not misindented:
        continue
    total += len(missing) + len(misindented)
    print('%-70s missing %d  misindented %d' % (f, len(missing), len(misindented)))
    for l in missing[:8]:
        print('    MISSING     ', l[:150])
    for l in misindented[:8]:
        print('    MISINDENTED ', l[:150])
print('lines to judge:', total)
sys.exit(1 if total else 0)
