<?
class Main
{
	var $content;
	var $menu;
	var $is_connected = false;
	var $remoteAddr;
	var $displayMenu = true;
	var $ver;
	var $multFactor;

	function getClassName()
	{
		return($this->className);
	}

	function getFileName()
	{
		return($this->fileName);
	}

	function setConnection($connect)
	{
		global $cfg;
		
		$this->is_connected = $connect;
		$this->remoteAddr = $_SERVER['REMOTE_ADDR'];
		
		isset($cfg['multiply_factor']) ? $this->multFactor = $cfg['multiply_factor'] : $this->multFactor = 1;		
	}
	
	function buildMenu($ver)
	{
		global $cfg;		
		
		include_once("../classes/menu.class.php");
		include_once("../include/config.php");

		$this->ver = $cfg['ver'];
		$this->menu = new Menu($this->ver, $this->displayMenu);
	}
	
	function readTemplate()
	{
		$this->buildMenu($this->ver);
		
		$this->content = file("../design/template.php");
		
		for($i=0;$i<count($this->content);$i++)
		{
			if(ereg("\[VER\]", $this->content[$i])) $this->content[$i] = ereg_replace("\[VER\]", $this->ver, $this->content[$i]);
			if(ereg("\[TITLE\]", $this->content[$i])) $this->content[$i] = ereg_replace("\[TITLE\]", $this->className, $this->content[$i]);
			if(ereg("\[MENU\]", $this->content[$i])) $this->content[$i] = $this->menu->strMenu;
		}
	}
	
	function displayTemplate()
	{
		for($i=0;$i<count($this->content);$i++) 
		{
			if(!ereg("\[CONTENT\]",$this->content[$i])) echo $this->content[$i];
			else $this->displayContent();
		}
	}
	
	function convertTraffic($x, $isConvert = true)
	{
		if($isConvert) $x *= $this->multFactor;
		
		if($x>1024) 
		{
			$x /= 1024;
			if($x>1024)
			{
				$x /= 1024;
				if($x>1024)
				{
					$x /= 1024;
					$x = round($x*100)/100;
					$x .= "G";
				}
				else
				{	
					$x = round($x*100)/100;
					$x .= "M";
				}
			}
			else 
			{
				$x = round($x*100)/100;
				$x .= "K";
			}
		}
		return($x);
	}
	
	function displayError($err)
	{
		?>
		
		<table border="0" cellpadding="0" cellspacing="0" width="100%">
		<tr valign="top">
			<td class="t1" width="100%">

			<table width="100%" bgcolor="#D55500" border="0" cellspacing="1" cellpadding="4">		
			<tr bgcolor="#FF6600">
				<td><div class="t1"><b>Ошибка</b></div></td>
			</tr>
			<tr bgcolor="#ffffff" class="t1">
				<td><?=$err?></td>
			</tr>
			</table>

			</td>
		</table>
		<br>
			
		<?
	}
	
	function writeLog($action)
	{
		mysql_query("INSERT INTO log_interface(date,ip,action) VALUES(NOW(),'$this->remoteAddr','$action')");
		if(mysql_error()) echo mysql_error();
	}
}
?>
