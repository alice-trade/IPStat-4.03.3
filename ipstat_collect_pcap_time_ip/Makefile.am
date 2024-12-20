## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES =

AM_CFLAGS =\
	 -Wall -DNDEBUG -ggdb

#libdir = \
#	/usr/lib

lib_LTLIBRARIES = libipstat_collect_pcap_time_ip.la

libipstat_collect_pcap_time_ip_la_SOURCES = \
	main.c\
	../src/conf.c\
	../src/log.c\
	../src/inet.c\
	../src/misc.c\
	../src/func_time.c\
	../src/func_ip.c\
	../src/gfunc.c\
	../src/glist.c\
	../src/gtree.c

libipstat_collect_pcap_time_ip_la_LDFLAGS = @CFLAG_PCAP@

libipstat_collect_pcap_time_ip_la_LIBADD = -lpthread -lpcap
