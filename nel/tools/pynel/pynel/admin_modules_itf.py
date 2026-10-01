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

import logging
from pynel.nel_message import CCallbackClient, CMessage


class CAdminServiceWeb(CCallbackClient):
	def global_cmd(self, command):
		msg = CMessage()
		msg.set_name("GCMD")
		msg.serial_string(command)
		return self.send_message(msg)

	def control_cmd(self, service_alias, command):
		msg = CMessage()
		msg.set_name("CCMD")
		msg.serial_string(service_alias)
		msg.serial_string(command)
		return self.send_message(msg)

	def service_cmd(self, service_alias, command):
		msg = CMessage()
		msg.set_name("SCMD")
		msg.serial_string(service_alias)
		msg.serial_string(command)
		return self.send_message(msg)

	def get_shard_orders(self):
		msg = CMessage()
		msg.set_name("GSO")
		self.send_message(msg)
		ret_msg = self.wait_message()
		if not ret_msg:
			print("get_shard_orders: Error in 'wait_message'")
			return False

		if ret_msg.msg_name != "R_GSO":
			print(f"get_shard_orders: Invalid response, awaited 'R_GSO', received: '{ret_msg.msg_name}'")
			return False

		nb_elem = ret_msg.serial_uint32()
		return [ret_msg.serial_string() for _ in range(nb_elem)]

	def get_states(self):
		msg = CMessage()
		msg.set_name("GS")
		self.send_message(msg)
		ret_msg = self.wait_message()
		if not ret_msg:
			print("get_states: Error in 'wait_message'")
			return False

		if ret_msg.msg_name != "R_GS":
			print(f"get_states: Invalid response, awaited 'R_GS', received: '{ret_msg.msg_name}'")
			return False

		nb_elem = ret_msg.serial_uint32()
		return [ret_msg.serial_string() for _ in range(nb_elem)]

	def wait_callback(self):
		message = self.wait_message()
		if not message:
			return False

		if message.msg_name == "CMDR":
			self.command_result_skel(message)
			return True
		return False

	def command_result_skel(self, message):
		service_alias = message.serial_string()
		result = message.serial_string()
		self.command_result(service_alias, result)

	def command_result(self, service_alias, result):
		self.command_return_data = result


def query_shard(service_name, full_cmd, wait_callback=True, is_control=False):
	nel_result = ""
	nel_status = not wait_callback
	res = ""

	admin_service = CAdminServiceWeb()

	if admin_service.connect("127.0.0.1", 46700):
		if isinstance(full_cmd, str):
			if is_control:
				admin_service.control_cmd(service_name, full_cmd)
			else:
				admin_service.service_cmd(service_name, full_cmd)

			if wait_callback and admin_service.wait_callback():
				nel_status = True
				nel_result += admin_service.command_return_data
		else:
			for service_command in full_cmd:
				if is_control:
					admin_service.control_cmd(service_name, service_command)
				else:
					admin_service.service_cmd(service_name, service_command)
		admin_service.close()

	return {
		"status": nel_status,
		"query": f"{service_name}:{full_cmd}",
		"raw": nel_result.split("\n")[1:]
	}
