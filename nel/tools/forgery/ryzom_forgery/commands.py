from dataclasses import dataclass
from typing import Callable


@dataclass
class Command:
	label: str
	callback: Callable[[list], None]


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

	def register_for_extension(self, extension, label, callback):
		extension = extension.lower()
		self._commands_by_extension.setdefault(extension, []).append(Command(label, callback))

	def commands_for_selection(self, items: list) -> list[Command]:
		commands = list(self._global_commands)

		extensions = {item.suffix.lower() for item in items if item.suffix}
		if len(extensions) == 1:
			extension = next(iter(extensions))
			commands.extend(self._commands_by_extension.get(extension, []))

		return commands
