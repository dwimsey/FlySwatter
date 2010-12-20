<?
require_once('config.defaults.php');

function rand_str($length = 32, $chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890')
{
    // Length of character list
    $chars_length = (strlen($chars) - 1);

    // Start our string
    $string = $chars{rand(0, $chars_length)};
    
    // Generate random string
    for ($i = 1; $i < $length; $i = strlen($string))
    {
        // Grab a random character from our list
        $r = $chars{rand(0, $chars_length)};
        
        // Make sure the same two characters don't appear next to each other
        if ($r != $string{$i - 1}) $string .=  $r;
    }
    
    // Return the string
    return $string;
}

if (get_magic_quotes_gpc()) {
    function undoMagicQuotes($array, $topLevel=true) {
        $newArray = array();
        foreach($array as $key => $value) {
            if (!$topLevel) {
                $key = stripslashes($key);
            }
            if (is_array($value)) {
                $newArray[$key] = undoMagicQuotes($value, false);
            }
            else {
                $newArray[$key] = stripslashes($value);
            }
        }
        return $newArray;
    }
    $_GET = undoMagicQuotes($_GET);
    $_POST = undoMagicQuotes($_POST);
    $_COOKIE = undoMagicQuotes($_COOKIE);
    $_REQUEST = undoMagicQuotes($_REQUEST);
}

//create the xml document
$xmlDoc = new DOMDocument();
//create the root element
$root = $xmlDoc->appendChild($xmlDoc->createElement("UploadedDebugReport")); 

$fscid = null;

$params = $root->appendChild($xmlDoc->createElement("Parameters")); 
foreach($_POST as $paramName => $paramValue) {
	if($paramName == "FlyTrapCrashId") {
		$fscid = $paramValue;
	}
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
// Add our timestamp
$params->appendChild($xmlDoc->createElement('SubmissionTimestamp', time()));

$files = $root->appendChild($xmlDoc->createElement("Files")); 

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

	//$savePath = $report_dir . '/' . basename($fdata['name']);
	//move_uploaded_file($fdata['tmp_name'], $savePath);
}

//make the output pretty
$xmlDoc->formatOutput = true;

// write to the file
if($fscid == null) {
	$fscid = "cd-no-fscid-" . rand_str(8);
} else if($fscid == "") {
	$fscid = "cd-no-fscid-" . rand_str(10);
}

$outputFilename = $report_dir . '/' . $fscid . ".xml";
$fp = @fopen($outputFilename, 'w');
if(!$fp) {
    die('Error cannot create XML file');
}
$outputXml = $xmlDoc->saveXML();
fwrite($fp, $outputXml);
fclose($fp);
echo "Error report queued for processing.\r\n\r\nCrash ID: ${fscid}";
 ?>
