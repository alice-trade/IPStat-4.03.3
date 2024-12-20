<?
include_once("../classes/main.class.php");

class Policy extends Main
{
	var $className = "Политика";
	var $classVersion = "1.0";
	var $fileName = "policy.php";
	var $sortNum = 5;
	
	var $arrPolicyList = array();
	var $method = "";
	var $policyID = 0;
	var $type = 0;
	var $subClass = "";
	var $policy;
	var $GET;

	function Policy($GET = null)
	{
		if(isset($GET['type'])) $this->type = $GET['type'];
		if(isset($GET['method'])) $this->method = $GET['method'];
		if(isset($GET['policyID'])) $this->policyID = $GET['policyID'];
		$this->GET = $GET;
	}
	
	function buildReportList()
	{
		if($handle = opendir("../classes")) 
		{
		   	while (false !== ($file = readdir($handle))) 
			{
				if(ereg("^policy_",$file)) 
				{
					$arr = explode(".",$file);
					include_once("../classes/".$file);

					$item = new $arr[0];
					$this->arrPolicyList[] = array("name"=>$item->getClassName(), "file"=>$item->getFileName(), "type"=>$item->getType());
					unset($item);
				}
			}
		    closedir($handle); 
		}
		
		if($this->type == 0) $this->type = $this->arrPolicyList[0]['type'];

		foreach($this->arrPolicyList as $item)
		{
			if($this->type == $item['type']) 
			{
				$arr = explode(".",$item['file']);
				$this->subClass = $arr[0];
			}
		}
		
		if(!empty($this->subClass))
		{
			$this->policy = new $this->subClass();
			$this->policy->policyID = $this->policyID;
		}
	}
	
	function displayContent()
	{
		if($this->is_connected)
		{
			if(count($this->arrPolicyList)>0)
			{
				?>
	
				<table border="0" cellpadding="0" cellspacing="0" width="100%">			
					<tr>
						<td class="t1" align="center">
							
							<?
							$i = 0;
							foreach($this->arrPolicyList as $item)
							{
								if($i>0 && $i<count($this->arrPolicyList)) echo "&nbsp;|&nbsp;";
	
								if($this->type == $item['type']) echo "<strong>".$item['name']."</strong>";
								else echo "<strong><a href=$this->fileName?type=".$item['type']." class=t1>".$item['name']."</a></strong>";
								$i++;
							}
							?>
							
							<br><br>
						</td>
					</tr>
				</table>
				
				<?
				$this->policy->displayContent();
			}
			else $this->displayError("Нет доступных политик ограничения");
		}
		else $this->displayError("MySQL сервер недоступен");
	}
	
	function processingPolicy()
	{
		if($this->method == "displayAddPolicy") $this->policy->displayAddPolicy();
		if($this->method == "processingAddPolicy") $this->policy->processingAddPolicy($this->GET);
		
		if($this->method == "displayEditPolicy") $this->policy->displayEditPolicy();
		if($this->method == "processingEditPolicy") $this->policy->processingEditPolicy($this->GET);
		
		if($this->method == "processingDelPolicy")
		{
			$this->policy->processingDelPolicy();
			header("Location: $this->fileName?type=$this->type\n\n");
		}
	}
}
?>