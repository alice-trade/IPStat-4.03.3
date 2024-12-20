#!/bin/sh
DIR_LIB="/opt/IPStat/lib/"
DIR_SBIN="/opt/IPStat/sbin"

_CP="/bin/cp"
_STRIP="/usr/bin/strip"
_CHMOD="/bin/chmod"
_RM="/bin/rm"
_TAR="/bin/tar"

$_RM -f $DIR_LIB/*
$_RM -f $DIR_SBIN/*

$_CP ipstat_collect_netfilter_time_ip/.libs/libipstat_collect_netfilter_time_ip.so.0.0.0 $DIR_LIB/libipstat_collect_netfilter_time_ip.so
$_CP ipstat_collect_pcap_time_ip/.libs/libipstat_collect_pcap_time_ip.so.0.0.0 $DIR_LIB/off.libipstat_collect_pcap_time_ip.so
$_CP ipstat_collect_pppd/.libs/libipstat_collect_pppd.so.0.0.0 $DIR_LIB/off.libipstat_collect_pppd.so
$_CP ipstat_collect_squid/.libs/libipstat_collect_squid.so.0.0.0 $DIR_LIB/libipstat_collect_squid.so
$_CP ipstat_mysql/.libs/libipstat_mysql.so.0.0.0 $DIR_LIB/libipstat_mysql.so
$_CP ipstat_system/.libs/libipstat_system.so.0.0.0 $DIR_LIB/libipstat_system.so
$_CP ipstat_system_group_limit/.libs/libipstat_system_group_limit.so.0.0.0 $DIR_LIB/libipstat_system_group_limit.so
$_CP ipstat_system_time_policy/.libs/libipstat_system_time_policy.so.0.0.0 $DIR_LIB/libipstat_system_time_policy.so
$_CP ipstat_system_users_group_limit/.libs/libipstat_system_users_group_limit.so.0.0.0 $DIR_LIB/libipstat_system_users_group_limit.so
$_CP src/IPStat $DIR_SBIN/IPStat
$_CP redirector/redirector $DIR_SBIN/redirector

$_STRIP $DIR_LIB/libipstat_collect_netfilter_time_ip.so
$_STRIP $DIR_LIB/off.libipstat_collect_pcap_time_ip.so
$_STRIP $DIR_LIB/off.libipstat_collect_pppd.so
$_STRIP $DIR_LIB/libipstat_collect_squid.so
$_STRIP $DIR_LIB/libipstat_mysql.so
$_STRIP $DIR_LIB/libipstat_system.so
$_STRIP $DIR_LIB/libipstat_system_group_limit.so
$_STRIP $DIR_LIB/libipstat_system_time_policy.so
$_STRIP $DIR_LIB/libipstat_system_users_group_limit.so
$_STRIP $DIR_SBIN/IPStat
$_STRIP $DIR_SBIN/redirector

$_CHMOD 400 $DIR_LIB/*
$_CHMOD 700 $DIR_SBIN/*

$_TAR -czf /opt/IPStat.tgz /opt/IPStat/
