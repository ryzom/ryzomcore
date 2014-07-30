#!/bin/sh

CMD=$1
#DOMAIN=$(pwd|sed s%/home/nevrax/%%)
DOMAIN=shard

if [ "$CMD" = "" ]
then
    echo
    echo Screen sessions currently running:
    screen -list
    echo
    echo "Commands:"
    echo "  'start' to start the shard"
    echo "  'stop'  to stop the ${DOMAIN}"
    echo "  'join'  to join the ${DOMAIN}'s screen session"
    echo "  'share' to join the screen session in shared mode"
    echo "  'state' to view state information for the ${DOMAIN}"
    echo
    printf "Enter a command: "
    read CMD
fi

if [ "$CMD" = "stop" ]
then
    if [ $(screen -list | grep \\\.${DOMAIN} | wc -l) != 1 ]
    then
        echo Cannot stop domain \'${DOMAIN}\' because no screen by that name appears to be running
        screen -list
    else
        screen -d -r $(screen -list | grep \\\.${DOMAIN}| sed 's/(.*)//') -X quit> /dev/null
        rm -v */*.state
        rm -v */*launch_ctrl ./global.launch_ctrl
    fi
fi

STARTARGS=
if [ "$CMD" = "batchstart" ]
then
    STARTARGS='-d -m'
    CMD='start'
fi

if [ "$CMD" = "start" ]
then
    ulimit -c unlimited
    screen -wipe > /dev/null
    if [ $( screen -list | grep \\\.${DOMAIN} | wc -w ) != 0 ]
    then
        echo Cannot start domain \'${DOMAIN}\' because this domain is already started
        screen -list | grep $DOMAIN
    else
        screen $STARTARGS -S ${DOMAIN} -c ${DOMAIN}.screen.rc
    fi

    if [ "$STARTARGS" != "" ]
    then
        # on "batchstart", AES needs to be launched and AES will then launch other services
        printf LAUNCH > aes/aes.launch_ctrl
    fi
fi

if [ "$CMD" = "join" ]
then
    if [ $(screen -list | grep \\\.${DOMAIN} | wc -l) != 1 ]
    then
        echo Cannot join domain \'${DOMAIN}\' because no screen by that name appears to be running
        screen -list
    else
        screen -r $(screen -list | grep \\\.${DOMAIN}| sed 's/(.*)//')
    fi
fi

if [ "$CMD" = "share" ]
then
    if [ $(screen -list | grep \\\.${DOMAIN} | wc -l) != 1 ]
    then
        echo Cannot join domain \'${DOMAIN}\' because no screen by that name appears to be running
        screen -list
    else
        screen -r -x $(screen -list | grep \\\.${DOMAIN}| sed 's/(.*)//')
    fi
fi

if [ "$CMD" = "state" ]
then
    echo State of domain ${DOMAIN}:
    if [ "$(echo */*.state)" = "*/*.state" ]
    then
        echo - No state files found
    else
        grep RUNNING */*state
    fi
fi
