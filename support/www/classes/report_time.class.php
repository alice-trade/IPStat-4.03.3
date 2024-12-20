<?
include_once("../classes/main.report.class.php");

class Report_TIME extends MainReport
{
	var $className = "Отчет по времени";
	var $classVersion = "1.0";
	var $fileName = "report_time.class.php";
	var $displayRootOnly = false;

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
		$res = mysql_query("select SUM(Traffic) AS Sum from Log_Time where $this->sql1 Time>='$this->date1' and Time<='$this->date2'");
		$this->trafficSum = mysql_result($res,0);
	
		$res = mysql_query("select DATE_FORMAT(Time, \"%d.%m.%Y\") AS Date, DATE_FORMAT(Time, \"%Y-%m-%d\") AS Date2, SUM(Traffic) AS Sum from Log_Time where $this->sql1 Time>='$this->date1' and Time<='$this->date2' group by LEFT(Time, 10)");
		$this->itemsTotal = mysql_num_rows($res);

		if($this->itemsTotal < $this->reportItemsPerPage) $this->itemsPages = 1;
		elseif($this->itemsTotal/$this->reportItemsPerPage == floor($this->itemsTotal/$this->reportItemsPerPage)) $this->itemsPages = $this->itemsTotal/$this->reportItemsPerPage;
		else $this->itemsPages = floor($this->itemsTotal/$this->reportItemsPerPage)+1;

		if($this->period == 1 || $this->period == 2 || $this->period == 5) $res = mysql_query("select DATE_FORMAT(Time, \"%d.%m.%Y\") AS Date, DATE_FORMAT(Time, \"%Y-%m-%d\") AS Date2, SUM(Traffic) AS Sum from Log_Time where $this->sql1 Time>='$this->date1' and Time<='$this->date2' group by LEFT(Time, 10) $this->sql3");
		else $res = mysql_query("select DATE_FORMAT(Time, \"%d.%m.%Y %H:00\") AS Date, SUM(Traffic) AS Sum from Log_Time where $this->sql1 Time>='$this->date1' and Time<='$this->date2' group by LEFT(Time, 13)");
		?>
		
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		<tr bgcolor="#dddddd" align="center">
			<td class="t1" width="40%"><strong>Дата</strong></td>
			<td class="t1" width="30%"><strong>Объем: <?=$this->convertTraffic($this->trafficSum)?></strong></td>
			<td class="t1" width="30%"><strong>Гистограмма</strong></td>
		</tr>
	
		<?
		while($row = mysql_fetch_array($res))
		{
			?>
	
			<tr bgcolor="#ffffff" class="t1">
			
				<?
				if($this->period == 1 || $this->period == 2 || $this->period == 5) {?> <td align="center"><a href="<?=$this->URL?>&detailView=true&detailParam=<?=$row['Date2']?>" class="t1"><?=$row['Date']?></a></td> <?}
				else {?> <td align="center"><?=$row['Date']?></td> <?}
				?>
				
				<td align="center"><?=$this->convertTraffic($row['Sum'])?></td>
				<td>
					<table border="0" width="100%">
						<tr>
							<td bgcolor="#cccccc" width="<?=round($row['Sum']/$this->trafficSum*1000)/10?>%"><img src="../design/pics/clear.gif" width=1 height=5 border=0 alt=""></td>
							<td width="100%"></td>
						</tr>
					</table>
				</td>
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

	function displayReportDetail()
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
		$res = mysql_query("select SUM(Traffic) AS Sum from Log_Time where $this->sql1 Time>='$this->detailParam 00:00:00' and Time<='$this->detailParam 23:59:59'");
		$this->trafficSum = mysql_result($res,0);

		$res = mysql_query("select DATE_FORMAT(Time, \"%d.%m.%Y %H:00\") AS Date, SUM(Traffic) AS Sum from Log_Time where $this->sql1 Time>='$this->detailParam 00:00:00' and Time<='$this->detailParam 23:59:59' group by LEFT(Time, 13)");
		?>
		
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		<tr bgcolor="#dddddd" align="center">
			<td class="t1" width="40%"><strong>Дата</strong></td>
			<td class="t1" width="30%"><strong>Объем: <?=$this->convertTraffic($this->trafficSum)?></strong></td>
			<td class="t1" width="30%"><strong>Гистограмма</strong></td>
		</tr>
	
		<?
		while($row = mysql_fetch_array($res))
		{
			?>
	
			<tr bgcolor="#ffffff" class="t1">
			
				<td align="center"><?=$row['Date']?></td>
				<td align="center"><?=$this->convertTraffic($row['Sum'])?></td>
				<td>
					<table border="0" width="100%">
						<tr>
							<td bgcolor="#cccccc" width="<?=round($row['Sum']/$this->trafficSum*1000)/10?>%"><img src="../design/pics/clear.gif" width=1 height=5 border=0 alt=""></td>
							<td width="100%"></td>
						</tr>
					</table>
				</td>
			</tr>
	
			<?
		}
		?>
		
		</table>
		
		</body>
		</html>

		<?		
	}
}
?>