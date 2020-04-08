import logging
import socket

from nel_message import *


class CAdminServiceWeb(CCallbackClient):

	def globalCmd(self, command):
		msg = CMessage()
		msg.setName("GCMD")
		msg.serialString(command)
		return self.sendMessage(msg)

	def controlCmd(self, serviceAlias, command):
		msg = CMessage()
		msg.setName("CCMD")

		msg.serialString(serviceAlias)
		msg.serialString(command)

		return self.sendMessage(msg)


	def serviceCmd(self, serviceAlias, command):
		msg = CMessage()
		msg.setName("SCMD")

		msg.serialString(serviceAlias)
		msg.serialString(command)

		return self.sendMessage(msg)

	def getShardOrders(self):
		msg = CMessage()
		msg.setName("GSO")

		ret = ""
		ret = sendMessage(msg)
		if not ret:
			print("getShardOrders: Error in 'sendMessage'")
			return False

		retMsg = waitMessage()
		if not retMsg:
			print("getShardOrders: Error in 'waitMessage'")
			return False

		if not retMsg.MsgName == "R_GSO":
			print("getShardOrders: Invalid response, awaited 'R_GSO', received: "+retMsg.MsgName)
			return False

		nbElem = 0
		retMsg.serialUInt32(nbElem)
		retValue = []
		for i in range(nbElem):
			retMsg.serialString(item)
			retValue.append(item)

		return retValue


	def waitCallback(self):
		message = self.waitMessage()

		if not message:
			return False

		if message.MsgName == "CMDR":
			self.commandResult_skel(message)
		else:
			return False
		return True

	def commandResult_skel(self, message):
		serviceAlias = message.serialString()
		result = message.serialString()
		self.commandResult(serviceAlias, result)

	def commandResult(self, serviceAlias, result):
		global command_return_data
		command_return_data = result



def queryShard(service_name, fullcmd, waitCallback=True, is_control=False):
	global command_return_data

	nel_result = ""
	nel_status = not waitCallback
	res = ""
	p_result = None

	adminService = CAdminServiceWeb()

	if adminService.connect("yubo.ryzom.com", 46700, res):
		command_return_data = ""

		if isinstance(fullcmd, str):
			if is_control:
				adminService.controlCmd(service_name, fullcmd)
			else:
				adminService.serviceCmd(service_name, fullcmd)
			service_command = fullcmd

			if waitCallback and adminService.waitCallback():
				nel_status = True
				nel_result += command_return_data
		else:
			for service_command in fullcmd:
				if is_control:
					adminService.controlCmd(service_name, service_command)
				else:
					adminService.serviceCmd(service_name, service_command)
		adminService.close()

	return {"status": nel_status, "query": service_name+":"+fullcmd, "raw": nel_result.split("\n")[1:]}

def serveShard():
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.bind(('', 15555))

	adminService = CAdminServiceWeb()
	res = ""
	i = 1
	if adminService.connect("yubo.ryzom.com", 46700, res):
		print("Connected")
		while True:
				sock.listen(5)
				client, address = sock.accept()
				response = client.recv(255)
				if response != "":
					print(i, response)
					i = i + 1
					adminService.serviceCmd("egs", response)
					if adminService.waitCallback():
						pass

		print("Close")
		client.close()
		sock.close()

	
