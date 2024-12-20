<?
ini_set("include_path","../include");
include("auth.php"); 
include("connection.php"); 

include_once("../classes/class.help.php");

$obj = new Help;
$obj->readTemplate();
$obj->displayTemplate();
?>
