#
# Copyright (C) 2019-2020  Jan BOON <jan.boon@kaetemi.be>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# This script specifies by whom and when source files were modified 
# from their original, in order to comply with the AGPLv3 requirements.

# It will output a mapping of unidentified contributors, for matching 
# names more appropriately. If you are being paid by Winch Gate, map 
# your name to "winch_gate".

# There's also a mapping of commits to ignore, they do not count towards
# contribution, as well as a mapping of commits for which to override 
# the author.

# set PATH=C:\Python38;%PATH%
# python -m pip install --upgrade pip
# python -m pip install gitpython
# python annotate.py

import time
from datetime import datetime
from pathlib import Path

from git import Repo
repo = Repo("../..")

# Mapping for author short name to full display name
authors = { }
authors["winch_gate"] = "Winch Gate Property Limited"
authors["nevrax"] = "Nevrax Ltd."
authors["sfb"] = "Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>" # OpenNeL
authors["ace"] = "Vianney LECROART (ace) <vl@ryzom.com>" # Nevrax, Winch Gate
authors["rti"] = "Robert TIMM (rti) <mail@rtti.de>" # OpenNeL
authors["botanic"] = "Matthew LAGOE (Botanic) <cyberempires@gmail.com>" # OpenNeL
authors["rtsan"] = "Henri KUUSTE <al-rtsan@totl.net>" # OpenNeL
authors["kaetemi"] = "Jan BOON (Kaetemi) <jan.boon@kaetemi.be>"
authors["kervala"] = "Cédric OCHS (kervala) <kervala@gmail.com>"
authors["glorf"] = "Guillaume DUPUY (glorf) <glorf@glorf.fr>"
authors["ulukyn"] = "Nuno GONCALVES (Ulukyn) <ulukyn@ryzom.com>"
authors["nimetu"] = "Meelis MAGI (Nimetu) <nimetu@gmail.com>"
authors["dfighter"] = "Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>" # GSoC
authors["dnk-88"] = "Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>" # GSoC
authors["fhenon"] = "Fabien HENON" # GSoC
authors["adrianj"] = "Adrian JAEKEL <aj@elane2k.com>" # GSoC
authors["cemycc"] = "Emanuel COSTEA <cemycc@gmail.com>" # GSoC
authors["inky"] = "Inky <jnk@mailoo.org>"
authors["riasan"] = "Riasan <riasan@ryzom.com>"
authors["karu"] = "karu"
authors["kishan"] = "Kishan GRIMOUT <kishan.grimout@gmail.com>"
authors["vinicius"] = "Vinicius ARROYO"
authors["llapp"] = "llapp"
authors["depyraken"] = "depyraken"
authors["gary"] = "Gary PRESTON"
authors["tobiasp"] = "Tobias PETERS <tobias.peters@kreativeffekt.at>"
authors["rodolpheb"] = "Rodolphe BREARD <code@breard.tf>"
authors["geelsd"] = "Dylan GEELS <geelsd@live.nl>"
authors["shubham"] = "Shubham MEENA <shubham.iit17@gmail.com>"
authors["liria"] = "liria <liria@gmx.fr>"
authors["etrange"] = "StudioEtrange <nomorgan@gmail.com>"
authors["sircotare"] = "SirCotare"
authors["rolandw"] = "Roland WINKLMEIER <roland.m.winklmeier@gmail.com>"
authors["thibg"] = "Thibaut GIRKA (ThibG) <thib@sitedethib.com>" # LibVR support
authors["xtarsia"] = "Xtarsia"

# Mapping from git author name to short name, dash to ignore author
short_authors = { }
short_authors["sfb"] = "sfb"
short_authors["Matt Raykowski <matt.raykowski@gmail.com>"] = "sfb"
short_authors["mattraykowski"] = "sfb"
short_authors["\"Matt Raykowski\" <matt.raykowski@gmail.com>"] = "sfb"
short_authors["vl"] = "ace"
short_authors["acemtp@acemtp-desktop <acemtp@acemtp-desktop>"] = "ace"
short_authors["acemtp@users.sourceforge.net <acemtp@users.sourceforge.net>"] = "ace"
short_authors["botanicvelious <cyberempires@gmail.com>"] = "botanic"
short_authors["botanic"] = "botanic"
short_authors["Matthew Lagoe <matthew.lagoe@subrigo.net>"] = "botanic"
short_authors["Botanic"] = "botanic"
short_authors["Matthew Lagoe@MatthewLagoe-PC <Matthew Lagoe@MatthewLagoe-PC>"] = "botanic"
short_authors["Botanic <admin@tempestintheaether.org>"] = "botanic"
short_authors["kaetemi <kaetemi@gmail.com>"] = "kaetemi"
short_authors["Jan Boon <jan.boon@kaetemi.be>"] = "kaetemi"
short_authors["Jan Boon <kaetemi@no-break.space>"] = "kaetemi"
short_authors["Jan Boon <kaetemi@gmail.com>"] = "kaetemi"
short_authors["Jan BOON (jan.boon@kaetemi.be)"] = "kaetemi"
short_authors["kaetemi"] = "kaetemi"
short_authors["kaetemi@users.sourceforge.net <kaetemi@users.sourceforge.net>"] = "kaetemi"
short_authors["kaetemi@kaevm.localdomain <kaetemi@kaevm.localdomain>"] = "kaetemi"
short_authors["Jan Boon (Kaetemi)"] = "kaetemi"
short_authors["NO-BREAK SPACE OÜ <support@no-break.space>"] = "-" # bot
short_authors["Ryzom Pipeline <ryzom-pipeline@kaetemi.be>"] = "-" # bot
short_authors["Nimetu <nimetu@gmail.com>"] = "nimetu"
short_authors["nimetu@gmail.com <nimetu@gmail.com>"] = "nimetu"
short_authors["Meelis Mägi <nimetu@gmail.com>"] = "nimetu"
short_authors["nimetu <nimetu@gmail.com>"] = "nimetu"
short_authors["nimetu"] = "nimetu"
short_authors["kervala"] = "kervala"
short_authors["Cédric OCHS <kervala@gmail.com>"] = "kervala"
short_authors["Nuno Gonçalves <ulukyn@gmail.com>"] = "ulukyn"
short_authors["ulukyn"] = "ulukyn"
short_authors["Ulukyn <ulukyn@gmail.com>"] = "ulukyn"
short_authors["ulukyn@gmail.com <ulukyn@gmail.com>"] = "ulukyn"
short_authors["Ulu Kyn <ulukyn@gmail.com>"] = "ulukyn"
short_authors["Inky <jnk@mailoo.org>"] = "inky"
short_authors["inky <jnk@mailoo.org>"] = "inky"
short_authors["Riasan <riasan@ryzom.com>"] = "riasan"
short_authors["Guillaume Dupuy <glorf@glorf.fr>"] = "glorf"
short_authors["karu"] = "karu"
short_authors["kishan_grimout"] = "kishan"
short_authors["KISHAN GRIMOUT <kishan.grimout@gmail.com>"] = "kishan"
short_authors["Vinicius Arroyo <vinicius.arroyo@gmail.com>"] = "vinicius"
short_authors["Llapp"] = "llapp"
short_authors["depyraken"] = "depyraken"
short_authors["Gary Preston <gary@mups.co.uk>"] = "gary"
short_authors["Tobias Peters <tobias.peters@kreativeffekt.at>"] = "tobiasp"
short_authors["dfighter1985"] = "dfighter"
short_authors["dfighter1985 <dfighter1985@gmail.com>"] = "dfighter"
short_authors["Laszlo Kis-Adam <dfighter1985@gmail.com>"] = "dfighter"
short_authors["Laszlo Kis-Adam"] = "dfighter"
short_authors["dfighter1985 <dfighter1985.arcemu@gmail.com>"] = "dfighter"
short_authors["dfighter <dfighter1985.arcemu@gmail.com>"] = "dfighter"
short_authors["timon <dnk-88@tut.by>"] = "dnk-88"
short_authors["dnk-88"] = "dnk-88"
short_authors["Dzmitry Kamiahin <dnk-88@tut.by>"] = "dnk-88"
short_authors["Dzmitry Kamiahin <dzmitry.kamiahin@gmail.com>"] = "dnk-88"
short_authors["Fabien_HENON"] = "fhenon"
short_authors["Rodolphe Breard <rodolphe@what.tf>"] = "rodolpheb"
short_authors["Rodolphe Breard <code@breard.tf>"] = "rodolpheb"
short_authors["Dylan Geels <geelsd@live.nl>"] = "geelsd"
short_authors["shubham_meena <shubham.iit17@gmail.com>"] = "shubham"
short_authors["shubham_meena"] = "shubham"
short_authors["liria <liria@gmx.fr>"] = "liria"
short_authors["liria"] = "liria"
short_authors["StudioEtrange <nomorgan@gmail.com>"] = "etrange"
short_authors["rti <mail@rtti.de>"] = "rti"
short_authors["rti"] = "rti"
short_authors["SirCotare"] = "sircotare"
short_authors["SirCotare@Medivh <SirCotare@Medivh>"] = "sircotare"
short_authors["Roland Winklmeier <roland.m.winklmeier@googlemail.com>"] = "rolandw"
short_authors["Adrian Jaekel <aj at elane2k dot com>"] = "adrianj"
short_authors["Emanuel Costea <cemycc@gmail.com>"] = "cemycc"
short_authors["cemycc <cemycc@gmail.com>"] = "cemycc"
short_authors["cemycc"] = "cemycc"
short_authors["Thibaut Girka <thib@sitedethib.com>"] = "thibg"
short_authors["Thibaut Girka (ThibG)"] = "thibg"
short_authors["Xtarsia <69606701+Xtarsia@users.noreply.github.com>"] = "xtarsia"
short_authors["Xtarsia"] = "xtarsia"
# short_authors["\"picomancer ext:(%22) <pico-bitbucketpub-4mcdxm39-onlyham@picomancer.com>"] = "-"
# short_authors["Quitta"] = "-"
# short_authors["Krolock"] = "-"
# short_authors["aquiles"] = "-"
# short_authors["Piotr Kaczmarek <kaczorek@gmail.com>"] = "-"
# short_authors["kerozcak"] = "-"
# short_authors["thorbjorn"] = "-"
# short_authors["DJanssens <daan.codes@gmail.com>"] = "-"
# short_authors["Michael Witrant <mike@lepton.fr>"] = "-"

generic_authors = [ ]
generic_authors += [ "Ryzom Core <http://ryzom.dev/>" ]
generic_authors += [ "by authors" ]

# Reverse mapping for parsing original annotation
for short_author in authors:
	short_authors[authors[short_author]] = short_author

# Mapping to override author of a git commit, dash to ignore commit
# * invalid commit
# - no authorship or various authors
# / initial commit, we don't have history of 2008-2010 Ryzom SVN and older 2008 OpenNeL SVN
# ? copyright not identified
override_author = { }
override_author["af454dd1cf9306afe0f074ea2b348a0629112a9e"] = "*" # Fix EOL issues on Mercurial repository
override_author["e7e51f01e080bfacd87331d16e3e124ba90a04ac"] = "dfighter" # Merge GUI Editor branch
override_author["d5c601ffa5690b89f121561bac165b83f39a896f"] = "/" # Initial commit.
override_author["adb4963102e226ff8121caa45a802b7d31c25a81"] = "dnk-88" # Merge OVQT
override_author["02e8f6e95629ca88820a01e4f62f336b61001214"] = "-" # EOL
override_author["6b3b85962a898c718a16882574abbc167c087816"] = "-" # EOL
override_author["29b38937a58ffd06ba64829a49f060ff7c9a2d0a"] = "-" # Header
override_author["61eefa97c2ffbca94e123d766f518ce006fa3ed4"] = "-" # Header
override_author["a9250a74f1140c08655d31cbe185a5e543e1e942"] = "-" # Header
override_author["dd1043eaaec28399aa0013ddf5b5d7e0c32b1ce1"] = "-" # Header
override_author["ff4a521b07a4983140d82f27fc2f0468a507d095"] = "-" # Header
override_author["43452ea27c6e92488d8bd1417b2aee60d75d8a68"] = "-" # Header
override_author["8e21fed1e6b79bf92f6364c7cb4f0c56e1dda103"] = "-" # Header cleanup
override_author["c8e562f37781d62ebc54b68ef74f7693de79a907"] = "-" # Header cleanup
override_author["dc734ed66226b257becae9fcd140898e14510e6a"] = "-" # Header cleanup
override_author["e97cf09e043e2e4b6f0f899b9230f4b6303fcc23"] = "/" # Initial commit.
override_author["a3a074f455a3f52e6fa4d44214f6c34289fa6f8c"] = "-" # Sync
override_author["141e7c645966ee3475097a75a65def8c9bd7086a"] = "-" # Sync
override_author["e6a617b8bcd1630dba5fc3b6ae9815775ba2c19d"] = "-" # Sync
override_author["41c8499bd4f1e6229a03c954132fcc5ba4f5dc3e"] = "-" # Sync
override_author["ecf990f8ae8e04d946ce1d5519c065dc6fedf4c4"] = "-" # Sync

# Exclude some paths
exclude_paths = { }
exclude_paths["nel/3rdparty"] = True
exclude_paths["nel/src/3d/driver/opengl/GL"] = True
exclude_paths["nel/src/3d/driver/opengl/EGL"] = True
exclude_paths["nel/src/3d/driver/opengl/GLES"] = True
exclude_paths["nel/src/3d/driver/opengl/KHR"] = True
exclude_paths["studio/src/3rdparty"] = True
exclude_paths["ryzom/common/src/game_share/ring_session_manager_itf.cpp"] = True
exclude_paths["ryzom/common/src/game_share/ring_session_manager_itf.h"] = True
exclude_paths["ryzom/server/src/entities_game_service/database_guild.cpp"] = True
exclude_paths["ryzom/server/src/entities_game_service/database_guild.h"] = True
exclude_paths["ryzom/server/src/entities_game_service/database_outpost.cpp"] = True
exclude_paths["ryzom/server/src/entities_game_service/database_outpost.h"] = True
exclude_paths["ryzom/server/src/entities_game_service/database_plr.cpp"] = True
exclude_paths["ryzom/server/src/entities_game_service/database_plr.h"] = True
exclude_paths["ryzom/server/src/shard_unifier_service/database_mapping.cpp"] = True
exclude_paths["ryzom/server/src/shard_unifier_service/database_mapping.h"] = True

# Programmatical remappings
def remap_author(blob, commit, author):
	# Remap author to copyright owner
	# You can map based on
	# - blob.path
	# - commit.committed_date
	if override_author.get(commit.hexsha) != None:
		return override_author[commit.hexsha]
	authored_date = datetime.utcfromtimestamp(commit.authored_date) # time.gmtime(commit.authored_date) # datetime.date(2002, 12, 26)
	authored_timestamp = commit.authored_date
	short_author = short_authors.get(author)
	if short_author == None:
		short_author = "?"
	if "Update GPL headers" in commit.message:
		# Ignore if commit message matches "Update GPL headers"
		short_author = "-"
	# If you're paid by Winch Gate, or signed copyright transfer with 
	# them, remap here, limit by authored_date if needed.
	if short_author == "ulukyn" or short_author == "ace":
		short_author = "winch_gate"
	if short_author == "riasan":
		short_author = "winch_gate"
	if short_author == "sircotare" and authored_date.year >= 2012:
		short_author = "winch_gate"
	if short_author == "inky" and authored_date.year < 2020:
		short_author = "winch_gate"
	if "feature-export-assimp" in commit.message and authored_date.year <= 2015:
		# Project paid for by Winch Gate
		short_author = "winch_gate"
	if short_author == "nimetu" and authored_date.year >= 2009:
		short_author = "winch_gate"
	if short_author == "kervala":
		# Don't know if they signed the copyright assignment agreement with Winch Gate or Ryzom Forge
		short_author = "?"
	if short_author == "karu" or short_author == "kishan" or short_author == "glorf":
		short_author = "?"
	if short_author == "llapp" or short_author == "vinicius" or short_author == "gary":
		short_author = "?"
	if short_author == "depyraken" or short_author == "tobiasp" or short_author == "rodolpheb":
		short_author = "?"
	if short_author == "geelsd" or short_author == "shubham" or short_author == "liria":
		short_author = "?"
	return short_author

# Mapping for original author, which is not included in list of 
# modification, but instead adjusts the original copyright date range.
# Author is based on first commit.
#def remap_original_author(blob, author):
#	# Remap original file author to original copyright owner
#	# Todo
#	if blob.path.startswith("code/nel/3rdparty"):
#		return "-" 
#	return "winch_gate"

def stringify_author(author):
	if author.email == "none@none" or author.email == "":
		return author.name
	return author.name + " <"  + author.email + ">"

def list_unknown_authors():
	print("-- List unknown authors --")
	listed_authors = {}
	for commit in repo.iter_commits():
		author = stringify_author(commit.author)
		if short_authors.get(author) == None and listed_authors.get(author) == None:
			listed_authors[author] = True
			print(author)

#def count_tree_size(tree):
#	res = len(tree.blobs)
#	for subtree in tree.trees:
#		res = res + count_tree_size(subtree)
#	return res
#
#def list_large_commits():
#	print("-- List large commits --")
#	for commit in repo.iter_commits():
#		print(commit.hexsha + ": " + str(count_tree_size(commit.tree)))

assert not repo.bare
#assert not repo.is_dirty()

# repo.untracked_files
# assert repo.head.ref == repo.heads.develop

print(str(repo))

print(str(repo.index))

tree = repo.head.ref.commit.tree
assert repo.tree() == repo.head.commit.tree
assert repo.tree() == tree

print(str(tree))

assert len(tree.trees) > 0
assert len(tree.blobs) > 0
assert len(tree.blobs) + len(tree.trees) == len(tree)

# for entry in tree.trees:
# 	for entry2 in entry.trees:
# 		print(entry2.path)

commit_counter = {}

historic_commit = repo.commit("e9692f5fea4afb02cf27d8a33c40587007bb13c5");
historic_paths = { }

header_not_found = { }

modified_blurb = "This source file has been modified by the following contributors:"
#modified_blurb = "This file contains modifications from the following contributors:"

def make_copyright(oldest_year, newest_year, long_name):
	if newest_year > oldest_year:
		return "Copyright (C) " + str(oldest_year) + "-" + str(newest_year) + "  " + long_name
	else:
		if newest_year != oldest_year:
			raise str(oldest_year) + "-" + str(newest_year)
		return "Copyright (C) " + str(newest_year) + "  " + long_name

def rewrite_cpp(path, copyright_oldest, copyright_newest, copyright_lines):
	# Read all notices until "// This program is free software"
	# Everything before the first "// Copyright" remains intact
	# Parse existing notices, merge with lists, track which one is first
	# Write out new copyright and modification notices
	contents = Path("../../" + path).read_text()
	content_start = contents.find("// This program is free software")
	if content_start < 0:
		header_not_found[path] = True
		print("Could not find \"// This program is free software\"")
		return
	copyright_start = contents.find("// Copyright", 0, content_start)
	if copyright_start < 0:
		header_not_found[path] = True
		print("Could not find \"// Copyright\"")
		return
	copyright_block = contents[copyright_start:content_start].splitlines()
	first_copyright = "?"
	for statement in copyright_block:
		if statement.startswith("// Copyright"):
			# Parse this statement
			# // Copyright (C) 2010  Winch Gate Property Limited
			# // Copyright (C) 2010-2019  Winch Gate Property Limited
			# // Copyright (C) 2010 - 2019  Winch Gate Property Limited
			# // Copyright (C) 2010       Winch Gate Property Limited
			copyright_end = statement.find(")") + 1
			if copyright_end < 0:
				copyright_end = len("// Copyright")
			year_start = statement.find("2", copyright_end)
			year_end = statement.find("-", year_start, year_start + 7)
			endyear_start = -1
			endyear_end = -1
			name_start = -1
			if year_end < 0:
				year_end = statement.find(" ", year_start)
				name_start = year_end
			else:
				endyear_start = statement.find("2", year_end)
				if endyear_start >= 0:
					endyear_end = statement.find(" ", endyear_start)
					name_start = endyear_end
				else:
					name_start = year_end
			oldestyear = int(statement[year_start:year_end].strip())
			newestyear = oldestyear
			if endyear_end >= 0:
				newestyear = int(statement[endyear_start:endyear_end].strip())
			name_long = statement[name_start:].strip()
			name_short = "*"
			if name_long in generic_authors:
				highest_author = 0
				for author in copyright_lines:
					if len(author) > 1:
						if copyright_lines[author] > highest_author:
							name_short = author
			else:
				name_short = short_authors[name_long]
			if name_short == "*":
				header_not_found[path] = True
				print("Copyright by authors failed to be specified")
				return
			#print(statement)
			#print(str(oldestyear))
			#print(str(newestyear))
			#print(name_long)
			#print(name_short)
			#if first_copyright == "?" or (path.startswith('code/studio') and first_copyright == 'winch_gate'):
			if first_copyright == "?":
				first_copyright = name_short
			if copyright_oldest.get(name_short) == None or copyright_newest.get(name_short) == None:
				copyright_oldest[name_short] = oldestyear
				copyright_newest[name_short] = newestyear
			else:
				if oldestyear < copyright_oldest[name_short]:
					copyright_oldest[name_short] = oldestyear
				if newestyear > copyright_newest[name_short]:
					copyright_newest[name_short] = newestyear
	new_statement = "// " + make_copyright(copyright_oldest[first_copyright], copyright_newest[first_copyright], authors[first_copyright]) + "\n"
	#has_blurb = False
	#for author in copyright_oldest:
	#	if author != first_copyright and len(author) > 1:
	#		if not has_blurb:
	#			new_statement += "//\n"
	#			new_statement += "// " + modified_blurb + "\n"
	#			has_blurb = True
	#		new_statement += "// " + make_copyright(copyright_oldest[author], copyright_newest[author], authors[author]) + "\n"
	modified_statements = [ ]
	for author in copyright_oldest:
		if author != first_copyright and len(author) > 1:
			modified_statements += [ "// " + make_copyright(copyright_oldest[author], copyright_newest[author], authors[author]) + "\n" ]
	if len(modified_statements) > 0:
		modified_statements.sort()
		new_statement += "//\n"
		new_statement += "// " + modified_blurb + "\n"
		for notice in modified_statements:
			new_statement += notice;
	new_statement += "//\n"
	#print(copyright_block)
	#print(new_statement)
	new_contents = contents[0:copyright_start] + new_statement + contents[content_start:]
	if contents != new_contents:
		print(new_statement)
		Path("../../" + path).write_text(new_contents)

def process_cpp(cpp_entry):
	print(cpp_entry.path)
	blame = repo.blame(repo.head.ref, cpp_entry.path)
	commit_lines = {}
	author_lines = {}
	author_oldest = {}
	author_newest = {}
	retry_lines = {}
	# Count number of lines per author
	for tuple in blame:
		commit = tuple[0]
		lines = tuple[1]
		if commit.hexsha == "af454dd1cf9306afe0f074ea2b348a0629112a9e":
			# Special treatment since this commit breaks blame history before 2012
			for line in lines:
				# Allow blame from old history if this line matches
				retry_lines[line] = True
			# Do not count this line
			continue
		line_count = len(lines)
		author = remap_author(cpp_entry, commit, stringify_author(commit.author))
		authored_date = datetime.utcfromtimestamp(commit.authored_date)
		if author_lines.get(author) == None:
			author_lines[author] = 0
			author_oldest[author] = authored_date.year
			author_newest[author] = authored_date.year
		author_lines[author] += line_count
		if authored_date.year < author_oldest[author]:
			author_oldest[author] = authored_date.year
		if authored_date.year > author_newest[author]:
			author_newest[author] = authored_date.year
		if commit_lines.get(commit.hexsha) == None:
			commit_lines[commit.hexsha] = 0
			if commit_counter.get(commit.hexsha) == None:
				commit_counter[commit.hexsha] = 0
			commit_counter[commit.hexsha] += 1
		commit_lines[commit.hexsha] += line_count
		# print(commit.hexsha)
		# print(len(lines))
	if historic_paths.get(cpp_entry.path) == True:
		historic_blame = repo.blame(historic_commit, cpp_entry.path)
		for tuple in historic_blame:
			commit = tuple[0]
			lines = tuple[1]
			line_count = 0
			for line in lines:
				if retry_lines.get(line) == True:
					line_count += 1
			if line_count == 0:
				continue
			# Exact copy of above from here on
			author = remap_author(cpp_entry, commit, stringify_author(commit.author))
			authored_date = datetime.utcfromtimestamp(commit.authored_date)
			if author_lines.get(author) == None:
				author_lines[author] = 0
				author_oldest[author] = authored_date.year
				author_newest[author] = authored_date.year
			author_lines[author] += line_count
			if authored_date.year < author_oldest[author]:
				author_oldest[author] = authored_date.year
			if authored_date.year > author_newest[author]:
				author_newest[author] = authored_date.year
			if commit_lines.get(commit.hexsha) == None:
				commit_lines[commit.hexsha] = 0
				if commit_counter.get(commit.hexsha) == None:
					commit_counter[commit.hexsha] = 0
				commit_counter[commit.hexsha] += 1
			commit_lines[commit.hexsha] += line_count
	# also todo: keep oldest and newest year for each user
	if len(author_lines) > 1 or author_lines.get('/') == None:
		# Debug output for modified files
		# print(str(author_lines))
		# print(str(commit_lines))
		for author in author_lines:
			if author != "/":
				print(str(author_oldest[author]) + "-" + str(author_newest[author]) + " " + author + ": " + str(author_lines[author]))
		rewrite_cpp(cpp_entry.path, author_oldest, author_newest, author_lines)

def process_tree(tree_entry):
	for blob in tree_entry.blobs:
		# print("Blob: " + blob.path)
		if exclude_paths.get(blob.path) == True:
			continue
		if blob.path.endswith(".cpp") or blob.path.endswith(".h") or blob.path.endswith(".c"):
			process_cpp(blob)
	for tree in tree_entry.trees:
		# print("Tree: " + tree.path)
		if exclude_paths.get(tree.path) == True:
			continue
		process_tree(tree)

def list_historic_tree(tree_entry):
	for blob in tree_entry.blobs:
		historic_paths[blob.path] = True
	for tree in tree_entry.trees:
		list_historic_tree(tree)


# Go
#list_unknown_authors()
#list_large_commits()

print("-- Process files --")
list_historic_tree(historic_commit.tree)
process_tree(tree)

# Lazily sorted large commits
print("-- List large commits --")
for commit in commit_counter:
	count = commit_counter[commit]
	if count > 1000:
		print(commit + ": " + str(count))
for commit in commit_counter:
	count = commit_counter[commit]
	if count > 500 and count < 1000:
		print(commit + ": " + str(count))
for commit in commit_counter:
	count = commit_counter[commit]
	if count > 100 and count < 500:
		print(commit + ": " + str(count))
for commit in commit_counter:
	count = commit_counter[commit]
	if count > 50 and count < 100:
		print(commit + ": " + str(count))

# -- List large commits --
# af454dd1cf9306afe0f074ea2b348a0629112a9e: 4001
# 613d4ddc34ae0a7a16ee6cfb42957c300d0c654e: 267 # Remove throw statements, OK
# e7e51f01e080bfacd87331d16e3e124ba90a04ac: 460 # Merging GUI editor repo, author change todo
# c73f0d944917d8d51860430591450f490ce2635a: 113 # Add max file library, OK
# f605c5faa55148302e7538b801b6fd6b5b646b8a: 152 # Replace clear, OK
# d5c601ffa5690b89f121561bac165b83f39a896f: 148 # Initial version, author change to winch_gate
# c3b3db07f160ac3b1cfd17f249851d139e952f7c: 273 # Memory leaks detection, OK
# b7e3e4b4f0d1b9b64909657d4edbc701bd823947: 121 # Memory leaks detection, OK
# 2fad59f296a07534a7664296a020285e63a538e1: 105 # Memory leaks detection, OK
# adb4963102e226ff8121caa45a802b7d31c25a81: 126 # OVQT, change author to dnk-88
# ffb5b2678bb1028fbc5a92c156bddfdda87f7ae6: 120 # Code cleanup, OK
# 45c8b20f3981eeec3484e53f32ccdc8a372c6063: 60
# 31a540b35d916b2eb5ed9ed66f0f43cb5cc30704: 94
# 4b1959a5600b9e05b7ff2a27d5e564d07efd117d: 57
# 449f0061f86f851e1806d61ed86d17815df7cbfa: 54
# ab454f3d92e03b2721db9c0d687a4d6eb90620c8: 79
# 58c8606d5ddc2b6e3b1e578ad9646a96bbce22a3: 64
# f1cdcd1654e9522e347001a0c1918f36b23b506a: 61
# 0fb9906929f248a09b4998d46b324bb71b390c68: 51
# f5dec3c1caf4ca7f26f4c8c44ecb8aef01461a62: 59
# db31312b11c946a97bd95137ab68ccfd55c3c244: 53
# -- Done --

#print("-- Possible mismatching original author --")

print("-- Source files without header --")
for path in header_not_found:
	print(path)
#code/nel/include/nel/3d/fxaa.h
#code/nel/include/nel/3d/geometry_program.h
#code/nel/include/nel/3d/gpu_program_params.h
#code/nel/include/nel/3d/pixel_program.h
#code/nel/include/nel/3d/program.h
#code/nel/include/nel/3d/render_target_manager.h
#code/nel/include/nel/3d/stereo_debugger.h
#code/nel/include/nel/3d/stereo_display.h
#code/nel/include/nel/3d/stereo_hmd.h
#code/nel/include/nel/3d/stereo_libvr.h
#code/nel/include/nel/3d/stereo_ovr.h
#code/nel/include/nel/3d/stereo_ovr_04.h
#code/nel/include/nel/gui/string_case.h
#code/nel/include/nel/misc/callback.h
#code/nel/include/nel/misc/fast_id_map.h
#code/nel/include/nel/misc/wang_hash.h
#code/nel/include/nel/sound/audio_decoder.h
#code/nel/include/nel/sound/audio_decoder_vorbis.h
#code/nel/include/nel/sound/containers.h
#code/nel/include/nel/sound/group_controller.h
#code/nel/include/nel/sound/group_controller_root.h
#code/nel/include/nel/sound/source_music_channel.h
#code/nel/include/nel/sound/u_group_controller.h
#code/nel/include/nel/sound/decoder/dr_mp3.h
#code/nel/samples/3d/nel_qt/qtcolorpicker_cpp.h
#code/nel/samples/misc/callback/main.cpp
#code/nel/src/3d/fxaa.cpp
#code/nel/src/3d/fxaa_program.h
#code/nel/src/3d/geometry_program.cpp
#code/nel/src/3d/gpu_program_params.cpp
#code/nel/src/3d/pixel_program.cpp
#code/nel/src/3d/program.cpp
#code/nel/src/3d/render_target_manager.cpp
#code/nel/src/3d/stereo_debugger.cpp
#code/nel/src/3d/stereo_display.cpp
#code/nel/src/3d/stereo_hmd.cpp
#code/nel/src/3d/stereo_libvr.cpp
#code/nel/src/3d/stereo_ovr.cpp
#code/nel/src/3d/stereo_ovr_04.cpp
#code/nel/src/3d/stereo_ovr_04_program.h
#code/nel/src/3d/stereo_ovr_fp.cpp
#code/nel/src/3d/driver/direct3d/driver_direct3d_pixel_program.cpp
#code/nel/src/3d/driver/opengl/driver_opengl_pixel_program.cpp
#code/nel/src/3d/driver/opengl/EGL/egl.h
#code/nel/src/3d/driver/opengl/EGL/eglext.h
#code/nel/src/3d/driver/opengl/EGL/eglplatform.h
#code/nel/src/3d/driver/opengl/GLES/egl.h
#code/nel/src/3d/driver/opengl/GLES/gl.h
#code/nel/src/3d/driver/opengl/GLES/glext.h
#code/nel/src/3d/driver/opengl/GLES/glplatform.h
#code/nel/src/3d/driver/opengl/KHR/khrplatform.h
#code/nel/src/3d/shaders/fxaa3_11.h
#code/nel/src/ligo/primitive_utils.cpp
#code/nel/src/misc/config_file/cf_gramatical.cpp
#code/nel/src/misc/config_file/cf_lexical.cpp
#code/nel/src/sound/audio_decoder.cpp
#code/nel/src/sound/audio_decoder_vorbis.cpp
#code/nel/src/sound/group_controller.cpp
#code/nel/src/sound/group_controller_root.cpp
#code/nel/src/sound/source_music_channel.cpp
#code/nel/src/sound/stream_file_sound.cpp
#code/nel/src/sound/stream_file_source.cpp
#code/nel/tools/3d/ligo/plugin_max/ligoscape_utility.h
#code/nel/tools/3d/object_viewer_exe/resource.h
#code/nel/tools/3d/object_viewer_widget/src/entity.cpp
#code/nel/tools/3d/object_viewer_widget/src/entity.h
#code/nel/tools/3d/object_viewer_widget/src/interfaces.h
#code/nel/tools/3d/object_viewer_widget/src/object_viewer_widget.cpp
#code/nel/tools/3d/object_viewer_widget/src/object_viewer_widget.h
#code/nel/tools/3d/object_viewer_widget/src/stdpch.cpp
#code/nel/tools/3d/object_viewer_widget/src/stdpch.h
#code/nel/tools/3d/pipeline_max/class_data.cpp
#code/nel/tools/3d/pipeline_max/class_data.h
#code/nel/tools/3d/pipeline_max/class_directory_3.cpp
#code/nel/tools/3d/pipeline_max/class_directory_3.h
#code/nel/tools/3d/pipeline_max/config.cpp
#code/nel/tools/3d/pipeline_max/config.h
#code/nel/tools/3d/pipeline_max/derived_object.cpp
#code/nel/tools/3d/pipeline_max/derived_object.h
#code/nel/tools/3d/pipeline_max/dll_directory.cpp
#code/nel/tools/3d/pipeline_max/dll_directory.h
#code/nel/tools/3d/pipeline_max/dll_plugin_desc.cpp
#code/nel/tools/3d/pipeline_max/dll_plugin_desc.h
#code/nel/tools/3d/pipeline_max/scene.cpp
#code/nel/tools/3d/pipeline_max/scene.h
#code/nel/tools/3d/pipeline_max/scene_class.cpp
#code/nel/tools/3d/pipeline_max/scene_class.h
#code/nel/tools/3d/pipeline_max/scene_class_registry.cpp
#code/nel/tools/3d/pipeline_max/scene_class_registry.h
#code/nel/tools/3d/pipeline_max/scene_class_unknown.cpp
#code/nel/tools/3d/pipeline_max/scene_class_unknown.h
#code/nel/tools/3d/pipeline_max/storage_array.cpp
#code/nel/tools/3d/pipeline_max/storage_array.h
#code/nel/tools/3d/pipeline_max/storage_chunks.cpp
#code/nel/tools/3d/pipeline_max/storage_chunks.h
#code/nel/tools/3d/pipeline_max/storage_file.cpp
#code/nel/tools/3d/pipeline_max/storage_file.h
#code/nel/tools/3d/pipeline_max/storage_object.cpp
#code/nel/tools/3d/pipeline_max/storage_object.h
#code/nel/tools/3d/pipeline_max/storage_stream.cpp
#code/nel/tools/3d/pipeline_max/storage_stream.h
#code/nel/tools/3d/pipeline_max/storage_value.cpp
#code/nel/tools/3d/pipeline_max/storage_value.h
#code/nel/tools/3d/pipeline_max/super_class_desc.cpp
#code/nel/tools/3d/pipeline_max/super_class_desc.h
#code/nel/tools/3d/pipeline_max/typedefs.cpp
#code/nel/tools/3d/pipeline_max/typedefs.h
#code/nel/tools/3d/pipeline_max/wsm_derived_object.cpp
#code/nel/tools/3d/pipeline_max/wsm_derived_object.h
#code/nel/tools/3d/pipeline_max/builtin/animatable.cpp
#code/nel/tools/3d/pipeline_max/builtin/animatable.h
#code/nel/tools/3d/pipeline_max/builtin/base_object.cpp
#code/nel/tools/3d/pipeline_max/builtin/base_object.h
#code/nel/tools/3d/pipeline_max/builtin/bitmap_tex.cpp
#code/nel/tools/3d/pipeline_max/builtin/bitmap_tex.h
#code/nel/tools/3d/pipeline_max/builtin/builtin.cpp
#code/nel/tools/3d/pipeline_max/builtin/builtin.h
#code/nel/tools/3d/pipeline_max/builtin/editable_patch.cpp
#code/nel/tools/3d/pipeline_max/builtin/editable_patch.h
#code/nel/tools/3d/pipeline_max/builtin/geom_object.cpp
#code/nel/tools/3d/pipeline_max/builtin/geom_object.h
#code/nel/tools/3d/pipeline_max/builtin/i_node.cpp
#code/nel/tools/3d/pipeline_max/builtin/i_node.h
#code/nel/tools/3d/pipeline_max/builtin/modifier.cpp
#code/nel/tools/3d/pipeline_max/builtin/modifier.h
#code/nel/tools/3d/pipeline_max/builtin/mtl.cpp
#code/nel/tools/3d/pipeline_max/builtin/mtl.h
#code/nel/tools/3d/pipeline_max/builtin/mtl_base.cpp
#code/nel/tools/3d/pipeline_max/builtin/mtl_base.h
#code/nel/tools/3d/pipeline_max/builtin/multi_mtl.cpp
#code/nel/tools/3d/pipeline_max/builtin/multi_mtl.h
#code/nel/tools/3d/pipeline_max/builtin/node_impl.cpp
#code/nel/tools/3d/pipeline_max/builtin/node_impl.h
#code/nel/tools/3d/pipeline_max/builtin/object.cpp
#code/nel/tools/3d/pipeline_max/builtin/object.h
#code/nel/tools/3d/pipeline_max/builtin/param_block.cpp
#code/nel/tools/3d/pipeline_max/builtin/param_block.h
#code/nel/tools/3d/pipeline_max/builtin/param_block_2.cpp
#code/nel/tools/3d/pipeline_max/builtin/param_block_2.h
#code/nel/tools/3d/pipeline_max/builtin/patch_object.cpp
#code/nel/tools/3d/pipeline_max/builtin/patch_object.h
#code/nel/tools/3d/pipeline_max/builtin/poly_object.cpp
#code/nel/tools/3d/pipeline_max/builtin/poly_object.h
#code/nel/tools/3d/pipeline_max/builtin/reference_maker.cpp
#code/nel/tools/3d/pipeline_max/builtin/reference_maker.h
#code/nel/tools/3d/pipeline_max/builtin/reference_target.cpp
#code/nel/tools/3d/pipeline_max/builtin/reference_target.h
#code/nel/tools/3d/pipeline_max/builtin/root_node.cpp
#code/nel/tools/3d/pipeline_max/builtin/root_node.h
#code/nel/tools/3d/pipeline_max/builtin/scene_impl.cpp
#code/nel/tools/3d/pipeline_max/builtin/scene_impl.h
#code/nel/tools/3d/pipeline_max/builtin/std_mat.cpp
#code/nel/tools/3d/pipeline_max/builtin/std_mat.h
#code/nel/tools/3d/pipeline_max/builtin/std_mat_2.cpp
#code/nel/tools/3d/pipeline_max/builtin/std_mat_2.h
#code/nel/tools/3d/pipeline_max/builtin/super_class_unknown.cpp
#code/nel/tools/3d/pipeline_max/builtin/super_class_unknown.h
#code/nel/tools/3d/pipeline_max/builtin/texmap.cpp
#code/nel/tools/3d/pipeline_max/builtin/texmap.h
#code/nel/tools/3d/pipeline_max/builtin/track_view_node.cpp
#code/nel/tools/3d/pipeline_max/builtin/track_view_node.h
#code/nel/tools/3d/pipeline_max/builtin/tri_object.cpp
#code/nel/tools/3d/pipeline_max/builtin/tri_object.h
#code/nel/tools/3d/pipeline_max/builtin/storage/app_data.cpp
#code/nel/tools/3d/pipeline_max/builtin/storage/app_data.h
#code/nel/tools/3d/pipeline_max/builtin/storage/geom_buffers.cpp
#code/nel/tools/3d/pipeline_max/builtin/storage/geom_buffers.h
#code/nel/tools/3d/pipeline_max/epoly/editable_poly.cpp
#code/nel/tools/3d/pipeline_max/epoly/editable_poly.h
#code/nel/tools/3d/pipeline_max/epoly/epoly.cpp
#code/nel/tools/3d/pipeline_max/epoly/epoly.h
#code/nel/tools/3d/pipeline_max/update1/editable_mesh.cpp
#code/nel/tools/3d/pipeline_max/update1/editable_mesh.h
#code/nel/tools/3d/pipeline_max/update1/update1.cpp
#code/nel/tools/3d/pipeline_max/update1/update1.h
#code/nel/tools/3d/pipeline_max_dump/class_directory_3_2010.c
#code/nel/tools/3d/pipeline_max_dump/class_directory_3_3.c
#code/nel/tools/3d/pipeline_max_dump/config_2010.c
#code/nel/tools/3d/pipeline_max_dump/config_3.c
#code/nel/tools/3d/pipeline_max_dump/main.cpp
#code/nel/tools/3d/pipeline_max_dump/scene_2010.c
#code/nel/tools/3d/pipeline_max_rewrite_assets/main.cpp
#code/nel/tools/3d/plugin_max/nel_3dsmax_shared/resource.h
#code/nel/tools/3d/plugin_max/nel_export/nel_export_lightmap_v2.cpp
#code/nel/tools/3d/plugin_max/nel_export/resource.h
#code/nel/tools/3d/plugin_max/nel_patch_converter/nel_patch_converter.h
#code/nel/tools/3d/plugin_max/nel_patch_edit/editpat.h
#code/nel/tools/3d/plugin_max/nel_patch_edit/np.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_edit_patch_mod.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_editpops.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_add_patches.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_attach.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_bevel.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_del.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_detach.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_extrude.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_material.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_remember.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_selection.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_subdivide.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_surface.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_epm_tess.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_gui_bind.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_main.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_mods.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/np_rollup.cpp
#code/nel/tools/3d/plugin_max/nel_patch_edit/stdafx.cpp
#code/nel/tools/3d/plugin_max/nel_patch_paint/DllEntry.cpp
#code/nel/tools/3d/plugin_max/nel_patch_paint/nel_patch_paint.h
#code/nel/tools/3d/plugin_max/nel_patch_paint/paint.cpp
#code/nel/tools/3d/plugin_max/nel_patch_paint/paint_main.cpp
#code/nel/tools/3d/plugin_max/nel_patch_paint/paint_pops.cpp
#code/nel/tools/3d/plugin_max/nel_patch_paint/paint_rollup.cpp
#code/nel/tools/3d/plugin_max/nel_patch_paint/paint_ui.cpp
#code/nel/tools/3d/plugin_max/nel_patch_paint/paint_ui.h
#code/nel/tools/3d/plugin_max/nel_patch_paint/paint_undo.cpp
#code/nel/tools/3d/plugin_max/nel_patch_paint/paint_vcolor.cpp
#code/nel/tools/3d/plugin_max/nel_patch_paint/paint_vcolor.h
#code/nel/tools/3d/plugin_max/nel_patch_paint/stdafx.cpp
#code/nel/tools/3d/plugin_max/nel_patch_paint/stdafx.h
#code/nel/tools/3d/plugin_max/nel_vertex_tree_paint/Paint.cpp
#code/nel/tools/3d/plugin_max/nel_vertex_tree_paint/dllmain.cpp
#code/nel/tools/3d/plugin_max/nel_vertex_tree_paint/vertex_tree_paint.cpp
#code/nel/tools/3d/plugin_max/nel_vertex_tree_paint/vertex_tree_paint.h
#code/nel/tools/3d/shared_widgets/command_log.cpp
#code/nel/tools/3d/shared_widgets/command_log.h
#code/nel/tools/3d/shared_widgets/common.h
#code/nel/tools/3d/tga_resize/main.cpp
#code/nel/tools/3d/tile_edit_qt/items_edit_dlg.cpp
#code/nel/tools/3d/tile_edit_qt/main.cpp
#code/nel/tools/3d/tile_edit_qt/tile_widget.cpp
#code/nel/tools/3d/tile_edit_qt/tiles_model.h
#code/nel/tools/3d/unbuild_interface/unbuild_interface.cpp
#code/nel/tools/logic/logic_editor_dll/Condition.cpp
#code/nel/tools/logic/logic_editor_dll/ConditionPage.cpp
#code/nel/tools/logic/logic_editor_dll/ConditionsView.cpp
#code/nel/tools/logic/logic_editor_dll/Counter.cpp
#code/nel/tools/logic/logic_editor_dll/CounterPage.cpp
#code/nel/tools/logic/logic_editor_dll/EditorFormView.cpp
#code/nel/tools/logic/logic_editor_dll/MainFrm.cpp
#code/nel/tools/logic/logic_editor_dll/MainFrm.h
#code/nel/tools/logic/logic_editor_dll/ResizablePage.cpp
#code/nel/tools/logic/logic_editor_dll/State.cpp
#code/nel/tools/logic/logic_editor_dll/StatePage.cpp
#code/nel/tools/logic/logic_editor_dll/StatesView.cpp
#code/nel/tools/logic/logic_editor_dll/StdAfx.h
#code/nel/tools/logic/logic_editor_dll/VariablePage.cpp
#code/nel/tools/logic/logic_editor_dll/logic_editor.cpp
#code/nel/tools/logic/logic_editor_dll/logic_editor.h
#code/nel/tools/logic/logic_editor_dll/logic_editorDoc.cpp
#code/nel/tools/logic/logic_editor_dll/logic_editorDoc.h
#code/nel/tools/logic/logic_editor_exe/logic_editor_exe.cpp
#code/nel/tools/misc/bnp_make_qt/main.cpp
#code/nel/tools/misc/bnp_make_qt/main.h
#code/nel/tools/misc/bnp_make_qt/mainwindow.cpp
#code/nel/tools/misc/bnp_make_qt/mainwindow.h
#code/nel/tools/misc/data_mirror/StdAfx.h
#code/nel/tools/misc/message_box_qt/main.cpp
#code/nel/tools/misc/words_dic_qt/main.cpp
#code/nel/tools/misc/words_dic_qt/words_dicDlg.cpp
#code/nel/tools/sound/source_sounds_builder/SoundPage.cpp
#code/nel/tools/sound/source_sounds_builder/source_sounds_builderDlg.cpp
#code/nelns/login_system/nel_launcher_qt/nel_launcher_dlg.cpp
#code/nelns/login_system/nel_launcher_windows_ext/BarTabsWnd.h
#code/nelns/login_system/nel_launcher_windows_ext/Configuration.cpp
#code/nelns/login_system/nel_launcher_windows_ext/nel_launcher.cpp
#code/ryzom/client/src/interface_v3/lua_ihm_ryzom.h
#code/ryzom/client/src/interface_v3/view_pointer_ryzom.h
#code/ryzom/common/src/game_share/crypt_sha512.cpp
#code/ryzom/common/src/game_share/welcome_service_itf.h
#code/ryzom/tools/client/client_patcher/main.cpp
#code/ryzom/tools/client/ryzom_installer/src/profilesmodel.cpp
#code/ryzom/tools/client/ryzom_installer/src/profilesmodel.h
#code/ryzom/tools/client/ryzom_installer/src/qzip.cpp
#code/ryzom/tools/client/ryzom_installer/src/qzipreader.h
#code/ryzom/tools/client/ryzom_installer/src/qzipwriter.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/completer_line_edit.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/completer_line_edit.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/configuration.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/configuration.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/entity.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/entity.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/filesystem_model.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/filesystem_model.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/formdelegate.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/formdelegate.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/formitem.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/formitem.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georges.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georges.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georges_dirtree_dialog.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georges_dirtree_dialog.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georges_splash.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georges_splash.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georges_treeview_dialog.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georges_treeview_dialog.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georgesform_model.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georgesform_model.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georgesform_proxy_model.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/georgesform_proxy_model.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/log_dialog.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/log_dialog.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/main.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/main_window.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/main_window.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/modules.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/modules.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/new_dialog.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/new_dialog.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/objectviewer_dialog.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/objectviewer_dialog.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/progress_dialog.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/progress_dialog.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/qt_displayer.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/qt_displayer.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/settings_dialog.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/settings_dialog.h
#code/ryzom/tools/leveldesign/georges_editor_qt/src/stdpch.cpp
#code/ryzom/tools/leveldesign/georges_editor_qt/src/stdpch.h
#code/ryzom/tools/leveldesign/mission_compiler_fe/Resource.h
#code/ryzom/tools/leveldesign/world_editor/world_editor/resource.h
#code/ryzom/tools/xml_packer/xml_packer.cpp
#code/studio/src/description.h
#code/studio/src/plugins/core/qtwin.cpp
#code/studio/src/plugins/core/qtwin.h
#code/studio/src/plugins/example/example_plugin.cpp
#code/studio/src/plugins/example/example_plugin.h
#code/studio/src/plugins/gui_editor/action_list.cpp
#code/studio/src/plugins/gui_editor/action_list.h
#code/studio/src/plugins/gui_editor/add_widget_widget.cpp
#code/studio/src/plugins/gui_editor/add_widget_widget.h
#code/studio/src/plugins/log/log_plugin.h
#code/studio/src/plugins/log/qt_displayer.h
#code/studio/src/plugins/mission_compiler/mission_compiler_main_window.cpp
#code/studio/src/plugins/mission_compiler/mission_compiler_main_window.h
#code/studio/src/plugins/mission_compiler/mission_compiler_plugin.cpp
#code/studio/src/plugins/mission_compiler/mission_compiler_plugin.h
#code/studio/src/plugins/mission_compiler/mission_compiler_plugin_constants.h
#code/studio/src/plugins/mission_compiler/validation_file.cpp
#code/studio/src/plugins/mission_compiler/validation_file.h
#code/studio/src/plugins/object_viewer/graphics_viewport.cpp
#code/studio/src/plugins/object_viewer/graphics_viewport.h
#code/studio/src/plugins/object_viewer/main_window.cpp
#code/studio/src/plugins/object_viewer/main_window.h
#code/studio/src/plugins/object_viewer/modules.cpp
#code/studio/src/plugins/object_viewer/modules.h
#code/studio/src/plugins/object_viewer/object_viewer.cpp
#code/studio/src/plugins/object_viewer/object_viewer.h
#code/studio/src/plugins/object_viewer/object_viewer_plugin.cpp
#code/studio/src/plugins/object_viewer/object_viewer_plugin.h
#code/studio/src/plugins/object_viewer/stdpch.cpp
#code/studio/src/plugins/object_viewer/stdpch.h
#code/studio/src/plugins/object_viewer/particle_system/attrib_widget.cpp
#code/studio/src/plugins/object_viewer/particle_system/attrib_widget.h
#code/studio/src/plugins/object_viewer/particle_system/auto_lod_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/auto_lod_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/basic_edit_widget.cpp
#code/studio/src/plugins/object_viewer/particle_system/basic_edit_widget.h
#code/studio/src/plugins/object_viewer/particle_system/bin_op_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/bin_op_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/constraint_mesh_widget.cpp
#code/studio/src/plugins/object_viewer/particle_system/constraint_mesh_widget.h
#code/studio/src/plugins/object_viewer/particle_system/curve_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/curve_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/direction_widget.cpp
#code/studio/src/plugins/object_viewer/particle_system/direction_widget.h
#code/studio/src/plugins/object_viewer/particle_system/emitter_page.cpp
#code/studio/src/plugins/object_viewer/particle_system/emitter_page.h
#code/studio/src/plugins/object_viewer/particle_system/follow_path_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/follow_path_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/located_bindable_page.cpp
#code/studio/src/plugins/object_viewer/particle_system/located_bindable_page.h
#code/studio/src/plugins/object_viewer/particle_system/located_page.cpp
#code/studio/src/plugins/object_viewer/particle_system/located_page.h
#code/studio/src/plugins/object_viewer/particle_system/mesh_widget.cpp
#code/studio/src/plugins/object_viewer/particle_system/mesh_widget.h
#code/studio/src/plugins/object_viewer/particle_system/morph_mesh_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/morph_mesh_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/multi_tex_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/multi_tex_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/particle_control_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_control_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/particle_editor.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_editor.h
#code/studio/src/plugins/object_viewer/particle_system/particle_force_page.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_force_page.h
#code/studio/src/plugins/object_viewer/particle_system/particle_light_page.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_light_page.h
#code/studio/src/plugins/object_viewer/particle_system/particle_link_skeleton_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_link_skeleton_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/particle_property_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_property_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/particle_sound_page.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_sound_page.h
#code/studio/src/plugins/object_viewer/particle_system/particle_system_page.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_system_page.h
#code/studio/src/plugins/object_viewer/particle_system/particle_texture_anim_widget.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_texture_anim_widget.h
#code/studio/src/plugins/object_viewer/particle_system/particle_texture_widget.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_texture_widget.h
#code/studio/src/plugins/object_viewer/particle_system/particle_tree_model.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_tree_model.h
#code/studio/src/plugins/object_viewer/particle_system/particle_workspace_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_workspace_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/particle_workspace_page.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_workspace_page.h
#code/studio/src/plugins/object_viewer/particle_system/particle_zone_page.cpp
#code/studio/src/plugins/object_viewer/particle_system/particle_zone_page.h
#code/studio/src/plugins/object_viewer/particle_system/ps_mover_page.cpp
#code/studio/src/plugins/object_viewer/particle_system/ps_mover_page.h
#code/studio/src/plugins/object_viewer/particle_system/spinner_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/spinner_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/tail_particle_widget.cpp
#code/studio/src/plugins/object_viewer/particle_system/tail_particle_widget.h
#code/studio/src/plugins/object_viewer/particle_system/value_blender_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/value_blender_dialog.h
#code/studio/src/plugins/object_viewer/particle_system/value_from_emitter_dialog.cpp
#code/studio/src/plugins/object_viewer/particle_system/value_from_emitter_dialog.h
#code/studio/src/plugins/object_viewer/scene/animation_dialog.cpp
#code/studio/src/plugins/object_viewer/scene/animation_dialog.h
#code/studio/src/plugins/object_viewer/scene/animation_set_dialog.cpp
#code/studio/src/plugins/object_viewer/scene/animation_set_dialog.h
#code/studio/src/plugins/object_viewer/scene/camera_control.cpp
#code/studio/src/plugins/object_viewer/scene/camera_control.h
#code/studio/src/plugins/object_viewer/scene/day_night_dialog.cpp
#code/studio/src/plugins/object_viewer/scene/day_night_dialog.h
#code/studio/src/plugins/object_viewer/scene/entity.cpp
#code/studio/src/plugins/object_viewer/scene/entity.h
#code/studio/src/plugins/object_viewer/scene/global_wind_dialog.cpp
#code/studio/src/plugins/object_viewer/scene/global_wind_dialog.h
#code/studio/src/plugins/object_viewer/scene/setup_fog_dialog.cpp
#code/studio/src/plugins/object_viewer/scene/setup_fog_dialog.h
#code/studio/src/plugins/object_viewer/scene/skeleton_scale_dialog.cpp
#code/studio/src/plugins/object_viewer/scene/skeleton_scale_dialog.h
#code/studio/src/plugins/object_viewer/scene/skeleton_tree_model.cpp
#code/studio/src/plugins/object_viewer/scene/skeleton_tree_model.h
#code/studio/src/plugins/object_viewer/scene/slot_manager_dialog.cpp
#code/studio/src/plugins/object_viewer/scene/slot_manager_dialog.h
#code/studio/src/plugins/object_viewer/scene/sun_color_dialog.cpp
#code/studio/src/plugins/object_viewer/scene/sun_color_dialog.h
#code/studio/src/plugins/object_viewer/scene/tune_mrm_dialog.cpp
#code/studio/src/plugins/object_viewer/scene/tune_mrm_dialog.h
#code/studio/src/plugins/object_viewer/scene/tune_timer_dialog.cpp
#code/studio/src/plugins/object_viewer/scene/tune_timer_dialog.h
#code/studio/src/plugins/object_viewer/scene/water_pool_dialog.cpp
#code/studio/src/plugins/object_viewer/scene/water_pool_dialog.h
#code/studio/src/plugins/object_viewer/vegetable/vegetable_appearance_page.cpp
#code/studio/src/plugins/object_viewer/vegetable/vegetable_appearance_page.h
#code/studio/src/plugins/object_viewer/vegetable/vegetable_density_page.cpp
#code/studio/src/plugins/object_viewer/vegetable/vegetable_density_page.h
#code/studio/src/plugins/object_viewer/vegetable/vegetable_dialog.cpp
#code/studio/src/plugins/object_viewer/vegetable/vegetable_dialog.h
#code/studio/src/plugins/object_viewer/vegetable/vegetable_editor.cpp
#code/studio/src/plugins/object_viewer/vegetable/vegetable_editor.h
#code/studio/src/plugins/object_viewer/vegetable/vegetable_landscape_page.cpp
#code/studio/src/plugins/object_viewer/vegetable/vegetable_landscape_page.h
#code/studio/src/plugins/object_viewer/vegetable/vegetable_node.cpp
#code/studio/src/plugins/object_viewer/vegetable/vegetable_node.h
#code/studio/src/plugins/object_viewer/vegetable/vegetable_noise_value_widget.cpp
#code/studio/src/plugins/object_viewer/vegetable/vegetable_noise_value_widget.h
#code/studio/src/plugins/object_viewer/vegetable/vegetable_rotate_page.cpp
#code/studio/src/plugins/object_viewer/vegetable/vegetable_rotate_page.h
#code/studio/src/plugins/object_viewer/vegetable/vegetable_scale_page.cpp
#code/studio/src/plugins/object_viewer/vegetable/vegetable_scale_page.h
#code/studio/src/plugins/object_viewer/widgets/color_edit_widget.cpp
#code/studio/src/plugins/object_viewer/widgets/color_edit_widget.h
#code/studio/src/plugins/object_viewer/widgets/edit_range_widget.cpp
#code/studio/src/plugins/object_viewer/widgets/edit_range_widget.h
#code/studio/src/plugins/translation_manager/ftp_selection.h
#code/studio/src/plugins/zone_painter/painter_dock_widget.cpp
#code/studio/src/plugins/zone_painter/painter_dock_widget.h
#code/studio/src/plugins/zone_painter/zone_painter_main_window.cpp
#code/studio/src/plugins/zone_painter/zone_painter_main_window.h
#code/studio/src/plugins/zone_painter/zone_painter_plugin.cpp
#code/studio/src/plugins/zone_painter/zone_painter_plugin.h

print("-- Done --")
