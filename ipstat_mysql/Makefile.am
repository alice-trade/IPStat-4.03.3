## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES =

AM_CFLAGS =\
	 -Wall -DNDEBUG -ggdb

#libdir = \
#	/usr/lib

lib_LTLIBRARIES = libipstat_mysql.la

libipstat_mysql_la_SOURCES = \
	main.c

libipstat_mysql_la_LDFLAGS = @CFLAG_MYSQL@

libipstat_mysql_la_LIBADD = @LIB_MYSQL@

