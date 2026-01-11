import socket
import struct
import logging

SOCK_TIMEOUT=1

class CMemStream:
	def __init__(self):
		self.input_stream = False
		self.pos = 0
		self.buffer = b""

	def set_buffer(self, buffer):
		self.input_stream = True
		self.buffer = buffer
		self.pos = 0

	def is_reading(self):
		return self.input_stream

	def serial_uint8(self, val=None):
		if self.is_reading():
			val = self.buffer[self.pos]
			self.pos += 1
			return val
		else:
			self.buffer += struct.pack("B", val & 0xFF)
			self.pos += 1

	def serial_uint32(self, val=None):
		if self.is_reading():
			val = struct.unpack_from("<I", self.buffer, self.pos)[0]
			self.pos += 4
			return val
		else:
			self.buffer += struct.pack("<I", val)
			self.pos += 4

	def serial_string(self, val=None):
		if self.is_reading():
			size = self.serial_uint32()
			val = self.buffer[self.pos:self.pos + size].decode()
			self.pos += size
			return val
		else:
			encoded = val.encode()
			self.serial_uint32(len(encoded))
			self.buffer += encoded
			self.pos += len(encoded)


class CMessage(CMemStream):
	def __init__(self):
		super().__init__()
		self.msg_name = ""

	def set_name(self, name):
		self.msg_name = name


class CCallbackClient:
	def __init__(self):
		self.con_sock = None
		self.msg_num = 0

	def connect(self, addr, port):
		try:
			self.con_sock = socket.create_connection((addr, port), timeout=SOCK_TIMEOUT)
			self.con_sock.settimeout(SOCK_TIMEOUT)
		except Exception as e:
			print(f"Can't connect to the callback server '{addr}:{port}' ({e})")
			return False
		return True

	def close(self):
		if self.con_sock:
			self.con_sock.close()

	def send_message(self, message):
		if not self.con_sock:
			print("No connection")
			return False

		hd = CMemStream()
		hd.serial_uint32(self.msg_num)
		self.msg_num += 1
		hd.serial_uint8(0)
		hd.serial_string(message.msg_name)

		size = hd.pos + message.pos
		buffer = struct.pack(">I", size) + hd.buffer + message.buffer

		try:
			self.con_sock.sendall(buffer)
			return True
		except Exception:
			return False

	def wait_message(self):
		if not self.con_sock:
			return False

		try:
			size_bytes = self.con_sock.recv(4)
			if len(size_bytes) < 4:
				return False
			size = struct.unpack(">I", size_bytes)[0]

			fake = self.con_sock.recv(5)
			size -= 5

			buffer = b""
			while len(buffer) < size:
				chunk = self.con_sock.recv(size - len(buffer))
				if not chunk:
					return False
				buffer += chunk

			msgin = CMemStream()
			msgin.set_buffer(buffer)
			name = msgin.serial_string()

			message = CMessage()
			message.set_buffer(buffer[msgin.pos:])
			message.set_name(name)

			return message
		except socket.timeout:
			return False
