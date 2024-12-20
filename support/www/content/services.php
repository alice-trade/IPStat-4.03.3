<?
ini_set("include_path","../include");
include("auth.php"); 
include("connection.php"); 

include_once("../classes/class.services.php");
	
$obj = new Services($_GET);

$obj->setConnection($is_connected);


if($obj->method == "displayAddService") $obj->displayAddService();
if($obj->method == "processingAddService") $obj->processingAddService($_GET);

if($obj->method == "displayEditService") $obj->displayEditService();
if($obj->method == "processingEditService") $obj->processingEditService($_GET);

if($obj->method == "processingDelService") 
{
	$obj->processingDelService();
	header("Location: $PHP_SELF\n\n");
}

$obj->readTemplate();
if($obj->method == "") $obj->displayTemplate();
?>
