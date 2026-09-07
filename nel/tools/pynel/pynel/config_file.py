#!/usr/bin/env python3
# Copyright (C) 2026  Nuno Gonçalves (Ulukyn) <nuno@troispetits.net>
# Copyright (C) 2026  Claude Sonnet 5 (Anthropic) <noreply@anthropic.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Read/write NeL's CConfigFile format (.cfg -- client.cfg, panoply_*.cfg, and
every other NeL/Ryzom tool config). Port of the real flex/bison grammar in
nel/src/misc/config_file/cf_lexical.lpp + cf_gramatical.ypp -- both used by
CConfigFile::load() (nel/src/misc/config_file/config_file.cpp), which is
what client.cfg (ryzom/client/src/network_connection.cpp's NCConfigFile) and
panoply_maker.cpp both load. Confirmed with Nuno (2026-08-29) as a
cross-project dependency (ryztart's own hand-rolled .cfg parser doesn't
cover the full format), so this lives in pynel rather than Forgery/panoply
-specific code, and supports round-trip editing, not just reading.

Grammar summary (see the .lpp/.ypp for the authoritative source):
- `// line` and `/* block */` comments.
- `name = expr;` or `name += expr;` statements.
- expr is `{ v1, v2, ... }` (array, trailing comma ok, `{}` empty ok) or a
  single value -- a bare (non-array) assignment is still stored as a
  1-element value list (confirmed against a real production .cfg:
  `additionnal_paths = "...";` with no braces).
- A value is int/real/hex/string literal, `-x`/`+x` (unary), `a+b`/`a-b`/
  `a*b`/`a/b`, `(expr)`, or a reference to another already-defined variable
  (by name) -- real production panoply_*.cfg/client.cfg files observed so
  far only ever use int/real/string literals and unary minus, but the full
  grammar is ported since this is meant to replace ryztart's parser, not
  just cover panoply's own configs.
- `+=` extends an existing array: if the var was *also* touched earlier in
  the *same* file being parsed, the new values are appended at the end; if
  it came from an earlier-loaded file (see RootConfigFilename below), the
  new values are prepended instead -- both confirmed against
  cf_gramatical.ypp's ADD_ASSIGN action.
- `RootConfigFilename = "other.cfg";`: after parsing a file, CConfigFile
  looks for this var and, if present, loads that file too, but with
  `=` assignments only filling in variables *not already set* (the
  first-loaded file's own `=` values always win) -- see reparse()'s
  `cf_OverwriteExistingVariable`/`LoadRoot`/`CVar::Root` handling. This is
  the real mechanism behind e.g. client.cfg overriding client_default.cfg.

NOT ported (confirmed unused by every real .cfg sample available while
writing this -- panoply_common/fyros/matis/tryker/zorai/generique.cfg,
current_panoply.cfg): `#fileline` (an internal multi-file line-tracking
marker only emitted by an in-engine preprocessor, never hand-authored).
Parsing a file that uses it raises ConfigError rather than silently
mis-parsing.

Format-preserving edits: Document keeps the file's exact original text;
set() only rewrites the touched statement's value text (locating it via
token spans), splices it in, then re-tokenizes to keep every span accurate
for the next edit. Every other statement, comment, and blank line is left
byte-for-byte untouched -- dumps()/save() round-trips byte-identically to
the original file when nothing was set(). New variables are appended at
the end of the file. String values containing a literal `"` cannot be
represented (the format has no escape sequences, confirmed by the lexer's
`\"[^\"\n]*\"` rule) -- set() raises ConfigError rather than emit
unparseable output.

Usage:
	from pynel import config_file

	cfg = config_file.ConfigFile()
	cfg.load("client.cfg")  # follows RootConfigFilename automatically
	print(cfg.get("PatchServer"))

	doc = config_file.Document.load("panoply_common.cfg")
	print(doc.get("user_hues"))
	doc.set("user_hues", [10, 30, 78, 153, 212, 345, 35, 235])
	doc.save()
"""

import argparse
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Union


class ConfigError(Exception):
	pass


@dataclass
class _Token:
	kind: str
	text: str  # raw source text of just this token (no trivia)
	value: object  # decoded value for literal tokens (int/float/str), else None
	start: int  # offset in Document.text where trivia (comments/whitespace) before this token begins
	end: int  # offset in Document.text right after this token's own text

	@property
	def text_start(self) -> int:
		"""Offset where this token's own characters begin, i.e. right after its leading trivia."""
		return self.end - len(self.text)


_TOKEN_SPECS = [
	# order matters: alternatives are tried left-to-right, first match wins
	# (unlike flex's longest-match), so more specific forms must come first.
	("WS", r"[ \t\r\n]+"),
	("LINECOMMENT", r"//[^\n]*"),
	("BLOCKCOMMENT", r"/\*.*?\*/"),
	("STRING", r'"[^"\n]*"'),
	("HEX", r"0x[0-9a-fA-F]+"),
	# num2 (fractional digits required) must be tried before num1 (bare
	# trailing dot) -- plain regex alternation is first-match, not flex's
	# longest-match, so "0.1" would otherwise be cut into "0." + "1".
	("REAL", r"\d*\.\d+(?:[eE][-+]?\d+)?|\d+\.(?:[eE][-+]?\d+)?"),
	("INT", r"\d+"),
	("IDENT", r"[A-Za-z_][A-Za-z0-9_]*"),
	("ADD_ASSIGN", r"\+="),
	("ASSIGN", r"="),
	("PLUS", r"\+"),
	("MINUS", r"-"),
	("MULT", r"\*"),
	("DIVIDE", r"/"),
	("LPAREN", r"\("),
	("RPAREN", r"\)"),
	("LBRACE", r"\{"),
	("RBRACE", r"\}"),
	("COMMA", r","),
	("SEMICOLON", r";"),
]
_MASTER_RE = re.compile("|".join(f"(?P<{name}>{pattern})" for name, pattern in _TOKEN_SPECS), re.DOTALL)
_TRIVIA_KINDS = {"WS", "LINECOMMENT", "BLOCKCOMMENT"}


def _tokenize(text: str) -> List[_Token]:
	tokens: List[_Token] = []
	pos = 0
	trivia_start = 0
	n = len(text)
	while pos < n:
		m = _MASTER_RE.match(text, pos)
		if m is None:
			if text[pos] == "#":
				raise ConfigError(f"unsupported '#fileline'-style directive at offset {pos} (not ported, see module docstring)")
			raise ConfigError(f"unexpected character {text[pos]!r} at offset {pos}")
		kind = m.lastgroup
		raw = m.group()
		if kind in _TRIVIA_KINDS:
			pos = m.end()
			continue
		value = None
		if kind == "STRING":
			value = raw[1:-1]
		elif kind == "HEX":
			value = int(raw, 16)
		elif kind == "REAL":
			value = float(raw)
		elif kind == "INT":
			value = int(raw)
		tokens.append(_Token(kind=kind, text=raw, value=value, start=trivia_start, end=m.end()))
		pos = m.end()
		trivia_start = pos
	tokens.append(_Token(kind="EOF", text="", value=None, start=trivia_start, end=n))
	return tokens


@dataclass
class Assignment:
	name: str
	op: str  # "=" or "+="
	values: List[object]
	is_array: bool
	value_start: int  # offset in Document.text where the RHS expression's own text begins (after trivia)
	value_end: int  # offset right after the RHS expression's last token (before ';'s trivia)


@dataclass
class _VarState:
	values: List[object] = field(default_factory=list)
	is_array: bool = False
	root: bool = False
	from_local_file: bool = True


class _Parser:
	"""Recursive-descent parser matching cf_gramatical.ypp's expression rules
	(expr2: +/-, expr3: * //, expr4: unary +/-, parens, literals, variable
	refs) and the top-level `name = expr;` / `name += expr;` statement form."""

	def __init__(self, tokens: List[_Token], vars_so_far: Dict[str, _VarState]):
		self.tokens = tokens
		self.i = 0
		self.vars = vars_so_far

	def _peek(self) -> _Token:
		return self.tokens[self.i]

	def _expect(self, kind: str) -> _Token:
		tok = self.tokens[self.i]
		if tok.kind != kind:
			raise ConfigError(f"expected {kind}, got {tok.kind} ({tok.text!r}) at offset {tok.start}")
		self.i += 1
		return tok

	def parse_statements(self) -> List[Assignment]:
		out = []
		while self._peek().kind != "EOF":
			out.append(self._parse_statement())
		return out

	def _parse_statement(self) -> Assignment:
		name_tok = self._expect("IDENT")
		op_tok = self._peek()
		if op_tok.kind not in ("ASSIGN", "ADD_ASSIGN"):
			raise ConfigError(f"expected '=' or '+=' after '{name_tok.text}' at offset {op_tok.start}")
		self.i += 1
		value_start = self._peek().text_start
		values, is_array = self._parse_expression()
		value_end = self.tokens[self.i - 1].end
		self._expect("SEMICOLON")
		asn = Assignment(
			name=name_tok.text, op=("=" if op_tok.kind == "ASSIGN" else "+="),
			values=values, is_array=is_array, value_start=value_start, value_end=value_end,
		)
		self._apply_forward(asn)
		return asn

	def _apply_forward(self, asn: Assignment) -> None:
		"""Makes this statement's result visible to later `variable` references
		within the same parse, matching the grammar's shared, incrementally-
		built _Vars vector."""
		existing = self.vars.get(asn.name)
		if asn.op == "=" or existing is None:
			self.vars[asn.name] = _VarState(values=list(asn.values), is_array=asn.is_array)
		else:
			self.vars[asn.name] = _VarState(values=existing.values + list(asn.values), is_array=True)

	def _parse_expression(self):
		if self._peek().kind == "LBRACE":
			self.i += 1
			values = []
			if self._peek().kind == "RBRACE":
				self.i += 1
				return values, True
			while True:
				values.append(self._parse_expr2())
				tok = self._peek()
				if tok.kind == "COMMA":
					self.i += 1
					if self._peek().kind == "RBRACE":
						self.i += 1
						break
					continue
				elif tok.kind == "RBRACE":
					self.i += 1
					break
				else:
					raise ConfigError(f"expected ',' or '}}' at offset {tok.start}")
			return values, True
		value = self._parse_expr2()
		return [value], False

	def _parse_expr2(self):
		v = self._parse_expr3()
		while self._peek().kind in ("PLUS", "MINUS"):
			op = self._peek().kind
			self.i += 1
			rhs = self._parse_expr3()
			v = self._apply_op(v, rhs, op)
		return v

	def _parse_expr3(self):
		v = self._parse_expr4()
		while self._peek().kind in ("MULT", "DIVIDE"):
			op = self._peek().kind
			self.i += 1
			rhs = self._parse_expr4()
			v = self._apply_op(v, rhs, op)
		return v

	def _parse_expr4(self):
		tok = self._peek()
		if tok.kind == "PLUS":
			self.i += 1
			return self._parse_expr4()
		if tok.kind == "MINUS":
			self.i += 1
			v = self._parse_expr4()
			if isinstance(v, str):
				raise ConfigError(f"cannot negate a string value at offset {tok.start}")
			return -v
		if tok.kind == "LPAREN":
			self.i += 1
			v = self._parse_expr2()
			self._expect("RPAREN")
			return v
		if tok.kind in ("INT", "REAL", "HEX", "STRING"):
			self.i += 1
			return tok.value
		if tok.kind == "IDENT":
			self.i += 1
			state = self.vars.get(tok.text)
			if state is None or not state.values:
				raise ConfigError(f"reference to undefined variable '{tok.text}' at offset {tok.start}")
			return state.values[0]
		raise ConfigError(f"unexpected token {tok.kind} ({tok.text!r}) in expression at offset {tok.start}")

	@staticmethod
	def _apply_op(a, b, op: str):
		if isinstance(a, str) or isinstance(b, str):
			raise ConfigError(f"arithmetic on string values is not supported ({op})")
		if op == "PLUS":
			return a + b
		if op == "MINUS":
			return a - b
		if op == "MULT":
			return a * b
		if op == "DIVIDE":
			result = a / b
			return int(result) if isinstance(a, int) and isinstance(b, int) else result
		raise AssertionError(op)


def _render_literal(v) -> str:
	if isinstance(v, bool):
		return "1" if v else "0"
	if isinstance(v, str):
		if '"' in v:
			raise ConfigError(f"cannot represent string {v!r}: the format has no escape sequences, literal '\"' is unrepresentable")
		return f'"{v}"'
	if isinstance(v, int):
		return str(v)
	if isinstance(v, float):
		return repr(v)
	raise ConfigError(f"unsupported value type: {type(v)!r}")


class Document:
	"""One .cfg file: parses to a name -> values dict while keeping the exact
	original text, so set() can rewrite just the touched value and dumps()/
	save() reproduces everything else byte-for-byte."""

	def __init__(self, text: str, path: Optional[Path] = None):
		self.path = path
		self.text = text
		self._reparse()

	def _reparse(self) -> None:
		tokens = _tokenize(self.text)
		vars_so_far: Dict[str, _VarState] = {}
		self._assignments = _Parser(tokens, vars_so_far).parse_statements()
		self._vars = vars_so_far

	@classmethod
	def parse(cls, text: str) -> "Document":
		return cls(text)

	@classmethod
	def load(cls, path: Union[str, Path]) -> "Document":
		path = Path(path)
		try:
			text = path.read_text(encoding="utf-8")
		except UnicodeDecodeError:
			text = path.read_text(encoding="latin-1")
		return cls(text, path=path)

	def names(self) -> List[str]:
		return list(self._vars.keys())

	def has(self, name: str) -> bool:
		return name in self._vars

	def get(self, name: str) -> List[object]:
		state = self._vars.get(name)
		if state is None:
			raise KeyError(name)
		return list(state.values)

	def get_str(self, name: str, index: int = 0) -> str:
		return str(self.get(name)[index])

	def get_int(self, name: str, index: int = 0) -> int:
		return int(self.get(name)[index])

	def get_float(self, name: str, index: int = 0) -> float:
		return float(self.get(name)[index])

	def is_array(self, name: str) -> bool:
		state = self._vars.get(name)
		if state is None:
			raise KeyError(name)
		return state.is_array

	def set(self, name: str, values: List[object], is_array: Optional[bool] = None) -> None:
		"""Overwrites `name`'s value with `values` (a plain Python list), only
		rewriting that one statement's RHS text -- every other statement,
		comment, and blank line in the file is left untouched. If `name`
		doesn't exist yet, appends a new `name = ...;` statement at the end
		of the file."""
		if not values:
			raise ConfigError("set() requires at least one value")
		array = is_array if is_array is not None else (len(values) != 1)
		rendered = ", ".join(_render_literal(v) for v in values)
		literal = f"{{ {rendered} }}" if array else _render_literal(values[0])

		existing = [a for a in self._assignments if a.name == name]
		if existing:
			target = existing[-1]  # last occurrence is the effective one, matches CConfigFile's last-wins semantics
			self.text = self.text[:target.value_start] + literal + self.text[target.value_end:]
		else:
			sep = "" if self.text.endswith("\n") or not self.text else "\n"
			self.text = f"{self.text}{sep}{name} = {literal};\n"
		self._reparse()

	def dumps(self) -> str:
		return self.text

	def save(self, path: Optional[Union[str, Path]] = None) -> None:
		target = Path(path) if path is not None else self.path
		if target is None:
			raise ConfigError("no path to save to -- pass one explicitly or use Document.load()")
		target.write_text(self.text, encoding="utf-8")
		self.path = target


class ConfigFile:
	"""Multi-file merged view, matching CConfigFile::load()/reparse(): the
	first-loaded file's `=` values always win; a later file (discovered via
	that first file's own RootConfigFilename variable) only fills in
	variables not already set, though `+=` can still extend an existing
	array either way. See the module docstring for the exact rules.

	This class is read-only over the merge -- to edit a specific file, load
	it directly as a Document and use its own set()/save()."""

	def __init__(self):
		self.documents: List[Document] = []
		self._vars: Dict[str, _VarState] = {}

	def load(self, path: Union[str, Path]) -> None:
		path = Path(path)
		is_first = len(self.documents) == 0
		overwrite = is_first
		load_root = not is_first

		for state in self._vars.values():
			state.from_local_file = False

		doc = Document.load(path)
		self.documents.append(doc)

		for asn in doc._assignments:
			self._apply_assignment(asn, overwrite=overwrite, load_root=load_root)

		root_state = self._vars.get("RootConfigFilename")
		if root_state and root_state.values:
			root_path = Path(str(root_state.values[0]))
			if not root_path.is_absolute():
				root_path = path.parent / root_path
			already_loaded = any(d.path is not None and Path(d.path).resolve() == root_path.resolve() for d in self.documents)
			if not already_loaded and root_path.exists():
				self.load(root_path)

	def _apply_assignment(self, asn: Assignment, overwrite: bool, load_root: bool) -> None:
		existing = self._vars.get(asn.name)
		if asn.op == "=":
			if existing is None:
				self._vars[asn.name] = _VarState(values=list(asn.values), is_array=asn.is_array, root=load_root, from_local_file=True)
			elif overwrite or existing.root or asn.name == "RootConfigFilename":
				existing.values = list(asn.values)
				existing.is_array = asn.is_array
				existing.root = load_root
				existing.from_local_file = True
			# else: an already-locally-set variable is not overridden by a later (root/fallback) file
		else:  # "+="
			if existing is None:
				self._vars[asn.name] = _VarState(values=list(asn.values), is_array=True, root=load_root, from_local_file=True)
			else:
				if existing.from_local_file:
					existing.values = existing.values + list(asn.values)  # extending within the same file: append
				else:
					existing.values = list(asn.values) + existing.values  # extending from a later/root file: prepend
				existing.is_array = True
				existing.from_local_file = True

	def names(self) -> List[str]:
		return list(self._vars.keys())

	def has(self, name: str) -> bool:
		return name in self._vars

	def get(self, name: str) -> List[object]:
		state = self._vars.get(name)
		if state is None:
			raise KeyError(name)
		return list(state.values)

	def get_str(self, name: str, index: int = 0) -> str:
		return str(self.get(name)[index])

	def get_int(self, name: str, index: int = 0) -> int:
		return int(self.get(name)[index])

	def get_float(self, name: str, index: int = 0) -> float:
		return float(self.get(name)[index])


def _dump(names: List[str], get) -> None:
	for name in names:
		values = get(name)
		print(f"{name} = {values[0]!r}" if len(values) == 1 else f"{name} = {', '.join(repr(v) for v in values)}")


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read/write NeL .cfg files (CConfigFile format)")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="print every variable in a .cfg file, following RootConfigFilename")
	p_dump.add_argument("path", type=Path)

	p_set = sub.add_parser("set", help="set one variable's value(s) in place, preserving the rest of the file")
	p_set.add_argument("path", type=Path)
	p_set.add_argument("name")
	p_set.add_argument("values", nargs="+", help="new value(s) -- parsed as int, then float, then kept as a string")

	return parser


def _coerce(raw: str):
	try:
		return int(raw)
	except ValueError:
		pass
	try:
		return float(raw)
	except ValueError:
		pass
	return raw


def _main() -> None:
	args = _build_arg_parser().parse_args()

	if args.command == "dump":
		cfg = ConfigFile()
		cfg.load(args.path)
		_dump(cfg.names(), cfg.get)
	elif args.command == "set":
		doc = Document.load(args.path)
		doc.set(args.name, [_coerce(v) for v in args.values])
		doc.save()
		print(f"wrote {args.path}")


if __name__ == "__main__":
	_main()
