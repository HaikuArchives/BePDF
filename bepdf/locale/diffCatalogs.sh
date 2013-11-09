#!/bin/sh

cd $(dirname "$0")
function every2ndLine() {
displayNext=1
while read line ; do
	if [ $displayNext -eq 1 ] ; then	
		echo $line
		displayNext=0
	else
		displayNext=1
	fi
done
}

if [ "$2" = "" ] ; then
	echo arguments missing!	
	echo $0 Template.catalog OtherLanguage.catalog expected
	exit
fi

every2ndLine < "$1" | sort > /tmp/"$1"
every2ndLine < "$2" | sort > /tmp/"$2"
diff /tmp/"$1" /tmp/"$2"