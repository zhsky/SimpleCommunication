#!/bin/sh
# --*makefile*--
# @Author: Payton
# @Last  : Payton

function handle_module()
{
	echo -n "
$1_CFLAGS=-Wall -Wno-reorder -O0 -g -fmessage-length=0 -std=c++17 -ldl -rdynamic
$1_CXXFLAGS=-Wall -Wno-reorder -O0 -g -fmessage-length=0 -std=c++17 -ldl -rdynamic
$1_LDADD= \${prefix}/../../lib/libsc.a
$1_LDFLAGS=-lpthread
"
	echo -n "$1_SOURCES=";
    for SOURCE_FILE in `find $1  -iname "*.cpp"| sort -ur`
    do
        echo -n "../$SOURCE_FILE ";
    done;
    echo

    echo -n "$1_CPPFLAGS=";
    for PATH_NAME in `find ../include $1 -iname "*.h" | sed 's/\/[A-Z_a-z0-9]*\.h$//g' | sort -u`
    do
        echo -n "-I../$PATH_NAME ";
    done;
    echo

	echo "$1dir=\${prefix}/../$1"
	echo "$1_PROGRAMS = $1"
    echo
}


echo "AM_CXXFLAGS=-std=c++17 -g -O0"
echo "ACLOCAL_AMFLAGS=-I m4"
echo "DEFS =-DDEBUG"

for _DIR in `ls -d */ | grep -v build`
do
	DIR=`basename $_DIR`
	handle_module $DIR
done