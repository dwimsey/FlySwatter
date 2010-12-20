<?
require_once('config.defaults.php');
require_once('classes/minidump.php');

$dumpid = $_GET['dumpid'];
$fn = $report_dir . '/' . $dumpid . '.xml';

if($_GET['dfile'] != null) {
	$dfile = $_GET['dfile'];
	$xml = simplexml_load_file($fn);
	$result = $xml->xpath("/UploadedDebugReport/Files/File[". ($dfile + 1) ."]/Field");
	$fieldName = $result['0'];

	$result = $xml->xpath("/UploadedDebugReport/Files/File[". ($dfile + 1) ."]/Name");
	$fieldFileName = $result['0'];

	$result = $xml->xpath("/UploadedDebugReport/Files/File[". ($dfile + 1) ."]/Size");
	$fieldSize = $result['0'];

	$result = $xml->xpath("/UploadedDebugReport/Files/File[". ($dfile + 1) ."]/Type");
	$fieldType = $result['0'];

	$result = $xml->xpath("/UploadedDebugReport/Files/File[". ($dfile + 1) ."]/Data");
	$fieldData = $result['0'];
	$binData = base64_decode($fieldData);

	if($_GET['type'] == 'json') {
		$tmpfname = tempnam("/tmp", "dmp");

		$handle = fopen($tmpfname, "w");
		fwrite($handle, $binData);
		fclose($handle);

		$stacktrace = new MinidumpInfo();
		$stacktrace->ReadMinidumpFile($tmpfname);
		unlink($tmpfname);
		
		header('Content-Type: text/plain');
		echo json_encode($stacktrace);
		exit;
	} else if(($_GET['type'] == 'xml') || ($_GET['type'] == 'html')) {
		$cache_filename = $dumpid . '_md' . $dfile . '.xml';
		$cache_file = $cache_dir . '/' . $cache_filename;
		$outStr = '';
		if(file_exists($cache_file)) {
			$fp = fopen($cache_file, 'r');
			if($fp) {
				$outStr = fread($fp, filesize($cache_file));
				fclose($fp);
				$stacktrace = new MinidumpInfo();
				$stacktrace->FromXML($outStr);
				$outStr = $stacktrace->GetXML();
			}
		}

		// if we haven't got an xml string by here, reprocess it and recache it		
		if($outStr == '') {
			$tmpfname = tempnam("/tmp", "dmp");
	
			$handle = fopen($tmpfname, "w");
			fwrite($handle, $binData);
			fclose($handle);
	
			$stacktrace = new MinidumpInfo();
			$stacktrace->ReadMinidumpFile($tmpfname);
			unlink($tmpfname);
	
			$outStr = $stacktrace->GetXML();
			$fp =fopen($cache_file, 'w+');
			fwrite($fp, $outStr);
			fclose($fp);
		}

		if($_GET['type'] == 'xml') {
			header("Content-Disposition: filename=\"" . urlencode($cache_filename) . "\"");
			header("Content-type: text/xml; charset=utf-8;");
			header("Cache-control: private");
			print("<?xml version=\"1.0\"?>\n" . str_replace("&nbsp;", "&amp;nbsp;", $outStr));
		} else {
			if($use_browser_xslt) {
				header("Content-Disposition: filename=\"" . urlencode($cache_filename) . "\"");
				header("Content-type: text/xml; charset=utf-8;");
				header("Cache-control: private");
				print("<?xml version=\"1.0\"?>\n<?xml-stylesheet type=\"text/xsl\" href=\"minidump.xsl\"?>\n" . str_replace('&nbsp;', '&amp;nbsp;', $outStr));
			} else {
				# LOAD XML FILE
				$XML = new DOMDocument();
				$XML->loadXml($outStr);
				# START XSLT
				$xslt = new XSLTProcessor();
				$XSL = new DOMDocument();
				
				$XSL->load('minidump.xsl', LIBXML_NOCDATA);
				
				$xslt->importStylesheet( $XSL );
				#PRINT
				header('Content-Type: text/html; charset=utf-8;');
				print $xslt->transformToXML( $XML );
			}
		}
		
		exit;
	} else	if($_GET['type'] == 'text') {
		$tmpfname = tempnam("/tmp", "dmp");

		$handle = fopen($tmpfname, "w");
		fwrite($handle, $binData);
		fclose($handle);

		$cmdpath = $stackwalker_path . " " . $tmpfname . " /Users/dwimsey/Sites/flyswatter/symbols";
		header('Content-Type: text/plain');
		passthru(escapeshellcmd($cmdpath));

		unlink($tmpfname);
	} else 	if($_GET['type'] == 'raw') {
		$tmpfname = tempnam("/tmp", "dmp");

		$handle = fopen($tmpfname, "w");
		fwrite($handle, $binData);
		fclose($handle);

		$cmdpath = $stackwalker_path . " -m " . $tmpfname . " /Users/dwimsey/Sites/flyswatter/symbols"; 
		header('Content-Type: text/plain');
		passthru(escapeshellcmd($cmdpath));

		unlink($tmpfname);
	} else {
		switch ($ext) {
			case "1":
				//header("Content-type: $fieldType"); // add here more headers for diff. extensions
				//header("Content-Disposition: attachment; filename=\"".$path_parts["basename"]."\""); // use 'attachment' to force a download
				break;
			default;
				//header("Content-type: application/octet-stream");
		}

		//echo "Size: " . count($binData);
		header("Content-Disposition: attachment; filename=\"${dumpid}_${dfile}.dmp\"");
		header("Content-type: application/octet-stream");
		header("Cache-control: private"); //use this to open files directly

		header('Content-Description: File Transfer');
		header('Content-Transfer-Encoding: binary');
		header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
		//header('Expires: 0');
		//header('Pragma: public');
	
		echo $binData;
	}
} else {
	header( 'Location: report_list.php' ) ;
}
?>

