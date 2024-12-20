<?
include_once("../classes/main.class.php");

class Services extends Main
{
	var $className = "Сервисы";
	var $classVersion = "1.0";
	var $fileName = "services.php";
	var $sortNum = 6;
	
	var $method = "";
	var $serviceID = 0;
	
	function Services($GET = null)
	{
		if(isset($GET['method'])) $this->method = $GET['method'];
		if(isset($GET['serviceID'])) $this->serviceID = $GET['serviceID'];
	}
	
	function displayContent()
	{
		if($this->is_connected)
		{
			?>

			<table border="0" cellpadding="0" cellspacing="0" width="100%">
				<tr valign="top">
					<td class="t1" width="40%">

					<?
					$res = mysql_query("SELECT * FROM Services ORDER BY Name");
					if(mysql_num_rows($res)>0)
					{
						?>
					
						<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
						<tr bgcolor="#dddddd">
							<td colspan="3"><div class="t1"><b>Список сервисов</b></div></td>
						</tr>
						
						<?
						while($row = mysql_fetch_array($res))
						{
							?>
					
							<tr bgcolor="#ffffff" class="t1">
								<td width="100%"><?=$row['Name']?></td>
								<td width="18"><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayEditService&serviceID=<?=$row['id']?>','','height=206,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-103)+'')" class="t1"><img src="../design/pics/p-edit.gif" width=18 height=18 border=0 alt="Редактировать"></a><br></td>
								<td width="18"><a href="<?=$this->fileName?>?method=processingDelService&serviceID=<?=$row['id']?>" onclick="return confirm('Подтверждаете удаление сервиса?');" class="t1"><img src="../design/pics/p-del.gif" width=18 height=18 border=0 alt="Удалить"></a><br></td> 
							</tr>
							
							<?
						}
						?>	
							
							
						</table>
						<br>
					
						<?
					}
					?>

					<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
						<tr bgcolor="#dddddd">
							<td><div class="t1"><b>Новый сервис?</b></div></td>
						</tr>
						<tr bgcolor="#ffffff">
							<td align="right"><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayAddService','','height=206,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-103)+'')" class="t1">Создать новый сервис</a></td>
						</tr>
					</table>
					<br>
					
					</td>
					<td width="10"><img src="../design/pics/clear.gif" width=10 height=5 border=0 alt=""><br></td>
					<td width="60%" class="t1">&nbsp;</td>
					
				</tr>
			</table>
							
			<?	
		}
		else $this->displayError("MySQL сервер недоступен");
	}
	
	function displayAddService()
	{
		?>
	
		<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
		
		<html>
		<head>
			<title>Создать</title>
			<link rel=stylesheet type="text/css" href="../design/styles/style.css">
		</head>
		
		<body leftmargin="0" topmargin="0">
		
		<script language=JavaScript>
		function check() 
		{
			if(document.forms[0].name.value != '' && document.forms[0].port.value >= 0 && document.forms[0].port.value <= 65535) document.forms[0].submit();
			else alert('Заполните, пожалуйста, все поля');
		}
		</script>					
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
		<td ><div class="t1" align="center"><b>Создать новый сервис</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Название</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td ><div class="t1"><input type="text" name="name" size="20" maxlength="100" style="width: 100%" value=""></div></td>
			</tr>
			
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Протокол</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<select name="proto" class="t1" style="width: 100%">
					<option value="1">ICMP</option>
					<option value="2">IGMP</option>
					<option value="4">IPIP</option>
					<option value="6" selected>TCP</option>
					<option value="8">EGP</option>
					<option value="12">PUP</option>
					<option value="17">UDP</option>
					<option value="46">RSVP</option>
					<option value="47">GRE</option>
					<option value="92">MTP</option>
				</select>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Порт</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td><div class="t1"><input type="text" name="port" maxlength="5" style="width: 100%" value="0"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="submit1" value="Добавить" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
		
		</table>
		<input type="hidden" name="method" value="processingAddService">
		</form>
		
		</body>
		</html>
	
		<?
	}

	function processingAddService($GET)
	{
		mysql_query("INSERT INTO Services(name, proto, port) VALUES('".$GET['name']."', '".$GET['proto']."', '".$GET['port']."')");
		$this->writeLog("Создание сервиса &laquo;".$GET['name']."&raquo;");
		?>
		
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>';
			window.close();
		</script>
		
		<?
	}
	
	function processingDelService()
	{
		$res = mysql_query("SELECT name FROM Services WHERE id=$this->serviceID");
		$this->writeLog("Удаление сервиса &laquo;".mysql_result($res,0)."&raquo;");		
		
		mysql_query("DELETE FROM Services WHERE id=$this->serviceID");
	}	
	
	function displayEditService()
	{
		?>

		<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
		
		<html>
		<head>
			<title>Редактировать</title>
			<link rel=stylesheet type="text/css" href="../design/styles/style.css">
		</head>
		
		<body leftmargin="0" topmargin="0">
		
		<script language=JavaScript>
		function check() 
		{
			if(document.forms[0].name.value != '' && document.forms[0].port.value > 0 && document.forms[0].port.value < 65535) document.forms[0].submit();
			else alert('Заполните, пожалуйста, все поля');
		}
		</script>					
		
		<?
		$res = mysql_query("SELECT * FROM Services WHERE id=$this->serviceID");
		$row = mysql_fetch_array($res);
		?>
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
		<td ><div class="t1" align="center"><b>Редактировать сервис</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Название</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td ><div class="t1"><input type="text" name="name" size="20" maxlength="100" style="width: 100%" value="<?=$row['Name']?>"></div></td>
			</tr>
			
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Протокол</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<select name="proto" class="t1" style="width: 100%">
					<option value="1" <? if($row['Proto'] == 1) echo "selected"; ?>>ICMP</option>
					<option value="2" <? if($row['Proto'] == 2) echo "selected"; ?>>IGMP</option>
					<option value="4" <? if($row['Proto'] == 4) echo "selected"; ?>>IPIP</option>
					<option value="6" <? if($row['Proto'] == 6) echo "selected"; ?>>TCP</option>
					<option value="8" <? if($row['Proto'] == 8) echo "selected"; ?>>EGP</option>
					<option value="12" <? if($row['Proto'] == 12) echo "selected"; ?>>PUP</option>
					<option value="17" <? if($row['Proto'] == 17) echo "selected"; ?>>UDP</option>
					<option value="46" <? if($row['Proto'] == 46) echo "selected"; ?>>RSVP</option>
					<option value="47" <? if($row['Proto'] == 47) echo "selected"; ?>>GRE</option>
					<option value="92" <? if($row['Proto'] == 92) echo "selected"; ?>>MTP</option>
				</select>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Порт</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td><div class="t1"><input type="text" name="port" maxlength="5" style="width: 100%" value="<?=$row['Port']?>"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="submit1" value="Изменить" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
		
		</table>
		<input type="hidden" name="method" value="processingEditService">
		<input type="hidden" name="serviceID" value="<?=$this->serviceID?>">
		</form>
		
		</body>
		</html>
		
		<?
	}
		
	function processingEditService($GET)
	{
		mysql_query("UPDATE Services SET name='".$GET['name']."', proto='".$GET['proto']."', port='".$GET['port']."' WHERE id=$this->serviceID");
		$this->writeLog("Редактирование сервиса &laquo;".$GET['name']."&raquo;");		
		?>
		
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>';
			window.close();
		</script>
		
		<?
	}		
}
?>