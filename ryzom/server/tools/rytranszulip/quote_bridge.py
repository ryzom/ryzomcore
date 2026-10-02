"""Validate Zulip quote references and render game chat quotes in Zulip."""

import re
import urllib.parse


_QUOTE = re.compile(
	r"^(?:@_\*\*[^*\n]+\*\* )?\[[^\n]*?\]\(([^\n)]+)\):\n"
	r"(?P<fence>`{3,})quote\n(.*?)\n(?P=fence)\n?",
	re.S,
)


class QuoteNotForwarded(ValueError):
	def __init__(self, original_id):
		super().__init__("Quoted message has not been forwarded or has expired")
		self.original_id = original_id


def extract_quote(message, base_url, fetch_original, can_reference):
	"""Return the answer and quoted Zulip ID after checking the original.

    ``fetch_original`` receives the linked ID and returns the Zulip message.
    ``can_reference`` confirms that the original has a game message.
    Both are supplied by the bridge's fetcher.
    """
	match = _QUOTE.match(message["content"])
	if not match:
		return message["content"], None

	site = urllib.parse.urlparse(base_url)
	link = urllib.parse.urlparse(urllib.parse.urljoin(base_url, match.group(1)))
	near = re.search(r"(?:^|/)near/(\d+)(?:$|/)", link.fragment)
	if link.scheme != site.scheme or link.netloc != site.netloc or not near:
		raise ValueError("Quote does not refer to this Zulip server")

	original_id = int(near.group(1))
	original = fetch_original(original_id)
	if not original:
		raise ValueError("Quoted message is unavailable")
	if message["type"] != original["type"]:
		raise ValueError("Quoted message belongs to another conversation")
	if message["type"] == "stream":
		if message["stream_id"] != original["stream_id"]:
			raise ValueError("Quoted message belongs to another channel")
	elif {user["id"] for user in message["display_recipient"]} != {
		user["id"] for user in original["display_recipient"]
	}:
		raise ValueError("Quoted message belongs to another private conversation")
	quoted = match.group(3).strip()
	quoted = re.sub(r"@_\*\*([^*\n]+)\*\*", r"@**\1**", quoted)
	if not quoted or quoted not in original["content"]:
		raise ValueError("Quote text does not match the original message")
	if not can_reference(original_id):
		raise QuoteNotForwarded(original_id)
	return message["content"][match.end():].lstrip("\r\n"), original_id


def render_quote(author, text, base_url, zulip_id=None, stream_id=None, recipients=None):
	"""Render a compact native Zulip quote with a link when one is known."""
	author = re.sub(r"\$[^$]*\$", "", author).replace("@**", "@\\*\\*")
	text = text.replace("@**", "@\\*\\*")
	link = ""
	if zulip_id and stream_id:
		link = base_url.rstrip("/") + "/#narrow/channel/" + str(stream_id) + "/near/" + str(zulip_id)
	elif zulip_id and recipients:
		users = ",".join(str(user) for user in recipients)
		link = base_url.rstrip("/") + "/#narrow/dm/" + users + "-dm/near/" + str(zulip_id)
	if link:
		name = author.replace("[", "\\[").replace("]", "\\]")
		author = "[" + name + "](" + link + ")"
	fence = "`" * max(3, max((len(run.group(0)) for run in re.finditer(r"`+", text)), default=0) + 1)
	return author + ":\n" + fence + "quote\n" + text + "\n" + fence + "\n"
