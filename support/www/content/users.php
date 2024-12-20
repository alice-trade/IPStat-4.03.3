<?
ini_set("include_path","../include");
include("auth.php"); 
include("connection.php"); 

include_once("../classes/class.users.php");

$obj = new Users($_GET);
$obj->setConnection($is_connected);


if($obj->method == "displayAddGroup") $obj->displayAddGroup();
if($obj->method == "processingAddGroup") $obj->processingAddGroup($_GET);

if($obj->method == "displayEditGroup") $obj->displayEditGroup();
if($obj->method == "processingEditGroup") $obj->processingEditGroup($_GET);

if($obj->method == "displayEditLimitGroup") $obj->displayEditLimitGroup();
if($obj->method == "processingEditLimitGroup") $obj->processingEditLimitGroup($_GET);

if($obj->method == "processingDelGroup") 
{
	$obj->processingDelGroup();
	header("Location: $PHP_SELF\n\n");
}


if($obj->method == "displayAddUser") $obj->displayAddUser();
if($obj->method == "processingAddUser") $obj->processingAddUser($_GET);

if($obj->method == "displayEditUser") $obj->displayEditUser();
if($obj->method == "processingEditUser") $obj->processingEditUser($_GET);

if($obj->method == "processingDelUser") 
{
	$obj->processingDelUser();
	header("Location: $PHP_SELF?groupID=$obj->groupID\n\n");
}

if($obj->method == "processingBlockUsers") 
{
	$obj->processingBlockUsers($_GET);
	header("Location: $PHP_SELF?groupID=$obj->groupID\n\n");
}


if($obj->method == "displayAddRule") $obj->displayAddRule();
if($obj->method == "processingAddRule") $obj->processingAddRule($_GET);

if($obj->method == "displayEditRule") $obj->displayEditRule();
if($obj->method == "processingEditRule") $obj->processingEditRule($_GET);

if($obj->method == "processingDelRule") 
{
	$obj->processingDelRule();
	header("Location: $PHP_SELF?groupID=$obj->groupID&userID=$obj->userID\n\n");
}


if($obj->method == "displayAddRuleGroup") $obj->displayAddRuleGroup();
if($obj->method == "processingAddRuleGroup") $obj->processingAddRuleGroup($_GET);

if($obj->method == "displayEditRuleGroup") $obj->displayEditRuleGroup();
if($obj->method == "processingEditRuleGroup") $obj->processingEditRuleGroup($_GET);

if($obj->method == "processingDelRuleGroup") 
{
	$obj->processingDelRuleGroup();
	header("Location: $PHP_SELF?groupID=$obj->groupID&userID=$obj->userID\n\n");
}


$obj->readTemplate();
if($obj->method == "") $obj->displayTemplate();
?>
