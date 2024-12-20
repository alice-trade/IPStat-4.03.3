<?
include_once("../classes/main.class.php");

class Update extends Main
{
	var $className = "Обновление";
	var $classVersion = "1.0";
	var $fileName = "update.php";
	var $sortNum = 8;
	
	var $method = "";
	var $tmpDir = "../tmp";
	var $errMsg = "";
	
	function Update($POST = null)
	{
		if(isset($POST['method'])) $this->method = $POST['method'];
	}
	
	function displayContent()
	{
		if(!extension_loaded('zip')) $this->displayError("Не загружен модуль ZIP");
		else
		{
			?>
	
			<table border="0" cellpadding="0" cellspacing="0" width="100%">
			<tr valign="top">
				<td class="t1" width="50%">
	
				<form enctype="multipart/form-data" name="f" action="<?=$this->fileName?>" method="post" style="margin-top: 0px; margin-bottom: 0px;">
				
				<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
				<tr bgcolor="#dddddd">
					<td colspan="2"><div class="t1"><b>Обновление</b></div></td>
				</tr>
				<tr bgcolor="#ffffff" class="t1">
					<td width="30%">Файл (*.zip)</td>
					<td><input type="file" name="file" style="width: 100%"></td>
				</tr>
				</table>
				<br>
	
				<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
				<tr bgcolor="#dddddd">
					<td align="center"><input type="submit" name="btnUpload" value="Загрузить"></td>
				</tr>
				</table>
				<input type="hidden" name="method" value="processingUpload">
				</form>
			
				</td>
				<td width="10"><img src="../design/pics/clear.gif" width=10 height=5 border=0 alt=""><br></td>
				<td width="50%" class="t1">
				
				<?
				if($this->method == "processingUpload")
				{
					if(!is_dir($this->tmpDir)) mkdir($this->tmpDir);
					
					if($handle = opendir($this->tmpDir)) 
					{
					    while (false !== ($file = readdir($handle))) if ($file != "." && $file != "..") unlink($this->tmpDir."/".$file); 
					    closedir($handle); 
					}
					
					move_uploaded_file($_FILES['file']['tmp_name'], $this->tmpDir."/update.zip");
			
					$fileInstallFound = false;		
					$zip = @zip_open($_SERVER['DOCUMENT_ROOT'].ereg_replace("/content/update.php","",$_SERVER['PHP_SELF'])."/tmp/update.zip");
					
					if($zip) 
					{
					    while($zip_entry = zip_read($zip)) 
						{
							if(zip_entry_name($zip_entry) == "install") $fileInstallFound = true;
							
							$f = fopen($this->tmpDir."/".zip_entry_name($zip_entry), "wb");
					        if(zip_entry_open($zip, $zip_entry, "r")) 
							{
					            $buf = zip_entry_read($zip_entry, zip_entry_filesize($zip_entry));
								fwrite($f, $buf);
					            zip_entry_close($zip_entry);
					        }
							fclose($f);
					    }
					    zip_close($zip);
					}
					else $this->errMsg = "Невозможно распознать формат файла";
					
					if($fileInstallFound)
					{
						?>
						
						<form name="f" action="<?=$this->fileName?>" method="post" style="margin-top: 0px; margin-bottom: 0px;">
						
						<table width="100%" bgcolor="#cccccc" border="0" cellspacing="1" cellpadding="4">
						<tr bgcolor="#dddddd">
							<td colspan="2"><div class="t1"><b>Список файлов</b></div></td>
						</tr>
						
						<?
						$f = file($this->tmpDir."/install");
						for($i=0;$i<count($f);$i++)
						{
							?>
							
							<tr bgcolor="#ffffff" class="t1">
								<td><input type="checkbox" name="check_<?=$i?>" checked></td>
								<td width="100%"><?=ereg_replace(".*/","",$f[$i])?></td>
							</tr>
							
							<?
						}
						?>
						
						<tr bgcolor="#dddddd">
							<td align="center" colspan="2"><input type="submit" name="btnInstall" value="Установить"></td>
						</tr>
						</table>
						<input type="hidden" name="method" value="processingInstall">
						</form>
						
						<?
					}
					else $this->errMsg = "Невозможно распознать формат файла";
				}
				else echo "&nbsp;";
				?>
				
				</td>
				
			</tr>
			</table>
			<br>
							
			<?
		}	
		
		if(!empty($this->errMsg)) $this->displayError($this->errMsg);
	}
	
	function processingInstall($POST)
	{
		$arr1 = array();
		while (list($key, $val) = each($POST)) if(ereg("check_",$key)) $arr1[] = ereg_replace("check_","",$key);
		
		if(count($arr1)>0)
		{
			$arr2 = file($this->tmpDir."/install");
			
			for($i=0;$i<count($arr1);$i++)
			{
				$file = chop($arr2[$arr1[$i]]);
				$fileName = basename($file);
				$fileDir = dirname($file);

				if(file_exists($file)) copy($file, $fileDir."/backup.".$fileName);
				copy($this->tmpDir."/".$fileName, $file);
			}
			
			$this->writeLog("Загрузка обновления");
		}
	}
}
?>