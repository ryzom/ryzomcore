"""diffreport.py REPO A B OUT.json [--jobs=N]

Content-level diff of two revisions for the merge report:
  - whole-file additions/removals/renames are recorded as one entry each
  - modified text files are compared token-wise (whitespace, wrapping and
    line-ending differences vanish); hunks that are only mechanical
    (NL_OVERRIDE, NULL/0 -> nullptr, copyright years, integer casts) are
    counted per file instead of shown; the remaining hunks are kept with the
    original lines of both sides.
Direction: A is the base side, B the side whose additions show as '+'.
"""
import sys, os, re, json, subprocess, difflib
from multiprocessing import Pool

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))  # tokdiff.py lives one level up
import tokdiff

REPO, A, B, OUT = sys.argv[1:5]
JOBS = 8
for a in sys.argv[5:]:
    if a.startswith('--jobs='):
        JOBS = int(a.split('=')[1])

BIN_EXT = {'.png', '.jpg', '.jpeg', '.gif', '.tga', '.dds', '.bmp', '.ico', '.ttf', '.otf', '.bnp', '.zip', '.gz', '.7z',
           '.exe', '.dll', '.lib', '.so', '.a', '.pdb', '.wav', '.ogg', '.mp3', '.pdf', '.psd', '.max', '.shape',
           '.skel', '.anim', '.ig', '.zone', '.packed_sheets', '.sitem', '.sbrick', '.pyc', '.bin', '.dat', '.cdb'}
MECH_NULL = {'NULL', 'nullptr', '0'}
CAST_WORDS = {'int', 'sint', 'uint', 'sint32', 'uint32', 'sint8', 'uint8', 'sint16', 'uint16', 'size_t', 'float', 'double'}


def git_bytes(*args):
    return subprocess.run(['git'] + list(args), cwd=REPO, capture_output=True).stdout


def blob(rev, path):
    return git_bytes('cat-file', '-p', rev + ':' + path)


def is_binary(data):
    return b'\0' in data[:8000]


def decode_text(data):
    """utf-8 first; the older sources are Windows-1252 / Latin-1"""
    try:
        return data.decode('utf-8')
    except UnicodeDecodeError:
        try:
            return data.decode('cp1252')
        except UnicodeDecodeError:
            return data.decode('latin-1')


def classify(removed, added, la_lines, lb_lines):
    """mechanical category for a hunk, or None"""
    r = [t for t, _ in removed]
    a = [t for t, _ in added]
    sr, sa = set(r), set(a)
    if not r and sa <= {'NL_OVERRIDE'}:
        return 'nl_override'
    if sr <= {'virtual'} and sa <= {'NL_OVERRIDE'} and a:
        return 'nl_override'
    if sr <= {'NL_OVERRIDE'} and not a:
        return 'nl_override'
    if r and a and sr <= MECH_NULL and sa <= MECH_NULL and sr != sa:
        return 'nullptr'
    if r and a and (sa - sr) <= {'(', ')'} | CAST_WORDS and (sr - sa) <= set() and '(' in a:
        return 'cast'
    if r and a and (sr - sa) <= {'(', ')'} | CAST_WORDS and (sa - sr) <= set() and '(' in r:
        return 'cast'
    text = ' '.join(la_lines + lb_lines)
    if 'Copyright' in text and all(l.strip().startswith('//') for l in la_lines + lb_lines if l.strip()):
        return 'copyright'
    return None


def one_file(item):
    status, path, old = item
    ext = os.path.splitext(path)[1].lower()
    rec = {'path': path, 'status': status}
    if old:
        rec['old'] = old
    if status in ('A', 'D'):
        return rec
    src = old or path
    da = blob(A, src)
    db = blob(B, path)
    if ext in BIN_EXT or is_binary(da) or is_binary(db):
        rec['binary'] = True
        rec['size_a'], rec['size_b'] = len(da), len(db)
        return rec
    ta = decode_text(da).replace('\r\n', '\n')
    tb = decode_text(db).replace('\r\n', '\n')
    lang = tokdiff.lang_for(path)
    xa, xb = tokdiff.tokenize(ta, lang), tokdiff.tokenize(tb, lang)
    sa, sb = [t for t, _ in xa], [t for t, _ in xb]
    big = len(sa) > 80000 or len(sb) > 80000
    sm = difflib.SequenceMatcher(None, sa, sb, autojunk=big)
    hunks = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag != 'equal':
            hunks.append([i1, i2, j1, j2])
    merged = []
    for h in hunks:
        if merged:
            p = merged[-1]
            if (tokdiff.line_range(xb, h[2], h[3])[0] <= tokdiff.line_range(xb, p[2], p[3])[1] + 1 and
                    tokdiff.line_range(xa, h[0], h[1])[0] <= tokdiff.line_range(xa, p[0], p[1])[1] + 1):
                merged[-1] = [p[0], h[1], p[2], h[3]]
                continue
        merged.append(list(h))
    la, lb = ta.split('\n'), tb.split('\n')
    mech = {}
    shown = []
    tot_rem = tot_add = 0
    for i1, i2, j1, j2 in merged:
        a1, a2 = tokdiff.line_range(xa, i1, i2)
        b1, b2 = tokdiff.line_range(xb, j1, j2)
        ra = la[a1 - 1:a2] if i2 > i1 else []
        rb = lb[b1 - 1:b2] if j2 > j1 else []
        cat = classify(xa[i1:i2], xb[j1:j2], ra, rb)
        tot_rem += i2 - i1
        tot_add += j2 - j1
        if cat:
            mech[cat] = mech.get(cat, 0) + 1
            continue
        shown.append({'a1': a1, 'a2': a2 if i2 > i1 else a1 - 1, 'b1': b1, 'b2': b2 if j2 > j1 else b1 - 1,
                      'a': ra, 'b': rb, 'rem': i2 - i1, 'add': j2 - j1})
    rec['tokens_a'], rec['tokens_b'] = len(sa), len(sb)
    rec['tok_rem'], rec['tok_add'] = tot_rem, tot_add
    rec['mech'] = mech
    rec['hunks'] = shown
    rec['formatting_only'] = (not merged)
    return rec


def main():
    out = git_bytes('diff', '--name-status', '-M', A, B).decode('utf-8', errors='replace')
    items = []
    for line in out.split('\n'):
        if not line.strip():
            continue
        parts = line.split('\t')
        st = parts[0]
        if st.startswith('R'):
            items.append(('R', parts[2], parts[1]))
        elif st.startswith('C'):
            items.append(('A', parts[2], None))
        else:
            items.append((st[0], parts[1], None))
    print('entries:', len(items), flush=True)
    with Pool(JOBS) as pool:
        recs = []
        for n, rec in enumerate(pool.imap_unordered(one_file, items, chunksize=4), 1):
            recs.append(rec)
            if n % 200 == 0:
                print('  done', n, flush=True)
    recs.sort(key=lambda r: r['path'])
    json.dump({'a': A, 'b': B, 'files': recs}, open(OUT, 'w', encoding='utf-8'))
    print('written', OUT, 'files', len(recs))


if __name__ == '__main__':
    main()
