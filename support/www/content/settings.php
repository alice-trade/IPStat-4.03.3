<?
ini_set("include_path","../include");
include("auth.php"); 
include("connection.php"); 

include_once("../classes/class.settings.php");

if(count($_GET)>0) $obj = new Settings($_GET);
else $obj = new Settings($_POST);

$obj->setConnection($is_connected);
$obj->setConfig($cfg);


if($obj->method == "processingSaveSettings") 
{
	$err = $obj->processingSaveSettings($_POST);
	header("Location: $PHP_SELF?err=$err\n\n");
}

if($obj->method == "displayLog") $obj->displayLog();
if($obj->method == "processingClearLog") $obj->processingClearLog();

$obj->readTemplate();
if($obj->method == "") $obj->displayTemplate();
?>
