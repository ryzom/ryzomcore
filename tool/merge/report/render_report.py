"""render_report.py DATA.json ANNOT.py OUT_DIR [--no-addrem] [--split]

Renders the diffreport JSON as a theme-aware HTML page (or, with --split, an
index page plus one page per area). ANNOT.py must define TITLE, SUBTITLE,
INTRO (html), AREAS (ordered list of (key, title, prefixes)), NOTES (dict area
key -> html paragraphs) and optionally COUNT_ONLY (set of area keys whose files
are only counted, never listed)."""
import sys, os, json, html, re, importlib.util
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))  # tokdiff.py lives one level up
import tokdiff

data_path, annot_path, out_dir = sys.argv[1:4]
NO_ADDREM = '--no-addrem' in sys.argv
SPLIT = '--split' in sys.argv
COMPACT = '--compact' in sys.argv
spec = importlib.util.spec_from_file_location('annot', annot_path)
annot = importlib.util.module_from_spec(spec)
spec.loader.exec_module(annot)
COUNT_ONLY = getattr(annot, 'COUNT_ONLY', set())
data = json.load(open(data_path, encoding='utf-8'))
files = data['files']

URL_RE = re.compile(r'https?://\S+')
WS_RE = re.compile(r'\s+')


def url_only(h):
    a = WS_RE.sub(' ', URL_RE.sub('URL', ' '.join(h['a']))).strip()
    b = WS_RE.sub(' ', URL_RE.sub('URL', ' '.join(h['b']))).strip()
    return a == b and a != ''


def wiki_redirect(h):
    b = ' '.join(h['b'])
    return 'http-equiv="refresh"' in b and 'wiki.ryzom.dev' in b and not any('wiki.ryzom.dev' in l for l in h['a'])


NULLISH = {'NULL', 'nullptr', '0'}


def norm_tokens(lines, lang):
    out = []
    for t, _ in tokdiff.tokenize(chr(10).join(lines), lang):
        if t == 'NL_OVERRIDE':
            continue
        out.append('nullptr' if t in NULLISH else t)
    return out


def mech_equiv(h, lang):
    """the hunk differs only by NL_OVERRIDE annotations and/or NULL/0 <-> nullptr"""
    if not h['a'] or not h['b']:
        return False
    return norm_tokens(h['a'], lang) == norm_tokens(h['b'], lang)


for rec in files:
    hs = rec.get('hunks')
    if not hs:
        continue
    lang = tokdiff.lang_for(rec['path'])
    keep = []
    for h in hs:
        if lang == 'c' and mech_equiv(h, lang):
            rec.setdefault('mech', {})['mechanical'] = rec.get('mech', {}).get('mechanical', 0) + 1
        else:
            keep.append(h)
    rec['hunks'] = keep

if COMPACT:
    for rec in files:
        hs = rec.get('hunks')
        if not hs:
            continue
        keep = []
        for h in hs:
            if url_only(h):
                rec.setdefault('mech', {})['url'] = rec.get('mech', {}).get('url', 0) + 1
            elif wiki_redirect(h):
                rec.setdefault('mech', {})['wiki_redirect'] = rec.get('mech', {}).get('wiki_redirect', 0) + 1
            else:
                keep.append(h)
        rec['hunks'] = keep


def area_of(path):
    for key, title, prefixes in annot.AREAS:
        for p in prefixes:
            if p.endswith('/') and path.startswith(p):
                return key
            if not p.endswith('/') and (path == p or path.startswith(p)):
                return key
    return 'other'


MECH_LABEL = {'nl_override': 'NL_OVERRIDE', 'nullptr': 'nullptr', 'cast': 'casts', 'copyright': 'copyright', 'url': 'url only', 'wiki_redirect': 'wiki redirect', 'mechanical': 'NL_OVERRIDE+nullptr'}


def esc(s):
    return html.escape(s, quote=False).replace('�', '&#xFFFD;')


def chips(rec):
    out = []
    if rec.get('binary'):
        out.append('<span class="chip">binary</span>')
        return ''.join(out)
    if rec.get('status') == 'R':
        out.append('<span class="chip">renamed from ' + esc(rec['old']) + '</span>')
    n = len(rec.get('hunks', []))
    if n:
        out.append('<span class="chip hunks">%d hunk%s</span>' % (n, '' if n == 1 else 's'))
        out.append('<span class="chip tok"><span class="del">-%d</span> <span class="add">+%d</span> tokens</span>' % (
            sum(h['rem'] for h in rec['hunks']), sum(h['add'] for h in rec['hunks'])))
    for k, v in sorted(rec.get('mech', {}).items()):
        out.append('<span class="chip mech">%s &times;%d</span>' % (MECH_LABEL.get(k, k), v))
    if rec.get('formatting_only'):
        out.append('<span class="chip fmt">formatting only</span>')
    elif not n and rec.get('mech'):
        out.append('<span class="chip fmt">mechanical only</span>')
    return ''.join(out)


def render_hunks(rec):
    parts = []
    for h in rec['hunks']:
        hdr = '@@ -%d,%d +%d,%d @@' % (h['a1'], max(0, h['a2'] - h['a1'] + 1), h['b1'], max(0, h['b2'] - h['b1'] + 1))
        lines = ['<span class="hd">' + esc(hdr) + '</span>']
        for l in h['a']:
            lines.append('<span class="del">-' + esc(l) + '</span>')
        for l in h['b']:
            lines.append('<span class="add">+' + esc(l) + '</span>')
        parts.append('<pre class="diff">' + '\n'.join(lines) + '</pre>')
    return ''.join(parts)


def file_entry(rec):
    path = esc(rec['path'])
    if rec.get('hunks'):
        return ('<details class="file"><summary><code>%s</code> %s</summary>%s</details>' %
                (path, chips(rec), render_hunks(rec)))
    return '<div class="file flat"><code>%s</code> %s</div>' % (path, chips(rec))


CSS = r"""
:root {
  /* layout: one reading column for the notes, full-width scrollable blocks for the diffs */
  --bg: #f6f7f9; --panel: #ffffff; --fg: #1d2330; --muted: #5a6475; --rule: #d9dde5;
  --accent: #0f766e; --accent-soft: #d8efec; --del-bg: #fbe9e9; --del-fg: #8f1d1d;
  --add-bg: #e6f4ea; --add-fg: #1b5e32; --chip: #eceff4; --hd: #6b7280;
  --sans: "Source Sans 3", "Segoe UI", system-ui, sans-serif;
  --mono: "JetBrains Mono", Consolas, "Liberation Mono", monospace;
}
@media (prefers-color-scheme: dark) { :root:not([data-theme="light"]) {
  --bg: #15181e; --panel: #1c2027; --fg: #e4e7ec; --muted: #9aa3b2; --rule: #2d333d;
  --accent: #4fd1c5; --accent-soft: #163a37; --del-bg: #3a1d1f; --del-fg: #f2a3a3;
  --add-bg: #1b3524; --add-fg: #9ed9ae; --chip: #262b34; --hd: #8b94a5; color-scheme: dark } }
:root[data-theme="dark"] {
  --bg: #15181e; --panel: #1c2027; --fg: #e4e7ec; --muted: #9aa3b2; --rule: #2d333d;
  --accent: #4fd1c5; --accent-soft: #163a37; --del-bg: #3a1d1f; --del-fg: #f2a3a3;
  --add-bg: #1b3524; --add-fg: #9ed9ae; --chip: #262b34; --hd: #8b94a5; color-scheme: dark }
body { background: var(--bg); color: var(--fg); font-family: var(--sans); font-size: 16px; line-height: 1.5;
  padding-left: max(16px, env(safe-area-inset-left, 0px)); padding-right: max(16px, env(safe-area-inset-right, 0px)); padding-block: 24px 64px; }
main { max-width: 1180px; margin: 0 auto; }
h1 { font-size: 2rem; line-height: 1.15; margin: 0 0 .25rem; text-wrap: balance; }
h2 { font-size: 1.4rem; margin: 2.5rem 0 .5rem; padding-top: 1rem; border-top: 1px solid var(--rule); text-wrap: balance; }
h3 { font-size: 1.05rem; margin: 1.5rem 0 .5rem; color: var(--muted); text-transform: uppercase; letter-spacing: .06em; font-weight: 600; }
p, li { max-width: 72ch; }
.sub { color: var(--muted); margin: 0 0 1.5rem; }
.legend { display: flex; flex-wrap: wrap; gap: .5rem 1.25rem; font-size: .9rem; color: var(--muted); margin: 1rem 0; }
.stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: .75rem; margin: 1rem 0 1.5rem; }
.stat { background: var(--panel); border: 1px solid var(--rule); border-radius: 6px; padding: .6rem .8rem; }
.stat b { display: block; font-size: 1.4rem; font-variant-numeric: tabular-nums; }
.stat span { color: var(--muted); font-size: .85rem; }
.toc { columns: 2; column-gap: 2rem; font-size: .95rem; }
.toc a { color: var(--accent); text-decoration: none; }
.toc li { break-inside: avoid; }
.notes p { margin: .5rem 0; }
.file { border-left: 3px solid var(--rule); padding: .25rem .75rem; margin: .35rem 0; min-width: 0; }
.file.flat { color: var(--muted); font-size: .9rem; }
details.file { border-left-color: var(--accent); }
details.file > summary { cursor: pointer; list-style: none; }
details.file > summary::-webkit-details-marker { display: none; }
details.file > summary::before { content: "\25B8"; display: inline-block; width: 1em; color: var(--accent); }
details[open].file > summary::before { content: "\25BE"; }
code { font-family: var(--mono); font-size: .86em; }
.chip { display: inline-block; font-size: .75rem; padding: .05rem .45rem; border-radius: 999px; background: var(--chip); color: var(--muted); margin-left: .35rem; white-space: nowrap; }
.chip.hunks { background: var(--accent-soft); color: var(--accent); }
.chip.tok .del { color: var(--del-fg); } .chip.tok .add { color: var(--add-fg); }
pre.diff { font-family: var(--mono); font-size: .8rem; line-height: 1.4; background: var(--panel); border: 1px solid var(--rule); border-radius: 4px; padding: .5rem .75rem; overflow-x: auto; margin: .5rem 0; }
pre.diff .hd { color: var(--hd); }
pre.diff .del { display: block; background: var(--del-bg); color: var(--del-fg); }
pre.diff .add { display: block; background: var(--add-bg); color: var(--add-fg); }
.addrem { font-family: var(--mono); font-size: .8rem; columns: 2; column-gap: 2rem; }
.addrem div { break-inside: avoid; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.addrem .A::before { content: "+ "; color: var(--add-fg); } .addrem .D::before { content: "\2212  "; color: var(--del-fg); }
.tools { display: flex; gap: .5rem; flex-wrap: wrap; align-items: center; margin: 1rem 0; }
.tools input { font: inherit; padding: .3rem .5rem; border: 1px solid var(--rule); border-radius: 4px; background: var(--panel); color: var(--fg); min-width: 0; flex: 1 1 220px; }
.tools button { font: inherit; padding: .3rem .7rem; border: 1px solid var(--rule); border-radius: 4px; background: var(--panel); color: var(--fg); cursor: pointer; }
.tools button:focus-visible, .tools input:focus-visible, details.file > summary:focus-visible { outline: 2px solid var(--accent); outline-offset: 2px; }
@media (max-width: 700px) { .toc, .addrem { columns: 1; } h1 { font-size: 1.5rem; } }
@media (prefers-reduced-motion: reduce) { * { scroll-behavior: auto !important; } }
"""

JS = r"""
<script>
(function(){
  var q=document.getElementById('filter'), items=Array.prototype.slice.call(document.querySelectorAll('.file'));
  if(q){ q.addEventListener('input', function(){ var v=q.value.toLowerCase(); items.forEach(function(el){ el.hidden = v && el.textContent.toLowerCase().indexOf(v)<0; }); }); }
  var ex=document.getElementById('expand'), co=document.getElementById('collapse');
  if(ex) ex.addEventListener('click', function(){ document.querySelectorAll('details.file').forEach(function(d){ if(!d.hidden) d.open=true; }); });
  if(co) co.addEventListener('click', function(){ document.querySelectorAll('details.file').forEach(function(d){ d.open=false; }); });
})();
</script>
"""

HEAD = ('<title>%s</title>\n<meta name="claude:viewport-fit-sides" content="cover">\n'
        '<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Source+Sans+3:wght@400;600;700&family=JetBrains+Mono:wght@400;600&display=swap">\n'
        '<style>%s</style>\n')

# group files by area
by_area = {}
for rec in files:
    by_area.setdefault(area_of(rec['path']), []).append(rec)

totals = {'A': 0, 'D': 0, 'R': 0, 'M': 0, 'content': 0, 'fmt': 0, 'mech': 0, 'binary': 0, 'hunks': 0}
area_stats = {}
for key, recs in by_area.items():
    st = {'A': 0, 'D': 0, 'R': 0, 'M': 0, 'content': 0, 'fmt': 0, 'mech': 0, 'binary': 0, 'hunks': 0}
    for r in recs:
        st[r['status'] if r['status'] in 'ADR' else 'M'] += 1
        if r['status'] in ('A', 'D'):
            continue
        if r.get('binary'):
            st['binary'] += 1
        elif r.get('hunks'):
            st['content'] += 1
            st['hunks'] += len(r['hunks'])
        elif r.get('formatting_only'):
            st['fmt'] += 1
        elif r.get('mech'):
            st['mech'] += 1
        else:
            st['fmt'] += 1
    area_stats[key] = st
    for k in totals:
        totals[k] += st[k]


def stat_cards(st, with_addrem=True):
    cards = []
    if with_addrem:
        cards.append(('%d' % st['A'], 'files only in ' + annot.RIGHT))
        cards.append(('%d' % st['D'], 'files only in ' + annot.LEFT))
    cards.append(('%d' % st['content'], 'files with content changes'))
    cards.append(('%d' % st['hunks'], 'content hunks shown'))
    cards.append(('%d' % st['mech'], 'files with mechanical changes only'))
    cards.append(('%d' % st['fmt'], 'files with formatting changes only'))
    if st['binary']:
        cards.append(('%d' % st['binary'], 'binary files that differ'))
    return '<div class="stats">' + ''.join('<div class="stat"><b>%s</b><span>%s</span></div>' % c for c in cards) + '</div>'


def render_area(key, title, recs, st):
    out = ['<h2 id="%s">%s</h2>' % (esc(key), esc(title))]
    note = annot.NOTES.get(key)
    if note:
        out.append('<div class="notes">' + note + '</div>')
    out.append(stat_cards(st, with_addrem=True))
    if key in COUNT_ONLY:
        return '\n'.join(out)
    content = [r for r in recs if r.get('hunks')]
    quiet = [r for r in recs if r['status'] not in ('A', 'D') and not r.get('hunks')]
    adds = [r for r in recs if r['status'] == 'A']
    dels = [r for r in recs if r['status'] == 'D']
    if content:
        out.append('<h3>Content changes</h3>')
        out.extend(file_entry(r) for r in content)
    if quiet and not COMPACT:
        out.append('<h3>Mechanical, formatting-only, renamed or binary</h3>')
        out.extend(file_entry(r) for r in quiet)
    elif quiet and COMPACT:
        n_mech = sum(1 for r in quiet if r.get('mech') and not r.get('formatting_only'))
        n_fmt = sum(1 for r in quiet if r.get('formatting_only') or (not r.get('mech') and not r.get('binary')))
        n_bin = sum(1 for r in quiet if r.get('binary'))
        bits = []
        if n_mech: bits.append('%d with only NL_OVERRIDE, nullptr, cast, copyright or URL changes' % n_mech)
        if n_fmt: bits.append('%d with formatting differences only' % n_fmt)
        if n_bin: bits.append('%d binary' % n_bin)
        out.append('<p class="sub">Not listed: %s.</p>' % '; '.join(bits))
    if not NO_ADDREM and (adds or dels):
        out.append('<h3>Whole files</h3><div class="addrem">')
        out.extend('<div class="A"><code>%s</code></div>' % esc(r['path']) for r in adds)
        out.extend('<div class="D"><code>%s</code></div>' % esc(r['path']) for r in dels)
        out.append('</div>')
    return '\n'.join(out)


def page(title, body):
    return HEAD % (esc(title), CSS) + '<main>' + body + '</main>' + JS


os.makedirs(out_dir, exist_ok=True)
ordered = [(k, t) for k, t, _ in annot.AREAS if k in by_area] + ([('other', 'Everything else')] if 'other' in by_area else [])
page_title = annot.TITLE + (' (content only)' if COMPACT else '')
intro = ['<h1>%s</h1><p class="sub">%s</p>' % (esc(page_title), annot.SUBTITLE + (' This version lists only files with content changes; formatting-only, NL_OVERRIDE-only, nullptr-only, copyright-only and URL-only files and hunks are counted, not shown.' if COMPACT else '')), annot.INTRO,
         '<div class="legend"><span><b>&minus;</b> lines only on %s</span><span><b>+</b> lines only on %s</span><span>Hunks are token-level: whitespace, wrapping and line-ending differences are not shown.</span></div>' % (annot.LEFT, annot.RIGHT),
         stat_cards(totals, with_addrem=not NO_ADDREM)]
toc = '<ul class="toc">' + ''.join('<li><a href="%s#%s">%s</a> <span class="chip">%d files</span></li>' % (
    ('area-' + k + '.html') if SPLIT else '', esc(k), esc(t), len(by_area[k])) for k, t in ordered) + '</ul>'
intro.append(toc)
tools = '<div class="tools"><input id="filter" type="search" placeholder="Filter files by path or text" aria-label="Filter files"><button id="expand" type="button">Expand all</button><button id="collapse" type="button">Collapse all</button></div>'
if SPLIT:
    open(os.path.join(out_dir, 'index.html'), 'w', encoding='utf-8').write(page(annot.TITLE, '\n'.join(intro)))
    for k, t in ordered:
        body = '<p><a href="index.html">&larr; %s</a></p>' % esc(annot.TITLE) + tools + render_area(k, t, by_area[k], area_stats[k])
        open(os.path.join(out_dir, 'area-' + k + '.html'), 'w', encoding='utf-8').write(page(page_title + ': ' + t, body))
    print('split pages written to', out_dir)
else:
    body = '\n'.join(intro) + tools + '\n'.join(render_area(k, t, by_area[k], area_stats[k]) for k, t in ordered)
    p = os.path.join(out_dir, 'index.html')
    open(p, 'w', encoding='utf-8').write(page(page_title, body))
    print('written', p, os.path.getsize(p) // 1024, 'KB')
print(json.dumps({k: area_stats[k] for k, _ in ordered}, indent=0)[:4000])
