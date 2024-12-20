## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES =

AM_CFLAGS =\
	 -DHAVE_CONFIG_H\
	 -Wall -ggdb

sbin_PROGRAMS = IPStat

IPStat_SOURCES = \
	main.c\
	core.c\
	core_users.c\
	collectors.c\
	inet.c\
	store.c\
	sched.c\
	server.c\
	misc.c\
	libxml.c\
	tasks.c\
	system.c\
	conf.c\
	log.c\
	threads.c\
	gtree.c\
	glist.c\
	gfunc.c

IPStat_LDFLAGS = 

IPStat_LDADD = \
	 -lpthread -ldl -lssl
