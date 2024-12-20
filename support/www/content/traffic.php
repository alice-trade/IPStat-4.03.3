<?
ini_set("include_path","../include");
include("auth.php"); 
include("connection.php"); 

include_once("../classes/class.traffic.php");
	
$obj = new Traffic($_GET);

$obj->setConnection($is_connected);


if($obj->method == "displayAddGroup") $obj->displayAddGroup();
if($obj->method == "processingAddGroup") $obj->processingAddGroup($_GET);

if($obj->method == "displayEditGroup") $obj->displayEditGroup();
if($obj->method == "processingEditGroup") $obj->processingEditGroup($_GET);

if($obj->method == "processingDelGroup") 
{
	$obj->processingDelGroup();
	header("Location: $PHP_SELF\n\n");
}


if($obj->method == "displayAddTraffic") $obj->displayAddTraffic();
if($obj->method == "processingAddTraffic") $obj->processingAddTraffic($_GET);

if($obj->method == "displayEditTraffic") $obj->displayEditTraffic();
if($obj->method == "processingEditTraffic") $obj->processingEditTraffic($_GET);

if($obj->method == "processingDelTraffic") 
{
	$obj->processingDelTraffic();
	header("Location: $PHP_SELF?groupID=$obj->groupID\n\n");
}


$obj->readTemplate();
if($obj->method == "") $obj->displayTemplate();
?>
