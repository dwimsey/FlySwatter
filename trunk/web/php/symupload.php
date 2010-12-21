<?
require_once('config.defaults.php');
require_once('classes/common.php');

global $symbol_dir;
global $symbol_dir_mask;

$symbols_os = trim($_POST['os']);
$symbols_debug_file = trim($_POST['debug_file']);
$symbols_code_file = trim($_POST['code_file']);
$symbols_cpu = trim($_POST['cpu']);
$symbols_debug_id = trim($_POST['debug_identifier']);

$target_filename = substr($symbols_debug_file, 0, (strlen($symbols_code_file)-4)) . '.sym';
//$target_dir = $symbols_dir . '/' . $symbols_os . '/' . $symbols_cpu . '/' . $symbols_debug_file . '/' . $symbols_debug_id;
$target_dir = $symbols_dir . '/' . $symbols_debug_file . '/' . $symbols_debug_id;
if(!file_exists($target_dir)) {
	mkdir($target_dir, $symbol_dir_mask, true);
}

//create the xml document
$xmlDoc = new DOMDocument();
//create the root element
$root = $xmlDoc->appendChild($xmlDoc->createElement("SymbolPackage")); 

$fscid = null;

//    <os>windows</os> os_str
//    <debug_file>CrashTestDummy.pdb&#xD;</debug_file> debug_file
//    <code_file>CrashTestDummy.pdb&#xD;</code_file> code_file
//    <cpu>x86</cpu> cpu_str
//    <debug_identifier>4AC5D892ABBE4C1EB88CEDA9BA47AA4911</debug_identifier> debug_id

$params = $root->appendChild($xmlDoc->createElement("Parameters")); 
foreach($_POST as $paramName => $paramValue) {
	$params->appendChild($xmlDoc->createElement($paramName, $paramValue));
}

$params = $root->appendChild($xmlDoc->createElement("UploadEnvironment")); 
foreach($_SERVER as $paramName => $paramValue) {
	if($paramName == null) {
		if($paramValue == null) {
			$params->appendChild($xmlDoc->createElement('NULL', 'NULL'));
		} else {
			$params->appendChild($xmlDoc->createElement('NULL', $paramValue));
		}
	} else {
		if($paramValue == null) {
			$params->appendChild($xmlDoc->createElement($paramName, 'NULL'));
		} else {
			$params->appendChild($xmlDoc->createElement($paramName, $paramValue));
		}
	}
}

$files = $root->appendChild($xmlDoc->createElement("Files"));
$i = 0;
foreach($_FILES as $fieldName => $fdata) {
	$fileNode = $files->appendChild($xmlDoc->createElement("File")); 
	$fileNodeEntry = $fileNode->appendChild($xmlDoc->createElement("Field", $fieldName));
	$fileNodeEntry = $fileNode->appendChild($xmlDoc->createElement("Name", $fdata['name']));
	$fileNodeEntry = $fileNode->appendChild($xmlDoc->createElement("Size", $fdata['size']));
	$fileNodeEntry = $fileNode->appendChild($xmlDoc->createElement("Type", $fdata['type']));

	$fp = fopen($fdata['tmp_name'], "r");
	$binDataBuf = fread($fp, filesize($fdata['tmp_name']));
	fclose($fp);

	$b64data = base64_encode($binDataBuf);
	$fileNodeEntry = $xmlDoc->createElement("Data");

	$fileNodeAttribute = $xmlDoc->createAttribute("encoding");
	$fileNodeAttribute->appendChild($xmlDoc->createTextNode('base64'));
	$fileNodeEntry->appendChild($fileNodeAttribute);
	//$fileNodeAttribute->appendChild($fileNodeAttribute);
	$fileNodeEntry->appendChild($xmlDoc->createTextNode($b64data));

	$fileNode->appendChild($fileNodeEntry);

	if($i > 0) {
		move_uploaded_file($fdata['tmp_name'], $target_dir . "/$i_" . $target_filename);
	} else {
		move_uploaded_file($fdata['tmp_name'], $target_dir . '/' . $target_filename);
	}
}

//make the output pretty
$xmlDoc->formatOutput = true;

$fscid = rand_str(16);
$outputFilename = $symbols_dir . '/raw/symupload-' . $fscid . ".xml";
if(!file_exists($symbols_dir . '/raw')) {
	mkdir($symbols_dir . '/raw', $symbol_dir_mask, true);
}
$fp = @fopen($outputFilename, 'w');
if(!$fp) {
    die('Error cannot create XML file');
}
$outputXml = $xmlDoc->saveXML();
fwrite($fp, $outputXml);
fclose($fp);

echo "Symbols uploaded, transaction id: ${fscid}";
?>
