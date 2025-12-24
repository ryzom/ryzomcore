import os, sys

SHARD_PATH="/home/nevrax/shard/"
DAGS_PATH="/home/data/dagu/dags/"

def getServiceLog(service):
	serviceLogs = {
		"egs" : "entities_game_service",
		"ios" : "input_output_service",
		"gpms" : "gpm_service",
		"aes" : "admin_executor_service",
		"bms_master" : "backup_service",
		"fes" : "frontend_service",
		"las" : "log_analyser_service",
		"lgs" : "logger_service",
		"mos" : "monitor_service",
		"ms" : "mirror_service",
		"mfs" : "mail_forum_service",
		"ras" : "admin_service",
		"rns" : "naming_service",
		"rws" : "welcome_service",
		"sbs" : "session_browser_server",
		"su" : "shard_unifier_service",
		"ts" : "tick_service",
		"rms" : "monitor_service",
	}

	if service[:4] == "ais_" :
		return "ai_service_"+service
	else:
		return serviceLogs[service]

def getPriority(service):
	prios = {
		"egs": "89",
		"ios": "88",
		"gpms": "86",
		"ras": "85",
		"su": "84"
	}
	if service[:4] == "ais_" :
		return "87"
	elif service in prios:
		return prios[service]
	else:
		return "00"

if len(sys.argv) < 2:
    sys.exit(1)

shard = sys.argv[1]

with open("shard.screen.rc") as f:
	lines = f.read().split("\n")
	names = []
	for line in lines:
		if line[:7] == "screen ":
			sline = line.split(" ", 6)
			cmd = sline[6]
			name = sline[5]
			NAME = sline[5].upper()
			desc = sline[6].split(" ")[0].split("/")[-1].replace("_", " ").title()
			log = SHARD_PATH+"logs/"+getServiceLog(sline[5])+".log"
			prio = getPriority(name)
			source = (
f'name: {shard}_{NAME}\n'
f'description: {desc}\n'
f'tags: P_{prio}\n'
f'group: Shard\n'
f'maxActiveRuns: -1\n'
f'env:\n'
f'  - LC_ALL: "C"\n'
f'  - LANGUAGE: ""\n'
f'  - LOG_FILE: "{log}"\n'
f'  - LOG_FORMAT_REGEX: "([0-9\/]+ [0-9:]+) (INF|WRN)?[^:]+: (.*)"\n'
f'  - LOG_FORMAT_FINAL: "[white]\\\\1[/white][\\\\2] \\\\3[/\\\\2]"\n'
"  - LOG_FORMAT_SUBST: \"{'[]': '[bright_white]', '[/]': '[/bright_white]', 'INF': 'bright_yellow', 'WRN': 'deep_pink2'}\"\n"
f'  - STATUS_UPDATER: "Dag-{name}-"\n'
f'steps:\n'
f'  - name: run\n'
f'    dir: {SHARD_PATH}run/\n'
f'    command: python3 {SHARD_PATH}tools/service_launcher.py {name} {cmd}\n'
f'handlerOn:\n'
f'  cancel:\n'
f'    command: echo "Canceled"\n'
f'  failure:\n'
f'    command: bash {SHARD_PATH}tools/start_service.sh {NAME}\n'
f'  success:\n'
f'    command: bash {SHARD_PATH}tools/start_service.sh {NAME}\n'
)
			names.append(NAME)
			with open(shard+"_"+NAME+".yaml", "w") as f:
				f.write(source)
	names.sort()
	for n in names:
		print(n)














