<?
include_once("../classes/main.class.php");

class Statistics extends Main
{
	var $className = "����������";
	var $classVersion = "1.0";
	var $fileName = "stat.php";
	var $sortNum = 2;
	
	var $type = 1;
	var $sort = "name";
	var $id_user = 0;
	
	function Statistics($GET = null)
	{
		if(isset($GET['type'])) $this->type = $GET['type'];
		if(isset($GET['id_user'])) $this->id_user = $GET['id_user'];
		if(isset($GET['sort'])) $this->sort = $GET['sort'];
	}
	
	function displayContent()
	{
		if($this->is_connected)
		{
			?>

			<table border="0" cellpadding="0" cellspacing="0" width="100%">
				<tr>
					<td class="t1" align="center">
						
						
						<?
						if($this->type == 1) echo "<strong>�����</strong> | <strong><a href=$this->fileName?type=2 class=t1>�� �������</a></strong> | <strong><a href=$this->fileName?type=3 class=t1>�� ��������</a></strong>";
						if($this->type == 2) echo "<strong><a href=$this->fileName?type=1 class=t1>�����</a></strong> | <strong>�� �������</strong> | <strong><a href=$this->fileName?type=3 class=t1>�� ��������</a></strong>";
						if($this->type == 3) echo "<strong><a href=$this->fileName?type=1 class=t1>�����</a></strong> | <strong><a href=$this->fileName?type=2 class=t1>�� �������</a></strong> | <strong>�� ��������</strong>";
						if($this->type == 4) echo "<strong><a href=$this->fileName?type=1 class=t1>�����</a></strong> | <strong><a href=$this->fileName?type=2 class=t1>�� �������</a></strong> | <strong><a href=$this->fileName?type=3 class=t1>�� ��������</a></strong>";
						?>
						
						</strong>
						<br><br>
					</td>
				</tr>
				
				<tr valign="top">
					<td>
						
						<?
						if($this->type == 1)
						{
							?>
							
							<table width="50%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
							<tr bgcolor="#dddddd" class="t1" align="center">
								<td width="25%"><strong>������</strong></td>
								<td width="25%"><strong>������������</strong></td>
								<td width="25%"><strong>���������</strong></td>
								<td width="25%"><strong>�������������</strong></td>
							</tr>
							<tr bgcolor="#ffffff" class="t1" align="center">
							
							<?
							$res = mysql_query("SELECT COUNT(*) FROM Group_Users");
							echo "<td>����� �����: ".mysql_result($res,0)."</td>";
							$res = mysql_query("SELECT COUNT(*) FROM Users");
							echo "<td>����� �������������: ".mysql_result($res,0)."</td>";
							$res = mysql_query("SELECT COUNT(*) FROM Users WHERE onActive='0'");
							echo "<td>".mysql_result($res,0)."</td>";
							$res = mysql_query("SELECT COUNT(*) FROM Users WHERE onBlocked='1'");
							echo "<td>".mysql_result($res,0)."</td>";
							?>

							</tr>
							</table>
							<br>
							
							<table width="50%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
							<tr bgcolor="#dddddd" class="t1" align="center">
								<td width="25%"><strong>������� �� �����</strong></td>
								<td width="25%"><strong>�� ����. �����</strong></td>
								<td width="25%"><strong>�� �������</strong></td>
								<td width="25%"><strong>�� �����</strong></td>
							</tr>
							<tr bgcolor="#ffffff" class="t1" align="center">
							
							<?
							$res = mysql_query("SELECT SUM(TMonth) FROM Users");
							echo "<td>".$this->convertTraffic(mysql_result($res,0))."</td>";
							$res = mysql_query("SELECT SUM(YMonth) FROM Users");
							echo "<td>".$this->convertTraffic(mysql_result($res,0))."</td>";
							$res = mysql_query("SELECT SUM(TDay) FROM Users");
							echo "<td>".$this->convertTraffic(mysql_result($res,0))."</td>";
							$res = mysql_query("SELECT SUM(YDay) FROM Users");
							echo "<td>".$this->convertTraffic(mysql_result($res,0))."</td>";
							?>
							
							</tr>
							</table>
							<br>

							<?
						}
						elseif($this->type == 2)
						{
							$res_gr = mysql_query("SELECT Group_Users.*, COUNT(*) FROM Group_Users, Users WHERE Group_Users.id = Users.id_group GROUP BY Group_Users.Name");
							if(mysql_num_rows($res_gr)>0)
							{
								while($row_gr = mysql_fetch_array($res_gr))
								{
									$trafficPeriod = 0;
									$trafficPrevPeriod = 0;
									$trafficToday = 0;
									?>
							
									<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
									
									<?
									$res_us = mysql_query("SELECT * FROM Users WHERE id_group='".$row_gr['id']."' ORDER BY Name");
									if(mysql_num_rows($res_us)>0)
									{
										?>
										
										<tr bgcolor="#dddddd" class="t1" align="center">
											<td width="20"><strong>#</strong></td>
											<td><strong><?=$row_gr['Name']?></strong></td>
											<td width="100"><strong>IP-�����</strong></td>
											<td width="75"><strong>������</strong></td>
											<td width="100"><strong>�����</strong></td>
											<td width="75"><strong>�������� <br>�� ������</strong></td>
											<td width="75"><strong>�������� ��<br>����. �����</strong></td>
											<td width="75"><strong>��������<br>�������</strong></td>
											<td width="50"><strong>������</strong></td>
										</tr>
										
										<?
										$c = 1;
										while($row_us = mysql_fetch_array($res_us))
										{
											if($row_us['Type'] == 3) $period = "�����";
											if($row_us['Type'] == 2) $period = "������";
											if($row_us['Type'] == 1) $period = "����";
											if($row_us['Type'] == 0) $period = "�����������";

											$row_us['Limitation'] == -1 ? $limit = "�����������" : $limit = $this->convertTraffic($row_us['Limitation']);
											
											$trafficPeriod += $row_us['Traffic'];
											$trafficPrevPeriod += $row_us['YMonth'];
											$trafficToday += $row_us['TDay'];
											
											$row_us['onActive'] == 1 ? $status = "<img src=../design/pics/p-on.gif alt=\"�������\">" : $status = "<img src=../design/pics/p-off.gif alt=\"��������\">";
											?>
							
											<tr bgcolor="#ffffff" class="t1">
												<td align="center"><?=$c?></td>
												<td><a href="<?=$this->fileName?>?type=4&id_user=<?=$row_us['id']?>" class=t1><?=$row_us['Name']?></a></td>
												<td align="center"><?=$row_us['Addr']?></td>
												<td align="center"><?=$period?></td>
												<td align="center"><?=$limit?></td>
												<td align="center"><?=$this->convertTraffic($row_us['Traffic'])?></td>
												<td align="center"><?=$this->convertTraffic($row_us['YMonth'])?></td>
												<td align="center"><?=$this->convertTraffic($row_us['TDay'])?></td>
												<td align="center"><?=$status?></td>
											</tr>
											
											<?
											$c++;
										}
									}
									else
									{
										?>
										
										<tr bgcolor="#ffffff">
											<td width="100%"><div class="t1">�����</div></td>
										</tr>
										
										<?
									}
									
									if(mysql_num_rows($res_us)>0)
									{
										?>
										
										<tr bgcolor="dddddd" class="t1" align="center">
											<td colspan="5">&nbsp;</td>
											<td><strong><?=$this->convertTraffic($trafficPeriod)?></strong></td>
											<td><strong><?=$this->convertTraffic($trafficPrevPeriod)?></strong></td>
											<td><strong><?=$this->convertTraffic($trafficToday)?></strong></td>
											<td>&nbsp;</td>
										</tr>
										
										<?
									}
									?>
									
									</table>
									<br>
									
									<?
								}
							}
						}
						elseif($this->type == 3)
						{
							if($this->sort == "name") $res_us = mysql_query("SELECT * FROM Users ORDER BY Name");
							if($this->sort == "addr") $res_us = mysql_query("SELECT * FROM Users ORDER BY Addr");
							if($this->sort == "traffic") $res_us = mysql_query("SELECT * FROM Users ORDER BY Traffic DESC");
							if($this->sort == "traffic_today") $res_us = mysql_query("SELECT * FROM Users ORDER BY TDay DESC");
							if($this->sort == "traffic_ymonth") $res_us = mysql_query("SELECT * FROM Users ORDER BY YMonth DESC");
							
							if(mysql_num_rows($res_us)>0)
							{
								?>
								
								<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
								<tr bgcolor="#dddddd" class="t1" align="center">
									<td width="20"><strong>#</strong></td>
									<td><a href=<?=$this->fileName?>?type=3&sort=name class=t1><strong>������������</strong></a></td>
									<td width="100"><a href=<?=$this->fileName?>?type=3&sort=addr class=t1><strong>IP-�����</strong></a></td>
									<td width="75"><strong>������</strong></td>
									<td width="100"><strong>�����</strong></td>
									<td width="75"><a href=<?=$this->fileName?>?type=3&sort=traffic class=t1><strong>�������� �� <br>������</strong></a></td>
									<td width="85"><a href=<?=$this->fileName?>?type=3&sort=traffic_ymonth class=t1><strong>�������� ��<br>�����. �����</strong></a></td>
									<td width="75"><a href=<?=$this->fileName?>?type=3&sort=traffic_today class=t1><strong>��������<br>�������</strong></a></td>
									<td width="50"><strong>������</strong></td>
								</tr>
								
								<?
								$c = 1;
								$total_traffic = 0;
								
								while($row_us = mysql_fetch_array($res_us))
								{
									if($row_us['Type'] == 3) $period = "�����";
									if($row_us['Type'] == 2) $period = "������";
									if($row_us['Type'] == 1) $period = "����";
									if($row_us['Type'] == 0) $period = "�����������";

									$row_us['Limitation'] == -1 ? $limit = "�����������" : $limit = $this->convertTraffic($row_us['Limitation']);
									$total_traffic += $row_us['Traffic'];
									
									$row_us['onActive'] == 1 ? $status = "<img src=../design/pics/p-on.gif alt=\"�������\">" : $status = "<img src=../design/pics/p-off.gif alt=\"��������\">";
									?>
						
									<tr bgcolor="#ffffff" class="t1">
										<td align="center"><?=$c?></td>
										<td><a href="<?=$this->fileName?>?type=4&id_user=<?=$row_us['id']?>" class=t1><?=$row_us['Name']?></a></td>
										<td align="center"><?=$row_us['Addr']?></td>
										<td align="center"><?=$period?></td>
										<td align="center"><?=$limit?></td>
										<td align="center"><?=$this->convertTraffic($row_us['Traffic'])?></td>
										<td align="center"><?=$this->convertTraffic($row_us['YMonth'])?></td>
										<td align="center"><?=$this->convertTraffic($row_us['TDay'])?></td>
										<td align="center"><?=$status?></td>
									</tr>
									
									<?
									$c++;
								}
								?>
								
								<tr bgcolor="dddddd" class="t1" align="center">
									<td colspan="5">&nbsp;</td>
									<td><strong><?=$this->convertTraffic($total_traffic)?></strong></td>
									<td colspan="3">&nbsp;</td>
								</tr>
								
								</table>
								<br>
								
								<?
							}
						}
						elseif($this->type == 4)
						{
							$res = mysql_query("SELECT Users.*, Group_Users.Name AS Gr FROM Users, Group_Users WHERE Users.id=$this->id_user AND Users.id_group = Group_Users.id");
							$user = mysql_fetch_array($res);
	
							if($user['Type'] == 3) $period = "�����";
							if($user['Type'] == 2) $period = "������";
							if($user['Type'] == 1) $period = "����";
							if($user['Type'] == 0) $period = "�����������";
							if($user['Type'] == 4) 
							{
								$period = "---------";
								$limit = "�� �������";
								$balance = $user['Balance']." �.�.";
							}
							else
							{
								$user['Limitation'] == -1 ? $limit = "�����������" : $limit = $this->convertTraffic($user['Limitation']);
								$balance = "---------";
							}
								
							$user['onActive'] == 1 ? $status = "���." : $status = "<font color=#FF0000>����.</font>";
							?>						
							
							<table border="0" cellpadding="0" cellspacing="0" width="100%">
							<tr valign="top">
								<td width="30%">
							
								<table align="center" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4" width="100%">
								<tr bgcolor="#dddddd">
									<td colspan="2"><div class="t1"><b>����� ����������</b></div></td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center">
									<td align="right"><strong>���</strong></td>
									<td align="left"><?=$user['Name']?>, <?=$user['Gr']?></td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center">
									<td align="right"><strong>IP-�����</strong></td>
									<td align="left"><?=$user['Addr']?></td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center">
									<td align="right"><strong>�����������</strong></td>
									<td align="left"><? if($user['Type'] == 4) echo "$limit / $balance"; else echo "$limit / $period"; ?></td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center" valign="top">
									<td align="right"><strong>��������</strong></td>
									<td align="left">
										�� ������: <?=$this->convertTraffic($user['Traffic'])?><br>
										�������: <?=$this->convertTraffic($user['TDay'])?><br>
										�����: <?=$this->convertTraffic($user['YDay'])?><br>
										��������� �����: <?=$this->convertTraffic($user['YMonth'])?>
									</td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center">
									<td align="right"><strong>�����������</strong></td>
									<td align="left"><?=$user['Comment']?></td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center">
									<td align="right"><strong>������</strong></td>
									<td align="left"><?=$status?></td>
								</tr>
								</table>
	
								<td width="10"><img src="../design/pics/clear.gif" width=10 height=5 border=0 alt=""><br></td>
								<td>
									
								<?
								$res = mysql_query("SELECT a.id_user, c.Name, SUM(a.Traffic) AS sumTotal, SUM(a.YDay) AS sumYDay, SUM(a.TDay) AS sumTDay, SUM(a.YMonth) AS sumYMonth, SUM(a.TMonth) AS sumTMonth, IF(ISNULL(d.id),0,1) as lim, IF (ISNULL(d.id), 0, d.onActive) as close, IF (ISNULL(d.id),0, d.Limitation) as Limitation FROM Rules a LEFT JOIN Type_Traffic b ON b.id = a.id_type LEFT JOIN Group_Traffic c ON c.id = b.id_group LEFT JOIN Rules_GroupTraffic d ON (d.id_group = b.id_group and d.id_user=a.id_user) WHERE a.id_user=$this->id_user AND a.Action != 2 GROUP BY b.id_group");
								if(mysql_num_rows($res)>0)
								{
									?>
									
									<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
									<tr bgcolor="#cccccc" align="center">
										<td colspan="6"><div class="t1"><b>���������� �� ������� ������</b></div></td>
									</tr>
									<tr bgcolor="#dddddd" class="t1" align="center">
										<td><strong>��������</strong></td>
										<td><strong>�� ������</strong></td>
										<td><strong>�������</strong></td>
										<td><strong>�����</strong></td>
										<td><strong>� ���� ������</strong></td>
										<td><strong>� ����. ������</strong></td>
										<td><strong>�����</strong></td>
									</tr>
									
									<?
									while($row = mysql_fetch_array($res))
									{
										$row['Limitation'] == -1 ? $groupLimit = "" : $groupLimit = $this->convertTraffic($row['Limitation'], false);
										if($row['lim'] == 1) $row['close'] == 1 ? $ruleGroupLimit = "<img src=../design/pics/p-restrict-open.gif alt=\"���������� ����� $groupLimit\">" : $ruleGroupLimit = "<img src=../design/pics/p-restrict.gif alt=\"����� ������������\">";
										else $ruleGroupLimit = "";
										?>
										
										<tr bgcolor="#ffffff" class="t1"  align="center">
											<td><?=$row['Name']?></td>
											<td><?=$this->convertTraffic($row['sumTotal'])?></td>
											<td><?=$this->convertTraffic($row['sumTDay'])?></td>
											<td><?=$this->convertTraffic($row['sumYDay'])?></td>
											<td><?=$this->convertTraffic($row['sumTMonth'])?></td>
											<td><?=$this->convertTraffic($row['sumYMonth'])?></td>
											<td><?=$ruleGroupLimit?></td>
										</tr>
										
										<?
									}
									?>
									
									</table>
									<br>

									<?									
								}
																
								$res = mysql_query("SELECT Rules.*, Type_Traffic.Name AS RuleName FROM Type_Traffic, Rules WHERE Rules.id_user=$this->id_user AND Type_Traffic.id=Rules.id_type ORDER BY Proto, Mask DESC, Ports DESC, Direct");
								if(mysql_num_rows($res)>0)
								{
									?>
									
									<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
									<tr bgcolor="#cccccc" class="t1" align="center">
										<td colspan="10"><div class="t1"><b>���������� �� ��������� ��������</b></div></td>
									</tr>
									<tr bgcolor="#dddddd" class="t1" align="center">
										<td><strong>��������</strong></td>
										<td><strong>������</strong></td>
										<td><strong>����� ��������</strong></td>
										<td><strong>�� ������</strong></td>
										<td><strong>�������</strong></td>
										<td><strong>�����</strong></td>
										<td><strong>� ���� ������</strong></td>
										<td><strong>� ����. ������</strong></td>
										<td><strong>���������</strong></td>
										<td><strong>����������</strong></td>
									</tr>
									
									<?
									while($row = mysql_fetch_array($res))
									{
										if($row['Type'] == 3) $period = "�����";
										if($row['Type'] == 2) $period = "������";
										if($row['Type'] == 1) $period = "����";
										if($row['Type'] == 0) $period = "���";
									
										$row['Limitation'] == -1 ? $limit = "�����������" : $limit = $this->convertTraffic($row['Limitation']);
										$row['onActive'] == 1 ? $status = "���." : $status = "<font color=#FF0000>����.</font>";
										$row['onBlocked'] == 1 ? $block = "��" : $block = "���";
										?>
										
										<tr bgcolor="#ffffff" class="t1"  align="center">
											<td><?=$row['RuleName']?></td>
											<td><?=$period?></td>
											<td><?=$limit?></td>
											<td><?=$this->convertTraffic($row['Traffic'])?></td>
											<td><?=$this->convertTraffic($row['TDay'])?></td>
											<td><?=$this->convertTraffic($row['YDay'])?></td>
											<td><?=$this->convertTraffic($row['TMonth'])?></td>
											<td><?=$this->convertTraffic($row['YMonth'])?></td>
											<td><?=$status?></td>
											<td><?=$block?></td>
										</tr>
										
										<?
									}
									?>
									
									</table>
									
									<?
								}
								?>						
									
								</td>
							</tr>								
							</table>

							<?
						}
						?>				
						
					</td>
				</tr>
			</table>
		
			<?	
		}
		else $this->displayError("MySQL ������ ����������");
	}
}
?>