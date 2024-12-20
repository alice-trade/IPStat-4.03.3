<?
class MainReport
{
	var $user = 0;
	var $sql1 = "";
	var $sql2 = "";
	var $sql3 = "";
	var $date = "";
	var $date1 = "";
	var $date2 = "";
	var $period = 1;
	var $URL = "";
	var $detailView = false;
	var $detailParam = "";
	
	var $reportLessThan = false;
	var $reportLessThanValue = 50;
	var $reportStartPage = 0;
	var $reportItemsPerPage = 24;
	
	function MainReport($GET = null)
	{
		global $cfg;

		isset($cfg['multiply_factor']) ? $this->multFactor = $cfg['multiply_factor'] : $this->multFactor = 1;		
		
		$this->user = $GET['user'];
		$this->period = $GET['period'];
		$this->date = $GET['date'];
		$this->reportItemsPerPage = $GET['reportItemsPerPage'];
		$this->URL = $_SERVER['PHP_SELF']."?".$_SERVER['QUERY_STRING'];
		
		if(isset($GET['reportStartPage'])) $this->reportStartPage = $GET['reportStartPage'];
		if(isset($GET['reportLessThan'])) $this->reportLessThan = true;
		if(isset($GET['reportLessThanValue'])) $this->reportLessThanValue = $GET['reportLessThanValue']*1024;

		if(isset($GET['detailView'])) $this->detailView = true;
		if(isset($GET['detailParam'])) $this->detailParam = $GET['detailParam'];;
	}
	
	function getClassName()
	{
		return($this->className);
	}

	function getFileName()
	{
		return($this->fileName);
	}
	
	function setReportParam()
	{
		if($this->user > 0) $this->sql1 = "id_user=$this->user and";
		if($this->reportLessThan) $this->sql2 = "having Sum>$this->reportLessThanValue";
		$this->sql3 = " LIMIT $this->reportStartPage, $this->reportItemsPerPage";
		
		if($this->period == 1)
		{
			$this->date1 = date("Y-m-1 00:00:00");
			$this->date2 = date("Y-m-d 23:59:59");
		}

		if($this->period == 2)
		{
			$d = mktime(0, 0, 0, date("m"), 0, date("Y"));
			$m = date("m")-1; $y = date("Y");
			if($m == 0) { $m = 12; $y--; }
		
			$this->date1 = "$y-$m-1 00:00:00";
			$this->date2 = "$y-$m-".strftime("%d", $d)." 23:59:59";
		}
		
		if($this->period == 3)
		{
			$this->date1 = date("Y-m-d 00:00:00");
			$this->date2 = date("Y-m-d 23:59:59");
		}

		if($this->period == 4)
		{
			$this->date1 = date("Y-m-d 00:00:00", strtotime("-1 day"));
			$this->date2 = date("Y-m-d 23:59:59", strtotime("-1 day"));
		}

		if($this->period == 5)
		{
			$arr = explode("-",$this->date);
			$arr[0] = trim($arr[0]);
			$arr[1] = trim($arr[1]);
			
			$this->date1 = substr($arr[0],6,4)."-".substr($arr[0],3,2)."-".substr($arr[0],0,2)." 00:00:00";
			$this->date2 = substr($arr[1],6,4)."-".substr($arr[1],3,2)."-".substr($arr[1],0,2)." 23:59:59";
		}
	}
	
	function convertTraffic($x)
	{
		$x *= $this->multFactor;
	
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
}
?>