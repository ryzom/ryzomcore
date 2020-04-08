import logging
import socket

SockTimeOut=10

class CMemStream:
	def __init__(self):
		self.InputStream = False
		self.Pos = 0
		self.Buffer = b""

	def setBuffer(self, Buffer):
		self.InputStream = True
		self.Buffer = Buffer
		self.Pos = 0
	

	def isReading(self):
		return self.InputStream

	def serialUInt8(self, val=b""):
		if self.isReading():
			val = ord(self.Buffer[self.Pos])
			self.Pos += 1
			return val
		else:
			self.Buffer += bytes([val & 0xFF])
			self.Pos += 1

	def serialUInt32(self, val=b""):
		if self.isReading():
			val = ord(self.Buffer[self.Pos])
			self.Pos += 1
			val += ord(self.Buffer[self.Pos])*256
			self.Pos += 1
			val += ord(self.Buffer[self.Pos])*256*256
			self.Pos += 1
			val += ord(self.Buffer[self.Pos])*256*256*256
			self.Pos += 1
			return val
		else:
			self.Buffer += bytes([val & 0xFF])
			self.Buffer += bytes([(val>>8) & 0xFF])
			self.Buffer += bytes([(val>>16) & 0xFF])
			self.Buffer += bytes([(val>>24) & 0xFF])
			self.Pos += 4

	def serialString(self, val=b""):
		if self.isReading():
			size = self.serialUInt32()
			val = self.Buffer[self.Pos:self.Pos+size]
			self.Pos += len(val)
			return val
		else:
			try:
				#Convert to bytes if need
				val = val.encode("utf-8")
			except AttributeError:
				pass
			valLen = len(val)
			self.serialUInt32(valLen)
			self.Buffer += val
			self.Pos += valLen

	def serialEnum(self, val=""):
		pass


class CMessage(CMemStream):

	def __init__(self):
		super().__init__()

	def setName(self, name):
		self.MsgName = name


class CCallbackClient():

	ConSock = False
	MsgNum = 0

	def connect(self, addr, port, res):
		global SockTimeOut
		self.MsgNum = 0
		

		self.ConSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		try:
			self.ConSock.connect((addr, port))#, $errno, $errstr, $SockTimeOut)
		except socket.error:
			print("Connection failed...")
			return False

		print("Connection enabled!")
		return True
 
		#TODO: set time out on the socket to 2 secondes
		#stream_set_timeout(self.ConSock, $SockTimeOut)

	def close(self):
		if self.ConSock:
			self.ConSock.close()


	def sendMessage(self, message):
		hd = CMemStream()
		hd.serialUInt32(self.MsgNum) #number the packet
		self.MsgNum += 1
		messageType = 0
		hd.serialUInt8(messageType)
		hd.serialString(message.MsgName)

		size = hd.Pos + message.Pos
		Buffer = chr((size>>24)&0xFF)
		Buffer += chr((size>>16)&0xFF)
		Buffer += chr((size>>8)&0xFF)
		Buffer += chr(size&0xFF)
		Buffer = Buffer.encode("iso-8859-1")
		Buffer += hd.Buffer
		Buffer += message.Buffer

		print(Buffer.decode("utf-8", errors="replace"))
		sent = self.ConSock.send(Buffer)
		if not sent:
			raise RuntimeError("socket connection broken")

	def waitMessage(self):
		size = 0
		val = self.ConSock.recv(1)
		if val:
			size = ord(val) << 24
		else:
			print("Error")
		
		val = self.ConSock.recv(1)
		if val:
			size = ord(val) << 16
		else:
			print("Error")

		val = self.ConSock.recv(1)
		size += ord(val) << 8
		
		val = self.ConSock.recv(1)
		size += ord(val)
		
		fake = self.ConSock.recv(5)
		size -= 5 #remove the fake

		Buffer = ""
		while size > 0 and len(Buffer) != size:
			Buffer += self.ConSock.recv(size - len(Buffer)).decode("utf-8", errors="replace")
				
		msgin = CMemStream()
		msgin.setBuffer(Buffer)

		#decode msg name
		name = ""
		name = msgin.serialString(name)

		logging.info("Message name = '%s'" % name)

		message = CMessage()
		message.setBuffer(msgin.Buffer[msgin.Pos:])
		message.setName(name)

		return message


