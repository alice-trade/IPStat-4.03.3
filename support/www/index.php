<?
ini_set("register_globals", "On");
ini_set("include_path","include");

include("include/connection.php");

session_start();

unset($_SESSION['auth']);

header("Content-Type: text/html");
header("Pragma: no-cache");
header("Cache-Control: no-cache");
header("Expires: Thu Jan  1 00:00:00 1970");

if(isset($PHP_AUTH_USER))
{
	if(($cfg['user_login'] == $PHP_AUTH_USER) && ($cfg['user_passwd'] == $PHP_AUTH_PW)) 
	{
		$_SESSION['auth'] = session_id();
		header("Location: content/user_reports.php\n\n");
	}
	else
	{
		$admAuth = false;
		
		if(isset($cfg['admin_passwd_md5']) && ($cfg['admin_passwd_md5'] == md5($PHP_AUTH_PW))) $admAuth = true;
		elseif(isset($cfg['admin_passwd']) && $cfg['admin_passwd'] == $PHP_AUTH_PW) $admAuth = true;

		if($admAuth)
		{
			$_SESSION['auth'] = session_id();
			
			if($is_connected)
			{
				if($handle = opendir("content")) 
				{
				   	while (false !== ($file = readdir($handle))) 
					{
						if(ereg("php$",$file) && $file<>"user_reports.php") header("Location: content/$file\n\n");
					}
				    closedir($handle); 
				}
			}
			else header("Location: content/settings.php\n\n");
		}
		else
		{
			echo "<div style=\"font-family: tahoma; font-size: 9pt\">Вы неверно ввели пароль</div>";
			exit;
		}
	}
}
else
{
	Header("WWW-Authenticate: Basic realm=\"IPStat ".$cfg['ver'].": авторизация\"");
	Header("HTTP/1.0 401 Unauthorized");  

	echo "<div style=\"font-family: tahoma; font-size: 9pt\">Вы неверно ввели пароль</div>";
	exit;
}
?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">

<html>

<head>
	<title>IPStat <?=$cfg['ver']?></title>
	<link rel=stylesheet type="text/css" href="../design/style.css">
	<meta http-equiv="content-type" content="text/html; charset=windows-1251">
</head>

<body></body>

</html>
