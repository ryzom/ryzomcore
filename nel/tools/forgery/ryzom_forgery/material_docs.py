"""Extracts per-option tooltip text from `docs/material_options.md`, so
tool UIs (e.g. object_editor.py's material editor) can show the same
player-friendly explanations as inline ImGui tooltips instead of
duplicating that text in code.

Relies on the doc's own documented convention: each option is a
`## Title {#stable-key}` section, with a `**Résumé :**` line right after
the header holding the short text meant for a tooltip.
"""

import re
from pathlib import Path
from typing import Dict, NamedTuple

DOC_PATH = Path(__file__).resolve().parent.parent / "docs" / "material_options.md"

_HEADER_RE = re.compile(r"^#{2,3} (?P<title>.+?) \{#(?P<key>[a-z0-9-]+)\}\s*$", re.MULTILINE)
_SUMMARY_RE = re.compile(r"\*\*Résumé\s*:\*\*\s*(?P<summary>.+?)(?:\n\n|\Z)", re.DOTALL)


class MaterialDoc(NamedTuple):
	title: str
	summary: str  # one-paragraph text meant for a tooltip
	full_text: str  # the option's full section body, summary included


def load_material_docs(path: Path = DOC_PATH) -> Dict[str, MaterialDoc]:
	"""Returns {key: MaterialDoc} for every `## Title {#key}` section in the
	doc. Missing/unreadable file -> empty dict, so a UI can degrade to no
	tooltips instead of crashing."""
	try:
		text = path.read_text(encoding="utf-8")
	except OSError:
		return {}

	headers = list(_HEADER_RE.finditer(text))
	docs = {}
	for i, header in enumerate(headers):
		start = header.end()
		end = headers[i + 1].start() if i + 1 < len(headers) else len(text)
		body = text[start:end].strip()

		summary_match = _SUMMARY_RE.search(body)
		summary = " ".join(summary_match.group("summary").split()) if summary_match else ""

		docs[header.group("key")] = MaterialDoc(title=header.group("title"), summary=summary, full_text=body)
	return docs
