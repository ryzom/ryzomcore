import os

SHARD_PATH="/home/nevrax/shard/"
DAGS_PATH="/home/data/dagu/dags/"
names = []
for ai in ["ark", "fyros", "matis", "tryker", "zorai", "newbieland", "roots"]:
	ai_upper = ai.upper()
	source = (
f'name: Logs_AIS_{ai_upper}\n'
f'description: Check Logs of Ryzom Ai Service\n'
f'group: Services\n'
f'env:\n'
f'  - LC_ALL: "C"\n'
f'  - LANGUAGE: ""\n'
f'  - LOG_FILE: "/home/nevrax/shard/logs/ai_service_ais_{ai}.log"\n'
f'steps:\n'
f'  - name: run\n'
f'    dir: /home/nevrax/shard/tools/Monitoring\n'
f'    command: python3 monitor_ais.py ${{LOG_FILE}}\n'
)
	print(ai)
	with open("Monitor_AIS_"+ai.title()+".yaml", "w") as f:
		f.write(source)















