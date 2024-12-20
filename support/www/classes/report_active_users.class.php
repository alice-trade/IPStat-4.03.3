<?
include_once("../classes/main.report.class.php");

class Report_Active_Users extends MainReport
{
	var $className = "Активные пользователи";
	var $classVersion = "1.0";
	var $fileName = "report_active_users.class.php";
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
		$res = mysql_query("select t1.id_user, t2.Name from Log_Time t1, Users t2 where t2.id = t1.id_user and t1.Time>='$this->date1' and t1.Time<'$this->date2' group by t1.id_user");
		$this->itemsTotal = mysql_num_rows($res);

		if($this->itemsTotal < $this->reportItemsPerPage) $this->itemsPages = 1;
		elseif($this->itemsTotal/$this->reportItemsPerPage == floor($this->itemsTotal/$this->reportItemsPerPage)) $this->itemsPages = $this->itemsTotal/$this->reportItemsPerPage;
		else $this->itemsPages = floor($this->itemsTotal/$this->reportItemsPerPage)+1;
		
		$res = mysql_query("select t1.id_user, t2.Name from Log_Time t1, Users t2 where t2.id = t1.id_user and t1.Time>='$this->date1' and t1.Time<'$this->date2' group by t1.id_user order by Name $this->sql3");
		?>

		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		<tr bgcolor="#dddddd" align="center">
			<td class="t1"><strong>Пользователь</strong></td>
		</tr>
		
		<?
		while($row = mysql_fetch_array($res))
		{
			?>
	
			<tr bgcolor="#ffffff" class="t1">
				<td><?=$row['Name']?></td>
			</tr>
	
			<?
		}
		?>
		
		</table>
		<br>

		<?		
		if($this->itemsPages>1)
		{
			?> <div class="t1" align="center">Страницы: <?

			for($i=1;$i<=$this->itemsPages;$i++)
			{
				if(($i-1)*$this->reportItemsPerPage <> $this->reportStartPage) echo "<a href=$this->URL&reportStartPage=".($i-1)*$this->reportItemsPerPage." class=t1>$i</a> ";
				else echo $i." ";
			}

			?> </div><br> <?
		}
		?>		
		</body>
		</html>

		<?		
	}
}
?>