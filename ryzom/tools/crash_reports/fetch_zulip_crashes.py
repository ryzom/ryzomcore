#!/usr/bin/env python3
"""Download the crash reports posted in a topic of a Zulip channel.

Each message is saved as crash_<message id>.md (raw Markdown, as posted) in the
output directory (default: the directory of this script). Messages already
downloaded are skipped, so the script can be run again to get the new ones.

Credentials are read from a zuliprc file (INI, section [api] with the keys
email, key and site), the same file the official Zulip tools use.

Example:
	fetch_zulip_crashes.py --channel <channel> --topic <topic>
"""

import argparse
import base64
import configparser
import json
import os
import re
import sys
import urllib.error
import urllib.parse
import urllib.request

BATCH_SIZE = 1000
FILE_RE = re.compile(r"^crash_(\d+)\.md$")


def load_config(path):
	config = configparser.ConfigParser()
	if not config.read(os.path.expanduser(path)):
		sys.exit("Cannot read zuliprc file: %s" % path)
	try:
		api = config["api"]
		return api["email"], api["key"], api["site"].rstrip("/")
	except KeyError as e:
		sys.exit("Missing %s in [api] section of %s" % (e, path))


def get_messages(site, auth, narrow, anchor):
	params = urllib.parse.urlencode({
		"anchor": anchor,
		"include_anchor": "false",
		"num_before": 0,
		"num_after": BATCH_SIZE,
		"narrow": json.dumps(narrow),
		"apply_markdown": "false",
	})
	request = urllib.request.Request("%s/api/v1/messages?%s" % (site, params))
	request.add_header("Authorization", "Basic " + auth)
	try:
		with urllib.request.urlopen(request, timeout=60) as response:
			data = json.load(response)
	except urllib.error.HTTPError as e:
		sys.exit("Zulip error %d: %s" % (e.code, e.read().decode("utf-8", "replace")))
	except urllib.error.URLError as e:
		sys.exit("Cannot reach Zulip: %s" % e.reason)
	if data.get("result") != "success":
		sys.exit("Zulip error: %s" % data.get("msg"))
	return data["messages"]


def last_downloaded_id(out_dir):
	ids = [int(m.group(1)) for m in map(FILE_RE.match, os.listdir(out_dir)) if m]
	return max(ids) if ids else None


def main():
	parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument("--channel", required=True, help="name of the Zulip channel (stream)")
	parser.add_argument("--topic", required=True, help="name of the topic in the channel")
	parser.add_argument("--zuliprc", default="~/.zuliprc", help="credentials file (default: %(default)s)")
	parser.add_argument("--out", default=os.path.dirname(os.path.abspath(__file__)), help="output directory (default: this script's directory)")
	args = parser.parse_args()

	email, key, site = load_config(args.zuliprc)
	auth = base64.b64encode(("%s:%s" % (email, key)).encode()).decode()

	narrow = [
		{"operator": "channel", "operand": args.channel},
		{"operator": "topic", "operand": args.topic},
	]

	os.makedirs(args.out, exist_ok=True)
	last_id = last_downloaded_id(args.out)
	# only fetch what is newer than the last downloaded message
	anchor = last_id if last_id is not None else "oldest"

	saved = 0
	while True:
		messages = get_messages(site, auth, narrow, anchor)
		for message in messages:
			path = os.path.join(args.out, "crash_%d.md" % message["id"])
			with open(path, "w", encoding="utf-8", newline="\n") as f:
				f.write(message["content"])
			saved += 1
		if len(messages) < BATCH_SIZE:
			break
		anchor = messages[-1]["id"]

	print("%d new report(s) saved in %s" % (saved, args.out))


if __name__ == "__main__":
	main()
