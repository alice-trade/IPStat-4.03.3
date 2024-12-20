<?
include_once("../classes/main.class.php");

class Traffic extends Main
{
	var $className = "Траффик";
	var $classVersion = "1.0";
	var $fileName = "traffic.php";
	var $sortNum = 4;
	
	var $method = "";
	var $groupID = 0;
	var $trafficID = 0;
	
	function Traffic($GET = null)
	{
		if(isset($GET['method'])) $this->method = $GET['method'];
		if(isset($GET['groupID'])) $this->groupID = $GET['groupID'];
		if(isset($GET['trafficID'])) $this->trafficID = $GET['trafficID'];
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
					$res = mysql_query("SELECT * FROM Group_Traffic ORDER BY Name");
					if(mysql_num_rows($res)>0)
					{
						?>
					
						<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
						<tr bgcolor="#dddddd">
							<td colspan="4"><div class="t1"><b>Список групп</b></div></td>
						</tr>
						
						<?
						while($row = mysql_fetch_array($res))
						{
							$tmp = mysql_query("SELECT COUNT(*) FROM Type_Traffic WHERE id_group=".$row['id']);
							$numItems = mysql_result($tmp,0);
							?>
					
							<tr bgcolor="#ffffff">
								
								<td width="100%"><a href="<?=$this->fileName?>?groupID=<?=$row['id']?>" class="t1"><?=$row['Name']?></a></td>
								
								<?
								if($row['id']>1) 
								{ 
									?> 
					
									<td width="18"><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayEditGroup&groupID=<?=$row['id']?>','','height=154,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-77)+'')" class="t1"><img src="../design/pics/p-edit.gif" width=18 height=18 border=0 alt="Редактировать"></a><br></td>
									
									<?
									if($numItems == 0)	{ ?> <td width="18"><a href="<?=$this->fileName?>?method=processingDelGroup&groupID=<?=$row['id']?>" onclick="return confirm('Подтверждаете удаление группы?');" class="t1"><img src="../design/pics/p-del.gif" width=18 height=18 border=0 alt="Удалить"></a><br></td>  <? }
									else { ?>  <td width="18"><img src="../design/pics/p-del.gif" width=18 height=18 border=0 alt="Удалить" onclick="alert('Группа содержит вложенные типы траффика и не может быть удалена')"></a><br></td> <? }
								} 
								else 
								{
									?> 
									
									<td width="18"><img src="../design/pics/clear.gif" width=18 height=18 border=0><br></td> 
									<td width="18"><img src="../design/pics/clear.gif" width=18 height=18 border=0><br></td> 
									
									<?
								}
								?>
								
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
							<td><div class="t1"><b>Новая группа?</b></div></td>
						</tr>
						<tr bgcolor="#ffffff">
							<td align="right"><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayAddGroup','','height=154,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-77)+'')" class="t1">Создать новую группу</a></td>
						</tr>
					</table>
					<br>
					
					</td>
					<td width="10"><img src="../design/pics/clear.gif" width=10 height=5 border=0 alt=""><br></td>
					<td width="60%" class="t1">				
					
					<?
					if($this->groupID > 0)
					{
						$res = mysql_query("SELECT name FROM Group_Traffic WHERE id=$this->groupID");
						$group_name = mysql_result($res,0);
						
						$res = mysql_query("SELECT id, name FROM Type_Traffic WHERE id_group=$this->groupID ORDER BY name");
						
						if(mysql_num_rows($res)>0)
						{
							?>
						
							<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
							<tr bgcolor="#dddddd">
								<td colspan="4"><div class="t1"><b><?=$group_name?></b></div></td>
							</tr>
							
							<?
							while($row = mysql_fetch_array($res))
							{
								$tmp = mysql_query("SELECT COUNT(*) FROM Rules WHERE id_type=".$row['id']);
								$numItems = mysql_result($tmp,0);
								?>
						
								<tr bgcolor="#ffffff">
									<td width="100%"><div class="t1"><?=$row['name']?></div></td>
									<td width="18"><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayEditTraffic&groupID=<?=$this->groupID?>&trafficID=<?=$row['id']?>','','height=408,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-204)+'')" class="t1"><img src="../design/pics/p-edit.gif" width=18 height=18 border=0 alt="Редактировать"></a><br></td>

									<?
									if($numItems == 0)	{ ?> <td width="18"><a href="<?=$this->fileName?>?method=processingDelTraffic&groupID=<?=$this->groupID?>&trafficID=<?=$row['id']?>" onclick="return confirm('Подтверждаете удаление траффика?');" class="t1"><img src="../design/pics/p-del.gif" width=18 height=18 border=0 alt="Удалить"></a><br></td>  <? }
									else { ?>  <td width="18"><img src="../design/pics/p-del.gif" width=18 height=18 border=0 alt="Удалить" onclick="alert('Данный тип трафика не может быть удален, т.к. он используется в правилах для пользователей')"></a><br></td> <? }
									?>
																		
								</tr>
								
								<?
							}
							?>	
								
							</table>
							<br>
						
							<?
						}
						else {?> Список этой группы пуст<br><br> <? }
						?>
						
						<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
							<tr bgcolor="#dddddd">
								<td><div class="t1"><b>Новый тип траффика?</b></div></td>
							</tr>
							<tr bgcolor="#ffffff">
								<td align="right"><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayAddTraffic&groupID=<?=$this->groupID?>','','height=408,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-204)+'')" class="t1">Создать новый тип траффика</a></td>
							</tr>
						</table>
						<br>
						
						<?
					}
					else {?> &laquo;&laquo;&laquo; Выберите группу <?}					
					?>
						
					</td>
					
				</tr>
			</table>
		
			<?	
		}
		else $this->displayError("MySQL сервер недоступен");
	}
	
	function displayAddGroup()
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
			if(f.name.value != '') f.submit();
			else alert('Заполните, пожалуйста, все поля');
		}
		</script>					
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
			<tr>
				<td><div class="t1" align="center"><b>Создать группу</b></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Название</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td><div class="t1"><input type="text" name="name" size="20" maxlength="100" style="width: 100%" value=""></div></td>
			</tr>
			
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Ограничение по времени</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="id_time_policy" class="t1" style="width: 100%">
					
					<option value="0">Нет ограничения</option>
					
					<?
					$res = mysql_query("SELECT id,Name FROM Policy_Time ORDER BY Name");
					while($row = mysql_fetch_array($res))
					{
						?> <option value="<?=$row['id']?>"><?=$row['Name']?></option> <?
					}
					?>
						
					</select>
				</div>
			</td>
			</tr>

			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="submit1" value="Создать" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
			
		</table>
		<input type="hidden" name="method" value="processingAddGroup">
		</form>
		
		</body>
		</html>
		
		<?
	}	
	
	function processingAddGroup($GET)
	{
		mysql_query("INSERT INTO Group_Traffic(name) VALUES('".$GET['name']."')");
		$groupID = mysql_insert_id();
		if($GET['id_time_policy'] > 0) mysql_query("INSERT INTO Policy_Time_GroupTraffic(id_group_traffic, id_policy) VALUES('$groupID','".$GET['id_time_policy']."')");
		
		$this->writeLog("Создание группы трафика &laquo;".$GET['name']."&raquo;");		
		?>
			
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>';
			window.close();
		</script>
			
		<?
	}	
	
	function displayEditGroup()
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
			if(f.name.value != '') f.submit();
			else alert('Заполните, пожалуйста, все поля');
		}
		</script>			
		
		<?
		$res = mysql_query("SELECT name FROM Group_Traffic WHERE id=$this->groupID");
		$name = mysql_result($res,0);
		?>		
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
		<td ><div class="t1" align="center"><b>Редактировать группу</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Название</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="text" name="name" size="20" maxlength="100" style="width: 100%" value="<?=$name?>"></div></td>
			</tr>
			
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Ограничение по времени</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="id_time_policy" class="t1" style="width: 100%">
					
					<option value="0">Нет ограничения</option>
					
					<?
					$group['id_time_policy'] = 0;
					$res = mysql_query("SELECT id_policy FROM Policy_Time_GroupTraffic WHERE id_group_traffic=$this->groupID");
					if(mysql_num_rows($res) == 1) $group['id_time_policy'] = mysql_result($res,0);
					
					$res = mysql_query("SELECT id,Name FROM Policy_Time ORDER BY Name");
					while($row = mysql_fetch_array($res))
					{
						?> <option value="<?=$row['id']?>" <? if($row['id'] == $group['id_time_policy']) echo "selected"; ?>><?=$row['Name']?></option> <?
					}
					?>
						
					</select>
				</div>
			</td>
			</tr>

			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="submit1" value="Изменить" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
			
		</table>
		<input type="hidden" name="groupID" value="<?=$this->groupID?>">
		<input type="hidden" name="method" value="processingEditGroup">
		</form>
		
		</body>
		</html>
		
		<?
	}
		
	function processingEditGroup($GET)
	{
		mysql_query("UPDATE Group_Traffic SET name='".$GET['name']."' WHERE id=$this->groupID");
		
		mysql_query("DELETE FROM Policy_Time_GroupTraffic WHERE id_group_traffic=$this->groupID");
		if($GET['id_time_policy'] > 0) mysql_query("INSERT INTO Policy_Time_GroupTraffic(id_group_traffic, id_policy) VALUES($this->groupID,'".$GET['id_time_policy']."')");
		
		$this->writeLog("Редактирование группы трафика &laquo;".$GET['name']."&raquo;");
		?>
			
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>';
			window.close();
		</script>
			
		<?
	}
			
	function processingDelGroup()
	{
		$res = mysql_query("SELECT name FROM Group_Traffic WHERE id=$this->groupID");
		$this->writeLog("Удаление группы трафика &laquo;".mysql_result($res,0)."&raquo;");		

		$res = mysql_query("SELECT id FROM Type_Traffic WHERE id_group=$this->groupID");
		while($row = mysql_fetch_array($res)) mysql_query("DELETE FROM Policy_Time_TypeTraffic WHERE id_type_traffic=".$row['id']);
			
		mysql_query("DELETE FROM Type_Traffic WHERE id_group=$this->groupID");
		mysql_query("DELETE FROM Policy_Time_GroupTraffic WHERE id_group_traffic=$this->groupID");
		mysql_query("DELETE FROM Group_Traffic WHERE id=$this->groupID");
	}	
	
	
	function displayAddTraffic()
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
			if(document.forms[0].name.value != '') 
			{
				net = document.forms[0].net.value;
				arr = net.split(/\//);
				if(check_ip(arr[0]))
				{
					if(!isNaN(arr[1]) && arr[1]>=0 && arr[1] <=255) document.forms[0].submit();
					else alert('Вы неверно указали сеть/маску');
				}
				else alert('Вы неверно указали сеть/маску (1)');
			}
			else alert('Заполните, пожалуйста, все поля');
		}
		
		function check_ip(ip) 
		{
			var err = false;
			var arr = ip.split(/\./);
			if(arr.length == 4)
			{
				for(i=0;i<4;i++)
				{
					if(isNaN(arr[i])) err = true;
					else
					{
						if(arr[i]<0 || arr[i]>255) err = true;
					}
				}
				
				if(!err) return(true)
			}
			
			return(false);
		}
		</script>					
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
		<td ><div class="t1" align="center"><b>Создать новый тип</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Название</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td ><div class="t1"><input type="text" name="name" size="20" maxlength="100" style="width: 100%" value=""></div></td>
			</tr>
			
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Группа</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="id_group" class="t1" style="width: 100%">
					
						<?
						$res = mysql_query("SELECT id,name FROM Group_Traffic ORDER BY name");
						while($row = mysql_fetch_array($res))
						{
							?> <option value="<?=$row['id']?>" <? if($this->groupID == $row['id']) echo "selected"; ?>><?=$row['name']?></option> <?
						}
						?>
						
					</select>
				</div>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Направление</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td>
				<select name="direct" class="t1" style="width: 100%">
					<option value="1">Входящий</option>
					<option value="2">Исходящий</option>
					<option value="3" selected>Суммарный</option>
				</select>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Протокол</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<select name="proto" class="t1" style="width: 100%">
					<option value="255">Все протоколы</option>
					<option value="1">ICMP</option>
					<option value="2">IGMP</option>
					<option value="4">IPIP</option>
					<option value="6">TCP</option>
					<option value="8">EGP</option>
					<option value="12">PUP</option>
					<option value="17">UDP</option>
					<option value="46">RSVP</option>
					<option value="47">GRE</option>
					<option value="92">MTP</option>
					<option value="254">SQUID</option>
				</select>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Сеть/маска</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="text" name="net" maxlength="255" style="width: 100%" value="0.0.0.0/0"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Порты</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="text" name="ports" maxlength="255" style="width: 100%" value="Все"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Ограничение по времени</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="id_time_policy" class="t1" style="width: 100%">
					
					<option value="0">Нет ограничения</option>
					
					<?
					$res = mysql_query("SELECT id,Name FROM Policy_Time ORDER BY Name");
					while($row = mysql_fetch_array($res))
					{
						?> <option value="<?=$row['id']?>"><?=$row['Name']?></option> <?
					}
					?>
						
					</select>
				</div>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="submit1" value="Добавить" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
		
		</table>
		<input type="hidden" name="method" value="processingAddTraffic">
		<input type="hidden" name="groupID" value="<?=$this->groupID?>">
		</form>
		
		</body>
		</html>
	
		<?
	}

	function processingAddTraffic($GET)
	{
		$GET['ports'] = $this->setPorts($GET['ports']);
		$arr = explode("/",$GET['net']);

		mysql_query("INSERT INTO Type_Traffic(name, id_group, direct, proto, net, mask, ports, price) VALUES('".$GET['name']."', '".$GET['id_group']."', '".$GET['direct']."', '".$GET['proto']."', '$arr[0]', '$arr[1]', '".$GET['ports']."', '0')");
		$trafficID = mysql_insert_id();
		if($GET['id_time_policy'] > 0) mysql_query("INSERT INTO Policy_Time_TypeTraffic(id_type_traffic, id_policy) VALUES('$trafficID','".$GET['id_time_policy']."')");

		$this->writeLog("Создание трафика &laquo;".$GET['name']."&raquo;");
		?>
		
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>?groupID=<?=$this->groupID?>';
			window.close();
		</script>
		
		<?
	}
	
	function processingDelTraffic()
	{
		$res = mysql_query("SELECT name FROM Type_Traffic WHERE id=$this->trafficID");
		$this->writeLog("Удаление трафика &laquo;".mysql_result($res,0)."&raquo;");		
	
		mysql_query("DELETE FROM Policy_Time_TypeTraffic WHERE id_type_traffic=$this->trafficID");
		mysql_query("DELETE FROM Type_Traffic WHERE id=$this->trafficID");
	}	
	
	
	function displayEditTraffic()
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
			if(document.forms[0].name.value != '') 
			{
				net = document.forms[0].net.value;
				arr = net.split(/\//);
				if(check_ip(arr[0]))
				{
					if(!isNaN(arr[1]) && arr[1]>=0 && arr[1] <=255) document.forms[0].submit();
					else alert('Вы неверно указали сеть/маску');
				}
				else alert('Вы неверно указали сеть/маску (1)');
			}
			else alert('Заполните, пожалуйста, все поля');
		}
		
		function check_ip(ip) 
		{
			var err = false;
			var arr = ip.split(/\./);
			if(arr.length == 4)
			{
				for(i=0;i<4;i++)
				{
					if(isNaN(arr[i])) err = true;
					else
					{
						if(arr[i]<0 || arr[i]>255) err = true;
					}
				}
				
				if(!err) return(true)
			}
			
			return(false);
		}
		</script>					
		
		<?
		$res = mysql_query("SELECT * FROM Type_Traffic WHERE id=$this->trafficID");
		$traffic = mysql_fetch_array($res);
		?>
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
		<td ><div class="t1" align="center"><b>Редактировать тип</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Название</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td ><div class="t1"><input type="text" name="name" size="20" maxlength="100" style="width: 100%" value="<?=$traffic['Name']?>"></div></td>
			</tr>
			
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Группа</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="id_group" class="t1" style="width: 100%">
					
						<?
						$res = mysql_query("SELECT id,name FROM Group_Traffic ORDER BY name");
						while($row = mysql_fetch_array($res))
						{
							?> <option value="<?=$row['id']?>" <? if($traffic['id_group'] == $row['id']) echo "selected"; ?>><?=$row['name']?></option> <?
						}
						?>
						
					</select>
				</div>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Направление</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td>
				<select name="direct" class="t1" style="width: 100%">
					<option value="1" <? if($traffic['Direct'] == 1) echo "selected"; ?>>Входящий</option>
					<option value="2" <? if($traffic['Direct'] == 2) echo "selected"; ?>>Исходящий</option>
					<option value="3" <? if($traffic['Direct'] == 3) echo "selected"; ?>>Суммарный</option>
				</select>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Протокол</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<select name="proto" class="t1" style="width: 100%">
					<option value="255" <? if($traffic['Proto'] == 255) echo "selected"; ?>>Все протоколы</option>
					<option value="1" <? if($traffic['Proto'] == 1) echo "selected"; ?>>ICMP</option>
					<option value="2" <? if($traffic['Proto'] == 2) echo "selected"; ?>>IGMP</option>
					<option value="4" <? if($traffic['Proto'] == 4) echo "selected"; ?>>IPIP</option>
					<option value="6" <? if($traffic['Proto'] == 6) echo "selected"; ?>>TCP</option>
					<option value="8" <? if($traffic['Proto'] == 8) echo "selected"; ?>>EGP</option>
					<option value="12" <? if($traffic['Proto'] == 12) echo "selected"; ?>>PUP</option>
					<option value="17" <? if($traffic['Proto'] == 17) echo "selected"; ?>>UDP</option>
					<option value="46" <? if($traffic['Proto'] == 46) echo "selected"; ?>>RSVP</option>
					<option value="47" <? if($traffic['Proto'] == 47) echo "selected"; ?>>GRE</option>
					<option value="92" <? if($traffic['Proto'] == 92) echo "selected"; ?>>MTP</option>
					<option value="254" <? if($traffic['Proto'] == 254) echo "selected"; ?>>SQUID</option>
				</select>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Сеть/маска</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="text" name="net" maxlength="255" style="width: 100%" value="<?=$traffic['Net']."/".$traffic['Mask']?>"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Порты</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="text" name="ports" maxlength="255" style="width: 100%" value="<?=$this->getPorts($traffic['Ports'])?>"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Ограничение по времени</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="id_time_policy" class="t1" style="width: 100%">
					
					<option value="0">Нет ограничения</option>
					
					<?
					$traffic['id_time_policy'] = 0;
					$res = mysql_query("SELECT id_policy FROM Policy_Time_TypeTraffic WHERE id_type_traffic=".$traffic['id']);
					if(mysql_num_rows($res) == 1) $traffic['id_time_policy'] = mysql_result($res,0);
					
					$res = mysql_query("SELECT id,Name FROM Policy_Time ORDER BY Name");
					while($row = mysql_fetch_array($res))
					{
						?> <option value="<?=$row['id']?>" <? if($row['id'] == $traffic['id_time_policy']) echo "selected"; ?>><?=$row['Name']?></option> <?
					}
					?>
						
					</select>
				</div>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="submit1" value="Изменить" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
		
		</table>
		<input type="hidden" name="method" value="processingEditTraffic">
		<input type="hidden" name="trafficID" value="<?=$this->trafficID?>">
		<input type="hidden" name="groupID" value="<?=$this->groupID?>">
		</form>
		
		</body>
		</html>
		
		<?
	}
		
	function processingEditTraffic($GET)
	{
		$GET['ports'] = $this->setPorts($GET['ports']);
		$arr = explode("/",$GET['net']);

		mysql_query("UPDATE Type_Traffic SET name='".$GET['name']."', id_group=".$GET['id_group'].", direct='".$GET['direct']."', proto='".$GET['proto']."', net='$arr[0]', mask='$arr[1]', ports='".$GET['ports']."', price='0' WHERE id=$this->trafficID");

		mysql_query("DELETE FROM Policy_Time_TypeTraffic WHERE id_type_traffic=$this->trafficID");
		if($GET['id_time_policy'] > 0) mysql_query("INSERT INTO Policy_Time_TypeTraffic(id_type_traffic, id_policy) VALUES($this->trafficID,'".$GET['id_time_policy']."')");

		$this->writeLog("Редактирование трафика &laquo;".$GET['name']."&raquo;");
		?>
		
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>?groupID=<?=$this->groupID?>';
			window.close();
		</script>
		
		<?
	}		
	
	
	function setPorts($ports)
	{
	   if($ports=="Все") return 0;
	                                                                                                                                                             
	   if (ereg ("\[([0-9]+)..([0-9]+)\]", $ports, $reg))
	   {
	    $lo = $reg[1];
	    $hi = $reg[2];
	   }else
	   if (ereg ("\[<([0-9]+)\]", $ports, $reg))
	   {
	    $lo = 0;
	    $hi = $reg[1];
	   }else
	   if (ereg ("\[>([0-9]+)\]", $ports, $reg))
	   {
	    $lo = $reg[1];
	    $hi = 65535;
	   }else
	   if (ereg ("([0-9]+)", $ports, $reg))
	   {
	     $lo = $reg[1];
	     $hi = 0;
	   }
	   $tmp = $hi << 16;
	   $tmp+=$lo;
	   return $tmp;
	}

	function getPorts($ports)
	{
	   if ($ports==0) return "Все";
	   $lo = $ports & 65535;
	   $hi = $ports >>16;
	                                                                                                                                                             
	   if ($hi==0 && $lo!=0) $tmp = $lo;else
	   if ($hi!=0 && $lo!=0) $tmp = "[".$lo."..".$hi."]";else
	   if ($hi!=0 && $lo==0) $tmp = "[<".$hi."]";else
	   if ($hi==-1 && $lo!=0)$tmp = "[>".$hi."]";
	   return $tmp;
	}	
}
?>