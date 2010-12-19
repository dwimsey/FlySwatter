<?
require_once('config.defaults.php');
require_once('classes/stackwalk.php');

$dumpid = $_GET['dumpid'];
$fn = $report_dir . '/' . $dumpid . '.xml';

if($_GET['afile'] != null) {
	$afile = $_GET['afile'];
	$xml = simplexml_load_file($fn);
	$result = $xml->xpath("/UploadedDebugReport/Parameters/Report_AttachFiles_${afile}");
	if(count($result) == 0) {
		// look for old style registry attachment names
		$result = $xml->xpath("/UploadedDebugReport/Parameters/FlyTrap_AttachFiles_${afile}");
	}
	$fieldData = $result['0'];

	$parts = explode(';', $fieldData);
	$fname = $parts[0];
	$tfsize = $parts[1];
	$fdata = $parts[2];

	if($tfsize < 0) {
		$binData = $fieldData;
		echo "File was not included, Error: $tfsize: $fdata";
	} else {
		if ($tfsize == 0) {
			$binData = "";
		} else {
			$binData = base64_decode($fdata);
		}
		//header("Content-type: application/octet-stream");
		// for now just mark it as text
		header("Content-type: text/plain; charset=utf-8;");
		// header("Content-Disposition: attachment; filename=\"". basename($fname) . "\"");
//		header("Content-Disposition: filename=\"". basename($fname) . "\"");
		//header("Content-length: " . count($binData));
		header("Cache-control: private"); //use this to open files directly
		echo $binData;
	}
} else if($_GET['rfile'] != null) {
	$rfile = $_GET['rfile'];

	$xml = simplexml_load_file($fn);
	$result = $xml->xpath("/UploadedDebugReport/Parameters/Report_AttachRegKeys_${rfile}");
	if(count($result) == 0) {
		// look for old style registry attachment names
		$result = $xml->xpath("/UploadedDebugReport/Parameters/FlyTrap_AttachRegKeys_${rfile}");
	}
	$fieldData = $result['0'];

	$parts = explode(';', $fieldData, 2);
	$fname = $parts[0];
	$fdata = ltrim($parts[1]);
	$tfsize = strlen($fdata);

	if($tfsize < 0) {
		$binData = $fieldData;
		echo "Registry key was not included, Error: $tfsize: $fdata";
	} else {
		header("Content-type: text/plain; charset=utf-8;");//application/octet-stream");
		// header("Content-Disposition: attachment; filename=\"". basename($fname) . "\"");
//		header("Content-Disposition: filename=\"FlyTrapRegistryDump_${rfile}.reg\"");
		header("Content-length: " . $tfsize);
		header("Cache-control: private"); //use this to open files directly
		echo trim($fdata);
	}
} else if($_GET['type'] == 'xml') {
	// pass the file through
	header("Content-type: text/xml; charset=utf-8;");//application/octet-stream");
	// header("Content-Disposition: attachment; filename=\"". basename($fname) . "\"");
	// header("Content-Disposition: filename=\"FlyTrapRegistryDump_${rfile}.reg\"");
	header("Cache-control: private"); //use this to open files directly
	$fdata = file($fn);
	$len = count($fdata);
	$cur = 0;
	while($cur < $len) {
		print($fdata[$cur]);
		$cur += 1;
	}
} else {
	if($use_browser_xslt) {
		header("Content-Disposition: filename=\"${dumpid}.xml\"");
		header("Content-type: text/xml; charset=utf-8;");
		header("Cache-control: private"); //use this to open files directly
		$fdata = file($fn);
		print($fdata[0]);
		print("<?xml-stylesheet type=\"text/xsl\" href=\"reportsummary.xsl\"?>\r\n");
		$len = count($fdata);
		$cur = 1;
		while($cur < $len) {
			print($fdata[$cur]);
			$cur += 1;
		}
	} else {
		# LOAD XML FILE
		$XML = new DOMDocument();
		$XML->load($fn);
		# START XSLT
		$xslt = new XSLTProcessor();
		$XSL = new DOMDocument();
		
		$XSL->load('reportsummary.xsl', LIBXML_NOCDATA);
		
		$xslt->importStylesheet( $XSL );
		#PRINT
		print $xslt->transformToXML( $XML );
	}
}
?>
