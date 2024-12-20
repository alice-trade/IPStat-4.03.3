<?
include_once("../classes/main.class.php");

class Policy_Access extends Main
{
	var $className = "Ограничение доступа";
	var $classVersion = "1.0";
	var $fileName = "policy_access.php";
	var $parentFileName = "policy.php";
	
	var $method = "";
	var $policyID = 0;
	var $type = 2;
	var $tmpDir = "../tmp";

	function Policy_Access($GET = null)
	{
		if(isset($GET['type'])) $this->type = $GET['type'];
		if(isset($GET['method'])) $this->method = $GET['method'];
		if(isset($GET['policyID'])) $this->policyID = $GET['policyID'];
	}
	
	function getType()
	{
		return($this->type);
	}
	
	function displayContent()
	{
		?>

		<table border="0" cellpadding="0" cellspacing="0" width="100%">
			<tr valign="top">
				<td class="t1" width="40%">

				<?
				$res = mysql_query("SELECT * FROM Policy_URL ORDER BY Name");
				
				if(mysql_num_rows($res)>0)
				{
					?>
				
					<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
					<tr bgcolor="#dddddd">
						<td colspan="3"><div class="t1"><b>Список политик</b></div></td>
					</tr>
					
					<?
					while($row = mysql_fetch_array($res))
					{
						?>
				
						<tr bgcolor="#ffffff">
							<td width="100%"><a href="<?=$this->parentFileName?>?policyID=<?=$row['id']?>&type=<?=$this->type?>" class="t1"><?=$row['Name']?></a></td>
							<td width="18"><a href="#" onclick="javascript:window.open('<?=$this->parentFileName?>?method=displayEditPolicy&policyID=<?=$row['id']?>&type=<?=$this->type?>','','height=474,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-237)+'')" class="t1"><img src="../design/pics/p-edit.gif" width=18 height=18 border=0 alt="Редактировать"></a><br></td>
							<td width="18"><a href="<?=$this->parentFileName?>?method=processingDelPolicy&policyID=<?=$row['id']?>&type=<?=$this->type?>" onclick="return confirm('Подтверждаете удаление политики?');" class="t1"><img src="../design/pics/p-del.gif" width=18 height=18 border=0 alt="Удалить"></a><br></td> 
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
						<td><div class="t1"><b>Новая политика?</b></div></td>
					</tr>
					<tr bgcolor="#ffffff">
						<td align="right"><a href="#" onclick="javascript:window.open('<?=$this->parentFileName?>?method=displayAddPolicy&type=<?=$this->type?>','','height=474,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-237)+'')" class="t1">Создать политику</a></td>
					</tr>
				</table>
				<br>
				
				</td>
				<td width="10"><img src="../design/pics/clear.gif" width=10 height=5 border=0 alt=""><br></td>
				<td width="60%" class="t1">				
				
				<?
				if($this->policyID > 0)
				{
					$arr = "";
					
					$res = mysql_query("SELECT name FROM Policy_URL WHERE id=$this->policyID");
					$policyName = mysql_result($res,0);
	
					$res = mysql_query("SELECT id_user FROM Policy_URL_Users WHERE id_policy=$this->policyID");
					while($row = mysql_fetch_array($res)) $arr .= ($row['id_user'].",");
					$arr = ereg_replace(",$","",$arr);
					
					if(!empty($arr))
					{
						$res = mysql_query("SELECT id, name FROM Users WHERE id IN ($arr) ORDER BY name");
						?>
					
						<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
						<tr bgcolor="#dddddd">
							<td colspan="4"><div class="t1"><b><?=$policyName?></b></div></td>
						</tr>
						
						<?
						while($row = mysql_fetch_array($res))
						{
						
							?>
					
							<tr bgcolor="#ffffff">
								<td width="100%"><div class="t1"><?=$row['name']?></div></td>
							</tr>
							
							<?
						}
						?>	
							
						</table>
						<br>
					
						<?
					}
					else {?> Пользователей с данной политикой нет<br><br> <? }
				}
				else {?> &laquo;&laquo;&laquo; Выберите политику <?}					
				?>
					
				</td>
				
			</tr>
		</table>
	
		<?	
	}

	
	function displayAddPolicy()
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
		
		<form name="f" action="<?=$this->parentFileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
			<td colspan="2"><div class="t1" align="center"><b>Создать политику</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%" colspan="2"><div class="t1"><b>Название</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td colspan="2"><div class="t1"><input type="text" name="name" size="20" maxlength="100" style="width: 100%" value=""></div></td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%" colspan="2"><div class="t1"><b>Тип</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td colspan="2">
					<div class="t1">
					<input type="radio" name="def" value="1" checked> Разрешить все, КРОМЕ данной политики<br>
					<input type="radio" name="def" value="0"> Запретить все, КРОМЕ данной политики
					</div>
				</td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%" colspan="2"><div class="t1"><b>Шаблон</b></div></td>
			</tr>	

			<tr bgcolor="#ffffff">
				<td colspan="2" height="100%">
					<div class="t1"><textarea name="list" rows="20" style="width:100%; height:100%; font-size:8pt"></textarea></div>
				</td>
			</tr>
			
			<tr bgcolor="#dddddd">
				<td align="center" colspan="2"><input type="submit" name="submit1" value="Создать" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
			
		</table>
		<input type="hidden" name="method" value="processingAddPolicy">
		<input type="hidden" name="type" value="<?=$this->type?>">
		</form>
		
		</body>
		</html>
		
		<?
	}	
		
	function processingAddPolicy($GET)
	{
		mysql_query("INSERT INTO Policy_URL(Name, Def) VALUES('".$GET['name']."','".$GET['def']."')");
		$lastID = mysql_insert_id();
		
		$GET['def'] == 0 ? $target = 1 : $target = 0;
		
		$arr = explode("\x0D\x0A",$GET['list']);
		for($i=0;$i<count($arr);$i++)
		{
			$arr[$i] = trim($arr[$i]);
			 if(!empty($arr[$i])) mysql_query("INSERT INTO Policy_URL_Def(id_policy, Pattern, Target) VALUES($lastID, '".$arr[$i]."', '$target')");
		}
		
		$this->writeLog("Добавление политики ".$GET['name']);
		?>
			
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->parentFileName?>?type=<?=$this->type?>';
			window.close();
		</script>
			
		<?
	}	
		
	function displayEditPolicy()
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
		$res = mysql_query("SELECT * FROM Policy_URL WHERE id=$this->policyID");
		$policy = mysql_fetch_array($res);
		
		$res = mysql_query("SELECT Pattern FROM Policy_URL_Def WHERE id_policy=$this->policyID ORDER BY Pattern");
		?>
		
		<form name="f" enctype="multipart/form-data" action="<?=$this->parentFileName?>" method="post" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
			<td colspan="2"><div class="t1" align="center"><b>Редактировать политику</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%" colspan="2"><div class="t1"><b>Название</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td colspan="2"><div class="t1"><input type="text" name="name" size="20" maxlength="100" style="width: 100%" value="<?=$policy['Name']?>"></div></td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%" colspan="2"><div class="t1"><b>Тип</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td colspan="2">
					<div class="t1">
					<input type="radio" name="def" value="1" <? if($policy['Def'] == 1) echo "checked"; ?>> Разрешить все, КРОМЕ данной политики<br>
					<input type="radio" name="def" value="0" <? if($policy['Def'] == 0) echo "checked"; ?>> Запретить все, КРОМЕ данной политики
					</div>
				</td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%" colspan="2"><div class="t1"><b>Шаблон</b></div></td>
			</tr>	

			<tr bgcolor="#ffffff">
				<td colspan="2" height="100%">
					<div class="t1"><textarea name="list" rows="16" style="width:100%; height:100%; font-size:8pt"><? while($row = mysql_fetch_array($res)) echo $row['Pattern']."\n"; ?></textarea></div>
				</td>
			</tr>
			
			<tr bgcolor="#dddddd">
				<td width="100%" colspan="2"><div class="t1"><b>Загрузить шаблон</b></div></td>
			</tr>	

			<tr bgcolor="#ffffff">
				<td colspan="2">
					<div class="t1"><input type="file" name="pattern" style="width: 100%"></div>
				</td>
			</tr>

			<tr bgcolor="#dddddd">
				<td align="center" colspan="2"><input type="submit" name="submit1" value="Изменить" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="Закрыть"></td>
			</tr>
			
		</table>
		<input type="hidden" name="method" value="processingEditPolicy">
		<input type="hidden" name="type" value="<?=$this->type?>">
		<input type="hidden" name="policyID" value="<?=$this->policyID?>">
		</form>
		
		</body>
		</html>
		
		<?
	}		
	
	function processingEditPolicy($GET)
	{
		global $_FILES;
		
		mysql_query("UPDATE Policy_URL SET Name='".$GET['name']."', Def='".$GET['def']."' WHERE id=$this->policyID");

		mysql_query("DELETE FROM Policy_URL_Def WHERE id_policy=$this->policyID");
		$GET['def'] == 0 ? $target = 1 : $target = 0;
		
		if(isset($_FILES['pattern']['tmp_name']) && $_FILES['pattern']['size'] > 0)
		{
			$newFile = $this->tmpDir."/".$_FILES['pattern']['name'];
			move_uploaded_file($_FILES['pattern']['tmp_name'], $newFile);
			$arr = file($newFile);
			unlink($newFile);
		}
		else $arr = explode("\x0D\x0A",$GET['list']);

		for($i=0;$i<count($arr);$i++)
		{
			$arr[$i] = trim($arr[$i]);
			 if(!empty($arr[$i])) mysql_query("INSERT INTO Policy_URL_Def(id_policy, Pattern, Target) VALUES($this->policyID, '".$arr[$i]."', '$target')");
		}
		
		$this->writeLog("Редактирование политики &laquo;".$GET['name']."&raquo;");
		?>
			
		<script language=JavaScript>
			window.opener.location.href = '<?=$this->parentFileName?>?type=<?=$this->type?>';
			window.close();
		</script>
			
		<?
	}
	
	function processingDelPolicy()
	{
		$res = mysql_query("SELECT Name FROM Policy_URL WHERE id=$this->policyID");
		$this->writeLog("Удаление политики &laquo;".mysql_result($res,0)."&raquo;");		
	
		mysql_query("DELETE FROM Policy_URL_Users WHERE id_policy=$this->policyID");
		mysql_query("DELETE FROM Policy_URL_Def WHERE id_policy=$this->policyID");
		mysql_query("DELETE FROM Policy_URL WHERE id=$this->policyID");
	}	
}
?>