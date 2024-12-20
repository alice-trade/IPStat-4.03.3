Summary: Billing system for small or middle office network
Name: IPStat
Version: 4.01
Release: 1
License: BSD
Group: System Environment/Daemons
URL: http://ipstat.perm.ru/
Source0: http://ipstat.perm.ru/download/%{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Requires: chkconfig, iptables, glib, glibc, zlib, krb5-libs, libpcap
BuildRequires: glibc-devel, iptables-devel, mysql-devel, openssl-devel, gcc, glib-devel, libpcap

%description
The IPStat Project is a high performance and highly configurable billing system.

%package web
Summary: Module for Web Interface
Group: System Environment/Daemons
License: BSD
Requires: IPStat, php, php-mysql, httpd

%description web
Module for Web Interface


%package mod_netfilter
Summary: NetFilter Collector Module
Group: System Environment/Daemons
License: BSD
Requires: IPStat

%description mod_netfilter
NetFilter Collector Module

%package mod_pcap
Summary: PCAP Collector Module
Group: System Environment/Daemons
License: BSD
Requires: IPStat

%description mod_pcap
PCAP Collector Module


%package mod_squid
Summary: SQUID Collector Module
Group: System Environment/Daemons
License: BSD
Requires: IPStat, squid >= 2.5

%description mod_squid
SQUID Collector Module

%package mod_time_policy
Summary: Time Policy module
Group: System Environment/Daemons
License: BSD
Requires: IPStat

%description mod_time_policy
IPStat module for Time Policy

%prep
%setup -q -n %{name}-%{version} -a 0

%build
%configure

make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/etc/{IPStat,rc.d/init.d,cron.d,httpd/conf.d}
mkdir -p $RPM_BUILD_ROOT/usr/sbin
mkdir -p $RPM_BUILD_ROOT/usr/lib
mkdir -p $RPM_BUILD_ROOT/usr/lib/IPStat
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/IPStat
mkdir -p $RPM_BUILD_ROOT/usr/share/IPStat
mkdir -p $RPM_BUILD_ROOT/var/log/IPStat

#install config-file
install -m 755 support/rc/rc.IPStatd $RPM_BUILD_ROOT/etc/rc.d/init.d/IPStatd
install -m 644 support/conf/IPStat.cfg $RPM_BUILD_ROOT/etc/IPStat/IPStat.cfg
install -m 644 support/cron.d/IPStat $RPM_BUILD_ROOT/etc/cron.d/IPStat
install -m 644 support/httpd/IPStat.conf $RPM_BUILD_ROOT/etc/httpd/conf.d/IPStat.conf

#install binary
install -m 700 support/cron.d/IPStat-watch $RPM_BUILD_ROOT/usr/sbin/IPStat-watch
install -sm 700 src/IPStat $RPM_BUILD_ROOT/usr/sbin/IPStat
install -sm 707 redirector/redirector $RPM_BUILD_ROOT/usr/sbin/redirector

# install mysql-library
install -sm 600 ipstat_mysql/.libs/libipstat_mysql.so.0.0.0 $RPM_BUILD_ROOT/usr/lib/IPStat/libipstat_mysql.so

#install modules
install -sm 600 ipstat_collect_netfilter_time_ip/.libs/libipstat_collect_netfilter_time_ip.so.0.0.0 $RPM_BUILD_ROOT/usr/lib/IPStat/libipstat_collect_netfilter_time_ip.so
install -sm 600 ipstat_collect_pcap_time_ip/.libs/libipstat_collect_pcap_time_ip.so.0.0.0 $RPM_BUILD_ROOT/usr/lib/IPStat/libipstat_collect_pcap_time_ip.so
install -sm 600 ipstat_collect_squid/.libs/libipstat_collect_squid.so.0.0.0 $RPM_BUILD_ROOT/usr/lib/IPStat/libipstat_collect_squid.so
install -sm 600 ipstat_system/.libs/libipstat_system.so.0.0.0 $RPM_BUILD_ROOT/usr/lib/IPStat/libipstat_system.so
install -sm 600 ipstat_system_time_policy/.libs/libipstat_system_time_policy.so.0.0.0 $RPM_BUILD_ROOT/usr/lib/IPStat/libipstat_system_time_policy.so

mkdir $RPM_BUILD_ROOT/etc/IPStat/rules
touch $RPM_BUILD_ROOT/etc/IPStat/rules/rules.on
touch $RPM_BUILD_ROOT/etc/IPStat/rules/rules.off


#install www-files
cp -R support/www/* $RPM_BUILD_ROOT/usr/share/IPStat

%clean
rm -rf $RPM_BUILD_ROOT

%post
chkconfig --add IPStatd
service crond reload > /dev/null 2>&1
iptables-save > /etc/IPStat/rules/rules.off

%preun
service IPStatd stop > /dev/null 2>&1
chkconfig --del IPStatd

%postun
service crond reload > /dev/null 2>&1

%post mod_netfilter
service IPStatd condrestart > /dev/null 2>&1

%postun mod_netfilter
service IPStatd condrestart > /dev/null 2>&1

%post mod_pcap
service IPStatd condrestart > /dev/null 2>&1

%postun mod_pcap
service IPStatd condrestart > /dev/null 2>&1

%post mod_squid
if [ `egrep "^\#+ *cache_access_log" /etc/squid/squid.conf | wc -l` -eq '0' ]; then
    echo "cache_access_log /var/log/squid/access.socket" >> /etc/squid/squid.conf
else
    perl -i -pe 's/.*cache_access_log .*$/cache_access_log \/var\/log\/squid\/access.socket/' /etc/squid/squid.conf
fi
	                                                                                                                                                             
if [ `egrep "^\#+ *redirect_program" /etc/squid/squid.conf | wc -l` -eq '0' ]; then
    echo "redirect_program /usr/sbin/redirector" >> /etc/squid/squid.conf
else
    perl -i -pe 's/.*redirect_program .*$/redirect_program \/usr\/sbin\/redirector/' /etc/squid/squid.conf
fi
	                                                                                                                                                             
if [ `egrep "^\#+ *redirect_children" /etc/squid/squid.conf | wc -l` -eq '0' ]; then
    echo "redirect_children 20" >> /etc/squid/squid.conf
else
    perl -i -pe 's/.*redirect_children .*$/redirect_children 20/' /etc/squid/squid.conf
fi

service IPStatd condrestart > /dev/null 2>&1

%postun mod_squid
perl -i -pe 's/.*cache_access_log .*$/\# cache_access_log \/var\/log\/squid\/access.log/' /etc/squid/squid.conf
perl -i -pe 's/.*redirect_program .*$/\# redirect_program none/' /etc/squid/squid.conf
perl -i -pe 's/.*redirect_children .*$/\# redirect_children 20/' /etc/squid/squid.conf

service IPStatd condrestart > /dev/null 2>&1

%post web
service httpd condrestart > /dev/null 2>&1

%postun web
service httpd condrestart > /dev/null 2>&1

%files
%defattr(-,root,root,-)
%config /etc/rc.d/init.d/IPStatd
%config (noreplace) /etc/IPStat/IPStat.cfg
%config (noreplace) /etc/IPStat/rules/rules.on
%config (noreplace) /etc/IPStat/rules/rules.off
%config /etc/cron.d/IPStat
%{_libdir}/IPStat/libipstat_mysql.so
%{_libdir}/IPStat/libipstat_system.so
%{_sbindir}/IPStat
%{_sbindir}/IPStat-watch
%attr(0700,root,root) %dir /var/log/IPStat

%files web
%defattr(-,root,root,-)
%config (noreplace) /etc/httpd/conf.d/IPStat.conf
%attr(0644,root,root) %{_datadir}/IPStat/classes/*
%attr(0644,root,root) %{_datadir}/IPStat/content/*
%attr(0644,root,root) %{_datadir}/IPStat/include/auth.php
%config (noreplace) %attr(0666,root,root) %{_datadir}/IPStat/include/config.php
%config (noreplace) %attr(0666,root,root) %{_datadir}/IPStat/include/connection.php
%attr(0644,root,root) %{_datadir}/IPStat/design/template.php
%attr(0644,root,root) %{_datadir}/IPStat/design/pics/*
%attr(0644,root,root) %{_datadir}/IPStat/design/styles/*
%attr(0644,root,root) %{_datadir}/IPStat/tmp
%attr(0644,root,root) %{_datadir}/IPStat/index.php

%files mod_netfilter
%defattr(-,root,root,-)
%{_libdir}/IPStat/libipstat_collect_netfilter_time_ip.so

%files mod_pcap
%defattr(-,root,root,-)
%{_libdir}/IPStat/libipstat_collect_pcap_time_ip.so

%files mod_time_policy
%defattr(-,root,root,-)
%{_libdir}/IPStat/libipstat_system_time_policy.so

%files mod_squid
%defattr(-,root,root,-)
%{_libdir}/IPStat/libipstat_collect_squid.so
%{_sbindir}/redirector


%changelog
* Tue Sep 20 2005 Dmitry N. Kuuev <kornet@ipstat.perm.ru>
Поправлен spec-файл для версии 4.01

* Sun May 15 2005 Dmitry N. Kukuev <kornet@ipstat.perm.ru>
Данные изменения вошли в 6 релиз.
  - исправлена ошибка в редиректоре, массив параметров для конфига был изменён
  - исправлена ошибка в модуле сквида, инициализация параметров происходила после открытия фифо-канала
  - в rpm -ках создаются 2 файла /etc/IPStat/rules/rules.(on,off)
  - изменён init.d файл запуска демона. При включении он загружает правила iptables-а из rules.on, при выключении rules.off 

* Fri Apr 22 2005 Dmitry N. Kukuev <kornet@ipstat.perm.ru>
- исправление в коде:
 - ipq коллектор, TERM понимает с полуслова
 - модуль сквида работает через cf_conf
 - исправлен формат лога
 - добавлен параметр конфиг-файла - debug
- исправление в интерфейсе:
 - добавлен отчёт - "Сводная таблица"
 - в разделе статистика сделаны суммы по остальным полям кроме "Трафик за период"
 
* Fri Apr  8 2005 Dmitry N. Kukuev <kornet@ipstat.perm.ru>
- Исправление в коде, смотри /usr/share/doc/IPStat/ChangeLog
- исправлены постинсталяционный скрипт у пакета IPStat-mod_squid и IPStat-web

* Mon Feb 21 2005 Dmitry N. Kukuev <kornet@perm.ru>
- Исправлен баг в init скрипте. опция status отрабатывается верно
- Веб. интерфейс вынесен в отдельный пакет - IPStat-web
- Исправлены права на файлы веб. интерфейса
- Изменён веб. интерфейс, выполнены ряд исправлений: исправление всех путей на относительные
- Добавлен пост.скрипт для автоматического изменения конфиг.файла /etc/php.ini
- Добавлена функция перезапуска IPStat при инсталяции и деинсталяции модулей

* Sat Jan 29 2005 Dmitry N. Kukuev <kornet@perm.ru>
- create package IPStat ver. 4.00

