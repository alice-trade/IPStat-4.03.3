<?
session_start();  
if(@$_SESSION['auth'] != session_id())
{
  	session_destroy();
	header("Location: ../index.php\n\n");
}
?>
