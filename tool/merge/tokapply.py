#!/usr/bin/env python3
"""Three-way, formatting-immune application of atys changes onto the branch.

Usage: tokapply.py A B --base=BASE [--ref=REF] [--out=FILE] [--dry-run] [--verbose] [--lang=c|plain]

A = branch version (ours), B = atys version (theirs), BASE = the common origin
of both. All comparisons are on token streams, so wrapping/spacing differences
never count as changes.

For every hunk of the A->B token diff, the A block and B block are widened by
mutual closure to whole lines holding the same tokens apart from the change,
and overlapping/adjacent blocks are merged into clusters. Each cluster is then
judged POSITIONALLY against BASE using the token alignments BASE<->A and
BASE<->B (diff3 semantics, not "occurs somewhere in base"):
  apply : A's tokens in the cluster equal the aligned BASE tokens (branch did
          not change this spot, atys did) -> A block replaced by unwrapped B block.
  keep  : B's tokens in the cluster equal the aligned BASE tokens (atys did not
          change this spot; branch/core did) -> A block kept.
  both  : neither equals BASE -> changed on both sides, reported as SKIP.
  include-shuffle : same set of #include lines on both sides -> nothing to do.
Without --base every cluster is applied (B wins). --ref=REF (e.g. the core4
version) only annotates kept clusters with whether REF holds the A text.
After applying, the result is token-diffed against B; residue outside kept or
skipped clusters is reported as unexpected.

Exit status 0 if nothing was skipped, 1 otherwise.
"""
import sys
import re
import difflib
import importlib.util
import os

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('tokdiff', os.path.join(HERE, 'tokdiff.py'))
tokdiff = importlib.util.module_from_spec(spec)
spec.loader.exec_module(tokdiff)

CONT_START = re.compile(r'^\s*(\+(?!\+)|-(?!-)|\*(?!/)|/(?![/*])|%|&&|\|\||&(?!&)|\|(?!\|)|\^|<<|>>|<(?!<)|>(?!>)|==|!=|<=|>=|\?|:(?!:)|\.(?!\.)|->|,|\))')
OPEN_END = re.compile(r'(\(|,|&&|\|\||\+|-|\*|/|%|=|<<|>>|\?|:)\s*$')
TERMINATED = re.compile(r'(;|\{|\}|\*/|\\)\s*$')


def is_code_line(s):
    t = s.strip()
    return bool(t) and not t.startswith('//') and not t.startswith('#') and not t.startswith('*') and not t.startswith('/*')


def unwrap_xml(lines):
    """Rejoin elements that a pretty-printer split one attribute per line:
    a line opening a tag without closing it is joined with the following
    lines until the tag closes ('>' or '/>'), attributes separated by one
    space. Text content and comments are left alone."""
    out = []
    buf = None
    for ln in lines:
        s = ln.rstrip('\r')
        if buf is None:
            t = s.strip()
            if t.startswith('<') and not t.startswith('<!--') and not t.startswith('<?') \
                    and not t.endswith('>') and '>' not in t:
                buf = s.rstrip()
                continue
            out.append(s)
        else:
            t = s.strip()
            buf = buf + ' ' + t
            if t.endswith('>'):
                out.append(buf)
                buf = None
    if buf is not None:
        out.append(buf)
    return out


def indent_of(s):
    return s[:len(s) - len(s.lstrip(' \t'))]


def indent_width(ind, tab=4):
    w = 0
    for ch in ind:
        w = (w // tab + 1) * tab if ch == '\t' else w + 1
    return w


def reindent(new_lines, anchor_line, side_lines):
    """Re-indent a block taken from B so that it uses the indentation style of
    A (tabs vs spaces) and sits at the depth of the A line next to it.
    anchor_line: the A line just before the block (or just after if none).
    side_lines: the B lines that surrounded the block in B (to learn B's unit).
    Relative depth inside the block is preserved."""
    if not new_lines or anchor_line is None:
        return new_lines
    a_ind = indent_of(anchor_line)
    use_tabs = '\t' in a_ind or not a_ind
    # B's indentation unit: smallest positive difference between successive indents
    widths = [indent_width(indent_of(l)) for l in new_lines + side_lines if l.strip()]
    diffs = sorted({abs(x - y) for x in widths for y in widths if x != y})
    unit = diffs[0] if diffs else 4
    unit = max(1, min(unit, 8))
    first = None
    for l in new_lines:
        if l.strip():
            first = indent_width(indent_of(l))
            break
    if first is None:
        return new_lines
    base_depth = len(a_ind) if use_tabs else indent_width(a_ind) // 4
    out = []
    for l in new_lines:
        if not l.strip():
            out.append('')
            continue
        rel = (indent_width(indent_of(l)) - first) // unit
        depth = max(0, base_depth + rel)
        out.append(('\t' * depth if use_tabs else ' ' * (4 * depth)) + l.lstrip(' \t'))
    return out


def unwrap(lines, lang):
    """Rejoin formatter-wrapped continuation lines. Conservative."""
    if lang == 'xml':
        return unwrap_xml(lines)
    if lang != 'c':
        return [l.rstrip('\r') for l in lines]
    out = []
    for ln in lines:
        s = ln.rstrip('\r')
        if out:
            prev = out[-1]
            if is_code_line(prev) and is_code_line(s) and not TERMINATED.search(prev) \
                    and (CONT_START.match(s) or OPEN_END.search(prev)) \
                    and '//' not in prev and '/*' not in prev:
                sep = '' if prev.rstrip().endswith('(') or s.lstrip().startswith(')') else ' '
                out[-1] = prev.rstrip() + sep + s.lstrip()
                continue
        out.append(s)
    return out


def include_set(lines):
    inc = []
    for l in lines:
        t = l.strip()
        if not t:
            continue
        if not t.startswith('#include'):
            return None
        inc.append(re.sub(r'\s+', ' ', t))
    return sorted(inc)


def alignment(src, dst):
    """Map token index in src -> token index in dst for tokens matched as equal."""
    m = {}
    sm = difflib.SequenceMatcher(None, src, dst, autojunk=False)
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == 'equal':
            for k in range(i2 - i1):
                m[i1 + k] = j1 + k
    return m


def base_span(x2base, i1, i2, nbase):
    """Base token range aligned with x tokens [i1,i2): from the base position
    after the last aligned token before i1 to the base position of the first
    aligned token at/after i2. Returns (lo, hi) or None if unalignable."""
    lo = None
    k = i1 - 1
    while k >= 0:
        if k in x2base:
            lo = x2base[k] + 1
            break
        k -= 1
    if lo is None:
        lo = 0
    hi = None
    k = i2
    while k in x2base or k < 0:
        break
    # find first aligned token at or after i2
    k = i2
    limit = i2 + 100000
    while k < limit:
        if k in x2base:
            hi = x2base[k]
            break
        k += 1
        if k > len(x2base) + i2 + 1 and hi is None and k > 10 ** 7:
            break
    if hi is None:
        hi = nbase
    return (lo, max(lo, hi))


MECH_NULLISH = ('NULL', 'nullptr', '0')


def mech_norm(seq):
    """token stream with the clang-tidy / NL_OVERRIDE mechanics folded away"""
    return ['nullptr' if t in MECH_NULLISH else t for t in seq if t != 'NL_OVERRIDE']


def _method_name(text):
    k = text.rfind('NL_OVERRIDE')
    if k < 0:
        return None
    head = re.sub(r'\s*const\s*$', '', text[:k].rstrip())
    if not head.endswith(')'):
        return None
    depth = 0
    i = len(head) - 1
    while i >= 0:
        c = head[i]
        if c == ')':
            depth += 1
        elif c == '(':
            depth -= 1
            if depth == 0:
                break
        i -= 1
    if i < 0:
        return None
    m = re.search(r'([A-Za-z_~]\w*)\s*$', head[:i])
    return m.group(1) if m else None


def retidy(new_lines, a_lines):
    """Re-apply core4's mechanical conventions to an atys block that replaces an A block:
    NULL -> nullptr, and NL_OVERRIDE on the declarations that carried it in A."""
    out = [re.sub(r'\bNULL\b', 'nullptr', l) for l in new_lines]
    names = []
    for i, l in enumerate(a_lines):
        if 'NL_OVERRIDE' in l:
            ctx = ' '.join(x.strip() for x in a_lines[max(0, i - 6):i + 1])
            n = _method_name(ctx)
            if n:
                names.append(n)
    for name in names:
        hits = [i for i, l in enumerate(out) if re.search(r'\b' + re.escape(name) + r'\s*\(', l) and 'NL_OVERRIDE' not in l]
        if len(hits) != 1:
            continue
        j = hits[0]
        while j < len(out):
            s = out[j].rstrip()
            if s.endswith(';') or '{' in s or ((s.endswith(')') or s.endswith('const')) and j + 1 < len(out) and out[j + 1].strip().startswith('{')):
                break
            j += 1
        if j >= len(out):
            continue
        s = out[j].rstrip()
        if re.search(r'=\s*0\s*;$', s):
            out[j] = re.sub(r'\s*=\s*0\s*;$', ' NL_OVERRIDE = 0;', s)
        elif s.endswith(';'):
            out[j] = s[:-1].rstrip() + ' NL_OVERRIDE;'
        elif '{' in s:
            k = s.index('{')
            out[j] = s[:k].rstrip() + ' NL_OVERRIDE ' + s[k:]
        else:
            out[j] = s + ' NL_OVERRIDE'
    return out


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    out = None
    base = None
    ref = None
    lang = None
    dry = '--dry-run' in sys.argv
    verbose = '--verbose' in sys.argv
    verbatim = '--verbatim' in sys.argv  # insert B lines as-is (B's formatting is authoritative)
    mech = '--mech' in sys.argv  # A may differ from base by nullptr / NL_OVERRIDE tidy changes only; re-tidy applied B blocks
    for a in sys.argv[1:]:
        if a.startswith('--out='):
            out = a.split('=', 1)[1]
        if a.startswith('--base='):
            base = a.split('=', 1)[1]
        if a.startswith('--ref='):
            ref = a.split('=', 1)[1]
        if a.startswith('--lang='):
            lang = a.split('=', 1)[1]
    pa, pb = args[0], args[1]
    if lang is None:
        lang = tokdiff.lang_for(out or pa)
    ta = open(pa, encoding='utf-8', errors='surrogateescape', newline='').read()
    tb = open(pb, encoding='utf-8', errors='surrogateescape', newline='').read()
    crlf = '\r\n' in ta[:5000]
    la = [l.rstrip('\r') for l in ta.split('\n')]
    lb = [l.rstrip('\r') for l in tb.split('\n')]
    xa, xb = tokdiff.tokenize(ta, lang), tokdiff.tokenize(tb, lang)
    sa, sb = [t for t, _ in xa], [t for t, _ in xb]
    sbase = None
    if base and os.path.exists(base) and os.path.getsize(base) > 0:
        sbase = [t for t, _ in tokdiff.tokenize(open(base, encoding='utf-8', errors='surrogateescape').read(), lang)]
    sref = None
    if ref and os.path.exists(ref) and os.path.getsize(ref) > 0:
        sref = [t for t, _ in tokdiff.tokenize(open(ref, encoding='utf-8', errors='surrogateescape').read(), lang)]

    sm = difflib.SequenceMatcher(None, sa, sb, autojunk=False)
    allops = sm.get_opcodes()
    ops = [o for o in allops if o[0] != 'equal']
    a2b, b2a = {}, {}
    for tag, i1, i2, j1, j2 in allops:
        if tag == 'equal':
            for k in range(j2 - j1):
                a2b[i1 + k] = j1 + k
                b2a[j1 + k] = i1 + k
    a2base = alignment(sa, sbase) if sbase is not None else None
    b2base = alignment(sb, sbase) if sbase is not None else None

    def first_last(x):
        first, last = {}, {}
        for idx, (_, ln) in enumerate(x):
            first.setdefault(ln, idx)
            last[ln] = idx
        return first, last
    a_first, a_last = first_last(xa)
    b_first, b_last = first_last(xb)

    def closure(i1, i2, j1, j2):
        a_lo, a_hi, b_lo, b_hi = i1, i2, j1, j2
        for _ in range(20):
            changed = False
            if a_hi > a_lo:
                na_lo = a_first[xa[a_lo][1]]
                na_hi = a_last[xa[a_hi - 1][1]] + 1
                if na_lo != a_lo or na_hi != a_hi:
                    a_lo, a_hi, changed = na_lo, na_hi, True
            if b_hi > b_lo:
                nb_lo = b_first[xb[b_lo][1]]
                nb_hi = b_last[xb[b_hi - 1][1]] + 1
                if lang == 'xml':
                    # a pretty-printed element spans several lines: widen to the
                    # whole element (previous lines not ending in '>', following
                    # lines until one ends in '>')
                    while nb_lo > 0:
                        pl = lb[xb[nb_lo - 1][1] - 1].rstrip()
                        if pl.endswith('>') or not pl.strip() or pl.lstrip().startswith('<!--'):
                            break
                        nb_lo = b_first[xb[nb_lo - 1][1]]
                    while nb_hi < len(xb):
                        ll = lb[xb[nb_hi - 1][1] - 1].rstrip()
                        if ll.endswith('>') or not ll.strip():
                            break
                        nb_hi = b_last[xb[nb_hi][1]] + 1
                if nb_lo != b_lo or nb_hi != b_hi:
                    b_lo, b_hi, changed = nb_lo, nb_hi, True
            if a_lo < i1 and a_lo in a2b and a2b[a_lo] < b_lo:
                b_lo, changed = a2b[a_lo], True
            if a_hi > i2 and (a_hi - 1) in a2b and a2b[a_hi - 1] + 1 > b_hi:
                b_hi, changed = a2b[a_hi - 1] + 1, True
            if b_lo < j1 and b_lo in b2a and b2a[b_lo] < a_lo:
                a_lo, changed = b2a[b_lo], True
            if b_hi > j2 and (b_hi - 1) in b2a and b2a[b_hi - 1] + 1 > a_hi:
                a_hi, changed = b2a[b_hi - 1] + 1, True
            if not changed:
                break
        return a_lo, a_hi, b_lo, b_hi

    clusters = []
    for tag, i1, i2, j1, j2 in ops:
        a_lo, a_hi, b_lo, b_hi = closure(i1, i2, j1, j2)
        if a_hi > a_lo:
            al1, al2 = xa[a_lo][1], xa[a_hi - 1][1]
        else:
            prev_line = xa[a_lo - 1][1] if a_lo > 0 else 0
            al1, al2 = prev_line + 1, prev_line
        if b_hi > b_lo:
            bl1, bl2 = xb[b_lo][1], xb[b_hi - 1][1]
        else:
            prev_line = xb[b_lo - 1][1] if b_lo > 0 else 0
            bl1, bl2 = prev_line + 1, prev_line
        c = {'a1': al1, 'a2': al2, 'b1': bl1, 'b2': bl2, 'i1': i1, 'i2': i2, 'j1': j1, 'j2': j2,
             'alo': a_lo, 'ahi': a_hi, 'blo': b_lo, 'bhi': b_hi}
        if clusters:
            p = clusters[-1]
            if al1 <= p['a2'] + 1 or bl1 <= p['b2'] + 1:
                p['a2'] = max(p['a2'], al2)
                p['b2'] = max(p['b2'], bl2)
                p['i2'] = i2
                p['j2'] = j2
                p['ahi'] = max(p['ahi'], a_hi)
                p['bhi'] = max(p['bhi'], b_hi)
                continue
        clusters.append(c)

    def judge(c):
        """diff3-style decision for a cluster using positional base alignment."""
        if sbase is None:
            return 'apply'
        i1, i2, j1, j2 = c['alo'], c['ahi'], c['blo'], c['bhi']
        blo_a, bhi_a = base_span(a2base, i1, i2, len(sbase))
        blo_b, bhi_b = base_span(b2base, j1, j2, len(sbase))
        a_unchanged = sa[i1:i2] == sbase[blo_a:bhi_a]
        b_unchanged = sb[j1:j2] == sbase[blo_b:bhi_b]
        if mech and not a_unchanged:
            a_unchanged = mech_norm(sa[i1:i2]) == mech_norm(sbase[blo_a:bhi_a])
        if mech and not b_unchanged:
            b_unchanged = mech_norm(sb[j1:j2]) == mech_norm(sbase[blo_b:bhi_b])
        if b_unchanged and not a_unchanged:
            return 'keep'
        if a_unchanged and not b_unchanged:
            return 'apply'
        if a_unchanged and b_unchanged:
            return 'apply'
        return 'both'

    edits = []
    skipped = []
    kept_clusters = []
    counts = {'apply': 0, 'keep': 0, 'include-shuffle': 0, 'keep-in-ref': 0}
    for c in clusters:
        a1, a2, b1, b2 = c['a1'], c['a2'], c['b1'], c['b2']
        a_lines = la[a1 - 1:a2] if a2 >= a1 else []
        b_lines = lb[b1 - 1:b2] if b2 >= b1 else []
        ia, ib = include_set(a_lines), include_set(b_lines)
        if ia is not None and ib is not None and ia == ib and a_lines and b_lines:
            counts['include-shuffle'] += 1
            kept_clusters.append(c)
            continue
        decision = judge(c)
        if decision == 'keep':
            counts['keep'] += 1
            kept_clusters.append(c)
            if sref is not None:
                ctx = sa[max(0, c['alo'] - 6):min(len(sa), c['ahi'] + 6)]
                if tokdiff.contains_seq(sref, ctx):
                    counts['keep-in-ref'] += 1
                elif verbose:
                    print(f"  KEEP-BRANCH-ONLY A {a1}-{a2} (B {b1}-{b2} equals base): "
                          f"{' | '.join(l.strip() for l in la[a1 - 1:min(a2, a1 + 2)])[:160]!r}")
            elif verbose:
                print(f"  KEEP A {a1}-{a2} (B {b1}-{b2} equals base)")
            continue
        if decision == 'both':
            skipped.append(('both-changed', (a1, a2), (b1, b2)))
            continue
        new_lines = [l.rstrip('\r') for l in b_lines] if verbatim else unwrap(b_lines, lang)
        if mech and lang == 'c':
            new_lines = retidy(new_lines, a_lines)
        if lang == 'xml' and new_lines and not verbatim:
            # structural re-indent: depth follows the element nesting, starting from
            # the nearest non-blank A line before the block
            anchor = None
            k = a1 - 2
            while k >= 0:
                if la[k].strip():
                    anchor = la[k]
                    break
                k -= 1
            if anchor is not None:
                a_ind = indent_of(anchor)
                use_tabs = '\t' in a_ind or not a_ind
                depth = len(a_ind) if use_tabs else indent_width(a_ind) // 4
                t = anchor.strip()
                def opens(t):
                    return t.startswith('<') and not t.startswith('</') and not t.startswith('<!') \
                        and not t.startswith('<?') and not t.endswith('/>') and '</' not in t and t.endswith('>')
                if opens(t):
                    # children of the anchor: use the depth the file actually uses for
                    # them (the next A line after the block, unless it closes the anchor)
                    nxt = None
                    k = a2
                    while k < len(la):
                        if la[k].strip():
                            nxt = la[k]
                            break
                        k += 1
                    if nxt is not None and not nxt.strip().startswith('</'):
                        n_ind = indent_of(nxt)
                        depth = len(n_ind) if use_tabs else indent_width(n_ind) // 4
                    else:
                        depth += 1
                fixed = []
                for l in new_lines:
                    t = l.strip()
                    if not t:
                        fixed.append('')
                        continue
                    if t.startswith('</'):
                        depth = max(0, depth - 1)
                    fixed.append(('\t' * depth if use_tabs else ' ' * (4 * depth)) + t)
                    if opens(t):
                        depth += 1
                new_lines = fixed
        elif lang == 'plain' and new_lines and not verbatim:
            # anchor on the nearest non-blank A line before the block, else after
            anchor = None
            k = a1 - 2
            while k >= 0:
                if la[k].strip():
                    anchor = la[k]
                    break
                k -= 1
            if anchor is None:
                k = a2
                while k < len(la):
                    if la[k].strip():
                        anchor = la[k]
                        break
                    k += 1
            # the anchor's depth applies to the first new line only if that line is a
            # sibling; approximate by using the B line before the block as reference
            side = lb[max(0, b1 - 3):b1 - 1] + lb[b2:b2 + 2]
            prev_b = None
            k = b1 - 2
            while k >= 0:
                if lb[k].strip():
                    prev_b = lb[k]
                    break
                k -= 1
            if anchor is not None and prev_b is not None:
                # depth of first new line relative to the B line that precedes it,
                # transplanted onto the A anchor's depth
                a_ind = indent_of(anchor)
                use_tabs = '\t' in a_ind or not a_ind
                widths = [indent_width(indent_of(l)) for l in [prev_b] + new_lines + side if l.strip()]
                diffs = sorted({abs(x - y) for x in widths for y in widths if x != y})
                unit = max(1, min(diffs[0] if diffs else 4, 8))
                pw = indent_width(indent_of(prev_b))
                base_depth = len(a_ind) if use_tabs else indent_width(a_ind) // 4
                fixed = []
                for l in new_lines:
                    if not l.strip():
                        fixed.append('')
                        continue
                    rel = (indent_width(indent_of(l)) - pw) // unit
                    depth = max(0, base_depth + rel)
                    fixed.append(('\t' * depth if use_tabs else ' ' * (4 * depth)) + l.lstrip(' \t'))
                new_lines = fixed
        edits.append((a1 - 1, a2, new_lines, c))
        counts['apply'] += 1

    res = list(la)
    for s, e, new, _ in sorted(edits, key=lambda x: (-x[0], -x[1])):
        res[s:e] = new
    nl = '\r\n' if crlf else '\n'
    text = nl.join(res)

    xr = tokdiff.tokenize(text, lang)
    sr = [t for t, _ in xr]
    sm2 = difflib.SequenceMatcher(None, sr, sb, autojunk=False)
    residual = [o for o in sm2.get_opcodes() if o[0] != 'equal']
    expected_ranges = [(c['b1'], c['b2']) for c in kept_clusters] + [(s[2][0], s[2][1]) for s in skipped]
    unexpected = []
    for tag, i1, i2, j1, j2 in residual:
        if j2 > j1:
            rb1, rb2 = xb[j1][1], xb[j2 - 1][1]
        else:
            rb1 = rb2 = xb[j1 - 1][1] if j1 > 0 else 1
        if not any(rb1 <= e2 + 1 and rb2 >= e1 - 1 for e1, e2 in expected_ranges):
            unexpected.append((tag, i1, i2, j1, j2, rb1, rb2))
    print(f"# {os.path.basename(pa)}: clusters={len(clusters)} apply={counts['apply']} keep={counts['keep']} "
          f"(in-ref={counts['keep-in-ref']}) include-shuffle={counts['include-shuffle']} skipped={len(skipped)} "
          f"residual-hunks={len(residual)} unexpected-residual={len(unexpected)}")
    for s in skipped:
        print(f"  SKIP {s[0]}: A {s[1][0]}-{s[1][1]}  B {s[2][0]}-{s[2][1]}")
    if verbose:
        for s, e, new, c in sorted(edits, key=lambda x: x[0]):
            print(f"  EDIT A {s + 1}-{e} ({e - s} lines) <- B {c['b1']}-{c['b2']} ({len(new)} lines after unwrap)")
    for tag, i1, i2, j1, j2, rb1, rb2 in unexpected[:12]:
        rl = xr[i1][1] if i1 < len(xr) else len(res)
        print(f"  UNEXPECTED {tag}: result line {rl}  B lines {rb1}-{rb2}: {' '.join(sr[i1:min(i2, i1 + 6)])!r} / {' '.join(sb[j1:min(j2, j1 + 6)])!r}")
    if not dry and edits:
        with open(out or pa, 'w', encoding='utf-8', errors='surrogateescape', newline='') as f:
            f.write(text)
    sys.exit(0 if not skipped else 1)


if __name__ == '__main__':
    main()
