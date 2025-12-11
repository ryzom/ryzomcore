import json

class JsonSerde(object):
	def serialize(self, key, value):
		return json.dumps(value).encode("utf-8"), 1

	def deserialize(self, key, value, flags):
		return json.loads(value.decode("utf-8")), 1
