#!/usr/bin/env python3
"""Whitespace/wrapping-immune content diff for C++ (and similar) sources.

Usage: tokdiff.py A B [--ref=C] [--context=N] [--summary] [--tsv]

Tokenises both files (identifiers, numbers, strings, operators; comments are
kept as word tokens so rewrapping is ignored but wording changes are not),
diffs the token streams, and prints each hunk with the ORIGINAL line ranges in
A and B plus the original lines, so a hunk can be hand-applied onto A.

With --ref=C, every hunk that removes tokens from A is annotated with whether
that removed token sequence occurs in C (e.g. the upstream version A was
derived from): REF-HAS means the text is upstream content (keep it), REF-LACKS
means A-only text that B dropped (likely an intentional removal on B's side).
"""
import re
import os
import sys
import difflib

PLAIN_TOK = re.compile(r'[A-Za-z_][A-Za-z0-9_]*|\d+|\S')

TOK = re.compile(
    r'"(?:\\.|[^"\\\n])*"'        # string literal (never spans lines)
    r"|'(?:\\.|[^'\\\n])*'"       # char literal
    r'|[A-Za-z_][A-Za-z0-9_]*'    # identifier
    r'|0[xX][0-9A-Fa-f]+[uUlL]*'  # hex
    r'|\d+(?:\.\d*)?(?:[eE][+-]?\d+)?[fFuUlL]*'  # number
    r'|->\*|<<=|>>=|->|\+\+|--|<<|>>|<=|>=|==|!=|&&|\|\|'
    r'|\+=|-=|\*=|/=|%=|&=|\|=|\^=|::|\.\.\.'
    r'|[^\sA-Za-z0-9_]'           # any other single char
)


C_EXTS = ('.cpp', '.h', '.c', '.cc', '.hpp', '.inl', '.cxx', '.hxx', '.mm', '.m')


def lang_for(path):
    ext = os.path.splitext(path)[1].lower()
    if ext in C_EXTS:
        return 'c'
    if ext in ('.xml', '.xslt', '.html', '.htm'):
        return 'xml'
    return 'plain'


def tokenize(text, lang='c'):
    if lang in ('plain', 'xml'):
        toks = []
        for ln, l in enumerate(text.split('\n'), 1):
            for m in PLAIN_TOK.finditer(l):
                toks.append((m.group(0), ln))
        return toks
    toks = []  # (token, line)
    i = 0
    line = 1
    n = len(text)
    while i < n:
        c = text[i]
        if c == '\n':
            line += 1
            i += 1
            continue
        if c in ' \t\r\f\v':
            i += 1
            continue
        if text.startswith('//', i):
            j = text.find('\n', i)
            j = n if j < 0 else j
            toks.append(('//', line))
            for w in re.findall(r'\S+', text[i + 2:j]):
                toks.append((w, line))
            i = j
            continue
        if text.startswith('/*', i):
            j = text.find('*/', i + 2)
            closed = j >= 0
            j = n if j < 0 else j + 2
            body = text[i + 2:j - 2] if (closed and j - 2 > i + 2) else text[i + 2:j]
            toks.append(('/*', line))
            for m2 in re.finditer(r'\S+', body):
                w = m2.group(0)
                if w.strip('*'):
                    toks.append((w, line + body.count('\n', 0, m2.start())))
            endline = line + text.count('\n', i, j)
            if closed:
                toks.append(('*/', endline))
            line = endline
            i = j
            continue
        m = TOK.match(text, i)
        if m:
            toks.append((m.group(0), line))
            i = m.end()
            continue
        i += 1
    return toks


def line_range(x, i1, i2):
    if not x:
        return (1, 1)
    if i2 > i1:
        return (x[i1][1], x[i2 - 1][1])
    k = min(i1, len(x) - 1)
    return (x[k][1], x[k][1])


def contains_seq(hay, needle):
    if not needle:
        return True
    n = len(needle)
    first = needle[0]
    i = 0
    while True:
        try:
            i = hay.index(first, i)
        except ValueError:
            return False
        if hay[i:i + n] == needle:
            return True
        i += 1


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    summary = '--summary' in sys.argv
    tsv = '--tsv' in sys.argv
    ctx = 3
    ref = None
    lang = None
    for a in sys.argv[1:]:
        if a.startswith('--context='):
            ctx = int(a.split('=', 1)[1])
        if a.startswith('--ref='):
            ref = a.split('=', 1)[1]
        if a.startswith('--lang='):
            lang = a.split('=', 1)[1]
    pa, pb = args[0], args[1]
    if lang is None:
        lang = lang_for(pa)
    ta = open(pa, encoding='utf-8', errors='replace').read()
    tb = open(pb, encoding='utf-8', errors='replace').read()
    la, lb = ta.split('\n'), tb.split('\n')
    xa, xb = tokenize(ta, lang), tokenize(tb, lang)
    sa, sb = [t for t, _ in xa], [t for t, _ in xb]
    sc = None
    if ref:
        try:
            sc = [t for t, _ in tokenize(open(ref, encoding='utf-8', errors='replace').read(), lang)]
        except OSError:
            sc = []
    sm = difflib.SequenceMatcher(None, sa, sb, autojunk=False)
    hunks = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == 'equal':
            continue
        hunks.append([tag, i1, i2, j1, j2])
    # merge hunks touching the same or adjacent lines on both sides
    merged = []
    for h in hunks:
        if merged:
            p = merged[-1]
            pa2 = line_range(xa, p[1], p[2])[1]
            pb2 = line_range(xb, p[3], p[4])[1]
            ha1 = line_range(xa, h[1], h[2])[0]
            hb1 = line_range(xb, h[3], h[4])[0]
            if hb1 <= pb2 + 1 and ha1 <= pa2 + 1:
                merged[-1] = ['mixed', p[1], h[2], p[3], h[4]]
                continue
        merged.append(h)
    out = []
    n_ins = n_del_has = n_del_lacks = n_rep = 0
    for tag, i1, i2, j1, j2 in merged:
        nd, ni = i2 - i1, j2 - j1
        kind = 'insert' if nd == 0 else ('delete' if ni == 0 else 'replace')
        refnote = ''
        if sc is not None and nd:
            refnote = 'REF-HAS' if contains_seq(sc, sa[i1:i2]) else 'REF-LACKS'
        if kind == 'insert':
            n_ins += 1
        elif kind == 'delete':
            if refnote == 'REF-LACKS':
                n_del_lacks += 1
            else:
                n_del_has += 1
        else:
            n_rep += 1
        out.append((kind, nd, ni, line_range(xa, i1, i2), line_range(xb, j1, j2), refnote))
    tot_del = sum(h[1] for h in out)
    tot_ins = sum(h[2] for h in out)
    if tsv:
        print(f"{len(out)}\t{n_ins}\t{n_rep}\t{n_del_has}\t{n_del_lacks}\t{tot_del}\t{tot_ins}")
        return
    print(f"# tokens: A={len(sa)} B={len(sb)}  content hunks: {len(out)}  "
          f"insert={n_ins} replace={n_rep} delete(ref-has)={n_del_has} delete(ref-lacks)={n_del_lacks}  "
          f"tokens removed={tot_del} added={tot_ins}")
    if summary:
        return
    for kind, nd, ni, (a1, a2), (b1, b2), refnote in out:
        print(f"\n@@ {kind}{(' ' + refnote) if refnote else ''}: A lines {a1}-{a2} (-{nd} tok)  B lines {b1}-{b2} (+{ni} tok)")
        if ni:
            lo, hi = max(1, b1 - ctx), min(len(lb), b2 + ctx)
            for k in range(lo, hi + 1):
                mark = '+' if b1 <= k <= b2 else ' '
                print(f"{mark}{k:6d}| {lb[k - 1]}")
        if nd:
            lo, hi = max(1, a1 - ctx), min(len(la), a2 + ctx)
            print("   --- A side:")
            for k in range(lo, hi + 1):
                mark = '-' if a1 <= k <= a2 else ' '
                print(f"{mark}{k:6d}| {la[k - 1]}")


if __name__ == '__main__':
    main()
