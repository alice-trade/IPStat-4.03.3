<?
include_once("../classes/main.class.php");

class Settings extends Main
{
	var $className = "Настройки";
	var $classVersion = "1.0";
	var $fileName = "settings.php";
	var $sortNum = 7;
	
	var $method = "";
	var $cfg;
	var $err = 0;
	
	function Settings($POST = null)
	{
		if(isset($POST['method'])) $this->method = $POST['method'];
		if(isset($POST['err'])) $this->err = $POST['err'];
	}
	
	function displayContent()
	{
		if(!$this->is_connected) $this->displayError("MySQL сервер недоступен");
		if($this->err == 1) $this->displayError("Неверно указан старый пароль администратора");
		?>

		<table border="0" cellpadding="0" cellspacing="0" width="100%">
		<tr valign="top">
			<td class="t1" width="50%">

			<form name="f" action="<?=$this->fileName?>" method="post" style="margin-top: 0px; margin-bottom: 0px;">
			
			<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
			<tr bgcolor="#dddddd">
				<td colspan="2"><div class="t1"><b>Настройки MySQL</b></div></td>
			</tr>
			<tr bgcolor="#ffffff" class="t1">
				<td width="30%">Сервер</td>
				<td><input type="text" name="mysql_server" value="<?=$this->cfg['mysql_server']?>" style="width: 100%"></td>
			</tr>
			<tr bgcolor="#ffffff" class="t1">
				<td>База</td>
				<td><input type="text" name="mysql_db" value="<?=$this->cfg['mysql_db']?>" style="width: 100%"></td>
			</tr>
			<tr bgcolor="#ffffff" class="t1">
				<td>Логин</td>
				<td><input type="text" name="mysql_login" value="<?=$this->cfg['mysql_login']?>" style="width: 100%"></td>
			</tr>
			<tr bgcolor="#ffffff" class="t1">
				<td>Пароль</td>
				<td><input type="text" name="mysql_passwd" value="<?=$this->cfg['mysql_passwd']?>" style="width: 100%"></td>
			</tr>
			</table>
			<br>

			<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
			<tr bgcolor="#dddddd">
				<td colspan="2"><div class="t1"><b>Интерфейс - Администратор</b></div></td>
			</tr>
			<tr bgcolor="#ffffff" class="t1">
				<td width="30%">Логин</td>
				<td><input type="text" name="admin_login" value="<?=$this->cfg['admin_login']?>" style="width: 100%"></td>
			</tr>
			<tr bgcolor="#ffffff" class="t1">
				<td width="30%">Старый пароль</td>
				<td><input type="text" name="admin_passwd_old" style="width: 100%"></td>
			</tr>
			<tr bgcolor="#ffffff" class="t1">
				<td width="30%">Новый пароль</td>
				<td><input type="text" name="admin_passwd_new" style="width: 100%"></td>
			</tr>
			</table>
			<br>

			<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
			<tr bgcolor="#dddddd">
				<td colspan="2"><div class="t1"><b>Интерфейс - Пользователь</b></div></td>
			</tr>
			<tr bgcolor="#ffffff" class="t1">
				<td width="30%">Логин</td>
				<td><input type="text" name="user_login" value="<?=$this->cfg['user_login']?>" style="width: 100%"></td>
			</tr>
			<tr bgcolor="#ffffff" class="t1">
				<td width="30%">Пароль</td>
				<td><input type="text" name="user_passwd" value="<?=$this->cfg['user_passwd']?>" style="width: 100%"></td>
			</tr>
			</table>
			<br>

			<?				
			if($this->is_connected)
			{
				?>
				
				<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
				<tr bgcolor="#dddddd">
					<td colspan="2"><div class="t1"><b>Общие настройки</b></div></td>
				</tr>
				<tr bgcolor="#ffffff" class="t1">
					<td width="30%">Коэффициент</td>
					<td><input type="text" name="multiply_factor" value="<?=$this->cfg['multiply_factor']?>" style="width: 100%"></td>
				</tr>
				<tr bgcolor="#ffffff" class="t1">
					<td width="30%">Лог</td>
					<td><input type="button" value="Просмотреть" onclick="window.open('<?=$this->fileName?>?method=displayLog','','resizable=yes,scrollbars=yes,height=600,width=550,left='+(screen.width/2-275)+',top='+(screen.height/2-300));"></td>
				</tr>
				</table>
				<br>
				
				<?
			}
			?>
			
			<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="save" value="Сохранить"></td>
			</tr>
			</table>
			<input type="hidden" name="method" value="processingSaveSettings">
			</form>
		
			</td>
			<td width="10"><img src="../design/pics/clear.gif" width=10 height=5 border=0 alt=""><br></td>
			<td width="60%" class="t1">&nbsp;</td>
			
		</tr>
		</table>
						
		<?	
	}
	
	function setConfig($cfg)
	{
		$this->cfg = $cfg;
		if(!isset($this->cfg['admin_passwd_md5'])) $this->cfg['admin_passwd_md5'] = md5($this->cfg['admin_passwd']);
		if(!isset($this->cfg['multiply_factor'])) $this->cfg['multiply_factor'] = 1;
	}
	
	function processingSaveSettings($GET)
	{
		$file = "../include/config.php";
		
		if(!empty($GET['admin_passwd_new']))
		{
			if(!empty($this->cfg['admin_passwd_md5']))
			{
				if(md5($GET['admin_passwd_old']) != $this->cfg['admin_passwd_md5']) return(1);
			}
			
			$admPasswd = md5($GET['admin_passwd_new']);
		}
		else $admPasswd = $this->cfg['admin_passwd_md5'];
		
		if(is_writable($file))
		{
			if(!is_numeric($GET['multiply_factor'])) $GET['multiply_factor'] = 1;
		
			$f = fopen($file,"w");
			fwrite($f,"<?\n\$cfg['ver'] = \"".$this->cfg['ver']."\";\n\$cfg['mysql_server'] = \"".$GET['mysql_server']."\";\n\$cfg['mysql_login'] = \"".$GET['mysql_login']."\";\n\$cfg['mysql_passwd'] = \"".$GET['mysql_passwd']."\";\n\$cfg['mysql_db'] = \"".$GET['mysql_db']."\";\n\$cfg['admin_login'] = \"".$GET['admin_login']."\";\n\$cfg['admin_passwd_md5'] = \"".$admPasswd."\";\n\$cfg['user_login'] = \"".$GET['user_login']."\";\n\$cfg['user_passwd'] = \"".$GET['user_passwd']."\";\n\$cfg['multiply_factor'] = ".$GET['multiply_factor'].";\n?>");
			fclose($f);
			
			if($this->cfg['multiply_factor'] != $GET['multiply_factor']) $this->writeLog("Изменение коэффициента ".$this->cfg['multiply_factor']." -> ".$GET['multiply_factor']);
		}
		
		return(0);
	}
	
	function displayLog()
	{
		?>
		
		<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
		
		<html>
		<head>
			<title>Лог</title>
			<link rel=stylesheet type="text/css" href="../design/styles/style.css">
		</head>
		
		<body leftmargin="0" topmargin="0">
		
		<form name="f" action="<?=$this->fileName?>" method="post" style="margin-top: 0px; margin-bottom: 0px;">
		
		<table width="100%" height="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
			<tr bgcolor="#ffffff">
			<td height="100%">
			<textarea name="log" style="width:100%; height:100%; font-size:8pt" readonly>
Дата - время - IP-адрес - действие
<?
$res = mysql_query("SELECT *, DATE_FORMAT(date, '%d.%m.%Y %H:%i:%s') AS d FROM log_interface ORDER BY date DESC");
while($row = mysql_fetch_array($res)) echo $row['d']."  ".$row['ip']."  ".$row['action']."\n";
?>
			</textarea>
			</td>
			</tr>
			<tr bgcolor="#ffffff">
				<td align="center"><input type="submit" name="btnClear" value="Очистить">&nbsp;&nbsp;&nbsp;<input type="button" name="btnClose" value="Закрыть" onclick="window.close()"></td>
			</tr>
		</table>

		<input type="hidden" name="method" value="processingClearLog">
		</form>
		
		</body>
		</html>
				
		<?
	}
	
	function processingClearLog()
	{
		mysql_query("TRUNCATE TABLE log_interface");
		$this->writeLog("Очистка лога");		
		?>
		
		<script language=JavaScript>
			window.close();
		</script>
		
		<?
	}
}
?>