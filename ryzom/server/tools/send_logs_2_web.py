#!/usr/bin/python

import os, time
import collections
import urllib
import urllib2

class SlackClient(object):

    BASE_URL = 'https://ryzom.slack.com/api'

    def __init__(self, token):
        self.token = token

    def _make_request(self, method, params):
        """Make request to API endpoint

        Note: Ignoring SSL cert validation due to intermittent failures
        http://requests.readthedocs.org/en/latest/user/advanced/#ssl-cert-verification
        """
        url = "%s/%s" % (SlackClient.BASE_URL, method)
        params['token'] = self.token
        data = urllib.urlencode(params)
        #result = requests.post(url, data=params, verify=False).json()
        req = urllib2.Request(url, data)
        result = urllib2.urlopen(req)
        return result.read()

    def chat_post_message(self, channel, text, username=None, icon=':rage2:', parse=None, link_names=None):
        """chat.postMessage

        This method posts a message to a channel.

        https://api.slack.com/methods/chat.postMessage
        """
        method = 'chat.postMessage'
        params = {
            'channel': channel,
            'text': text,
            'icon_emoji': icon,
        }
        if username is not None:
            params['username'] = username
        if parse is not None:
            params['parse'] = parse
        if link_names is not None:
            params['link_names'] = link_names
        return self._make_request(method, params)



critic_commands = ("renameplayer", "position", "exchange item", "hp", "createiteminbag","createitemintmpinv","createnamediteminbag","createfullarmorset","aggro","god","money","invulnerable","changevar","addxptoskill","learnallbricks","learnallforagephrases","learnallphrases","learnbrick","learnphrase","summonpet","summon","allowsummonpet", "setfameplayer")

#2013/05/17 03:34:51 INF 3073373952 newshard.ryzom.com/EGS-139 admin.cpp 3160 cbClientAdmin : ADMIN: Player ((0x0000a70890:00:00:89),Placio(atys)) will execute client admin command 'lockItem (0x0000a70890:00:00:89) bag 28 1' on target Himself
#2013/05/17 21:25:02 INF 3073373952 newshard.ryzom.com/EGS-139 admin.cpp 3278 cbClientAdminOffline : ADMINOFFLINE: Player ((0x00006dd281:00:00:89),Kemen(atys)) will execute client admin command 'addXPToSkill (0x00006dd281:00:00:00) 500 SMOEAEM 5' on target ((0x00006dd281:00:00:00),kemen(atys))
#2014/11/02 01:30:19 INF 4146730752 newshard.ryzom.com/EGS-132 character.cpp 10983 acceptExchange : ADMIN: CSR ((0x00007ac5b0:00:00:8b),Tamarea(Atys)) exchange ic_candy_stick.sitem Q1 with Aleeskandaro(Atys)
admin_commands = {}
global_commands = {}
new_sorbot_commands = {}

#data = urllib.urlencode({"t" : ".t ulukyn Running"})
#req = urllib2.Request('http://app.ryzom.com/sorbot/index.php?'+data)
#r = urllib2.urlopen(req)

def sorbot_old(line, ctype, name, command, command_args):
	global new_sorbot_commands
	new_sorbot_commands.append(line)
	if not line in sorbot_commands:
		data = urllib.urlencode({"t" : ".t ulukyn Player ["+name+"] using '/"+ctype+" "+command+" "+command_args+"' on target: "+line.split("' on target ")[-1][25:-7]+" http://app.ryzom.com/app_admin/player_stats.php?char_name="+name+"&stat=commands"})
		req = urllib2.Request('http://app.ryzom.com/sorbot/index.php?'+data)
		r = urllib2.urlopen(req)

def sorbot(line, ctype, name, command, command_args, target):
	if (name == 'Moondev'):
		return

	global new_sorbot_commands, sorbot_commands
	new_sorbot_commands.append(line)
	if (target == "" or target[:2] == 'on'):
		if ctype != "a" or command.lower() != "position":
			message = '<http://app.ryzom.com/app_admin/player_stats.php?char_name='+name+'&stat=commands|'+name+'> using \'/'+ctype+' '+command+' '+command_args+'\' '+target
		else:
			message = ""
		icon = ':suspect:'
	else:
		if ctype == " ":
			message = '<http://app.ryzom.com/app_admin/player_stats.php?char_name='+name+'&stat=commands|'+name+'> '+command+' '+command_args+'\' with <http://app.ryzom.com/app_admin/player_stats.php?char_name='+target+'|'+target+'>'
		elif ctype != "a" or command.lower() != "position":
			message = '<http://app.ryzom.com/app_admin/player_stats.php?char_name='+name+'&stat=commands|'+name+'> using \'/'+ctype+' '+command+' '+command_args+'\' on <http://app.ryzom.com/app_admin/player_stats.php?char_name='+target+'|'+target+'>'
		else:
			message = ""
		icon = ':rage2:'
	print target, message
	if message:
		if not line in sorbot_commands:
			print "SLACKIT!"
			client = SlackClient('xoxb-56970050663-z8Q1WyKLPh1m2K4iRMR4YsPI')
			print "RESPONSE:", client.chat_post_message('command-logs', line[:20]+message, 'EGS', icon)
			print "SLACK: ", line[:20]+message
		else:
			print "USED : ", line
	else:
		print "----"


def writelogs(admin_commands, is_current=False, base_path="logged/"):
	global global_commands
	base = "logs/commands/"+base_path
	for player, commands in admin_commands.items():
		s_player = str(player)
		if len(s_player) < 2:
			s_player = "0"+s_player
		if len(s_player) < 3:
			s_player = "0"+s_player

		fid = s_player[-1]
		if not os.path.isdir(base):
			os.mkdir(base)
		if not os.path.isdir(base+fid):
			os.mkdir(base+fid)
		fid2 = s_player[-2]
		if not os.path.isdir(base+fid+"/"+fid2):
			os.mkdir(base+fid+"/"+fid2)
		fid3 = s_player[-3]
		if not os.path.isdir(base+fid+"/"+fid2+"/"+fid3):
			os.mkdir(base+fid+"/"+fid2+"/"+fid3)
		if not os.path.isdir(base+fid+"/"+fid2+"/"+fid3+"/"+str(player)):
			os.mkdir(base+fid+"/"+fid2+"/"+fid3+"/"+str(player))
		for mouth, command in commands.items():
			if mouth != "name" and mouth != "total":
				if is_current:
					open(base+fid+"/"+fid2+"/"+fid3+"/"+str(player)+"/current.log", "w").write(command+"\n")
				else:
					if os.path.isfile(base+fid+"/"+fid2+"/"+fid3+"/"+str(player)+"/"+mouth.replace("/", "_")+".log"):
						previous = open(base+fid+"/"+fid2+"/"+fid3+"/"+str(player)+"/"+mouth.replace("/", "_")+".log").read()
					else:
						previous = ""
					open(base+fid+"/"+fid2+"/"+fid3+"/"+str(player)+"/"+mouth.replace("/", "_")+".log", "w").write(command+"\n"+previous)

	base = "logs/commands/"
	for mouth, command in global_commands.items():
		if not os.path.isdir(base+"global"):
				os.mkdir(base+"global")
		if is_current:
			open(base+"global/current.log", "w").write(command+"\n")
		else:
			if os.path.isfile(base+"global/"+mouth.replace("/", "_")+".log"):
				previous = open(base+"global/"+mouth.replace("/", "_")+".log").read()
			else:
				previsous = ""
			open(base+"global/"+mouth.replace("/", "_")+".log", "w").write(command+"\n"+previous)

def parse(filename):
	global global_commands, admin_commands, sorbot_commands, new_sorbot_commands
	file = open(filename).read().split("\n")
	last_commands = {}
	for line in file:
		sline = line.split(" ")
#		if len(sline) >= 10 and ((sline[9] == "ADMIN:" and sline[10] == "Player") or sline[9] == "ADMINOFFLINE:"):
		if "cbClientAdmin : ADMIN: " in line or "ADMINOFFLINE" in line or "ADMIN: CSR" in line :
			if "tried to execute a no valid client admin command" in line :
				continue
			sline = line.split("' on target ")
			#Only get text before "on target"
			real_line = sline[0]
			sline = real_line.split(" ")
			day = sline[0]
			mouth = day[:4]+day[5:7]
			hour = sline[1]
			player = sline[11]
			splayer = player.split(",")
			if splayer[0][4:-10] != '':
				cid = int(splayer[0][4:-10], 16)
			else:
				cid = 0
			if (len(splayer) == 2):
				name = splayer[1][:-7]
			else:
				name = "<undefined>"


			if "ADMIN: CSR" in line :
				print "Exchange :", sline
				command = "exchange item"
				command_args = sline[13]+" "+sline[14]
				target  = sline[16]
				target_name = sline[16].split("(")[0]
				ctype = " "
			else:
				#Get text between ' and '
				sline = real_line.split("'")
				full_command = "'".join(sline[1:])

				if full_command[-1] == "'":
					full_command = full_command[:-1]
				scommand = full_command.split(" ")
				command = scommand[0]
				command_args = " ".join(scommand[2:])

				if (len(scommand) > 1):
					target =  scommand[1]
				else:
					target = "<none>"
					target_name = ""
				ctype = "c"
				if target == "(0x0000000000:ff:00:00)":
					target = "<none>"
					target_name = "on Nothing"
				elif target[:-6] == "(0x"+splayer[0][4:-10]+":00:":
					target = "<self>"
					target_name = ""
					ctype = "a"
				elif target[-6:-4] != "00" :
					target = "<mob_npc>"
					target_name = "on a Mob or Npc"
					ctype = "b"
				elif len(target) > 20  :
					target = "#"+str(int(target[4:-10], 16))
					target_name = line.split(",")[-1].split("(")[0].capitalize()
					ctype = "b"
				else:
					ctype = "a"
				
				if line[100:120] == "ADMINOFFLINE: Player":
					ctype = "c"
					sorbot(line, ctype, name, command, command_args, target_name)

			if not cid in admin_commands:
				admin_commands[cid] = {}
				admin_commands[cid]["total"] = 0
				admin_commands[cid]["name"] = name
			if not mouth in admin_commands[cid]:
				admin_commands[cid][mouth] = ""
			admin_commands[cid][mouth] = "|".join((day, hour, ctype, command, target, command_args))+"\n" + admin_commands[cid][mouth]
			admin_commands[cid]["total"] += 1
			
#			print command.lower()
			if command.lower() in critic_commands:
				print command, command_args
				if not name in last_commands or not command.lower() in ("addxptoskill","learnbrick","learnphrase"):
					print "Setting..."
					last_commands[name] = [None, None, None, None, None]
					last_commands[name][0] = command
					last_commands[name][1] = 1
					last_commands[name][2] = [line, ctype, name, command, command_args, target_name]
					last_commands[name][3] = "|".join((str(cid), name, day, hour, ctype, command, target, command_args))
					last_commands[name][4] = mouth

				if command.lower() in ("addxptoskill","learnbrick","learnphrase") and command == last_commands[name][0]:
					print "+1"
					last_commands[name][1] += 1
					last_commands[name][2] = [line, ctype, name, command, command_args, target_name]
					last_commands[name][3] = "|".join((str(cid), name, day, hour, ctype, command, target, command_args))
					last_commands[name][4] = mouth
				else :
					if last_commands[name][1] > 1 :
						print "More than 1 time"
						last_commands[name][2][4] = "*"+str(last_commands[name][1])+" times*"
						last_commands[name][2][0] += "*"+str(last_commands[name][1])+" times*"
					else:
						last_commands[name][2][4] = command_args
					sorbot(last_commands[name][2][0], last_commands[name][2][1], last_commands[name][2][2], last_commands[name][2][3], last_commands[name][2][4], last_commands[name][2][5])
					print "Sorbot: ", last_commands[name][2], last_commands[name][1]
					if not last_commands[name][4] in global_commands:
						global_commands[last_commands[name][4]] = ""
					global_commands[last_commands[name][4]] = last_commands[name][3]+"\n"+global_commands[last_commands[name][4]]
					del(last_commands[name])


	for name, last_command in last_commands.items():
		if last_command[1] > 1:
			last_command[2][4] += " ["+str(last_command[1])+" times the same command]"
			#last_command[2][0] += "*"+str(last_command[1])+" times*"
			sorbot(last_command[2][0], last_command[2][1], last_command[2][2], last_command[2][3], last_command[2][4], last_command[2][5])
			print "Sorbot  ", last_command[2], last_command[1]
			if not last_command[4] in global_commands:
				global_commands[last_command[4]] = ""
			global_commands[mouth] = last_command[3]+"\n"+global_commands[last_command[4]]


BASE_PATH = "logs/commands/"

if not os.path.isdir(BASE_PATH):
	os.mkdir(BASE_PATH)

if not os.path.isdir(BASE_PATH+"backup"):
	os.mkdir(BASE_PATH+"backup")

if not os.path.isdir(BASE_PATH+"global"):
	os.mkdir(BASE_PATH+"global")


if not os.path.isfile(BASE_PATH+"last_log"):
	open(BASE_PATH+"last_log", "w").write("0")

if not os.path.isfile("cache/commands"):
	open("cache/commands", "w").write("")


last_log = int(open(BASE_PATH+"last_log").read())
sorbot_commands = []
sorbot_commands = open("cache/commands").read().split("\n")
new_sorbot_commands = []

print "Last Log with date : ", last_log
files =  os.listdir("logs/")
files.sort()
higher_time = last_log
for filename in files:
	if len(filename) > 25 and filename.split(".")[1] == "log" and filename.split(".")[0][:21] == "entities_game_service":
		if int(os.path.getmtime("logs/"+filename)) > last_log:
			if int(os.path.getmtime("logs/"+filename)) > higher_time:
				higher_time = os.path.getmtime("logs/"+filename)
			print filename, " NEED PARSING"
			parse("logs/"+filename)
				
logged_players = len(admin_commands)

print "Write logs..."
writelogs(admin_commands, False)

admin_commands = {}
global_commands = {}
filename = "entities_game_service.log"
print filename, " <NEED PARSING!>"
parse("logs/"+filename)
logged_players += len(admin_commands)
open(BASE_PATH+"global/current.log", "w").write("")
writelogs(admin_commands, True, "current/")

open(BASE_PATH+"last_log", "w").write(str(int(higher_time)))
open("cache/commands", 'w').write("\n".join(new_sorbot_commands))


print "Send logs to web..."
os.system("scp -r "+BASE_PATH+"logged/ nevrax@newweb.ryzom.com:/mnt/tmp/cache/ > /dev/null")
print "Backup..."
os.system("cp -r "+BASE_PATH+"logged/* "+BASE_PATH+"backup")
print "Clean..."
os.system("rm -rf "+BASE_PATH+"logged/*")
print "Send current..."
#os.system("scp -r current/* nevrax@newweb.ryzom.com:/mnt/tmp/cache/logged/ > /dev/null")
print "Done!"

#os.system("rsync -avz -e ssh logged/ nevrax@web.ryzom.com:/mnt/tmp/cache/logged/")
