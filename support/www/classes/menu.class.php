<?
class Menu
{
	var $strMenu = "";
	var $arrMenu = array();
	var $version = "1.0";
	var $displayMenu = true;
	
	function Menu($ver, $displayMenu = true)
	{
		$this->displayMenu = $displayMenu;
		
		if($handle = opendir("../classes")) 
		{
		   	while (false !== ($file = readdir($handle))) 
			{
				if(ereg("^class",$file)) 
				{
					$arr = explode(".",$file);
					include_once("../classes/".$file);

					$item = new $arr[1];
					$this->arrMenu[] = array("name"=>$item->getClassName(), "file"=>$item->getFileName(), "sortNum"=>$item->sortNum);
					unset($item);
				}
			}
		    closedir($handle); 
		}
		
		usort($this->arrMenu, array($this, "cmpElem"));

		$this->strMenu = <<< endl
		
			<tr><td bgcolor="#dddddd"><img src="../design/pics/clear.gif" width=1 height=7 border=0 alt=""><br></td></tr>
			<tr><td bgcolor="#ffffff" class="t1" height="50" style="background: #FFF url('../design/pics/logoipstat.gif') no-repeat"><div align="center" style="font-family: Times New Roman; font-size: 20pt; color: #bbbbbb"><strong>Административный интерфейс IPStat $ver</strong></div></td></tr>
		    <tr><td bgcolor="#dddddd"><img src="../design/pics/clear.gif" width=1 height=7 border=0 alt=""><br></td></tr>
			<tr><td>
			<table cellpadding="8" cellspacing="1" border="0" align="center" width="100%">
			<tr>
			<td bgcolor="#dddddd" width="50%">&nbsp;</td>
endl;

		if($this->displayMenu)
		{
			$arr = pathinfo($_SERVER['PHP_SELF']);
			$path = $arr['basename'];
			
			foreach($this->arrMenu as $item)
			{
				if($item['file'] == $path) $this->strMenu = $this->strMenu."<td bgcolor=\"#ffffff\" class=\"t1\" align=\"center\"><strong>".$item['name']."</strong></td>";
				else $this->strMenu = $this->strMenu."<td bgcolor=\"#dddddd\" align=\"center\"><a href=\"".$item['file']."\" class=\"t1\"><strong>".$item['name']."</strong></a></td>";
			}
		}
		
		$this->strMenu .= <<< endl

			<td bgcolor="#dddddd" width="50%">&nbsp;</td>
			</tr>
			</table>
			</td>
			</tr>
		    <tr><td bgcolor="#dddddd"><img src="../design/pics/clear.gif" width=1 height=7 border=0 alt=""><br></td></tr>
endl;

	}
	
	function cmpElem($a, $b) 
	{
    	if ($a['sortNum'] == $b['sortNum'])  return 0;
    	return ($a['sortNum'] < $b['sortNum']) ? -1 : 1;
	}
}
?>
