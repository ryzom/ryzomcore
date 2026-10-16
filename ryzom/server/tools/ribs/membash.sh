#!/bin/bash

# Gist: 11375877
# Url: https://gist.github.com/goodevilgenius/11375877
#
# All memcache functions are supported.
# 
# Can also be sourced from other scripts, e.g.
#    source membash.sh
#    MCSERVER="localhost"
#    MCPORT=11211
#    foobar=$(mc_get foobar)
#    [ -z "$foobar" ] && foobar="default value"
#    mc_set foobar 0 "$foobar"

# original author: wumin, https://gist.github.com/ri0day/1538831
# updated by goodevilgenius to support debian-based systems, support more
# functions, and be more user-friendly 

MCSERVER="localhost"
MCPORT=11211

mc_usage() {
	format_usage="membash: a memcache library for BASH \n\
https://gist.github.com/goodevilgenius/11375877\n\n\
Usage:\n
	\t $(basename "$0") [-hp] command [arguments] \n \
	\t [-h]\t memcached hostname or ip. \n \
	\t [-p]\t memcached port. \n\n\
Commands: \n \
	\t usage (print this help) \n \
	\t set/add/replace/append/prepend key exptime value \n \
	\t touch key exptime \n \
	\t incr/decr key value \n \
	\t get key \n \
	\t delete key [time] \n \
	\t stats \n \
	\t list_all_keys"
	echo -e $format_usage
}
mc_help() { mc_usage;}

mc_sendmsg() { echo -ne "$*\r\n" | nc -q 0 $MCSERVER $MCPORT;}

mc_stats() { mc_sendmsg "stats";}

mc_get_last_items_id() {
	LastID=$(mc_sendmsg "stats items"|tail -n 2|head -n 1|awk -F':' '{print $2}')
	echo $LastID
}

mc_list_all_keys() {
	:>/dev/shm/mc_all_keys_${MCSERVER}_${MCPORT}.txt
	max_item_num=$(mc_get_last_items_id)
	for i in `seq 1 $max_item_num`
	do
		mc_sendmsg "stats cachedump $i 0" | awk '{print $2}'
	done >>/dev/shm/mc_all_keys_${MCSERVER}_${MCPORT}.txt 
	sed -i '/^$/d' /dev/shm/mc_all_keys_${MCSERVER}_${MCPORT}.txt
	cat /dev/shm/mc_all_keys_${MCSERVER}_${MCPORT}.txt
}

mc_get() { mc_sendmsg "get $1" | awk "/^VALUE $1/{a=1;next}/^END/{a=0}a" ;}

mc_touch() {
	key="$1"
	shift
	let exptime="$1"
	shift
	mc_sendmsg "touch $key $exptime"
}

mc_doset() {
	command="$1"
	shift
	key="$1"
	shift
	let exptime="$1"
	shift
	val="$*"
	let bytes=$(echo -n "$val"|wc -c)
	mc_sendmsg "$command $key 0 $exptime $bytes\r\n$val"
}

mc_set() { mc_doset set "$@";}
mc_add() { mc_doset add "$@";}
mc_replace() { mc_doset replace "$@";}
mc_append() { mc_doset append "$@";}
mc_prepend() { mc_doset prepend "$@";}

mc_delete() { mc_sendmsg delete "$*";}
mc_incr() { mc_sendmsg incr "$*";}
mc_decr() { mc_sendmsg decr "$*";}

mc_superpurge() {
	mc_list_all_keys > /dev/null
	if [ ! -z "/dev/shm/mc_all_keys_${MCSERVER}_${MCPORT}.txt" ];then
		grep "$1" /dev/shm/mc_all_keys_${MCSERVER}_${MCPORT}.txt >/dev/shm/temp.swap.${MCSERVER}_${MCPORT}.txt
	fi
	while read keys 
	do
		mc_sendmsg "delete ${keys}"
	done </dev/shm/temp.swap.${MCSERVER}_${MCPORT}.txt

	rm -rf /dev/shm/temp.swap.${MCSERVER}_${MCPORT}.txt
}

if [ "$(basename "$0" .sh)" = "membash" ]
then

	while getopts "h:p:" flag
	do
		case $flag in
			h)
				MCSERVER=${OPTARG:="localhost"}
				;;
			p)
				MCPORT=${OPTARG:="11211"}
				;;
			\?)
				echo "Invalid option: $OPTARG" >&2
				;;
		esac
	done
	command="${@:$OPTIND:1}"
	[ -z "$command" ] && command="usage"
	let OPTIND++

	mc_$command "${@:$OPTIND}"

	exit $?

fi
