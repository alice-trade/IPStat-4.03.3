<?
include_once("../classes/main.report.class.php");

class Report_SERVICES extends MainReport
{
	var $className = "Отчет по сервисам";
	var $classVersion = "1.0";
	var $fileName = "report_services.class.php";
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
		if($this->user == 0)
		{
			$res = mysql_query("select SUM(Traffic) from Log_IP where Date>='$this->date1' and Date<='$this->date2'");
			$this->trafficSum = mysql_result($res,0);
	
			$res = mysql_query("select t2.id AS id, t2.Name, SUM(t1.Traffic) as Sum from Log_IP t1, Services t2 where t1.Proto = t2.Proto and t1.Port = t2.Port and Date>='$this->date1' and Date<='$this->date2' group by t2.id");
			$this->itemsTotal = mysql_num_rows($res);
			
			if($this->itemsTotal < $this->reportItemsPerPage) $this->itemsPages = 1;
			elseif($this->itemsTotal/$this->reportItemsPerPage == floor($this->itemsTotal/$this->reportItemsPerPage)) $this->itemsPages = $this->itemsTotal/$this->reportItemsPerPage;
			else $this->itemsPages = floor($this->itemsTotal/$this->reportItemsPerPage)+1;
			
			$res = mysql_query("select t2.id AS id, t2.Name, SUM(t1.Traffic) as Sum from Log_IP t1, Services t2 where t1.Proto = t2.Proto and t1.Port = t2.Port and Date>='$this->date1' and Date<='$this->date2' group by t2.id order by Sum desc $this->sql3");
			?>
			
			<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
			<tr bgcolor="#dddddd" align="center">
				<td class="t1" width="40%"><strong>Сервис</strong></td>
				<td class="t1" width="30%"><strong>Объем: <?=$this->convertTraffic($this->trafficSum)?></strong></td>
				<td class="t1" width="30%"><strong>Гистограмма</strong></td>
			</tr>
		
			<?
			while($row = mysql_fetch_array($res))
			{
				?>
		
				<tr bgcolor="#ffffff" class="t1">
				
					<td><a href="<?=$this->URL?>&detailView=true&detailParam=<?=$row['id']?>" class="t1"><?=$row['Name']?></a></td>
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
	
				?> </div> <?
			}
		}
		else
		{
			$res = mysql_query("SELECT SUM(t1.Traffic) AS Sum FROM Log_IP t1, Services t2 WHERE t1.$this->sql1 t1.Proto = t2.Proto AND t1.Port = t2.Port AND Date >= '$this->date1' AND Date <= '$this->date2'");
			$this->trafficSum = mysql_result($res,0);
	
			$res = mysql_query("select t2.id AS id, t2.Name, SUM(t1.Traffic) as Sum from Log_IP t1, Services t2 where t1.$this->sql1 t1.Proto = t2.Proto and t1.Port = t2.Port and Date>='$this->date1' and Date<='$this->date2' group by t2.id");
			$this->itemsTotal = mysql_num_rows($res);
			
			if($this->itemsTotal < $this->reportItemsPerPage) $this->itemsPages = 1;
			elseif($this->itemsTotal/$this->reportItemsPerPage == floor($this->itemsTotal/$this->reportItemsPerPage)) $this->itemsPages = $this->itemsTotal/$this->reportItemsPerPage;
			else $this->itemsPages = floor($this->itemsTotal/$this->reportItemsPerPage)+1;
			
			$res = mysql_query("select t2.id AS id, t2.Name, SUM(t1.Traffic) as Sum from Log_IP t1, Services t2 where t1.$this->sql1 t1.Proto = t2.Proto and t1.Port = t2.Port and Date>='$this->date1' and Date<='$this->date2' group by t2.id order by Sum desc $this->sql3");
			?>
			
			<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
			<tr bgcolor="#dddddd" align="center">
				<td class="t1" width="40%"><strong>Сервис</strong></td>
				<td class="t1" width="30%"><strong>Объем: <?=$this->convertTraffic($this->trafficSum)?></strong></td>
				<td class="t1" width="30%"><strong>Гистограмма</strong></td>
			</tr>
		
			<?
			while($row = mysql_fetch_array($res))
			{
				?>
		
				<tr bgcolor="#ffffff" class="t1">
				
					<td><?=$row['Name']?></td>
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
	
				?> </div> <?
			}		
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
		$res = mysql_query("SELECT Name FROM Services WHERE id=".$this->detailParam);
		$caption = mysql_result($res,0);
		
		$res = mysql_query("select SUM(t1.Traffic) as Sum from Log_IP t1, Services t2 where t1.Proto = t2.Proto and t1.Port = t2.Port and t2.id = $this->detailParam and t1.Date>='$this->date1' and t1.Date<='$this->date2' group by t2.id order by Sum desc");
		$this->trafficSum = mysql_result($res,0);
		
		$res = mysql_query("select t1.id_user, t2.Name, SUM(t1.Traffic) as Sum from Log_IP t1, Users t2, Services t3 where t2.id = t1.id_user and t3.id = $this->detailParam and t1.Proto = t3.Proto and t1.Port = t3.Port and t1.Date>='$this->date1' and t1.Date <='$this->date2' group by t1.id_user order by sum desc");
		?>
		
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		<tr bgcolor="#dddddd" align="center">
			<td class="t1" colspan="3"><strong><?=$caption?></strong></td>
		</tr>
		<tr bgcolor="#ffffff" align="center">
			<td class="t1" colspan="3"><img src="../design/pics/clear.gif" width=1 height=5 border=0 alt=""><br></td>
		</tr>
		<tr bgcolor="#dddddd" align="center">
			<td class="t1" width="40%"><strong>Пользователь</strong></td>
			<td class="t1" width="30%"><strong>Объем</strong></td>
			<td class="t1" width="30%"><strong>Гистограмма</strong></td>
		</tr>
		
		<?
		while($row = mysql_fetch_array($res))
		{
			?>
		
			<tr bgcolor="#ffffff" class="t1">
				<td align="center"><?=$row['Name']?></td>
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
		
		</body>
		</html>

		<?		
	}
}
?>