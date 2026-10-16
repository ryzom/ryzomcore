#!/usr/bin/env python3
# For every file/dir under ROOT whose name isn't already all-lowercase,
# add a lowercase-named symlink alongside it (skip if a same-named entry
# already exists - real file or prior symlink).
#
# Windows-authored sources #include headers with arbitrary casing, and
# vendor SDK headers sometimes #include each other with inconsistent
# casing (two casings of one file defeat #pragma once's file-identity
# tracking - C2011 class redefinition is the symptom). ciopfs would be
# the principled fix but is broken on stock Ubuntu 24.04; a lowercase
# alias farm across the SDK trees is the practical one.
import os, sys

root = sys.argv[1]
created = 0
skipped_collision = 0

for dirpath, dirnames, filenames in os.walk(root):
	for name in list(dirnames) + filenames:
		lower = name.lower()
		if name == lower:
			continue
		full = os.path.join(dirpath, name)
		lower_full = os.path.join(dirpath, lower)
		if os.path.lexists(lower_full):
			if not os.path.islink(lower_full):
				skipped_collision += 1
			continue
		os.symlink(name, lower_full)
		created += 1

print(f"{root}: created={created} skipped_collision={skipped_collision}")
