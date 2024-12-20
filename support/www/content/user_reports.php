<?
ini_set("include_path","../include");
include("auth.php"); 
include("connection.php"); 

include_once("../classes/user.report.class.php");
	
$obj = new UserReports($_GET);

$obj->setConnection($is_connected);
$obj->buildReportList();
$obj->displayMenu = false;


if($obj->method == "processingCreateReport") $obj->processingCreateReport($_GET);

if($obj->method == "displayChangePasswd") $obj->displayChangePasswd($_GET); 
if($obj->method == "processingChangePasswd") $obj->processingChangePasswd($_GET);

if($obj->method == "")
{
	$obj->readTemplate();
	$obj->displayTemplate();
}
?>
