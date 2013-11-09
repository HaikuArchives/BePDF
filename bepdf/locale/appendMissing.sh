#!/bin/sh

cd $(dirname "$0")
function dupLines() {
while read line ; do
	echo "$line"
	echo "$line"
done
}

if [ "$1" = "-a" ] ; then
	shift
	for file in *.catalog ; do
		appendMissing.sh $* "$file"
	done
	exit
fi

testOnly=0
if [ "$1" = "-t" ] ; then
	testOnly=1
	shift
fi

if [ "$1" = "" ] ; then
	echo arguments missing!	
	echo $0 catalog name expected
	echo $0 "(-a [-t]) | ([-t] file)"
	echo "Arguments:"
	echo "   -a process all *.catalog files"
	echo "   -t do not merge differences in catalog instead create a new file file.missing"
	exit
fi

catalog="$1"
temp=missing.temp

./diffCatalogs.sh Template.catalog "$catalog" | grep "<" | cut -c3- | dupLines > $temp
if [ "$testOnly" = "1" ] ; then
	cat $temp > "$catalog.missing"
else
	cat $temp >> "$catalog"
fi

rm $temp