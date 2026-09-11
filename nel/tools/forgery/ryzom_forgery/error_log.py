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

"""Every Forgery app/dialog keeps its own `self._x_error = "..."` string
field, shown only via `imgui.text_colored()` -- easy to miss since it only
exists inside whichever tab/panel happens to be visible. `report_error()` is
called alongside each such assignment (project-todos/forgery/
error_stderr_logging.md) so the same message also always reaches the
terminal. Deliberately just `print(..., file=sys.stderr)` -- no `logging`
module, no timestamp/app-name prefix (out of scope for now, see the
chantier's own scope notes)."""

import sys


def report_error(message: str) -> None:
	print(message, file=sys.stderr)
