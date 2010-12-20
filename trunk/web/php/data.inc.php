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
		global $report_dir;

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

		$tmpfname = tempnam("/tmp", "dmp");

		$fn = $report_dir . '/' . $this->DumpId . '.xml';
		$mdXml = simplexml_load_file($fn);
		
		$result = $mdXml->xpath("/UploadedDebugReport/Files/File[". ($dfile + 1) ."]/Data");
		$binData = base64_decode($result['0']);

		$handle = fopen($tmpfname, "w");
		fwrite($handle, $binData);
		fclose($handle);

		$minidump = new MinidumpInfo();
		$minidump->ReadMinidumpFile($tmpfname);
		unlink($tmpfname);

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
