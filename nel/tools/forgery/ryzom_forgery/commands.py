from dataclasses import dataclass
from typing import Callable


@dataclass
class Command:
	label: str
	callback: Callable[[list], None]
	# False: never listed in the context menu, but still the extension's
	# click action (see CommandRegistry.click_command_for_selection()).
	in_menu: bool = True


class CommandRegistry:
	"""Registry of actions the standard explorer's context menu can run on
	a file selection: global commands (offered for any selection) and
	per-extension commands (only offered when every selected file shares
	that single extension).

	Selection items just need a `.suffix` attribute (e.g. `pathlib.Path` or
	`ryzom_forgery.explorer.ExplorerItem`).
	"""

	def __init__(self):
		self._global_commands: list[Command] = []
		self._commands_by_extension: dict[str, list[Command]] = {}

	def register_global(self, label, callback):
		self._global_commands.append(Command(label, callback))

	def register_for_extension(self, extension, label, callback, in_menu=True):
		extension = extension.lower()
		self._commands_by_extension.setdefault(extension, []).append(Command(label, callback, in_menu))

	def _extension_commands_for_selection(self, items: list) -> list[Command]:
		extensions = {item.suffix.lower() for item in items if item.suffix}
		if len(extensions) == 1:
			return list(self._commands_by_extension.get(next(iter(extensions)), []))
		return []

	def commands_for_selection(self, items: list) -> list[Command]:
		"""Commands listed in the context menu (global first, then the
		selection's extension ones flagged `in_menu`)."""
		commands = list(self._global_commands)
		commands.extend(c for c in self._extension_commands_for_selection(items) if c.in_menu)
		return commands

	def click_command_for_selection(self, items: list):
		"""What a plain left-click on `items` runs: the first command
		registered for their extension (menu-listed or not), never a global
		one -- or None."""
		commands = self._extension_commands_for_selection(items)
		return commands[0] if commands else None
