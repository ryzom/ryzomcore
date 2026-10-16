#!/usr/bin/env python3
"""Undo pure-whitespace hunks that a merge brought in from the other side.

ws_restore.py WT [--apply] file...
For each file, diff index (:0:) against HEAD with -U0. A hunk whose +lines
are, after removing all whitespace, exactly the -lines (same order) is a
pure-whitespace hunk: HEAD's lines are restored. Mixed hunks are left alone.
"""
import sys, re, subprocess
wt = sys.argv[1]
apply = '--apply' in sys.argv
lines_mode = '--lines' in sys.argv
files = [a for a in sys.argv[2:] if not a.startswith('--')]
def run(*a):
    raw = subprocess.run(['git'] + list(a), cwd=wt, capture_output=True).stdout
    return raw.decode('utf-8', errors='surrogateescape').replace('\r\n', '\n')
def blob_is_crlf(spec):
    raw = subprocess.run(['git', 'show', spec], cwd=wt, capture_output=True).stdout
    return b'\r\n' in raw
def key(l): return re.sub(r'\s+', '', l)
for f in files:
    d = run('diff', '--cached', '-U0', 'HEAD', '--', f).split('\n')
    hunks = []; cur = None
    for l in d:
        m = re.match(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@', l)
        if m:
            cur = [int(m.group(3)), int(m.group(4) or 1), [], []]; hunks.append(cur); continue
        if cur is None or l.startswith('+++') or l.startswith('---'): continue
        if l.startswith('-'): cur[2].append(l[1:])
        elif l.startswith('+'): cur[3].append(l[1:])
    cur_text = run('show', ':0:' + f)
    lines = cur_text.split('\n')
    restored = 0
    for c, dcount, minus, plus in sorted(hunks, key=lambda h: -h[0]):
        if not plus or dcount == 0: continue
        assert lines[c-1:c-1+dcount] == plus, (f, c, lines[c-1:c-1+dcount], plus)
        if lines_mode:
            pool = {}
            for m in minus: pool.setdefault(key(m), []).append(m)
            newplus = []
            changed = 0
            for x in plus:
                k = key(x)
                if k and pool.get(k):
                    y = pool[k].pop(0)
                    if y != x: changed += 1
                    newplus.append(y)
                else:
                    newplus.append(x)
            if not changed: continue
            lines[c-1:c-1+dcount] = newplus
            restored += changed
        else:
            if len(plus) != len(minus): continue
            if [key(x) for x in plus] != [key(x) for x in minus]: continue
            if plus == minus: continue
            lines[c-1:c-1+dcount] = minus
            restored += 1
    print(f"{f}: hunks={len(hunks)} pure-ws restored={restored}")
    if apply and restored:
        text = '\n'.join(lines)
        if blob_is_crlf(':0:' + f):
            text = text.replace('\n', '\r\n')
        with open(f"{wt}/{f}", 'w', encoding='utf-8', errors='surrogateescape', newline='') as fh:
            fh.write(text)
        subprocess.run(['git', 'add', f], cwd=wt, capture_output=True)
