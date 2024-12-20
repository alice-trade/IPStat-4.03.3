<?
ini_set("include_path","../include");
include("auth.php"); 
include("connection.php"); 

include_once("../classes/class.reports.php");
	
$obj = new Reports($_GET);

$obj->setConnection($is_connected);
$obj->buildReportList();


if($obj->method == "processingCreateReport") $obj->processingCreateReport($_GET);

if($obj->method == "")
{
	$obj->readTemplate();
	$obj->displayTemplate();
}
?>
