<?
ini_set("include_path","../include");
include("auth.php"); 
include("connection.php"); 

include_once("../classes/class.statistics.php");
	
$obj = new Statistics($_GET);

$obj->setConnection($is_connected);
$obj->readTemplate();
$obj->displayTemplate();
?>
