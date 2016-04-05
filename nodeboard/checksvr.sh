#!/bin/bash
# Make script crontab friendly:
# ==> Change to directory where this script lives.
cd $(dirname $0)

LOGFILE=scheck.log
POORSERVICE=poornode
# SERVICECMD="tail -f cronservcheck.log"
SERVICECMD="nodejs index.js"

# want to redirect own stdout and stderr to logfile
exec >> $LOGFILE

echo `date`
echo "search for $POORSERVICE server process..."

PSOUT=`ps auxww | grep "SCREEN -dmS $POORSERVICE" | grep -v grep`

echo "output of ps is..."
echo $PSOUT

if [ -z "$PSOUT" ]
then
    echo "*** WARNING *** seems not to be running -- starting poor service"
    # do something smart here - launch it in detached screen
    /usr/bin/screen -dmS $POORSERVICE $SERVICECMD
else
    echo "seems to be running"
fi
echo "------------------------------------------------"
