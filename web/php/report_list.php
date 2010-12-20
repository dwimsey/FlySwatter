<?
require_once('config.defaults.php');
require_once('data.inc.php');

$dumpIds = getReportList();

$reportXml = '<ErrorReports>';
foreach($dumpIds as $rInfo) {
	$reportXml .= $rInfo->GetXML();
}
$reportXml .= '</ErrorReports>';

if($use_browser_xslt) {
	header("Content-Disposition: filename=\"report_list.xml\"");
	header("Content-type: text/xml; charset=utf-8;");
	header("Cache-control: private"); //use this to open files directly
	echo '<?xml version="1.0" encoding="UTF-8"?>';
	echo '<?xml-stylesheet type="text/xsl" href="report_list.xsl"?>';
	echo $reportXml;
} else {
	# LOAD XML FILE
	$XML = new DOMDocument();
	$XML->LoadXML($reportXml);
	# START XSLT
	$xslt = new XSLTProcessor();
	$XSL = new DOMDocument();
	
	$XSL->load('report_list.xsl', LIBXML_NOCDATA);
	
	$xslt->importStylesheet( $XSL );
	#PRINT
	print $xslt->transformToXML( $XML );
}
?>
