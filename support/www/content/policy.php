<?
ini_set("include_path","../include");
include("auth.php"); 
include("connection.php"); 

include_once("../classes/class.policy.php");

if(count($_GET)>0) $obj = new Policy($_GET);
else $obj = new Policy($_POST);

$obj->setConnection($is_connected);
$obj->buildReportList();


$obj->readTemplate();
$obj->processingPolicy();

if($obj->method == "") $obj->displayTemplate();
?>
