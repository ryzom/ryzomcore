import socket, os
hostname = socket.gethostname()
root = os.getenv('RC_ROOT').replace('\\', '/')
shardDev = root + '/pipeline/shard_dev'
with open("web_config.sql", "r") as fr:
	with open("web_config_local.sql", "w") as fw:
		for l in fr:
			fw.write(l.replace("%RC_HOSTNAME%", hostname).replace("%RC_SHARD_DEV%", shardDev))
