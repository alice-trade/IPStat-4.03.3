## Process this file with automake to produce Makefile.in
## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

SUBDIRS =  \
	ipstat_mysql\
	ipstat_collect_netfilter_time_ip\
	ipstat_collect_pcap_time_ip\
	ipstat_collect_pppd\
	ipstat_collect_squid\
	redirector\
	ipstat_system\
	ipstat_system_time_policy\
	ipstat_system_group_limit\
	ipstat_system_users_group_limit\
	include\
	src

IPStatdocdir = ${prefix}/share/doc/IPStat
IPStatdoc_DATA = \
	README\
	AUTHORS\
	ChangeLog\
	NEWS\
	TODO\
	support/conf/IPStat.cfg\
	support/cron.d/IPStat\
	support/cron.d/IPStat-watch\
	support/rc/rc.IPStatd\
	support/rc/IPStatd.suse\
	support/pppd/conf/options.pptpd\
	support/pppd/conf/pptpd.conf\
	support/httpd/IPStat.conf\
	support/IPStat.spec\
	support/www/classes/class.help.php\
	support/www/classes/class.policy.php\
	support/www/classes/class.reports.php\
	support/www/classes/class.services.php\
	support/www/classes/class.settings.php\
	support/www/classes/class.statistics.php\
	support/www/classes/class.traffic.php\
	support/www/classes/class.update.php\
	support/www/classes/class.users.php\
	support/www/classes/main.class.php\
	support/www/classes/main.report.class.php\
	support/www/classes/menu.class.php\
	support/www/classes/report_active_users.class.php\
	support/www/classes/report_ip.class.php\
	support/www/classes/report_services.class.php\
	support/www/classes/report_squid.class.php\
	support/www/classes/report_summary.class.php\
	support/www/classes/report_time.class.php\
	support/www/classes/user.report.class.php\
	support/www/content/help.php\
	support/www/content/policy.php\
	support/www/content/reports.php\
	support/www/content/services.php\
	support/www/content/settings.php\
	support/www/content/stat.php\
	support/www/classes/policy_access.php\
	support/www/classes/policy_time.php\
	support/www/content/traffic.php\
	support/www/content/update.php\
	support/www/content/user_reports.php\
	support/www/content/users.php\
	support/www/design/pics/clear.gif\
	support/www/design/pics/logoipstat.gif\
	support/www/design/pics/p-del.gif\
	support/www/design/pics/p-edit.gif\
	support/www/design/pics/warning.gif\
	support/www/design/pics/p-del.gif\
	support/www/design/pics/p-edit.gif\
	support/www/design/pics/p-off.gif\
	support/www/design/pics/p-on.gif\
	support/www/design/pics/p-restrict-open.gif\
	support/www/design/pics/p-restrict.gif\
	support/www/design/styles/style.css\
	support/www/design/template.php\
	support/www/include/auth.php\
	support/www/include/config.php\
	support/www/include/connection.php\
	support/www/tmp\
	support/www/index.php


EXTRA_DIST = $(IPStatdoc_DATA)




# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

