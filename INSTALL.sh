#!/bin/sh
HOME=""
_STRIP="/usr/bin/strip"
_CHMOD="/bin/chmod"

mkdir -p "$HOME/opt/IPStat/etc"
mkdir -p "$HOME/opt/IPStat/etc/httpd"
mkdir -p "$HOME/opt/IPStat/etc/init.d"
mkdir -p "$HOME/opt/IPStat/lib"
mkdir -p "$HOME/opt/IPStat/sbin"
mkdir -p "$HOME/opt/IPStat/libs"
mkdir -p "$HOME/opt/IPStat/log"
mkdir -p "$HOME/opt/IPStat/run"
mkdir -p "$HOME/opt/IPStat/tmp"

cp support/conf/IPStat.cfg "$HOME/opt/IPStat/etc"
cp support/pppd/conf/* "$HOME/opt/IPStat/etc"
cp support/httpd/IPStat.conf "$HOME/opt/IPStat/etc/httpd"
cp support/rc/rc.IPStatd "$HOME/opt/IPStat/etc/init.d/IPStatd"
cp support/libs/* "$HOME/opt/IPStat/libs"
cp ipstat_collect_netfilter_time_ip/.libs/libipstat_collect_netfilter_time_ip.so.0.0.0 $HOME/opt/IPStat/lib/libipstat_collect_netfilter_time_ip.so
cp ipstat_collect_pcap_time_ip/.libs/libipstat_collect_pcap_time_ip.so.0.0.0 $HOME/opt/IPStat/lib/off.libipstat_collect_pcap_time_ip.so
cp ipstat_collect_pppd/.libs/libipstat_collect_pppd.so.0.0.0 $HOME/opt/IPStat/lib/off.libipstat_collect_pppd.so
cp ipstat_collect_squid/.libs/libipstat_collect_squid.so.0.0.0 $HOME/opt/IPStat/lib/libipstat_collect_squid.so
cp ipstat_mysql/.libs/libipstat_mysql.so.0.0.0 $HOME/opt/IPStat/lib/libipstat_mysql.so
cp ipstat_system/.libs/libipstat_system.so.0.0.0 $HOME/opt/IPStat/lib/libipstat_system.so
cp ipstat_system_group_limit/.libs/libipstat_system_group_limit.so.0.0.0 $HOME/opt/IPStat/lib/libipstat_system_group_limit.so
cp ipstat_system_time_policy/.libs/libipstat_system_time_policy.so.0.0.0 $HOME/opt/IPStat/lib/libipstat_system_time_policy.so
cp ipstat_system_users_group_limit/.libs/libipstat_system_users_group_limit.so.0.0.0 $HOME/opt/IPStat/lib/libipstat_system_users_group_limit.so
cp redirector/redirector $HOME/opt/IPStat/sbin/redirector
cp src/IPStat $HOME/opt/IPStat/sbin/IPStat

DIR_LIB=$HOME/opt/IPStat/lib
DIR_SBIN=$HOME/opt/IPStat/sbin

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
