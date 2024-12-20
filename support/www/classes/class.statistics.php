<?
include_once("../classes/main.class.php");

class Statistics extends Main
{
	var $className = "Статистика";
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
						if($this->type == 1) echo "<strong>Общая</strong> | <strong><a href=$this->fileName?type=2 class=t1>По группам</a></strong> | <strong><a href=$this->fileName?type=3 class=t1>По алфавиту</a></strong>";
						if($this->type == 2) echo "<strong><a href=$this->fileName?type=1 class=t1>Общая</a></strong> | <strong>По группам</strong> | <strong><a href=$this->fileName?type=3 class=t1>По алфавиту</a></strong>";
						if($this->type == 3) echo "<strong><a href=$this->fileName?type=1 class=t1>Общая</a></strong> | <strong><a href=$this->fileName?type=2 class=t1>По группам</a></strong> | <strong>По алфавиту</strong>";
						if($this->type == 4) echo "<strong><a href=$this->fileName?type=1 class=t1>Общая</a></strong> | <strong><a href=$this->fileName?type=2 class=t1>По группам</a></strong> | <strong><a href=$this->fileName?type=3 class=t1>По алфавиту</a></strong>";
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
								<td width="25%"><strong>Группы</strong></td>
								<td width="25%"><strong>Пользователи</strong></td>
								<td width="25%"><strong>Выключено</strong></td>
								<td width="25%"><strong>Заблокировано</strong></td>
							</tr>
							<tr bgcolor="#ffffff" class="t1" align="center">
							
							<?
							$res = mysql_query("SELECT COUNT(*) FROM Group_Users");
							echo "<td>Всего групп: ".mysql_result($res,0)."</td>";
							$res = mysql_query("SELECT COUNT(*) FROM Users");
							echo "<td>Всего пользователей: ".mysql_result($res,0)."</td>";
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
								<td width="25%"><strong>Траффик за месяц</strong></td>
								<td width="25%"><strong>За пред. месяц</strong></td>
								<td width="25%"><strong>За сегодня</strong></td>
								<td width="25%"><strong>За вчера</strong></td>
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
											<td width="100"><strong>IP-адрес</strong></td>
											<td width="75"><strong>Период</strong></td>
											<td width="100"><strong>Лимит</strong></td>
											<td width="75"><strong>Закачано <br>за период</strong></td>
											<td width="75"><strong>Закачано за<br>прош. месяц</strong></td>
											<td width="75"><strong>Закачано<br>сегодня</strong></td>
											<td width="50"><strong>Статус</strong></td>
										</tr>
										
										<?
										$c = 1;
										while($row_us = mysql_fetch_array($res_us))
										{
											if($row_us['Type'] == 3) $period = "месяц";
											if($row_us['Type'] == 2) $period = "неделя";
											if($row_us['Type'] == 1) $period = "день";
											if($row_us['Type'] == 0) $period = "неограничен";

											$row_us['Limitation'] == -1 ? $limit = "неограничен" : $limit = $this->convertTraffic($row_us['Limitation']);
											
											$trafficPeriod += $row_us['Traffic'];
											$trafficPrevPeriod += $row_us['YMonth'];
											$trafficToday += $row_us['TDay'];
											
											$row_us['onActive'] == 1 ? $status = "<img src=../design/pics/p-on.gif alt=\"Включен\">" : $status = "<img src=../design/pics/p-off.gif alt=\"Выключен\">";
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
											<td width="100%"><div class="t1">Пусто</div></td>
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
									<td><a href=<?=$this->fileName?>?type=3&sort=name class=t1><strong>Пользователь</strong></a></td>
									<td width="100"><a href=<?=$this->fileName?>?type=3&sort=addr class=t1><strong>IP-адрес</strong></a></td>
									<td width="75"><strong>Период</strong></td>
									<td width="100"><strong>Лимит</strong></td>
									<td width="75"><a href=<?=$this->fileName?>?type=3&sort=traffic class=t1><strong>Закачано за <br>период</strong></a></td>
									<td width="85"><a href=<?=$this->fileName?>?type=3&sort=traffic_ymonth class=t1><strong>Закачано за<br>прошл. месяц</strong></a></td>
									<td width="75"><a href=<?=$this->fileName?>?type=3&sort=traffic_today class=t1><strong>Закачано<br>сегодня</strong></a></td>
									<td width="50"><strong>Статус</strong></td>
								</tr>
								
								<?
								$c = 1;
								$total_traffic = 0;
								
								while($row_us = mysql_fetch_array($res_us))
								{
									if($row_us['Type'] == 3) $period = "месяц";
									if($row_us['Type'] == 2) $period = "неделя";
									if($row_us['Type'] == 1) $period = "день";
									if($row_us['Type'] == 0) $period = "неограничен";

									$row_us['Limitation'] == -1 ? $limit = "неограничен" : $limit = $this->convertTraffic($row_us['Limitation']);
									$total_traffic += $row_us['Traffic'];
									
									$row_us['onActive'] == 1 ? $status = "<img src=../design/pics/p-on.gif alt=\"Включен\">" : $status = "<img src=../design/pics/p-off.gif alt=\"Выключен\">";
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
	
							if($user['Type'] == 3) $period = "месяц";
							if($user['Type'] == 2) $period = "неделя";
							if($user['Type'] == 1) $period = "день";
							if($user['Type'] == 0) $period = "неограничен";
							if($user['Type'] == 4) 
							{
								$period = "---------";
								$limit = "по деньгам";
								$balance = $user['Balance']." у.е.";
							}
							else
							{
								$user['Limitation'] == -1 ? $limit = "неограничен" : $limit = $this->convertTraffic($user['Limitation']);
								$balance = "---------";
							}
								
							$user['onActive'] == 1 ? $status = "вкл." : $status = "<font color=#FF0000>выкл.</font>";
							?>						
							
							<table border="0" cellpadding="0" cellspacing="0" width="100%">
							<tr valign="top">
								<td width="30%">
							
								<table align="center" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4" width="100%">
								<tr bgcolor="#dddddd">
									<td colspan="2"><div class="t1"><b>Общая статистика</b></div></td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center">
									<td align="right"><strong>Имя</strong></td>
									<td align="left"><?=$user['Name']?>, <?=$user['Gr']?></td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center">
									<td align="right"><strong>IP-адрес</strong></td>
									<td align="left"><?=$user['Addr']?></td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center">
									<td align="right"><strong>Ограничение</strong></td>
									<td align="left"><? if($user['Type'] == 4) echo "$limit / $balance"; else echo "$limit / $period"; ?></td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center" valign="top">
									<td align="right"><strong>Закачано</strong></td>
									<td align="left">
										за период: <?=$this->convertTraffic($user['Traffic'])?><br>
										сегодня: <?=$this->convertTraffic($user['TDay'])?><br>
										вчера: <?=$this->convertTraffic($user['YDay'])?><br>
										прошедший месяц: <?=$this->convertTraffic($user['YMonth'])?>
									</td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center">
									<td align="right"><strong>Комментарий</strong></td>
									<td align="left"><?=$user['Comment']?></td>
								</tr>
								<tr bgcolor="#ffffff" class="t1" align="center">
									<td align="right"><strong>Статус</strong></td>
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
										<td colspan="6"><div class="t1"><b>Статистика по группам правил</b></div></td>
									</tr>
									<tr bgcolor="#dddddd" class="t1" align="center">
										<td><strong>Название</strong></td>
										<td><strong>За период</strong></td>
										<td><strong>Сегодня</strong></td>
										<td><strong>Вчера</strong></td>
										<td><strong>В этом месяце</strong></td>
										<td><strong>В прош. месяце</strong></td>
										<td><strong>Лимит</strong></td>
									</tr>
									
									<?
									while($row = mysql_fetch_array($res))
									{
										$row['Limitation'] == -1 ? $groupLimit = "" : $groupLimit = $this->convertTraffic($row['Limitation'], false);
										if($row['lim'] == 1) $row['close'] == 1 ? $ruleGroupLimit = "<img src=../design/pics/p-restrict-open.gif alt=\"Установлен лимит $groupLimit\">" : $ruleGroupLimit = "<img src=../design/pics/p-restrict.gif alt=\"Лимит израсходован\">";
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
										<td colspan="10"><div class="t1"><b>Статистика по отдельным правилам</b></div></td>
									</tr>
									<tr bgcolor="#dddddd" class="t1" align="center">
										<td><strong>Название</strong></td>
										<td><strong>Период</strong></td>
										<td><strong>Лимит траффика</strong></td>
										<td><strong>За период</strong></td>
										<td><strong>Сегодня</strong></td>
										<td><strong>Вчера</strong></td>
										<td><strong>В этом месяце</strong></td>
										<td><strong>В прош. месяце</strong></td>
										<td><strong>Состояние</strong></td>
										<td><strong>Блокировка</strong></td>
									</tr>
									
									<?
									while($row = mysql_fetch_array($res))
									{
										if($row['Type'] == 3) $period = "месяц";
										if($row['Type'] == 2) $period = "неделя";
										if($row['Type'] == 1) $period = "день";
										if($row['Type'] == 0) $period = "нет";
									
										$row['Limitation'] == -1 ? $limit = "неограничен" : $limit = $this->convertTraffic($row['Limitation']);
										$row['onActive'] == 1 ? $status = "вкл." : $status = "<font color=#FF0000>выкл.</font>";
										$row['onBlocked'] == 1 ? $block = "да" : $block = "нет";
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
		else $this->displayError("MySQL сервер недоступен");
	}
}
?>