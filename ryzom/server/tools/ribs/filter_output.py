#!/usr/bin/env python

import sys

use_filter=""
if len(sys.argv) > 1:
	use_filter=sys.argv[1]

while True:
	i = ''
	line = ''
	while i != '\n':
		i = sys.stdin.read(1)
		if not i:
			exit();
		line += i
	if len(line) > 1:
		sline = line[:-1].split(" ")
		if len(sline) > 7 and "INF" in sline[0] and sline[7] and not "Exception" in line:
			if sline[7][-1] == "%":
				sline[7] = "[%3d%%]" % int(sline[7][:-1])
			fline = " ".join(sline[7:])
			if use_filter and use_filter in fline:
				print("<strong>"+fline+"</strong>")
			else:
				print(fline)
			sys.stdout.flush()
		elif ("error" in line or "Error" in line) and not "Failed to open file" in line:
			fline = " ".join(sline[7:])
			print("<span class='error'>"+fline+"</span>")
			sys.stdout.flush()

