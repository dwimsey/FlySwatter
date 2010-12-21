<?
date_default_timezone_set('UTC');

global $scripts_dir;
$scripts_dir = ".";

global $private_dir;
$private_dir = "${scripts_dir}";

global $report_dir;
$report_dir = "${private_dir}/data";

global $symbols_dir;
$symbols_dir = "${private_dir}/symbols";

global $cache_enabled;
$cache_enabled = 1;

global $cache_dir;
$cache_dir = "${private_dir}/cache";

global $cache_minidump;
$cache_minidump = 1;

global $cache_minidump_dir;
$cache_minidump_dir = "${cache_dir}/md";

global $cache_minidump_xml;
$cache_minidump_xml = 1;

global $cache_minidump_text;
$cache_minidump_text = 1;

global $cache_minidump_raw;
$cache_minidump_raw = 1;

global $use_browser_xslt;
$use_browser_xslt = true;

global $stackwalker_path;
$stackwalker_path = "${scripts_dir}/bin/minidump_stackwalk";

global $symbols_dir;
$symbols_dir = "${scripts_dir}/symbols";

global $report_url_sendcomplete;
$report_url_sendcomplete = 'http://localhost/flyswatter/reportsummary.php?dumpid=';

global $url_base;
$url_base = "http://localhost/flyswatter/";

global $url_submit_report;
$url_submit_report = "${url_base}sendreport.php";

if(file_exists('config.php')) {
	require_once('config.php');
}

// fix config errors
if(($cache_enabled == 0) || ($cache_minidump == 0)) {
	$cache_minidump = 0;
	$cache_minidump_xml = 0;
	$cache_minidump_text = 0;
	$cache_minidump_raw = 0;
}
?>
