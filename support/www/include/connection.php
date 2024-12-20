<?
include("config.php");

$db = @mysql_connect($cfg['mysql_server'], $cfg['mysql_login'], $cfg['mysql_passwd']);
$is_connected = @mysql_select_db($cfg['mysql_db'],$db);
?>