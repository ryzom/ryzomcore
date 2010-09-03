#!/usr/bin/python
# 
# \file projects.py
# \brief Projects configuration
# \date 2010-05-24-09-19-GMT
# \author Jan Boon (Kaetemi)
# Python port of game data build pipeline.
# Projects configuration.
# 
# NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
# Copyright (C) 2010  Winch Gate Property Limited
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
# 


ProjectsToProcess = [ ]

# Common projects
ProjectsToProcess += [ "common/fonts" ]
ProjectsToProcess += [ "common/interface" ]
ProjectsToProcess += [ "common/objects" ]
ProjectsToProcess += [ "common/sfx" ]
ProjectsToProcess += [ "common/fauna" ]

# Ecosystem projects
ProjectsToProcess += [ "ecosystems/desert" ]
ProjectsToProcess += [ "ecosystems/jungle" ]
ProjectsToProcess += [ "ecosystems/primes_racines" ]
ProjectsToProcess += [ "ecosystems/lacustre" ]

# Continent projects
ProjectsToProcess += [ "continents/newbieland" ]

# Common projects depending on continent projects
ProjectsToProcess += [ "common/construction" ] # Depends on jungle/newbieland due to ig_light tool usage of properties.cfg...
ProjectsToProcess += [ "common/outgame" ] # Depends on jungle/newbieland due to ig_light tool usage of properties.cfg...
ProjectsToProcess += [ "common/sky" ] # Depends on jungle/newbieland due to ig_light tool usage of properties.cfg...

# TODO
#ProjectsToProcess += [ "common/characters" ] # TODO
#ProjectsToProcess += [ "common/characters_maps_hr" ] # TODO
#ProjectsToProcess += [ "common/characters_maps_lr" ] # TODO
#ProjectsToProcess += [ "continents/indoors" ] # TODO


# end of file
