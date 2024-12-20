<?
include_once("../classes/main.report.class.php");

class Report_Summary extends MainReport
{
	var $className = "Сводный отчет";
	var $classVersion = "1.0";
	var $fileName = "report_summary.class.php";
	var $displayRootOnly = true;

	var $itemsTotal = 0;
	var $itemsPages = 0;
	var $trafficSum = 0;
	
	function displayReport()
	{
		?>
		
		<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
		
		<html>
		<head>
			<title>Отчет</title>
			<link rel=stylesheet type="text/css" href="../design/styles/style.css">
		</head>
		
		<body leftmargin="0" topmargin="0">
		
		<?
		$d1 = strtotime($this->date1);
		$d2 = strtotime(ereg_replace(" 23:59:59","",$this->date2));
		
		$arr_date = array("&nbsp;");
		$arr_date[] = strftime("%d.%m", $d1);
		
		while($d1<$d2)
		{
			$arr_date[] = strftime("%d.%m", strtotime("+1 day",$d1));
			$d1 = strtotime("+1 day",$d1);
		}
		
		$res = mysql_query("select a.id_user as id_user, b.Name, DATE_FORMAT(a.Date, '%d.%m') AS Date, SUM(a.Traffic) AS Traffic from System_Report a, Users b where a.id_user = b.id AND a.Action = 0 AND a.Date>='$this->date1' AND a.Date <= '$this->date2' group by a.id_user, a.Date order by b.Name, b.Date");
		
		$user = array();
		for($i=0;$i<count($arr_date);$i++) $user[] = "";
		
		$d1 = strtotime($this->date1);
		$d2 = strtotime($this->date2);		
		?>
		
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		<tr valign="middle">
			<td bgcolor="#cccccc" align="center" colspan="<?=count($arr_date)?>" class="t1"><strong>Сводная таблица за период с <?=strftime("%d.%m.%Y", $d1)?> по <?=strftime("%d.%m.%Y", $d2)?> (<a href="<?=$this->URL?>&detailView=true&detailParam=null" class="t1">экспорт в Excel</a>)</strong></td>
		</tr>
		
		<?
		$i = 0;
		$num_users = 0;
		
		while($row = mysql_fetch_array($res))
		{
			if($user[0] <> $row['Name']) 
			{
				for($k=0;$k<count($user);$k++) if(empty($user[$k])) $user[$k] = 0;
				
				if($i>0)
				{
					?> <tr bgcolor="#ffffff" align="right" class="t1"> <?
					echo "<td><nobr>$user[0]</nobr></td>";
					for($k=1;$k<count($user);$k++) echo "<td>$user[$k]</td>";
					?> </tr> <?
				}
		
				for($k=0;$k<count($user);$k++) $user[$k] = "";
				$user[0] = $row['Name'];
				
				if($num_users/15 == round($num_users/15))
				{
					?> <tr bgcolor="#dddddd" align="center" class="t1"> <?
					foreach($arr_date as $d)
					{
						?> <td><b><?=$d?></b></td> <?
					}
					?> </tr> <?
				}
				
				$num_users++;
			}
			
			$index = 0;
			for($j=1;$j<count($arr_date);$j++) 
			{
				if($arr_date[$j] == $row['Date']) 
				{
					$index = $j;
					break;
				}
			}
			
			if($index>0) $user[$index] = $this->convertTraffic($row['Traffic']);
			$i++;
		}
		?>
	
		</table>
		<br>

		</body>
		</html>

		<?		
	}
	
	function displayReportDetail()
	{
		$d1 = strtotime($this->date1);
		$d2 = strtotime(ereg_replace(" 23:59:59","",$this->date2));
		
		$arr_date = array("");
		$arr_date[] = strftime("%d.%m", $d1);
		
		while($d1<$d2)
		{
			$arr_date[] = strftime("%d.%m", strtotime("+1 day",$d1));
			$d1 = strtotime("+1 day",$d1);
		}
		
		$res = mysql_query("select a.id_user as id_user, b.Name, DATE_FORMAT(a.Date, '%d.%m') AS Date, SUM(a.Traffic) AS Traffic from System_Report a, Users b where a.id_user = b.id AND a.Action = 0 AND a.Date>='$this->date1' AND a.Date <= '$this->date2' group by a.id_user, a.Date order by b.Name, b.Date");
		
		$user = array();
		for($i=1;$i<count($arr_date);$i++) $user[] = "";
		
		$i = 0;
		$num_users = 0;
		
		header("Content-Type: application/csv\n"); 
		header("Content-Disposition: attachment; filename=\"summary.csv\""); 
		
		foreach($arr_date as $d) echo $d.";";
		echo "\n";
		
		while($row = mysql_fetch_array($res))
		{
			if($user[0] <> $row['Name']) 
			{
				for($k=0;$k<count($user);$k++) if(empty($user[$k])) $user[$k] = 0;
				
				if($i>0) 
				{
					for($k=0;$k<count($user);$k++) echo $user[$k].";";
					echo "\n";
				}
		
				for($k=0;$k<count($user);$k++) $user[$k] = "";
				$user[0] = $row['Name'];
				
				$num_users++;
			}
			
			$index = 0;
			for($j=1;$j<count($arr_date);$j++) 
			{
				if($arr_date[$j] == $row['Date']) 
				{
					$index = $j;
					break;
				}
			}
			
			if($index>0) $user[$index] = $row['Traffic'];
			$i++;
		}	
	}	
}
?>