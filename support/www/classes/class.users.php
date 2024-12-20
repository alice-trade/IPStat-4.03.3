<?
include_once("../classes/main.class.php");

class Users extends Main
{
	var $className = "Пользователи";
	var $classVersion = "1.0";
	var $fileName = "users.php";
	var $sortNum = 3;
	
	var $method = "";
	var $groupID = 0;
	var $userID = 0;
	var $ruleID = 0;
	var $ruleGroupID = 0;

	function Users($GET = null)
	{
		if(isset($GET['method'])) $this->method = $GET['method'];
		if(isset($GET['groupID'])) $this->groupID = $GET['groupID'];
		if(isset($GET['userID'])) $this->userID = $GET['userID'];
		if(isset($GET['ruleID'])) $this->ruleID = $GET['ruleID'];
		if(isset($GET['ruleGroupID'])) $this->ruleGroupID = $GET['ruleGroupID'];
	}

	function displayContent()
	{
		if($this->is_connected)
		{
			?>
			
			<table border="0" cellpadding="0" cellspacing="0" width="100%">
				<tr valign="top">
					<td class="t1" width="30%">

					<?
					$res = mysql_query("SELECT * FROM Group_Users ORDER BY Name");
					if(mysql_num_rows($res)>0)
					{
						?>
					
						<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
						<tr bgcolor="#dddddd">
							<td colspan="3"><div class="t1"><b>Список групп</b></div></td>
						</tr>
						
						<?
						while($row = mysql_fetch_array($res))
						{
							?>
					
							<tr bgcolor="#ffffff">
								
								<td width="100%" style="padding-left: 12px"><a href="<?=$this->fileName?>?groupID=<?=$row['id']?>" class="t1"><?=$row['Name']?></a></td>
								
								<?
								if($row['id']>1) 
								{ 
									?> 
					
									<td width="18"><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayEditGroup&groupID=<?=$row['id']?>','','height=154,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-77)+'')" class="t1"><img src="../design/pics/p-edit.gif" width=18 height=18 border=0 alt="Редактировать"></a><br></td>
									<td width="18"><a href="<?=$this->fileName?>?method=processingDelGroup&groupID=<?=$row['id']?>" onclick="return confirm('Подтверждаете удаление группы \'<?=$row['Name']?>\'?');" class="t1"><img src="../design/pics/p-del.gif" width=18 height=18 border=0 alt="Удалить"></a><br></td> 
									
									<? 
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
					<td width="30%" class="t1">
					
					<?
					if($this->groupID > 0)
					{
						$res = mysql_query("SELECT name FROM Group_Users WHERE id=$this->groupID");
						$group_name = mysql_result($res,0);

						$res = mysql_query("SELECT Limitation FROM Rules_UsersGroup WHERE id_group=$this->groupID");
						mysql_num_rows($res) == 1 ? $groupLimit = mysql_result($res,0) : $groupLimit = 0;
						
						$res = mysql_query("SELECT id, name, addr, onBlocked FROM Users WHERE id_group=$this->groupID ORDER BY name");
						$users_num = mysql_num_rows($res);
						
						if($users_num>0)
						{
							
							?>
						
							<form method="get" method="<?=$this->fileName?>">
							<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
							<tr bgcolor="#dddddd">
								<td colspan="2"><div class="t1"><b><?=$group_name?> <? if($groupLimit > 0) echo "(установлен лимит ".$this->convertTraffic($groupLimit,false).")"; ?></b></div></td>
								<td width="18"><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayEditLimitGroup&groupID=<?=$this->groupID?>','','height=268,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-134)+'')" class="t1"><img src="../design/pics/p-edit.gif" width=18 height=18 border=0 alt="Редактировать лимит группы"></a><br></td>
								<td width="18"><img src="../design/pics/clear.gif" alt="" width="1" height="1" border="0"></td> 
							</tr>
							
							<?
							$c = 0;
							while($row = mysql_fetch_array($res))
							{
							
								?>
						
								<tr bgcolor="#ffffff">
									<td width="100%" style="padding-left: 12px"><a href="<?=$this->fileName?>?groupID=<?=$this->groupID?>&userID=<?=$row['id']?>" title="<?=$row['addr']?>" class="t1"><?=$row['name']?></a></td>
									<td><input type="checkbox" style="margin: -1 -1 -1 -1" name="block[<?=$c?>]" value="<?=$row['id']?>" <? if($row['onBlocked'] == 1) echo "checked"; ?>></td>
									<td width="18"><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayEditUser&groupID=<?=$this->groupID?>&userID=<?=$row['id']?>','','height=614,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-307)+'')" class="t1"><img src="../design/pics/p-edit.gif" width=18 height=18 border=0 alt="Редактировать"></a><br></td>
									<td width="18"><a href="<?=$this->fileName?>?method=processingDelUser&groupID=<?=$this->groupID?>&userID=<?=$row['id']?>" onclick="return confirm('Подтверждаете удаление пользователя \'<?=$row['name']?>\'?');" class="t1"><img src="../design/pics/p-del.gif" width=18 height=18 border=0 alt="Удалить"></a><br></td>
								</tr>
								
								<?
								$c++;
							}
							?>	
								
							<tr bgcolor="#dddddd" class="t1">
								<td colspan="12" align="right"><input type="submit" name="block_users" value="Блокировка"></td>
							</tr>
							
							</table>
							<input type="hidden" name="method" value="processingBlockUsers">
							<input type="hidden" name="groupID" value="<?=$this->groupID?>">
							</form>
						
							<?
						}
						else {?> Список этой группы пуст<br><br> <? }
						?>
						
						<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
							<tr bgcolor="#dddddd">
								<td><div class="t1"><b>Новый пользователь?</b></div></td>
							</tr>
							<tr bgcolor="#ffffff">
								<td align="right"><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayAddUser&groupID=<?=$this->groupID?>','','height=614,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-307)+'')" class="t1">Создать нового пользователя</a></td>
							</tr>
						</table>
						<br>
						
						<?
					}
					else {?> &laquo;&laquo;&laquo; Выберите группу <?}
					?>
					
					</td>
					<td width="10"><img src="../design/pics/clear.gif" width=10 height=5 border=0 alt=""><br></td>
					<td class="t1">

					<?
					if($this->userID > 0)
					{
						$res = mysql_query("SELECT name FROM Users WHERE id=$this->userID");
						$user_name = mysql_result($res,0);
						?>
						
						<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
						<tr bgcolor="#dddddd">
							<td><div class="t1"><b><?=$user_name?>: правила</b></div></td>
						</tr>
						</table>
						
						<?
						$res = mysql_query("SELECT b.id_group, c.Name, IF(ISNULL(d.id), 0, 1) as lim, IF (ISNULL(d.id), 0, IF(d.onActive='1', 0, 1)) as close, d.Limitation FROM Rules a LEFT JOIN Type_Traffic b ON b.id = a.id_type LEFT JOIN Group_Traffic c ON c.id=b.id_group LEFT JOIN Rules_GroupTraffic d ON (d.id_group=b.id_group and d.id_user = a.id_user) where a.id_user=$this->userID group by b.id_group");
						if(mysql_num_rows($res)>0)
						{
							while($row = mysql_fetch_array($res))
							{
								$row['Limitation'] == -1 ? $groupLimit = "" : $groupLimit = $this->convertTraffic($row['Limitation'], false);
								if($row['lim'] == 1) $row['close'] == 0 ? $ruleGroupLimit = "<img src=../design/pics/p-restrict-open.gif alt=\"Установлен лимит $groupLimit\">" : $ruleGroupLimit = "<img src=../design/pics/p-restrict.gif alt=\"Лимит израсходован\">";
								else $ruleGroupLimit = "";
								
								$res2 = mysql_query("SELECT Rules.*, Type_Traffic.Name AS RuleName, IF (Limitation=-1, 0,1) as lim, IF (Limitation=-1, 0, IF(Traffic>=Limitation, 1, 0)) as close FROM Type_Traffic, Rules WHERE Rules.id_user=$this->userID AND Type_Traffic.id=Rules.id_type AND Type_Traffic.id_group=".$row['id_group']);
								if(mysql_num_rows($res2)>0)
								{
									?>
		
									<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
									<tr bgcolor="#cccccc" class="t1" align="center">
										<td colspan="2"><strong><?=$row['Name']?></strong></td>
										<td width="18">&nbsp;</td>
										<td align="center"><?=$ruleGroupLimit?></td>
										<td width="18"><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayEditRuleGroup&groupID=<?=$this->groupID?>&userID=<?=$this->userID?>&ruleGroupID=<?=$row['id_group']?>','','height=268,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-134)+'')" class="t1"><img src="../design/pics/p-edit.gif" width=18 height=18 border=0 alt="Редактировать"></a><br></td>
										<td width="18"><a href="<?=$this->fileName?>?method=processingDelRuleGroup&groupID=<?=$this->groupID?>&userID=<?=$this->userID?>&ruleGroupID=<?=$row['id_group']?>" onclick="return confirm('Подтверждаете удаление группы правил \'<?=$row['Name']?>\'?');" class="t1"><img src="../design/pics/p-del.gif" width=18 height=18 border=0 alt="Удалить"></a><br></td> 
									</tr>
									<tr bgcolor="#dddddd" class="t1" align="center">
										<td><strong>Название</strong></td>
										<td width="30"><strong>Период</strong></td>
										<td width="18"><strong>Огран.</strong></td>
										<td width="18"><strong>Сост.</strong></td>
										<td width="18"></td>
										<td width="18"></td>
									</tr>
									
									<?
									$c = 0;
									while($row2 = mysql_fetch_array($res2))
									{
										if($row2['Type'] == 3) $period = "месяц";
										if($row2['Type'] == 2) $period = "неделя";
										if($row2['Type'] == 1) $period = "день";
										if($row2['Type'] == 0) $period = "нет";
									
										$row2['Limitation'] == -1 ? $limit = "" : $limit = $this->convertTraffic($row2['Limitation'], false);
										
										if($row2['lim'] == 1) $row2['close'] == 0 ? $ruleLimit = "<img src=../design/pics/p-restrict-open.gif alt=\"Установлен лимит $limit\">" : $ruleLimit = "<img src=../design/pics/p-restrict.gif alt=\"Лимит израсходован\">";
										else $ruleLimit = "";
										
										$row2['onActive'] == 1 ? $status = "<img src=../design/pics/p-on.gif alt=\"Включено\">" : $status = "<img src=../design/pics/p-off.gif alt=\"Выключено\">";
										($row2['Action'] & 2) == 2 ? $is_calc = false : $is_calc = true;
										?>
										
										<tr bgcolor="#ffffff" class="t1"  align="center">
											<td align="left" style="padding-left: 12px"><?=$row2['RuleName']?></td>
											<td><?=$period?></td>
											<td><?=$ruleLimit?></td>
											<td><?=$status?></td>
											<td><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayEditRule&groupID=<?=$this->groupID?>&userID=<?=$this->userID?>&ruleID=<?=$row2['id']?>','','height=230,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-115)+'')" class="t1"><img src="../design/pics/p-edit.gif" width=18 height=18 border=0 alt="Редактировать"></a><br></td>
											<td><a href="<?=$this->fileName?>?method=processingDelRule&groupID=<?=$this->groupID?>&userID=<?=$this->userID?>&ruleID=<?=$row2['id']?>" onclick="return confirm('Подтверждаете удаление правила \'<?=$row2['RuleName']?>\'?');" class="t1"><img src="../design/pics/p-del.gif" width=18 height=18 border=0 alt="Удалить"></a><br></td> 
										</tr>
										
										<?
										$c++;
									}
									?>
									
									</table>
									<br>
									
									<?
								}
							}
						}
						else {?> Список правил пуст<br><br> <? }
						?>
						
						<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
							<tr bgcolor="#dddddd">
								<td><div class="t1"><b>Новое правило?</b></div></td>
							</tr>
							<tr bgcolor="#ffffff">
								<td align="right">
									<a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayAddRule&groupID=<?=$this->groupID?>&userID=<?=$this->userID?>','','height=230,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-115)+'')" class="t1">Добавить правило</a><br>
									<a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayAddRuleGroup&groupID=<?=$this->groupID?>&userID=<?=$this->userID?>','','height=268,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-134)+'')" class="t1">Добавить группу правил</a>
								</td>
							</tr>
						</table>
						<br>
						
						<?	
					}
					elseif($this->groupID > 0 && $users_num>0) {?> &laquo;&laquo;&laquo; Выберите пользователя <?}
					else {?> &nbsp; <?}
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
			if(document.forms[0].name.value != '') document.forms[0].submit();
			else alert('Заполните, пожалуйста, все поля');
		}
		</script>					
		
		<form name="f" method="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
		<td ><div class="t1" align="center"><b>Создать группу</b></div></td>
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
		mysql_query("INSERT INTO Group_Users(name) VALUES('".$GET['name']."')");
		$groupID = mysql_insert_id();
		if($GET['id_time_policy'] > 0) mysql_query("INSERT INTO Policy_Time_GroupUsers(id_group_user, id_policy) VALUES('$groupID','".$GET['id_time_policy']."')");

		$this->writeLog("Создание группы пользователей &laquo;".$GET['name']."&raquo;");
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
			if(document.forms[0].name.value != '') document.forms[0].submit();
			else alert('Заполните, пожалуйста, все поля');
		}
		</script>			
		
		<?
		$res = mysql_query("SELECT name FROM Group_Users WHERE id=$this->groupID");
		$name = mysql_result($res,0);
		?>		
		
		<form name="f" method="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
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
					$res = mysql_query("SELECT id_policy FROM Policy_Time_GroupUsers WHERE id_group_user=$this->groupID");
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
		mysql_query("UPDATE Group_Users SET name='".$GET['name']."' WHERE id=$this->groupID");
		
		mysql_query("DELETE FROM Policy_Time_GroupUsers WHERE id_group_user=$this->groupID");
		if($GET['id_time_policy'] > 0) mysql_query("INSERT INTO Policy_Time_GroupUsers(id_group_user, id_policy) VALUES($this->groupID,'".$GET['id_time_policy']."')");
		
		$this->writeLog("Редактирование группы пользователей &laquo;".$GET['name']."&raquo;");
		?>
			
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>';
			window.close();
		</script>
				
		<?
	}

	function processingDelGroup()
	{
		$res = mysql_query("SELECT name FROM Group_Users WHERE id=$this->groupID");
		$this->writeLog("Удаление группы пользователей &laquo;".mysql_result($res,0)."&raquo;");

		$res = mysql_query("SELECT id FROM Users WHERE id_group=$this->groupID");
		while($row = mysql_fetch_array($res)) 
		{
			mysql_query("DELETE FROM Rules WHERE id_user=".$row['id']);

			mysql_query("DELETE FROM Policy_Time_Users WHERE id_user=".$row['id']);
			mysql_query("DELETE FROM Policy_URL_Users WHERE id_user=".$row['id']);
		}

		mysql_query("DELETE FROM Policy_Time_GroupUsers WHERE id_group_user=$this->groupID");
		mysql_query("DELETE FROM Users WHERE id_group=$this->groupID");
		mysql_query("DELETE FROM Group_Users WHERE id=$this->groupID");
	}

	function displayEditLimitGroup()
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
		function restrict()
		{
			if(f.restriction.value == 0)
			{
				f.restriction.value = 1;
				f.type.disabled = true;
				f.type.style.background = "#dddddd";
				f.limitation.disabled = true;
				f.limitation.style.background = "#dddddd";
				f.manualBlock.disabled = true;
			}
			else
			{
				f.restriction.value = 0;
				f.type.disabled = false;
				f.type.style.background = "#ffffff";
				f.limitation.disabled = false;
				f.limitation.style.background = "#ffffff";
				f.manualBlock.disabled = false;
			}
		}
		</script>					
		
		<?
		$arr = array();
		$res = mysql_query("SELECT t1.Type, t1.Limitation, t1.Traffic, t1.onBlocked, t2.Name FROM Rules_UsersGroup t1, Group_Users t2 WHERE t1.id_group=$this->groupID AND t2.id=$this->groupID");
		if(mysql_num_rows($res) == 1)
		{
			$row = mysql_fetch_array($res);
			$arr = array(1,"checked",$row["Type"],$row["Limitation"],$row["onBlocked"],0);
		}
		else $arr = array(0,"","","","",1);
		
		$arr[3] = $this->convertTraffic($arr[3], false);
		?>
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
			<tr>
				<td><div class="t1" align="center"><b>Редактировать группу правил</b></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Группа</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td>
					<div class="t1">
					<select name="id_ruleGroup" class="t1" style="width: 100%" disabled>
					
					<?
					$res = mysql_query("SELECT Name FROM Group_Users WHERE id=$this->groupID");
					?>
						
					<option value="<?=$this->groupID?>"><?=mysql_result($res,0)?></option>
					</select>
					</div>
				</td>
			</tr>
			
			<tr bgcolor="#dddddd">
				<td width="100%"><img src="../design/pics/clear.gif" alt="" width="1" height="1" border="0"></td>
			</tr>	

			<tr bgcolor="#ffffff">
				<td width="100%"><div class="t1"><input type="checkbox" style="margin: -1 -1 -1 -1" name="restriction" value="<?=$arr[0]?>" onclick="restrict()" <?=$arr[1]?>>&nbsp;<b>Ограничить</b></div></td>
			</tr>	
			
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Период ограничения</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="type" class="t1" style="width: 100%">
						<option value="0" <? if($arr[2] == 0) echo "selected"; ?>>Нет</option>
						<option value="1" <? if($arr[2] == 1) echo "selected"; ?>>1 день</option>
						<option value="2" <? if($arr[2] == 2) echo "selected"; ?>>1 неделя</option>
						<option value="3" <? if($arr[2] == 3) echo "selected"; ?>>1 месяц</option>
					</select>
				</div>
			</td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Лимит</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="text" name="limitation" size="20" maxlength="50" style="width: 100%" value="<?=$arr[3]?>"></div></td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><input type="checkbox" style="margin: -1 -1 -1 -1" name="manualBlock" value="1" <? if($arr[4] == 1) echo "checked"; ?>>&nbsp;<b>Ручная блокировка</b></div></td>
			</tr>	

			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="submit1" value="Изменить" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
			
		</table>
		<input type="hidden" name="method" value="processingEditLimitGroup">
		<input type="hidden" name="groupID" value="<?=$this->groupID?>">
		<input type="hidden" name="isNew" value="<?=$arr[5]?>">
		</form>
		
		<script language=JavaScript>restrict()</script>
		
		</body>
		</html>
		
		<?
	}	
	
	function processingEditLimitGroup($GET)
	{
		if(isset($GET['restriction']))
		{
			isset($GET['manualBlock']) ? $onBlocked = 1 : $onBlocked = 0;

			if(ereg("K$",$GET['limitation'])) $GET['limitation'] *= 1024;
			if(ereg("M$",$GET['limitation'])) $GET['limitation'] *= 1048576;

			if($GET['isNew'] == 0) mysql_query("UPDATE Rules_UsersGroup SET Type='".$GET['type']."', Limitation='".$GET['limitation']."', onBlocked='$onBlocked' WHERE id_group=$this->groupID");
			else mysql_query("INSERT INTO Rules_UsersGroup(id_group, Type, Limitation, onBlocked) VALUES($this->groupID,'".$GET['type']."','".$GET['limitation']."','$onBlocked')");
		}
		else mysql_query("DELETE FROM Rules_UsersGroup WHERE id_group=$this->groupID");
		
		$res = mysql_query("SELECT Name FROM Group_Users WHERE id=$this->groupID");
		$this->writeLog("Редактирование лимита группы &laquo;".mysql_result($res,0)."&raquo;");
		?>
			
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>?groupID=<?=$this->groupID?>';
			window.close();
		</script>
			
		<?		
	}
		
	
	function displayAddUser()
	{
		?>
		
		<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
		
		<html>
		<head>
			<title>Добавить</title>
			<link rel=stylesheet type="text/css" href="../design/styles/style.css">
		</head>
		
		<body leftmargin="0" topmargin="0">
		
		<script language=JavaScript>
		function check() 
		{
			if(document.forms[0].name.value != '') 
			{
				if(check_ip(document.forms[0].addr.value)) document.forms[0].submit();
				else alert('Вы неверно указали IP-адрес');
			}
			else alert('Заполните, пожалуйста, все поля');
		}
		
		function check_ip(ip) 
		{
			err = false;
			arr = ip.split(/\./);
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
		<td ><div class="t1" align="center"><b>Добавить пользователя</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Имя</b></div></td>
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
						$res = mysql_query("SELECT id,name FROM Group_Users ORDER BY name");
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
			    <td width="100%"><div class="t1"><b>Логин</b></div></td>
			</tr>
			<tr bgcolor="#ffffff">
			    <td><div class="t1"><input type="text" name="login" size="20" maxlength="100" style="width: 100%" value=""></div></td>
			</tr>
			<tr bgcolor="#dddddd">
			    <td width="100%"><div class="t1"><b>Пароль</b></div></td>
			</tr>
			
			<tr bgcolor="#ffffff">
			    <td><div class="t1"><input type="text" name="passwd" size="20" maxlength="100" style="width: 100%" value=""></div></td>
			</tr>
			<tr bgcolor="#dddddd">
			    <td width="100%"><div class="t1"><b>IP-адрес</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td><div class="t1"><input type="text" name="addr" maxlength="15" style="width: 100%" value="192.168.0."></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>MAC-адрес (XX:XX:XX:XX:XX:XX)</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td><div class="t1"><input type="text" name="mac" maxlength="17" style="width: 100%" value=""></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Ограничение</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="type" class="t1" style="width: 100%">
						<option value="0">Нет</option>
						<option value="1">1 день</option>
						<option value="2">1 неделя</option>
						<option value="3">1 месяц</option>
					</select>
				</div>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Лимит</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td><div class="t1"><input type="text" name="limitation" maxlength="255" style="width: 100%" value="-1"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Комментарий</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td><div class="t1"><input type="text" name="comment" maxlength="255" style="width: 100%" value=""></div></td>
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
				<td width="100%"><div class="t1"><b>Ограничение доступа</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="id_url_policy" class="t1" style="width: 100%">
					
					<option value="0">Нет ограничения</option>
					
					<?
					$res = mysql_query("SELECT id,Name FROM Policy_URL ORDER BY Name");
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
		<input type="hidden" name="method" value="processingAddUser">
		<input type="hidden" name="groupID" value="<?=$this->groupID?>">
		</form>
		
		</body>
		</html>
		
		<?
	}
	
	function processingAddUser($GET)
	{
		$res = mysql_query("SELECT id, id_group, Name FROM Users WHERE Addr='".$GET['addr']."'");
		if(mysql_num_rows($res) == 0)
		{
			if(ereg("K$",$GET['limitation'])) $GET['limitation'] *= 1024;
			if(ereg("M$",$GET['limitation'])) $GET['limitation'] *= 1048576;

			mysql_query("INSERT INTO Users(login, passwd, name, id_group, comment, addr, mac, type, limitation, balance, date) VALUES('".$GET['login']."', '".$GET['passwd']."', '".$GET['name']."', '".$GET['id_group']."', '".$GET['comment']."', '".$GET['addr']."', '".$GET['mac']."', '".$GET['type']."', '".$GET['limitation']."', '0', NOW())");
			$user_id = mysql_insert_id();

			mysql_query("INSERT INTO Rules(id_user, id_type, type, limitation) VALUES('$user_id', '1', '0', '-1')");
			if($GET['id_time_policy'] > 0) mysql_query("INSERT INTO Policy_Time_Users(id_user, id_policy) VALUES('$user_id','".$GET['id_time_policy']."')");
			if($GET['id_url_policy'] > 0) mysql_query("INSERT INTO Policy_URL_Users(id_user, id_policy) VALUES('$user_id','".$GET['id_url_policy']."')");
		}
		else 
		{
			$username = mysql_result($res,0,"Name");
			
			$res = mysql_query("SELECT Name FROM Group_Users WHERE id=".mysql_result($res,0,"id_group"));
			$group = mysql_result($res,0,"Name");
			
			$err = "Пользователь с таким IP-адресом уже есть ($group/$username)";
		}
	
		if(!isset($err))
		{
			$this->writeLog("Создание пользователя &laquo;".$GET['name']."&raquo;");
			?>
			
			<script language=JavaScript>
				window.opener.location.href = '<?=$this->fileName?>?groupID=<?=$this->groupID?>';
				window.close();
			</script>
			
			<?
		}
		else
		{
			?> 
			
			<script language=JavaScript>
				alert('<?=$err?>');
				window.location.href = '<?=$this->fileName?>?method=displayAddUser&groupID=<?=$this->groupID?>';
			</script> 
					
			<?
		}
	}
	
	function displayEditUser()
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
				if(check_ip(document.forms[0].addr.value)) document.forms[0].submit();
				else alert('Вы неверно указали IP-адрес');
			}
			else alert('Заполните, пожалуйста, все поля');
		}
		
		function check_ip(ip) 
		{
			err = false;
			arr = ip.split(/\./);
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
		$res = mysql_query("SELECT * FROM Users WHERE id=$this->userID");
		$user = mysql_fetch_array($res);
		
		$user['Limitation'] = $this->convertTraffic($user['Limitation'], false);
		?>
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
		<td ><div class="t1" align="center"><b>Редактировать пользователя</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Имя</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td ><div class="t1"><input type="text" name="name" size="20" maxlength="100" style="width: 100%" value="<?=$user['Name']?>"></div></td>
			</tr>
			
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Группа</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="id_group" class="t1" style="width: 100%">
					
						<?
						$res = mysql_query("SELECT id,name FROM Group_Users ORDER BY name");
						while($row = mysql_fetch_array($res))
						{
							?> <option value="<?=$row['id']?>" <? if($user['id_group'] == $row['id']) echo "selected"; ?>><?=$row['name']?></option> <?
						}
						?>
						
					</select>
				</div>
			</td>
			</tr>

			<tr bgcolor="#dddddd">
			    <td width="100%"><div class="t1"><b>Логин</b></div></td>
			</tr>
			
			<tr bgcolor="#ffffff">
			    <td><div class="t1"><input type="text" name="login" size="20" maxlength="100" style="width: 100%" value="<?=$user['Login']?>"></div></td>
			</tr>
			
			<tr bgcolor="#dddddd">
			    <td width="100%"><div class="t1"><b>Пароль</b></div></td>
			</tr>
			
			<tr bgcolor="#ffffff">
			    <td><div class="t1"><input type="text" name="passwd" size="20" maxlength="100" style="width: 100%" value="<?=$user['Passwd']?>"></div></td>
			</tr>
			
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>IP-адрес</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td><div class="t1"><input type="text" name="addr" maxlength="15" style="width: 100%" value="<?=$user['Addr']?>"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>MAC-адрес (XX:XX:XX:XX:XX:XX)</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td><div class="t1"><input type="text" name="mac" maxlength="17" style="width: 100%" value="<?=$user['Mac']?>"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Ограничение</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="type" class="t1" style="width: 100%">
						<option value="0" <? if($user['Type'] == 0) echo "selected";?>>Нет</option>
						<option value="1" <? if($user['Type'] == 1) echo "selected";?>>1 день</option>
						<option value="2" <? if($user['Type'] == 2) echo "selected";?>>1 неделя</option>
						<option value="3" <? if($user['Type'] == 3) echo "selected";?>>1 месяц</option>
					</select>
				</div>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Лимит</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td><div class="t1"><input type="text" name="limitation" maxlength="255" style="width: 100%" value="<?=$user['Limitation']?>"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Комментарий</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="text" name="comment" maxlength="255" style="width: 100%" value="<?=$user['Comment']?>"></div></td>
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
					$user['id_time_policy'] = 0;
					$res = mysql_query("SELECT id_policy FROM Policy_Time_Users WHERE id_user=".$user['id']);
					if(mysql_num_rows($res) == 1) $user['id_time_policy'] = mysql_result($res,0);
					
					$res = mysql_query("SELECT id,Name FROM Policy_Time ORDER BY Name");
					while($row = mysql_fetch_array($res))
					{
						?> <option value="<?=$row['id']?>" <? if($row['id'] == $user['id_time_policy']) echo "selected"; ?>><?=$row['Name']?></option> <?
					}
					?>
						
					</select>
				</div>
			</td>
			</tr>
			
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Ограничение доступа</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="id_url_policy" class="t1" style="width: 100%">
					
					<option value="0">Нет ограничения</option>
					
					<?
					$user['id_url_policy'] = 0;
					$res = mysql_query("SELECT id_policy FROM Policy_URL_Users WHERE id_user=".$user['id']);
					if(mysql_num_rows($res) == 1) $user['id_url_policy'] = mysql_result($res,0);

					$res = mysql_query("SELECT id,Name FROM Policy_URL ORDER BY Name");
					while($row = mysql_fetch_array($res))
					{
						?> <option value="<?=$row['id']?>" <? if($row['id'] == $user['id_url_policy']) echo "selected"; ?>><?=$row['Name']?></option> <?
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
		<input type="hidden" name="method" value="processingEditUser">
		<input type="hidden" name="userID" value="<?=$this->userID?>">
		<input type="hidden" name="groupID" value="<?=$this->groupID?>">
		<input type="hidden" name="oldAddr" value="<?=$user['Addr']?>">
		</form>
		
		</body>
		</html>
		
		<?
	}
	
	function processingEditUser($GET)
	{
		if(ereg("K$",$GET['limitation'])) $GET['limitation'] *= 1024;
		if(ereg("M$",$GET['limitation'])) $GET['limitation'] *= 1048576;

		if($GET['addr'] == $GET['oldAddr']) 
		{
			mysql_query("UPDATE Users SET login='".$GET['login']."', passwd='".$GET['passwd']."', name='".$GET['name']."', id_group='".$GET['id_group']."', comment='".$GET['comment']."', addr='".$GET['addr']."', mac='".$GET['mac']."', type='".$GET['type']."', limitation='".$GET['limitation']."', balance='0' WHERE id=$this->userID");

			mysql_query("DELETE FROM Policy_Time_Users WHERE id_user=$this->userID");
			mysql_query("DELETE FROM Policy_URL_Users WHERE id_user=$this->userID");

			if($GET['id_time_policy'] > 0) mysql_query("INSERT INTO Policy_Time_Users(id_user, id_policy) VALUES($this->userID,'".$GET['id_time_policy']."')");
			if($GET['id_url_policy'] > 0) mysql_query("INSERT INTO Policy_URL_Users(id_user, id_policy) VALUES($this->userID,'".$GET['id_url_policy']."')");
		}
		else
		{
			$res = mysql_query("SELECT id, id_group, Name FROM Users WHERE Addr='".$GET['addr']."'");
			if(mysql_num_rows($res) == 0) 
			{
				mysql_query("UPDATE Users SET login='".$GET['login']."', passwd='".$GET['passwd']."', name='".$GET['name']."', id_group='".$GET['id_group']."', comment='".$GET['comment']."', addr='".$GET['addr']."', mac='".$GET['mac']."', type='".$GET['type']."', limitation='".$GET['limitation']."', balance='0' WHERE id=$this->userID");

				mysql_query("DELETE FROM Policy_Time_Users WHERE id_user=$this->userID");
				mysql_query("DELETE FROM Policy_URL_Users WHERE id_user=$this->userID");
	
				if($GET['id_time_policy'] > 0) mysql_query("INSERT INTO Policy_Time_Users(id_user, id_policy) VALUES($this->userID,'".$GET['id_time_policy']."')");
				if($GET['id_url_policy'] > 0) mysql_query("INSERT INTO Policy_URL_Users(id_user, id_policy) VALUES($this->userID,'".$GET['id_url_policy']."')");
			}
			else 
			{
				$username = mysql_result($res,0,"Name");
				
				$res = mysql_query("SELECT Name FROM Group_Users WHERE id=".mysql_result($res,0,"id_group"));
				$group = mysql_result($res,0,"Name");
				
				$err = "Пользователь с таким IP-адресом уже есть ($group/$username)";
			}
		}
	
		if(!isset($err))
		{
			$this->writeLog("Редактирование пользователя &laquo;".$GET['name']."&raquo;");
			?>
			
			<script language=JavaScript>
				window.opener.location.href = '<?=$this->fileName?>?groupID=<?=$this->groupID?>';
				window.close();
			</script>
			
			<?
		}
		else
		{
			?> 
	
			<script language=JavaScript>
				alert('<?=$err?>');
				window.location.href = '<?=$this->fileName?>?method=displayEditUser&groupID=<?=$this->groupID?>&userID=<?=$this->userID?>';
			</script>
			
			<?
		}
	}
	
	function processingDelUser()
	{
		$res = mysql_query("SELECT name FROM Users WHERE id=$this->userID");
		$this->writeLog("Удаление пользователя &laquo;".mysql_result($res,0)."&raquo;");
	
		mysql_query("DELETE FROM Rules WHERE id_user=$this->userID");
		mysql_query("DELETE FROM Policy_Time_Users WHERE id_user=$this->userID");
		mysql_query("DELETE FROM Policy_URL_Users WHERE id_user=$this->userID");
		mysql_query("DELETE FROM Users WHERE id=$this->userID");
	}
	
	function processingBlockUsers($GET)
	{
		mysql_query("UPDATE Users SET onBlocked='0' WHERE id_group=$this->groupID");
		while (list($n, $id) = @each($GET['block'])) 
		{
			$res = mysql_query("SELECT name FROM Users WHERE id=$id");
			$this->writeLog("Блокировка пользователя &laquo;".mysql_result($res,0)."&raquo;");

			mysql_query("UPDATE Users SET onBlocked='1' WHERE id=$id");
		}
	}	

	function displayAddRuleGroup()
	{
		?>
		
		<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
		
		<html>
		<head>
			<title>Добавить</title>
			<link rel=stylesheet type="text/css" href="../design/styles/style.css">
		</head>
		
		<body leftmargin="0" topmargin="0">
		
		<script language=JavaScript>
		function check() 
		{
			if(f.name.value != '') f.submit();
			else alert('Заполните, пожалуйста, все поля');
		}
		
		function restrict()
		{
			if(f.restriction.value == 0)
			{
				f.restriction.value = 1;
				f.type.disabled = true;
				f.type.style.background = "#dddddd";
				f.limitation.disabled = true;
				f.limitation.style.background = "#dddddd";
				f.manualBlock.disabled = true;
			}
			else
			{
				f.restriction.value = 0;
				f.type.disabled = false;
				f.type.style.background = "#ffffff";
				f.limitation.disabled = false;
				f.limitation.style.background = "#ffffff";
				f.manualBlock.disabled = false;
			}
		}
		</script>					
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
			<tr>
				<td><div class="t1" align="center"><b>Добавить группу правил</b></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Группа</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td>
					<div class="t1">
					<select name="id_ruleGroup" class="t1" style="width: 100%">
					
					<?
					$res = mysql_query("SELECT id,Name FROM Group_Traffic ORDER BY Name");
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
				<td width="100%"><img src="../design/pics/clear.gif" alt="" width="1" height="1" border="0"></td>
			</tr>	

			<tr bgcolor="#ffffff">
				<td width="100%"><div class="t1"><input type="checkbox" style="margin: -1 -1 -1 -1" name="restriction" value="0" onclick="restrict()">&nbsp;<b>Ограничить</b></div></td>
			</tr>	
			
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Период ограничения</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="type" class="t1" style="width: 100%">
						<option value="0">Нет</option>
						<option value="1">1 день</option>
						<option value="2">1 неделя</option>
						<option value="3">1 месяц</option>
					</select>
				</div>
			</td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Лимит</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="text" name="limitation" size="20" maxlength="50" style="width: 100%" value="-1"></div></td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><input type="checkbox" style="margin: -1 -1 -1 -1" name="manualBlock" value="1">&nbsp;<b>Ручная блокировка</b></div></td>
			</tr>	

			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="submit1" value="Добавить" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
			
		</table>
		<input type="hidden" name="method" value="processingAddRuleGroup">
		<input type="hidden" name="groupID" value="<?=$this->groupID?>">
		<input type="hidden" name="userID" value="<?=$this->userID?>">
		</form>
		
		<script language=JavaScript>restrict()</script>
		
		</body>
		</html>
		
		<?
	}	
	
	function processingAddRuleGroup($GET)
	{
		mysql_query("INSERT INTO Rules(id_user, id_type) select ".$GET['userID'].", id from Type_Traffic where id_group=".$GET['id_ruleGroup']);
		
		if(isset($GET['restriction']))
		{
			isset($GET['manualBlock']) ? $onBlocked = 1 : $onBlocked = 0;

			if(ereg("K$",$GET['limitation'])) $GET['limitation'] *= 1024;
			if(ereg("M$",$GET['limitation'])) $GET['limitation'] *= 1048576;
			
			mysql_query("INSERT INTO Rules_GroupTraffic(id_user, id_group, Type, Limitation, onBlocked) VALUES(".$GET['userID'].", ".$GET['id_ruleGroup'].",'".$GET['type']."','".$GET['limitation']."','$onBlocked')");
		}
		
		$res = mysql_query("SELECT t1.Name AS User, t2.Name AS GroupName FROM Users t1, Group_Traffic t2 WHERE t1.id=".$GET['userID']." AND t2.id=".$GET['id_ruleGroup']);
		$this->writeLog("Добавление группы правил &laquo;".mysql_result($res,0,"GroupName")."&raquo; для пользователя &laquo;".mysql_result($res,0,"User")."&raquo;");
		?>
		
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>?groupID=<?=$this->groupID?>&userID=<?=$this->userID?>';
			window.close();
		</script>
		
		<?
	}	
	
	function displayEditRuleGroup()
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
		function restrict()
		{
			if(f.restriction.value == 0)
			{
				f.restriction.value = 1;
				f.type.disabled = true;
				f.type.style.background = "#dddddd";
				f.limitation.disabled = true;
				f.limitation.style.background = "#dddddd";
				f.manualBlock.disabled = true;
			}
			else
			{
				f.restriction.value = 0;
				f.type.disabled = false;
				f.type.style.background = "#ffffff";
				f.limitation.disabled = false;
				f.limitation.style.background = "#ffffff";
				f.manualBlock.disabled = false;
			}
		}
		</script>					
		
		<?
		$arr = array();
		$res = mysql_query("SELECT id_group, Type, Limitation, Traffic, onBlocked FROM Rules_GroupTraffic WHERE id_user=$this->userID AND id_group=$this->ruleGroupID");
		if(mysql_num_rows($res) == 1)
		{
			$row = mysql_fetch_array($res);
			$arr = array(1,"checked",$row["Type"],$row["Limitation"],$row["onBlocked"],0);
		}
		else $arr = array(0,"","","","",1);
		
		$arr[3] = $this->convertTraffic($arr[3], false);
		?>
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
			<tr>
				<td><div class="t1" align="center"><b>Редактировать группу правил</b></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Группа</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td>
					<div class="t1">
					<select name="id_ruleGroup" class="t1" style="width: 100%" disabled>
					
					<?
					$res = mysql_query("SELECT id,Name FROM Group_Traffic ORDER BY Name");
					while($row = mysql_fetch_array($res))
					{
						?> <option value="<?=$row['id']?>" <? if($row['id'] == $this->ruleGroupID) echo "selected"; ?>><?=$row['Name']?></option> <?
					}
					?>
						
					</select>
					</div>
				</td>
			</tr>
			
			<tr bgcolor="#dddddd">
				<td width="100%"><img src="../design/pics/clear.gif" alt="" width="1" height="1" border="0"></td>
			</tr>	

			<tr bgcolor="#ffffff">
				<td width="100%"><div class="t1"><input type="checkbox" style="margin: -1 -1 -1 -1" name="restriction" value="<?=$arr[0]?>" onclick="restrict()" <?=$arr[1]?>>&nbsp;<b>Ограничить</b></div></td>
			</tr>	
			
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Период ограничения</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="type" class="t1" style="width: 100%">
						<option value="0" <? if($arr[2] == 0) echo "selected"; ?>>Нет</option>
						<option value="1" <? if($arr[2] == 1) echo "selected"; ?>>1 день</option>
						<option value="2" <? if($arr[2] == 2) echo "selected"; ?>>1 неделя</option>
						<option value="3" <? if($arr[2] == 3) echo "selected"; ?>>1 месяц</option>
					</select>
				</div>
			</td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Лимит</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="text" name="limitation" size="20" maxlength="50" style="width: 100%" value="<?=$arr[3]?>"></div></td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><input type="checkbox" style="margin: -1 -1 -1 -1" name="manualBlock" value="1" <? if($arr[4] == 1) echo "checked"; ?>>&nbsp;<b>Ручная блокировка</b></div></td>
			</tr>	

			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="submit1" value="Изменить" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
			
		</table>
		<input type="hidden" name="method" value="processingEditRuleGroup">
		<input type="hidden" name="groupID" value="<?=$this->groupID?>">
		<input type="hidden" name="userID" value="<?=$this->userID?>">
		<input type="hidden" name="ruleGroupID" value="<?=$this->ruleGroupID?>">
		<input type="hidden" name="isNew" value="<?=$arr[5]?>">
		</form>
		
		<script language=JavaScript>restrict()</script>
		
		</body>
		</html>
		
		<?
	}	
	
	function processingEditRuleGroup($GET)
	{
		if(isset($GET['restriction']))
		{
			isset($GET['manualBlock']) ? $onBlocked = 1 : $onBlocked = 0;

			if(ereg("K$",$GET['limitation'])) $GET['limitation'] *= 1024;
			if(ereg("M$",$GET['limitation'])) $GET['limitation'] *= 1048576;

			if($GET['isNew'] == 0) mysql_query("UPDATE Rules_GroupTraffic SET Type='".$GET['type']."', Limitation='".$GET['limitation']."', onBlocked='$onBlocked' WHERE id_user=$this->userID AND id_group=$this->ruleGroupID");
			else mysql_query("INSERT INTO Rules_GroupTraffic(id_user, id_group, Type, Limitation, onBlocked) VALUES($this->userID, $this->ruleGroupID,'".$GET['type']."','".$GET['limitation']."','$onBlocked')");
		}
		else mysql_query("DELETE FROM Rules_GroupTraffic WHERE id_user=$this->userID AND id_group=$this->ruleGroupID");
		
		$res = mysql_query("SELECT t1.Name AS User, t2.Name AS GroupName FROM Users t1, Group_Traffic t2 WHERE t1.id=$this->userID AND t2.id=$this->ruleGroupID");
		$this->writeLog("Редактирование группы трафика &laquo;".mysql_result($res,0,"GroupName")."&raquo; для пользователя &laquo;".mysql_result($res,0,"User")."&raquo;");
		?>
			
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>?groupID=<?=$this->groupID?>&userID=<?=$this->userID?>';
			window.close();
		</script>
			
		<?		
	}
		
	function processingDelRuleGroup()
	{
		$err = false;
		
		mysql_query("set autocommit=0");
		mysql_query("start transaction");
		mysql_query("DELETE Rules from Rules, Type_Traffic where Type_Traffic.id=Rules.id_type and Rules.id_user=$this->userID and Type_Traffic.id_group=$this->ruleGroupID");
		if(mysql_error()) $err = true;
		mysql_query("DELETE FROM Rules_GroupTraffic WHERE id_user=$this->userID AND id_group=$this->ruleGroupID");
		if(mysql_error()) $err = true;

		if($err) mysql_query("rollback");
		else 
		{
			mysql_query("commit");
			
			$res = mysql_query("SELECT t1.Name AS User, t2.Name AS GroupName FROM Users t1, Group_Traffic t2 WHERE t1.id=$this->userID AND t2.id=$this->ruleGroupID");
			$this->writeLog("Удаление группы правил &laquo;".mysql_result($res,0,"GroupName")."&raquo; для пользователя &laquo;".mysql_result($res,0,"User")."&raquo;");
		}
	}
		
	function displayAddRule()
	{
		?>
		
		<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
		
		<html>
		<head>
			<title>Добавить</title>
			<link rel=stylesheet type="text/css" href="../design/styles/style.css">
		</head>
		
		<body leftmargin="0" topmargin="0">
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
			<td><div class="t1" align="center"><b>Добавить правило</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Название</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td colspan="2">
				<select name="id_type" class="t1" style="width: 100%">
		
					<?
					$res = mysql_query("SELECT id,name FROM Type_Traffic ORDER BY name");
					while($row = mysql_fetch_array($res))
					{
						?> <option value="<?=$row['id']?>"><?=$row['name']?></option> <?
					}
					?>
				
				</select>
			</td>
			</tr>
			
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Ограничение</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="type" class="t1" style="width: 100%">
						<option value="0">Нет</option>
						<option value="1">1 день</option>
						<option value="2">1 неделя</option>
						<option value="3">1 месяц</option>
					</select>
				</div>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Лимит</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="text" name="limitation" maxlength="255" style="width: 100%" value="-1"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%">
				
					<table border="0" cellpadding="0" cellspacing="0" width="100%">
						<tr align="center">
							<td class="t1"><input type="checkbox" style="margin: -1 -1 -1 -1"  name="isBlock">&nbsp;<strong>Блокировка</strong></td>
							<td class="t1"><input type="checkbox" style="margin: -1 -1 -1 -1" name="isNotCalc">&nbsp;<strong>Не считать</strong></td>
						</tr>
					</table>
				
				</td>
			</tr>	

			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="add" value="Добавить">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
		
		</table>
		<input type="hidden" name="method" value="processingAddRule">
		<input type="hidden" name="groupID" value="<?=$this->groupID?>">
		<input type="hidden" name="userID" value="<?=$this->userID?>">
		</form>
		
		</body>
		</html>
		
		<?
	}
		
	function processingAddRule($GET)
	{
		if(ereg("K$",$GET['limitation'])) $GET['limitation'] *= 1024;
		if(ereg("M$",$GET['limitation'])) $GET['limitation'] *= 1048576;
		
		isset($GET['isNotCalc']) ? $Action = 2 : $Action = 0;
		isset($GET['isBlock']) ? $isBlock = 1 : $isBlock = 0;

		mysql_query("INSERT INTO Rules(id_user, id_type, Type, Limitation, Action, onBlocked) VALUES($this->userID, '".$GET['id_type']."', '".$GET['type']."', '".$GET['limitation']."', $Action, '$isBlock')");

		$res = mysql_query("SELECT t1.name as traffic, t2.name as user FROM Type_Traffic t1, Users t2 WHERE t1.id=".$GET['id_type']." AND t2.id=$this->userID");
		$this->writeLog("Создание правила &laquo;".mysql_result($res,0,"traffic")."&raquo; для пользователя &laquo;".mysql_result($res,0,"user")."&raquo;");
		?>
		
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>?groupID=<?=$this->groupID?>&userID=<?=$this->userID?>';
			window.close();
		</script>
		
		<?
	}
			
	function displayEditRule()
	{
		?>
		
		<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
		
		<html>
		<head>
			<title>Редактировать</title>
			<link rel=stylesheet type="text/css" href="../design/styles/style.css">
		</head>
		
		<body leftmargin="0" topmargin="0">
		
		<?
		$res = mysql_query("SELECT * FROM Rules WHERE id=$this->ruleID");
		$rule = mysql_fetch_array($res);
		
		($rule['Action'] & 2) == 2 ? $isNotCalc = true : $isNotCalc = false;
		?>
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
		<td ><div class="t1" align="center"><b>Редактировать правило</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Название</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
			<td colspan="2">
				<select name="id_type" class="t1" style="width: 100%">
		
					<?
					$res = mysql_query("SELECT id,name FROM Type_Traffic ORDER BY name");
					while($row = mysql_fetch_array($res))
					{
						?> <option value="<?=$row['id']?>" <? if($rule['id_type'] == $row['id']) echo "selected"; ?>><?=$row['name']?></option> <?
					}
					?>
				
				</select>
			</td>
			</tr>
			
			<tr bgcolor="#dddddd">
			<td width="100%"><div class="t1"><b>Ограничение</b></div></td>
			</tr>	
		
			<tr bgcolor="#ffffff">
			<td>
				<div class="t1">
					<select name="type" class="t1" style="width: 100%">
						<option value="0" <? if($rule['Type'] == 0) echo "selected"; ?>>Нет</option>
						<option value="1" <? if($rule['Type'] == 1) echo "selected"; ?>>1 день</option>
						<option value="2" <? if($rule['Type'] == 2) echo "selected"; ?>>1 неделя</option>
						<option value="3" <? if($rule['Type'] == 3) echo "selected"; ?>>1 месяц</option>
					</select>
				</div>
			</td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>Лимит</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="text" name="limitation" maxlength="255" style="width: 100%" value="<?=$this->convertTraffic($rule['Limitation'], false)?>"></div></td>
			</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%">
				
					<table border="0" cellpadding="0" cellspacing="0" width="100%">
						<tr align="center">
							<td class="t1"><input type="checkbox" style="margin: -1 -1 -1 -1"  name="isBlock" <? if($rule['onBlocked'] == "1") echo "checked"; ?>>&nbsp;<strong>Блокировка</strong></td>
							<td class="t1"><input type="checkbox" style="margin: -1 -1 -1 -1" name="isNotCalc" <? if($isNotCalc) echo "checked"; ?>>&nbsp;<strong>Не считать</strong></td>
						</tr>
					</table>
				
				</td>
			</tr>	

			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="change" value="Изменить">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
		
		</table>
		<input type="hidden" name="method" value="processingEditRule">
		<input type="hidden" name="ruleID" value="<?=$this->ruleID?>">
		<input type="hidden" name="userID" value="<?=$this->userID?>">
		<input type="hidden" name="groupID" value="<?=$this->groupID?>">
		</form>
		
		</body>
		</html>
		
		<?
	}
				
	function processingEditRule($GET)
	{
		if(ereg("K$",$GET['limitation'])) $GET['limitation'] *= 1024;
		if(ereg("M$",$GET['limitation'])) $GET['limitation'] *= 1048576;
		
		isset($GET['isNotCalc']) ? $Action = "|2" : $Action = "&253";
		isset($GET['isBlock']) ? $isBlock = 1 : $isBlock = 0;
	
		mysql_query("UPDATE Rules SET id_type='".$GET['id_type']."', Type='".$GET['type']."', Limitation='".$GET['limitation']."', onBlocked='$isBlock', Action=Action$Action  WHERE id=$this->ruleID");
		
		$res = mysql_query("SELECT t1.name as traffic, t2.name as user FROM Type_Traffic t1, Users t2 WHERE t1.id=".$GET['id_type']." AND t2.id=$this->userID");
		$this->writeLog("Редактирование правила &laquo;".mysql_result($res,0,"traffic")."&raquo; для пользователя &laquo;".mysql_result($res,0,"user")."&raquo;");
		?>
		
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->fileName?>?groupID=<?=$this->groupID?>&userID=<?=$this->userID?>';
			window.close();
		</script>
		
		<?	
	}
					
	function processingDelRule()
	{
		$res = mysql_query("SELECT id_type, id_user FROM Rules WHERE id=$this->ruleID");
		$res = mysql_query("SELECT t1.name as traffic, t2.name as user FROM Type_Traffic t1, Users t2 WHERE t1.id=".mysql_result($res,0,"id_type")." AND t2.id=".mysql_result($res,0,"id_user"));
		
		$this->writeLog("Удаление правила &laquo;".mysql_result($res,0,"traffic")."&raquo; пользователя &laquo;".mysql_result($res,0,"user")."&raquo;");
	
		mysql_query("DELETE FROM Rules WHERE id=$this->ruleID");
	}
}
?>
