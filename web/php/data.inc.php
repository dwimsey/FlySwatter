<?
require_once('config.defaults.php');
require_once('classes/minidump.php');

function getDirectoryList ($directory)  {
	// create an array to hold directory list
	$results = array();
	
	// create a handler for the directory
	$handler = opendir($directory);
	
	// open directory and walk through the filenames
	while ($file = readdir($handler)) {
		// if file isn't this directory or its parent, add it to the results
		if ($file != "." && $file != ".." && $file != ".svn" && $file != "CVS") {
			$results[] = $file;
		}
	}

	// tidy up: close the handler
	closedir($handler);

	// done!
	return $results;
}

class ReportInfo {
	public $DumpId = NULL;
	public $DateSubmitted = NULL;
	public $Filename = NULL;
	public $FileTimestamp = 0;
	public $SubmissionTimestamp = 0;

	public $AppName = NULL;
	public $AppVersion = NULL;
	public $AppGuid = NULL;
	public $ProductGuid = NULL;
	public $ProductName = NULL;
	
	function LoadFile($filename) {
		global $cache_dir;
		global $cache_minidump_xml;
		global $cache_minidump_dir;
		global $cache_dir_mask;

		if(!file_exists($filename)) {
			return(false);
		}	
		$this->Filename = $filename;
		$this->DumpId = basename($this->Filename, ".xml");
		$this->AppName = NULL;
		$this->AppVersion = NULL;
		$this->AppRevision = NULL;
		$this->FileTimestamp = (int)filemtime($this->Filename);
		$xml = simplexml_load_file($this->Filename);

		$result = $xml->xpath("/UploadedDebugReport/Parameters/AppName");
		$this->AppName = $result[0];
		$result = $xml->xpath("/UploadedDebugReport/Parameters/AppGuid");
		$this->AppGuid = $result[0];
		$result = $xml->xpath("/UploadedDebugReport/Parameters/AppVersion");
		$this->AppVersion = $result[0];
		$result = $xml->xpath("/UploadedDebugReport/Parameters/ProductName");
		$this->ProductName = $result[0];
		$result = $xml->xpath("/UploadedDebugReport/Parameters/ProductGuid");
		$this->ProductGuid = $result[0];
		$result = $xml->xpath("/UploadedDebugReport/UploadEnvironment/SubmissionTimestamp");
		$this->SubmissionTimestamp = (int)$result[0];
		if($this->SubmissionTimestamp > 0) {
			$this->DateSubmitted = date ("F d Y H:i:s", $this->SubmissionTimestamp);
		} else {
			//$this->DateSubmitted = date ("F d Y H:i:s", $this->FileTimestamp);
			$this->DateSubmitted = '';
		}

		$outStr = '';
		$minidump = NULL;
		$cache_file = NULL;
		if($cache_minidump_xml == 1) {
			$cache_filename = $this->DumpId . '_0.xml';
			$cache_file = $cache_minidump_dir . '/' . $cache_filename;
			if(file_exists($cache_file)) {
				$fp = fopen($cache_file, 'r');
				if($fp) {
					$outStr = fread($fp, filesize($cache_file));
					fclose($fp);
					$minidump = new MinidumpInfo();
					if(!$minidump->FromXML($outStr)) {
						$minidump = NULL;
					}
				}
			}
		}
		// if we haven't got an xml string by here, reprocess it and recache it		
		if($minidump == NULL) {
			$tmpfname = tempnam("/tmp", "dmp");
			$result = $xml->xpath("/UploadedDebugReport/Files/File[". ($dfile + 1) ."]/Data");
			$binData = base64_decode($result['0']);

			$minidump = new MinidumpInfo();
			$minidump->ReadMinidumpData($this->DumpId, 0, $binData);
	
			if($cache_minidump_xml == 1) {
				if(!file_exists($cache_minidump_dir)) {
					mkdir($cache_minidump_dir, $cache_dir_mask, true);
				}
				$outStr = $minidump->GetXML();
				$fp =fopen($cache_file, "w");
				if($fp != false) {
					fwrite($fp, $outStr);
					fclose($fp);
				}
			}
		}
		
		$this->Minidump = $minidump;
		return(true);
	}

	function GetXML() {	
		$reportXml = '<ReportInfo>';
		$reportXml .= '<DumpId>' . $this->DumpId . '</DumpId>';
		$reportXml .= '<SubmissionTimestamp>' . $this->SubmissionTimestamp . '</SubmissionTimestamp>';
		$reportXml .= '<DateSubmitted>' . $this->DateSubmitted . '</DateSubmitted>';
		$reportXml .= '<FileTimestamp>' . $this->FileTimestamp . '</FileTimestamp>';
		$reportXml .= '<OSName>' . $this->Minidump->OSName . '</OSName>';
		$reportXml .= '<OSVersion>' . $this->Minidump->OSVersion . '</OSVersion>';
		$reportXml .= '<CPUArchitecture>' . $this->Minidump->CPUArchitecture . '</CPUArchitecture>';
		$reportXml .= '<CPUModel>' . $this->Minidump->CPUModel . '</CPUModel>';
		$reportXml .= '<CPUCount>' . $this->Minidump->CPUCount . '</CPUCount>';
		$reportXml .= '<Exception>' . $this->Minidump->CrashReason . '</Exception>';
		$reportXml .= '<Assertion>' . $this->Minidump->CrashAssertion . '</Assertion>';
		$reportXml .= '<CrashData>' . $this->Minidump->CrashData . '</CrashData>';
		$reportXml .= '<AppName>' . $this->AppName . '</AppName>';
		$reportXml .= '<AppGuid>' . $this->AppGuid . '</AppGuid>';
		$reportXml .= '<AppVersion>' . $this->AppVersion	 . '</AppVersion>';
		$reportXml .= '<ProductName>' . $this->ProductName . '</ProductName>';
		$reportXml .= '<ProductGuid>' . $this->ProductGuid . '</ProductGuid>';
		$reportXml .= '</ReportInfo>';
		return($reportXml);
	}
}

function endswith($string, $test) {
	$strlen = strlen($string);
	$testlen = strlen($test);
	if ($testlen > $strlen) {
		return(false);
	}
	return(substr_compare($string, $test, -$testlen) === 0);
}

function getReportList() {
	global $report_dir;
	$files = getDirectoryList($report_dir);
	$errorReports = array();
	$fcount = count($files);
	$i = 0;
	$dfile = $_GET['dfile'];
	foreach($files as $key => $val) {
		if(!endswith($val, '.xml')) {
			continue;
		}
		$xmlFilename = $report_dir . '/' . $val;
		
		$rInfo = new ReportInfo();
		$rInfo->LoadFile($xmlFilename);
		$errorReports[$i++] = $rInfo;
	}
	return($errorReports);
}
?>
