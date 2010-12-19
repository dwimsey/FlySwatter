<?
require_once('config.defaults.php');

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
	public $SubmissionTimestamp = 0;

//	public $AppName = NULL;
//	public $AppVersion = NULL;
//	public $AppRevision = NULL;
}

function getReportList() {
	global $report_dir;
	$files = getDirectoryList($report_dir);
	$errorReports = array();
	$fcount = count($files);
	$i = 0;
	foreach($files as $key => $val) {
		$rInfo = new ReportInfo();
		$rInfo->DumpId = basename($val, ".xml");
//		$rInfo->AppName = NULL;
//		$rInfo->AppVersion = NULL;
//		$rInfo->AppRevision = NULL;
		$rInfo->Filename = $report_dir . '/' . trim($val);
		if(file_exists($rInfo->Filename)) {
			$rInfo->SubmissionTimestamp = filemtime($rInfo->Filename);
			$rInfo->DateSubmitted = date ("F d Y H:i:s", $rInfo->SubmissionTimestamp);
		}
		$errorReports[$i++] = $rInfo;
	}
	return($errorReports);
}
?>
