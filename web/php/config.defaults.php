<?
date_default_timezone_set('UTC');

global $scripts_dir;
$scripts_dir = ".";

global $report_dir;
$report_dir = "${scripts_dir}/xml";

global $cache_enabled;
$cache_enabled = 1;

global $cache_minidump_xml;
$cache_minidump_xml = 1;

global $cache_dir;
$cache_dir = "${scripts_dir}/cache";

global $use_browser_xslt;
$use_browser_xslt = true;

global $stackwalker_path;
$stackwalker_path = "${scripts_dir}/bin/minidump_stackwalk";

require_once('config.defaults.php');
if(file_exists('config.php')) {
	require_once('config.php');
}

// fix config errors
if($cache_enabled == 0) {
	$cache_minidump_xml = 0;
}
?>
