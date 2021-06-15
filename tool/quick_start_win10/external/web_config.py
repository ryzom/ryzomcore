import socket
hostname = socket.gethostname()
with open("web_config.sql", "r") as fr:
	with open("web_config_local.sql", "w") as fw:
		for l in fr:
			fw.write(l.replace("%RC_HOSTNAME%", hostname))
