<?
include_once("../classes/class.reports.php");

class UserReports extends Reports
{
	var $className = "������";
	var $classVersion = "1.0";
	var $fileName = "user_reports.php";
	var $sortNum = 1;

	var $arrReportList = array();
	var $method = "";
	var $period = 1;
	var $user = 0;
	var $date = "";

	var $reportType = "";
	var $reportLessThan = false;
	var $reportLessThanValue = 50;
	var $reportItemsPerPage = 24;
	
	function UserReports($GET = null)
	{
		if(isset($GET['method'])) $this->method = $GET['method'];
		if(isset($GET['user'])) $this->user = $GET['user'];
		if(isset($GET['period'])) $this->period = $GET['period'];
		if(isset($GET['reportType'])) $this->reportType = $GET['reportType'];
		if(isset($GET['reportLessThan'])) $this->reportLessThan = true;
		if(isset($GET['reportLessThanValue'])) $this->reportLessThanValue = $GET['reportLessThanValue'];
		if(isset($GET['reportItemsPerPage'])) $this->reportItemsPerPage = $GET['reportItemsPerPage'];
		
		isset($GET['date']) ? $this->date = $GET['date'] : $this->date = date("d.m.Y")." - ".date("d.m.Y");
	}
	
	function displayContent()
	{
		if($this->is_connected)
		{
			?>

			<script language=JavaScript>
			function check() 
			{
				var err = "";

				if(!isNaN(f.reportItemsPerPage.value))
				{
					if(f.reportItemsPerPage.value < 1 || f.reportItemsPerPage.value > 250) err = "������� ������� ���������� ������� �� �������� (1...250)";
				}
				else err = "������� ������� ���������� ������� �� �������� (1...250)";

				if(f.reportLessThan.checked)
				{
					if(!isNaN(f.reportLessThanValue.value))
					{
						if(f.reportLessThanValue.value < 1 || f.reportLessThanValue.value > 10000) err = "������� ������� ���������� ������� �� �������� (1...10000)";
					}
					else err = "������� ������� ���������� ������� �� �������� (1...10000)";
				}
				
				if(f.period.value == 5)
				{
					f.date.value = f.date.value.replace(/ +$/g, "");
					
					tmp = f.date.value.split(/-/);
					tmp[0] = tmp[0].replace(/ +$/g, "");
					tmp[1] = tmp[1].replace(/^ +/g, "");
					
					arr = tmp[0].split(/\./);
					if(isNaN(arr[0]) || isNaN(arr[0]) || isNaN(arr[0])) err = '������ � ���� ������ �������';
					else
					{
						var d1 = eval(arr[0]);
						var m1 = eval(arr[1]);
						var y1 = eval(arr[2]);
						
						if(d1<1 || d1>31 || m1<1 || m1>12 || y1<2002) err = '������ � ���� ������ �������';
					}
			
					arr = tmp[1].split(/\./);
					if(isNaN(arr[0]) || isNaN(arr[0]) || isNaN(arr[0])) err = '������ � ���� ����� �������';
					else
					{
						var d2 = eval(arr[0]);
						var m2 = eval(arr[1]);
						var y2 = eval(arr[2]);
						
						if(d2<1 || d2>31 || m2<1 || m2>12 || y1<2002) err = '������ � ���� ����� �������';
					}
					
					if(err == "") 
					{
						date1 = (y1-2002)*365 + m1*31 + d1;
						date2 = (y2-2002)*365 + m2*31 + d2;
						if(date2<date1) err = '������� ����� ������';
					}
				}
				
				var reportType = '';
				for(i=0;i<f.elements.length;i++) if(f.elements[i].type == 'radio' && f.elements[i].checked) reportType = f.elements[i].value;
				
				var s = '';
				if(f.reportLessThan.checked) s = '&reportLessThan=1&reportLessThanValue='+f.reportLessThanValue.value;
				s = s + '&reportItemsPerPage='+f.reportItemsPerPage.value;
				
				if(err == "") window.open('<?=$this->fileName?>?method=processingCreateReport&user='+f.user.value+'&period='+f.period.value+'&date='+f.date.value+'&reportType='+reportType+s,'','resizable=yes,scrollbars=yes,height=600,width=550,left='+(screen.width/2-275)+',top='+(screen.height/2-300));
				else alert(err);
			}
			
			function SelectPeriod() 
			{
				if(f.period.value == 5) document.all['CustomPeriod'].style.display = "inline";
				else document.all['CustomPeriod'].style.display = "none";
			}
			</script>	
			
			<?
			$res = mysql_query("SELECT * FROM Users WHERE Addr='".$_SERVER['REMOTE_ADDR']."'");
			if(mysql_num_rows($res) == 1)
			{
				$row = mysql_fetch_array($res);
				$this->user = $row['id'];
				?>
					
				<table border="0" cellpadding="0" cellspacing="0" width="100%">
				<tr valign="top">
					<td class="t1" width="50%">
		
					<form name="f" style="margin-top: 0px; margin-bottom: 0px;">
					
					<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
					<tr bgcolor="#dddddd">
						<td colspan="2"><div class="t1"><b>���������</b></div></td>
					</tr>
					<tr bgcolor="#ffffff" class="t1">
						<td width="30%">������������</td>
						<td><?=$row['Name']?></td>
					</tr>
					<tr bgcolor="#ffffff" class="t1">
						<td>IP-�����</td>
						<td><?=$row['Addr']?></td>
					</tr>
					<tr bgcolor="#ffffff" class="t1">
						<td>�����</td>
						<td><? if($row['Limitation'] == -1) echo "�����������"; else echo $this->convertTraffic($row['Limitation'])?></td>
					</tr>
					<tr bgcolor="#ffffff" class="t1">
						<td><strong>�������</strong></td>
						<td><strong><? if($row['Limitation'] == -1) echo "�����������"; else echo $this->convertTraffic($row['Limitation']-$row['Traffic'])?></strong></td>
					</tr>
					<tr bgcolor="#ffffff" class="t1">
						<td width="30%">������</td>
						<td><a href="#" onclick="javascript:window.open('<?=$this->fileName?>?method=displayChangePasswd','','resizable=no,scrollbars=no,height=208,width=300,left='+(screen.width/2-150)+',top='+(screen.height/2-104))" class="t1"><strong>������� ������</strong></a></td>
					</tr>	
					</table>
					<br>
					
					<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
					<tr bgcolor="#dddddd">
						<td colspan="2"><div class="t1"><b>������</b></div></td>
					</tr>
					<tr bgcolor="#ffffff" class="t1">
						<td width="30%">����� �� ������</td>
						<td><?=$this->convertTraffic($row['Traffic'])?></td>
					</tr>
					<tr bgcolor="#ffffff" class="t1">
						<td>�� ������� �����</td>
						<td><?=$this->convertTraffic($row['TMonth'])?></td>
					</tr>
					<tr bgcolor="#ffffff" class="t1">
						<td>�� ���������� �����</td>
						<td><?=$this->convertTraffic($row['YMonth'])?></td>
					</tr>
					<tr bgcolor="#ffffff" class="t1">
						<td>�� �������</td>
						<td><?=$this->convertTraffic($row['TDay'])?></td>
					</tr>
					<tr bgcolor="#ffffff" class="t1">
						<td>�� �����</td>
						<td><?=$this->convertTraffic($row['YDay'])?></td>
					</tr>
					</table>
					<br>
					
					<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
					<tr bgcolor="#dddddd">
						<td colspan="2"><div class="t1"><b>�����</b></div></td>
					</tr>
					<tr bgcolor="#ffffff" class="t1">
						<td width="30%">������ ������</td>
						<td>
						<select name="period" class="t1" style="width: 100%" onchange="SelectPeriod()">
						
							<?
							$s1 = date("01/m/Y - d/m/Y");
							$s2 = mktime(0,0,0,date("m"),1,date("Y"))."-".mktime(0,0,0,date("m"),date("d"),date("Y"));
							?>
							<option value="1" <? if($this->period == 1) echo "selected"; ?>>�� ������� ����� (<?=$s1?>)</option>
	
							<?
							$d = mktime(0, 0, 0, date("m"), 0, date("Y"));
							$m = date("m")-1; $y = date("Y");
							if($m == 0) { $m = 12; $y--; }
							
							$m = sprintf("%02d",$m);
	
							$s1 = "01/$m/$y - ".strftime("%d", $d)."/$m/$y";
							$s2 = mktime(0,0,0,$m,1,$y)."-".$d;
							?>
							<option value="2" <? if($this->period == 2) echo "selected"; ?>>�� ���������� ����� (<?=$s1?>)</option>
	
							<?						
							$s1 = date("d/m/Y");
							$s2 = mktime(0,0,0,date("m"),date("d"),date("Y"))."-".mktime(0,0,0,date("m"),date("d"),date("Y"));
							?>
							<option value="3" <? if($this->period == 3) echo "selected"; ?>>�� ������� (<?=$s1?>)</option>
	
							<? 
							$s1 = date("d/m/Y", strtotime("-1 day")); 
							$s2 = mktime(0,0,0,date("m"),date("d")-1,date("Y"))."-".mktime(0,0,0,date("m"),date("d")-1,date("Y"));
							?>						
							<option value="4" <? if($this->period == 4) echo "selected"; ?>>�� ����� (<?=$s1?>)</option>
							
							<option value="5" <? if($this->period == 5) echo "selected"; ?>>�� ������ ... </option>
						</select>
						</td>
					</tr>
					<tr bgcolor="#ffffff" class="t1" id="CustomPeriod" style="display:none">
						<td>&nbsp;</td>
						<td><input type="text" name="date" size="9" maxlength="23" value="<?=$this->date?>" style="width: 100%"></td>
					</tr>
					
					<?
					if(count($this->arrReportList) > 0)
					{
						foreach($this->arrReportList as $item)
						{
							if(empty($this->reportType)) $this->reportType = $this->arrReportList[0]['type'];
							?>
							
							<tr bgcolor="#ffffff" class="t1">
								<td colspan="2"><input type="radio" style="margin: -1 -1 -1 -1" name="reportType" value="<?=$item['type']?>" <? if($this->reportType == $item['type']) echo "checked"; ?>>&nbsp;<?=$item['name']?></td>
							</tr>
							
							<?
						}
					}
					else
					{
						?>
						
						<tr bgcolor="#ffffff" class="t1">
							<td>��� ��������� �������</td>
						</tr>
						
						<?					
					}
					?>
					
					</table>
					<br>
		
					<script language=JavaScript>
						SelectPeriod();
					</script>
		
					<?
					if(count($this->arrReportList) > 0)
					{
						?>
	
						<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
						<tr bgcolor="#dddddd">
							<td colspan="2"><div class="t1"><b>�������������</b></div></td>
						</tr>
						<tr bgcolor="#ffffff" class="t1">
							<td width="50%"><input type="checkbox" name="reportLessThan" value="1" <? if($this->reportLessThan) echo "checked"; ?>> �� ���������� ������� ����� ... ��</td>
							<td><input type="text" name="reportLessThanValue" size="4" maxlength="5" value="<?=$this->reportLessThanValue?>" style="width:100%" ></td>
						</tr>
						<tr bgcolor="#ffffff" class="t1">
							<td>���������� ������� �� ��������</td>
							<td><input type="text" name="reportItemsPerPage" size="4" maxlength="4" value="<?=$this->reportItemsPerPage?>" style="width:100%" ></td>
						</tr>
						</table>
						<br>
							
						<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
						<tr bgcolor="#dddddd">
							<td align="center"><input type="button" name="create" value="������������" onclick="check(); return false;"></td>
						</tr>
						</table>
					
						<input type="hidden" name="user" value="<?=$this->user?>">
						<input type="hidden" name="method" value="processingCreateReport">
						
						<?
					}
					?>
					
					</form>
				
					</td>
					<td width="10"><img src="../design/pics/clear.gif" width=10 height=5 border=0 alt=""><br></td>
					<td width="60%" class="t1">&nbsp;</td>
					
				</tr>
				</table>
								
				<?
			}
			else $this->displayError("���������� ���������� IP-�����");
		}
		else $this->displayError("MySQL ������ ����������");
	}
	
	function buildReportList()
	{
		if($handle = opendir("../classes")) 
		{
		   	while (false !== ($file = readdir($handle))) 
			{
				if(ereg("^report_",$file)) 
				{
					$arr = explode(".",$file);
					include_once("../classes/".$file);

					$item = new $arr[0];
					if(!$item->displayRootOnly) $this->arrReportList[] = array("name"=>$item->getClassName(), "type"=>$item->getFileName());
					unset($item);
				}
			}
		    closedir($handle); 
		}
	}
	
	function processingCreateReport($GET)
	{
		$arr = explode(".",$this->reportType);
		$report = new $arr[0]($GET);
		
		$report->setReportParam();

		if($report->detailView) $report->displayReportDetail();
		else $report->displayReport();
		
		unset($report);
	}
	
	function displayChangePasswd($GET)
	{
		?>
	
		<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
		
		<html>
		<head>
			<title>�������� ������</title>
			<link rel=stylesheet type="text/css" href="../design/styles/style.css">
		</head>
		
		<body leftmargin="0" topmargin="0">
		
		<script language=JavaScript>
		function check() 
		{
			if(f.oldPasswd.value != '' && f.newPasswd1.value != '' && f.newPasswd2.value != '') 
			{
				if(f.newPasswd1.value == f.newPasswd2.value != '') f.submit();
				else alert('�� ��������� ����� ������ � �������������');
			}
			else alert('���������, ����������, ��� ����');
		}
		</script>					
		
		<form name="f" action="<?=$this->fileName?>" method="get" style="margin-top: 0px; margin-bottom: 0px;">
		<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
		
			<tr>
				<td><div class="t1" align="center"><b>�������� ������</b></div></td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>������ ������</b></div></td>
			</tr>	
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="password" name="oldPasswd" size="20" maxlength="100" style="width: 100%" value=""></div></td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>����� ������</b></div></td>
			</tr>	
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="password" name="newPasswd1" size="20" maxlength="100" style="width: 100%" value=""></div></td>
			</tr>

			<tr bgcolor="#dddddd">
				<td width="100%"><div class="t1"><b>������������� ������</b></div></td>
			</tr>	
			<tr bgcolor="#ffffff">
				<td><div class="t1"><input type="password" name="newPasswd2" size="20" maxlength="100" style="width: 100%" value=""></div></td>
			</tr>

			<tr bgcolor="#dddddd">
				<td align="center"><input type="submit" name="btnChangePasswd" value="��������" onClick="check(); return false">&nbsp;&nbsp;&nbsp;<input type="button" onclick="javascript:window.close();" name="close" value="�������"></td>
			</tr>
		
		</table>
		<input type="hidden" name="method" value="processingChangePasswd">
		</form>
		
		</body>
		</html>
	
		<?	
	}	

	function processingChangePasswd($GET)
	{
		$errMsg = "";
		
		$res = mysql_query("SELECT * FROM Users WHERE Addr = '".$_SERVER['REMOTE_ADDR']."' AND Passwd = '".$GET['oldPasswd']."'");
		if(mysql_num_rows($res) == 1)
		{
			mysql_query("UPDATE Users SET Passwd = '".$GET['newPasswd1']."' WHERE Addr = '".$_SERVER['REMOTE_ADDR']."'");
			$errMsg = "������ ������� ��������";
		}
		else $errMsg = "�� ������� ������� ������ ������";
		?>
		
		<script language=JavaScript>
			alert('<?=$errMsg?>');
			window.close();
		</script>
		
		<?	
	}		
}
?>
