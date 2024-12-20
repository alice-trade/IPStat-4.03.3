<?
include_once("../classes/main.class.php");

class Policy_Time extends Main
{
	var $className = "Ограничение времени";
	var $classVersion = "1.0";
	var $fileName = "policy_time.php";
	var $parentFileName = "policy.php";
	
	var $method = "";
	var $policyID = 0;
	var $type = 1;

	function Policy_Time($GET = null)
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
				$res = mysql_query("SELECT * FROM Policy_Time ORDER BY Name");
				
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
							<td width="18"><a href="#" onclick="javascript:window.open('<?=$this->parentFileName?>?method=displayEditPolicy&policyID=<?=$row['id']?>&type=<?=$this->type?>','','height=318,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-159)+'')" class="t1"><img src="../design/pics/p-edit.gif" width=18 height=18 border=0 alt="Редактировать"></a><br></td>
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
						<td align="right"><a href="#" onclick="javascript:window.open('<?=$this->parentFileName?>?method=displayAddPolicy&type=<?=$this->type?>','','height=318,width=350,left='+(screen.width/2-175)+',top='+(screen.height/2-159)+'')" class="t1">Создать политику</a></td>
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
					
					$res = mysql_query("SELECT name FROM Policy_Time WHERE id=$this->policyID");
					$policyName = mysql_result($res,0);

					$res = mysql_query("SELECT id_user FROM Policy_Time_Users WHERE id_policy=$this->policyID");
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
			err = 0;
			
			if(f.period1.value == "") f.period1.value = "0:00-0:00";
			if(f.period2.value == "") f.period2.value = "0:00-0:00";
			if(f.period3.value == "") f.period3.value = "0:00-0:00";
			if(f.period4.value == "") f.period4.value = "0:00-0:00";
			if(f.period5.value == "") f.period5.value = "0:00-0:00";
			if(f.period6.value == "") f.period6.value = "0:00-0:00";
			if(f.period7.value == "") f.period7.value = "0:00-0:00";
			
			if(f.name.value == '') err = 1;
			if(!parseTime(f.period1.value)) err = 2;
			if(!parseTime(f.period2.value)) err = 2;
			if(!parseTime(f.period3.value)) err = 2;
			if(!parseTime(f.period4.value)) err = 2;
			if(!parseTime(f.period5.value)) err = 2;
			if(!parseTime(f.period6.value)) err = 2;
			if(!parseTime(f.period7.value)) err = 2;
			
			if(err == 0) f.submit();
			else
			{
				if(err == 1) alert('Заполните, пожалуйста, все поля');
				if(err == 2) alert('Неверно указан диапазон');
			}
		}
		
		function parseTime(time)
		{
			re = /[^-:0-9]/;
			
			if(re.test(time)) return(0)
			
			arr = time.split(/-/);
			arrBegin = arr[0].split(/:/);
			arrEnd = arr[1].split(/:/);
			
			time1 = eval(arrBegin[0])*3600 + eval(arrBegin[1])*60;
			time2 = eval(arrEnd[0])*3600 + eval(arrEnd[1])*60;
			
			if(time1 < time2) return(2);
			else return(1);
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
				<td width="50%"><div class="t1"><b>Понедельник</b></div></td>
				<td><div class="t1"><b>Вторник</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td width="50%"><div class="t1"><input type="text" name="period1" size="20" maxlength="11" style="width: 100%" value="9:00-18:00"></div></td>
				<td><div class="t1"><input type="text" name="period2" size="20" maxlength="11" style="width: 100%" value="9:00-18:00"></div></td>
			</tr>
	
			<tr bgcolor="#dddddd">
				<td width="50%"><div class="t1"><b>Среда</b></div></td>
				<td><div class="t1"><b>Четверг</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td width="50%"><div class="t1"><input type="text" name="period3" size="20" maxlength="11" style="width: 100%" value="9:00-18:00"></div></td>
				<td><div class="t1"><input type="text" name="period4" size="20" maxlength="11" style="width: 100%" value="9:00-18:00"></div></td>
			</tr>
	
			<tr bgcolor="#dddddd">
				<td width="50%"><div class="t1"><b>Пятница</b></div></td>
				<td><div class="t1"><b>Суббота</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td width="50%"><div class="t1"><input type="text" name="period5" size="20" maxlength="11" style="width: 100%" value="9:00-18:00"></div></td>
				<td><div class="t1"><input type="text" name="period6" size="20" maxlength="11" style="width: 100%" value="9:00-18:00"></div></td>
			</tr>
	
			<tr bgcolor="#dddddd">
				<td width="50%"><div class="t1"><b>Воскресенье</b></div></td>
				<td>&nbsp;</td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td width="50%"><div class="t1"><input type="text" name="period7" size="20" maxlength="11" style="width: 100%" value="9:00-18:00"></div></td>
				<td>&nbsp;</td>
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
		$arr[1] = $this->strToSec($GET['period1']);
		$arr[2] = $this->strToSec($GET['period2']);
		$arr[3] = $this->strToSec($GET['period3']);
		$arr[4] = $this->strToSec($GET['period4']);
		$arr[5] = $this->strToSec($GET['period5']);
		$arr[6] = $this->strToSec($GET['period6']);
		$arr[0] = $this->strToSec($GET['period7']);
		
		mysql_query("INSERT INTO Policy_Time(Name, Def) VALUES('".$GET['name']."','0')");
		$lastID = mysql_insert_id();
		
		for($i=0;$i<7;$i++) mysql_query("INSERT INTO Policy_Time_Def(id_policy, Week_Day, Time_Begin, Time_End, Target) VALUES($lastID, $i, ".$arr[$i][0].", ".$arr[$i][1].", '1')");
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
		$res = mysql_query("SELECT Name FROM Policy_Time WHERE id=$this->policyID");
		$name = mysql_result($res,0);
		
		$res = mysql_query("SELECT Week_Day, CONCAT(SUBSTRING(SEC_TO_TIME(Time_Begin), 1, 5), '-', SUBSTRING(SEC_TO_TIME(Time_End), 1, 5)) AS period FROM Policy_Time_Def WHERE id_policy=$this->policyID ORDER BY Week_Day");
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
			err = 0;
			
			if(f.period1.value == "") f.period1.value = "0:00-0:00";
			if(f.period2.value == "") f.period2.value = "0:00-0:00";
			if(f.period3.value == "") f.period3.value = "0:00-0:00";
			if(f.period4.value == "") f.period4.value = "0:00-0:00";
			if(f.period5.value == "") f.period5.value = "0:00-0:00";
			if(f.period6.value == "") f.period6.value = "0:00-0:00";
			if(f.period7.value == "") f.period7.value = "0:00-0:00";
			
			if(f.name.value == '') err = 1;
			if(!parseTime(f.period1.value)) err = 2;
			if(!parseTime(f.period2.value)) err = 2;
			if(!parseTime(f.period3.value)) err = 2;
			if(!parseTime(f.period4.value)) err = 2;
			if(!parseTime(f.period5.value)) err = 2;
			if(!parseTime(f.period6.value)) err = 2;
			if(!parseTime(f.period7.value)) err = 2;
			
			if(err == 0) f.submit();
			else
			{
				if(err == 1) alert('Заполните, пожалуйста, все поля');
				if(err == 2) alert('Неверно указан диапазон');
			}
		}
		
		function parseTime(time)
		{
			re = /[^-:0-9]/;
			
			if(re.test(time)) return(0)
			
			arr = time.split(/-/);
			arrBegin = arr[0].split(/:/);
			arrEnd = arr[1].split(/:/);
			
			time1 = eval(arrBegin[0])*3600 + eval(arrBegin[1])*60;
			time2 = eval(arrEnd[0])*3600 + eval(arrEnd[1])*60;
			
			if(time1 < time2) return(2);
			else return(1);
		}
		
		</script>					
		
		<form name="f" action="<?=$this->parentFileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
		<tr>
			<td colspan="2"><div class="t1" align="center"><b>Редактировать политику</b></div></td>
		</tr>
		
			<tr bgcolor="#dddddd">
				<td width="100%" colspan="2"><div class="t1"><b>Название</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td colspan="2"><div class="t1"><input type="text" name="name" size="20" maxlength="100" style="width: 100%" value="<?=$name?>"></div></td>
			</tr>
			
			<tr bgcolor="#dddddd">
				<td width="50%"><div class="t1"><b>Понедельник</b></div></td>
				<td><div class="t1"><b>Вторник</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td width="50%"><div class="t1"><input type="text" name="period1" size="20" maxlength="11" style="width: 100%" value="<?=mysql_result($res,1,"period")?>"></div></td>
				<td><div class="t1"><input type="text" name="period2" size="20" maxlength="11" style="width: 100%" value="<?=mysql_result($res,2,"period")?>"></div></td>
			</tr>
	
			<tr bgcolor="#dddddd">
				<td width="50%"><div class="t1"><b>Среда</b></div></td>
				<td><div class="t1"><b>Четверг</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td width="50%"><div class="t1"><input type="text" name="period3" size="20" maxlength="11" style="width: 100%" value="<?=mysql_result($res,3,"period")?>"></div></td>
				<td><div class="t1"><input type="text" name="period4" size="20" maxlength="11" style="width: 100%" value="<?=mysql_result($res,4,"period")?>"></div></td>
			</tr>
	
			<tr bgcolor="#dddddd">
				<td width="50%"><div class="t1"><b>Пятница</b></div></td>
				<td><div class="t1"><b>Суббота</b></div></td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td width="50%"><div class="t1"><input type="text" name="period5" size="20" maxlength="11" style="width: 100%" value="<?=mysql_result($res,5,"period")?>"></div></td>
				<td><div class="t1"><input type="text" name="period6" size="20" maxlength="11" style="width: 100%" value="<?=mysql_result($res,6,"period")?>"></div></td>
			</tr>
	
			<tr bgcolor="#dddddd">
				<td width="50%"><div class="t1"><b>Воскресенье</b></div></td>
				<td>&nbsp;</td>
			</tr>	
			
			<tr bgcolor="#ffffff">
				<td width="50%"><div class="t1"><input type="text" name="period7" size="20" maxlength="11" style="width: 100%" value="<?=mysql_result($res,0,"period")?>"></div></td>
				<td>&nbsp;</td>
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
		$arr[1] = $this->strToSec($GET['period1']);
		$arr[2] = $this->strToSec($GET['period2']);
		$arr[3] = $this->strToSec($GET['period3']);
		$arr[4] = $this->strToSec($GET['period4']);
		$arr[5] = $this->strToSec($GET['period5']);
		$arr[6] = $this->strToSec($GET['period6']);
		$arr[0] = $this->strToSec($GET['period7']);
	
		mysql_query("UPDATE Policy_Time SET Name='".$GET['name']."' WHERE id=$this->policyID");
		for($i=0;$i<7;$i++) mysql_query("UPDATE Policy_Time_Def SET Time_Begin=".$arr[$i][0].", Time_End=".$arr[$i][1]." WHERE Week_Day=$i AND id_policy=$this->policyID");
		
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
		$res = mysql_query("SELECT Name FROM Policy_Time WHERE id=$this->policyID");
		$this->writeLog("Удаление политики &laquo;".mysql_result($res,0)."&raquo;");		
	
		mysql_query("DELETE FROM Policy_Time_Users WHERE id_policy=$this->policyID");
		mysql_query("DELETE FROM Policy_Time_Def WHERE id_policy=$this->policyID");
		mysql_query("DELETE FROM Policy_Time WHERE id=$this->policyID");
	}	
	
	function strToSec($str)
	{
		$arr1 = explode("-",$str);
		$arrBegin = explode(":",$arr1[0]);
		$arrEnd = explode(":",$arr1[1]);
		
		$time[] = $arrBegin[0]*3600 + $arrBegin[1]*60;
		$time[] = $arrEnd[0]*3600 + $arrEnd[1]*60;
		
		return($time);
	}
}
?>