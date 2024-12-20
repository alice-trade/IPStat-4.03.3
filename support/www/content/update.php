<?
ini_set("include_path","../include");
include("auth.php"); 
include("connection.php"); 

include_once("../classes/class.update.php");
	
$obj = new Update($_POST);

$obj->setConnection($is_connected);


if($obj->method == "processingInstall") 
{
	$obj->processinginstall($_POST);
	header("Location: $PHP_SELF\n\n");
}

$obj->readTemplate();
$obj->displayTemplate();
?>
