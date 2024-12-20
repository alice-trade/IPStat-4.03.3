<?
include_once("../classes/main.report.class.php");

class Report_SQUID extends MainReport
{
	var $className = "Отчет по URL-адресам";
	var $classVersion = "1.0";
	var $fileName = "report_squid.class.php";
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
		$res = mysql_query("select SUM(Traffic) AS Sum from Log_Squid where $this->sql1 Time>='$this->date1' and Time<='$this->date2'");
		$this->trafficSum = mysql_result($res,0);
		
		$res = mysql_query("select REPLACE(REPLACE(SUBSTRING_INDEX(URL, '/', 3), 'http://',''),'www.','') as Address, SUM(Traffic) as Sum from Log_Squid where $this->sql1 Time>='$this->date1' and Time<='$this->date2' group by Address $this->sql2 order by Sum desc");
		$this->itemsTotal = mysql_num_rows($res);
		
		if($this->itemsTotal < $this->reportItemsPerPage) $this->itemsPages = 1;
		elseif($this->itemsTotal/$this->reportItemsPerPage == floor($this->itemsTotal/$this->reportItemsPerPage)) $this->itemsPages = $this->itemsTotal/$this->reportItemsPerPage;
		else $this->itemsPages = floor($this->itemsTotal/$this->reportItemsPerPage)+1;
		
		$res = mysql_query("select REPLACE(REPLACE(SUBSTRING_INDEX(URL, '/', 3), 'http://',''),'www.','') as Address, SUM(Traffic) as Sum from Log_Squid where $this->sql1 Time>='$this->date1' and Time<='$this->date2' group by Address $this->sql2 order by Sum desc $this->sql3");		
		?>
		
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		<tr bgcolor="#dddddd" align="center">
			<td class="t1" width="40%"><strong>Адрес</strong></td>
			<td class="t1" width="30%"><strong>Объем: <?=$this->convertTraffic($this->trafficSum)?></strong></td>
			<td class="t1" width="30%"><strong>Гистограмма</strong></td>
		</tr>
	
		<?
		while($row = mysql_fetch_array($res))
		{
			?>
	
			<tr bgcolor="#ffffff" class="t1">
			
				<td><a href="<?=$this->URL?>&detailView=true&detailParam=<?=$row['Address']?>" class="t1"><?=$row['Address']?></a></td>
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
		if($this->user == 0)
		{
			$res = mysql_query("select SUM(t1.Traffic) AS Sum from Log_Squid t1, Users t2 where t2.id=t1.id_user and REPLACE(REPLACE(SUBSTRING_INDEX(t1.URL, '/', 3), 'http://',''),'www.','')='$this->detailParam' and t1.Time>='$this->date1' and t1.Time<='$this->date2'");
			$this->trafficSum = mysql_result($res,0);
			
			$res = mysql_query("select t2.Name, SUM(t1.Traffic) AS Sum from Log_Squid t1, Users t2 where t2.id=t1.id_user and REPLACE(REPLACE(SUBSTRING_INDEX(t1.URL, '/', 3), 'http://',''),'www.','')='$this->detailParam' and t1.Time>='$this->date1' and t1.Time<='$this->date2' group by t1.id_user order by Sum DESC");
			?>
			
			<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
			<tr bgcolor="#dddddd" align="center">
				<td class="t1" colspan="3"><strong><?=$this->detailParam?></strong></td>
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
			
			<?		
		}
		else
		{
			?>
			
			<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
			<tr bgcolor="#dddddd" align="center">
				<td class="t1" colspan="3"><strong><?=$this->detailParam?></strong></td>
			</tr>
	
			<?
			$res = mysql_query("select DATE_FORMAT(Time, '%d.%m.%Y %H:%i') AS T, Url, Traffic from Log_Squid where Time>='$this->date1' and Time<'$this->date2' and REPLACE(REPLACE(SUBSTRING_INDEX(URL, '/', 3), 'http://',''),'www.','')='$this->detailParam' order by Time");
	
			while($row = mysql_fetch_array($res))
			{
				$url = $row['Url'];
				if(strlen($url)>40)
				{
					$arr = explode("/",$url);
	
					$url = $arr[0];
					$c = 40;
					
					for($i=1;$i<count($arr);$i++)
					{
						$url = $url."/".$arr[$i];
						if(strlen($url)>$c) 
						{
							$url .= "<br>";
							$c+=40;
						}
					}
				}
				?>
			
				<tr bgcolor="#ffffff" class="t1">
					<td align="center"><nobr><?=$row['T']?></nobr></td>
					<td align="center"><?=$url?></td>
					<td align="center"><?=$this->convertTraffic($row['Traffic'])?></td>
				</tr>
			
				<?
			}
			?>
				
			</table>
			<br>
			
			<?		
		}
		?>
		
		</body>
		</html>

		<?		
	}
}
?>