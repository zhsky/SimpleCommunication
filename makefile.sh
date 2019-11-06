#!/bin/sh
echo "
AM_CXXFLAGS=-std=c++17
libsc_la_CFLAGS=-Wall -Wno-reorder -O0 -g -fmessage-length=0 -std=c++17 -ldl -rdynamic
libsc_la_CXXFLAGS=-Wall -Wno-reorder -O0 -g -fmessage-length=0 -std=c++17 -ldl -rdynamic
DEFS =-DDEBUG
"

echo "ACLOCAL_AMFLAGS=-I m4"

function source_list()
{
    echo -n "libsc_la_SOURCES=";
    for SOURCE_FILE in `find ./src  -iname "*.cpp"| sort -ur`
    do
        echo -n ".$SOURCE_FILE ";
    done;
}

function header_list()
{
    echo -n "AM_CPPFLAGS=";
    for PATH_NAME in `find ./include  -iname "*.h" | sed 's/\/[A-Z_a-z0-9]*\.h$//g' | sort -u`
    do
        echo -n "-I.$PATH_NAME ";
    done;
}

echo "libsc_la_LTLIBRARIES = libsc.la"
echo "libsc_ladir=\${prefix}/../lib"
echo "libsc_la_LDFLAGS = -avoid-version"
echo
header_list
echo
source_list
echo