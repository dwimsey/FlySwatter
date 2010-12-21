<?
global $stackwalker_path;
//$stackwalker_path = 'minidump_stackwalk';
// comment this out if you define the above yourself
require_once('config.defaults.php');

class ModuleInfo {
	public $Name = NULL;
	public $PDBFile = NULL;
	public $FirstMemoryMappedAddress = NULL;
	public $LastMemoryMappedAddress = NULL;
	public $Version = NULL;
	public $Checksum = NULL;
	public $IsMain = NULL;
}

class ThreadInfo {
	public $ThreadNumber = NULL;
	public $Stack = array();
}

class StackEntry {
	public $Depth = NULL;
	public $Module = NULL;
	public $Function = NULL;
	public $File = NULL;
	public $Line = NULL;
	public $Address = NULL;
}

class MinidumpInfo {
	public $OSName = NULL;
	public $OSVersion = NULL;
	public $CPUArchitecture = NULL;
	public $CPUModel = NULL;
	public $CPUCount = NULL;

	public $CrashReason = NULL;
	public $CrashData = NULL;
	public $CrashAssertion = NULL;
	public $LoadedModules = array();
	public $Threads = array();

	function __construct($minidump_stackwalk_output = NULL) {
		if(!is_null($minidump_stackwalk_output)) {
			if(!is_array($minidump_stackwalk_output)) {
				// load this as a single string with newlines in it, convert to an array of lines and process that way
				$minidump_stackwalk_output = explode('\n', $minidump_stackwalk_output);
			}
			$this->ParseMinidumpStackWalk($minidump_stackwalk_output);
		}
	}



	function ParseMinidumpStackWalk($lines) {
		$lineCount = count($lines);
		$gotCPULine = false;
		$gotOSLine = false;
		$gotCrashLine = false;

		$parts = 0;
		$partsCount = 0;
	
		for($lineNo = 0; $lineNo < $lineCount; $lineNo++) {
			$parts = explode('|', $lines[$lineNo]);
			$partCount = count($parts);
			switch($parts[0]) {
				case 'OS':
					$gotOSLine = true;
					if($partCount < 3) {
						throw new Exception('ParseMinidumpStackWalk ' . $lineNo . ': Not enough parts for OS line: ' . $partCount);
					}
					$this->OSName = $parts[1];
					$this->OSVersion = $parts[2];
					break;
				case 'CPU':
					$gotCPULine = true;
					if($partCount < 4) {
						throw new Exception('ParseMinidumpStackWalk ' . $lineNo . ': Not enough parts for CPU line: ' . $partCount);
					}
					$this->CPUArchitecture = $parts[1];
					$this->CPUModel = $parts[2];
					$this->CPUCount = $parts[3];
					break;
				case 'Crash':
					$gotCrashLine = true;
					if($partCount < 4) {
						throw new Exception('ParseMinidumpStackWalk ' . $lineNo . ': Not enough parts for CPU line: ' . $partCount);
					}
					if(($parts[2] == NULL || $parts[2] == '') && ($parts[3] == NULL || $parts[3] == '')) {
						$this->CrashReason = NULL;
						$this->CrashAssertion = NULL;
						$this->CrashData = NULL;
					} else {
						$this->CrashReason = $parts[1];
						$this->CrashData = $parts[2];
						if($parts[3] == NULL || $parts[3] == '') {
							$this->CrashAssertion = 0;
						} else {
							$this->CrashAssertion = $parts[3] + 1;
							$this->CrashAssertion--;
						}
					}
					break;
				case 'Module':
					$mcount = count($this->LoadedModules);
					if($partCount < 8) {
						throw new Exception('ParseMinidumpStackWalk ' . $lineNo . ': Not enough parts for Module line: ' . $partCount);
					}
					$mInfo = new ModuleInfo();
					$mInfo->Name = $parts[1];
					$mInfo->Version = $parts[2];
					$mInfo->PDBFile = $parts[3];
					$mInfo->Checksum = $parts[4];
					$mInfo->FirstMemoryMappedAddress = $parts[5];
					$mInfo->LastMemoryMappedAddress = $parts[6];
					switch($parts[7]) {
						case '0':
							$mInfo->IsMain = 0;
							break;
						case '1':
							$mInfo->IsMain = 1;
							break;
						default:
							throw new Exception('ParseMinidumpStackWalk ' . $lineNo . ': Invalid value for IsMain: ' . $parts[7]);
							break;
					}
					$this->LoadedModules[$mcount++] = $mInfo;
					break;
				default:
					if($partCount == 1) {
						// Skip blank lines
						continue;
					}
					if($partCount < 7) {
						throw new Exception('ParseMinidumpStackWalk ' . $lineNo . ': Not enough parts for StackWalk line: ' . $partCount);
					}
					$tnum = $parts[0];
					if($this->Threads[$tnum] == NULL) {
						$this->Threads[$tnum] = new ThreadInfo();
						$this->Threads[$tnum]->ThreadNumber = $tnum;
					}
					$sEntry = new StackEntry();
					$sEntry->Depth = $parts[1];
					$sEntry->Module = $parts[2];
					$sEntry->Function = $parts[3];
					$sEntry->File = $parts[4];
					$sEntry->Line = $parts[5];
					$sEntry->Address = $parts[6];
					if($this->Threads[$tnum]->Stack[$sEntry->Depth] != NULL) {
						throw new Exception('ParseMinidumpStackWalk ' . $lineNo . ': Duplicate stack entry for thread ' . $tnum . ' entry ' . $sEntry->Depth . '.');
					}
					$this->Threads[$tnum]->Stack[$sEntry->Depth] = $sEntry;
					break;
			}
			
		}
		return(true);
	}
	

	function ReadMinidumpData($dumpid, $dfile, $binData) {
		$rawDump = getMinidumpStackwalkFile($dumpid, $dfile, $binData, false);
		return($this->ParseMinidumpStackWalk(explode("\n", $rawDump)));
	}
		
	function ReadMinidumpFile($minidump_filename) {
		global $stackwalker_path;
		global $cache_path;

		$cmdpath = $stackwalker_path . " -m " . $minidump_filename . " /Users/dwimsey/Sites/flyswatter/symbols";
		exec(escapeshellcmd($cmdpath), &$output);
		return($this->ParseMinidumpStackWalk($output));
	}

	function GetXML() {
		$xmlString = '<MinidumpSummary>';
		$xmlString .= '<OS><Name>' . htmlspecialchars($this->OSName) . '</Name><Version>' . htmlspecialchars($this->OSVersion) . '</Version></OS>';
		$xmlString .= '<CPU><Architecture>' . $this->CPUArchitecture . '</Architecture><Model>' . htmlspecialchars($this->CPUModel) . '</Model><Count>' . htmlspecialchars($this->CPUCount) . '</Count></CPU>';
		if($this->CrashReason == NULL && $this->CrashAddress == NULL && $this->CrashAssertion == NULL) {
			$xmlString .= '<ExceptionInfo/>';
		} else {
			$xmlString .= '<ExceptionInfo><Reason>' . $this->CrashReason . '</Reason><Address>' . $this->CrashData . '</Address><Assertion>' . $this->CrashAssertion . '</Assertion></ExceptionInfo>';
		}

		$iCount = count($this->LoadedModules);
		for($i = 0; $i < $iCount; $i++) {
			$mInfo = $this->LoadedModules[$i];
			$xmlString .= '<ModuleInfo><Name>' . $mInfo->Name . '</Name><PDBFile>' . $mInfo->PDBFile . '</PDBFile><Version>'. $mInfo->Version .'</Version><Checksum>' . $mInfo->Checksum . '</Checksum><LowAddress>'. $mInfo->FirstMemoryMappedAddress . '</LowAddress><HighAddress>' . $mInfo->LastMemoryMappedAddress . '</HighAddress><HasEntryPoint>' . $mInfo->IsMain . '</HasEntryPoint></ModuleInfo>';
		}

		$iCount = count($this->Threads);
		$xmlString .= '<Threads>';
		for($i = 0; $i < $iCount; $i++) {
			$sList = $this->Threads[$i];
			$xmlString .= '<ThreadInfo>';
			$xmlString .= '<ThreadNumber>' . $sList->ThreadNumber . '</ThreadNumber>';
			$sCount = count($sList->Stack);
			for($ii = 0; $ii < $sCount; $ii++) {
				$sInfo = $sList->Stack[$ii];
				$xmlString .= '<StackEntry><Depth>' . $sInfo->Depth . '</Depth><Module>' . $sInfo->Module . '</Module><Function>' . $sInfo->Function . '</Function><File>' . $sInfo->File . '</File><Line>' . $sInfo->Line . '</Line><Address>' . $sInfo->Address . '</Address></StackEntry>';
			}
			$xmlString .= '</ThreadInfo>';
		}
		$xmlString .= '</Threads>';
		$xmlString .= '</MinidumpSummary>';
		return($xmlString);
	}
	
	function FromXML($xmlString) {
	
		$xml = simplexml_load_string($xmlString);

		$result = $xml->xpath("/MinidumpSummary/OS/Name");
		$this->OSName = $result[0];
		$result = $xml->xpath("/MinidumpSummary/OS/Version");
		$this->OSVersion = $result[0];

		$result = $xml->xpath("/MinidumpSummary/CPU/Architecture");
		$this->CPUArchitecture = $result[0];
		$result = $xml->xpath("/MinidumpSummary/CPU/Model");
		$this->CPUModel = $result[0];
		$result = $xml->xpath("/MinidumpSummary/CPU/Count");
		$this->CPUCount = $result[0];

		$result = $xml->xpath("/MinidumpSummary/ExceptionInfo/Reason");
		if($result) {
			$this->CrashReason = $result[0];
		} else {
			$this->CrashReason = NULL;
		}
		$result = $xml->xpath("/MinidumpSummary/ExceptionInfo/Address");
		if($result) {
			$this->CrashData = $result[0];
		} else {
			$this->CrashData = NULL;
		}
		$result = $xml->xpath("/MinidumpSummary/ExceptionInfo/Assertion");
		if($result) {
			$this->CrashAssertion = $result[0];
		} else {
			$this->CrashAssertion = NULL;
		}

		$this->LoadedModules = array();

		$xmlmodules = $xml->xpath("/MinidumpSummary/ModuleInfo");
		$rcount = count($xmlmodules);
		if($rcount > 0) {
			
			$i = 0;
			for($i = 0; $i < $rcount; $i++) {
				$res = $xmlmodules[$i];

				$mInfo = new ModuleInfo();

				$result = $res->xpath("Name");
				$mInfo->Name = $result[0];
				$result = $res->xpath("PDBFile");
				$mInfo->PDBFile = $result[0];
				$result = $res->xpath("Version");
				$mInfo->Version = $result[0];
				$result = $res->xpath("Checksum");
				$mInfo->Checksum = $result[0];
				$result = $res->xpath("LowAddress");
				$mInfo->FirstMemoryMappedAddress = $result[0];
				$result = $res->xpath("HighAddress");
				$mInfo->LastMemoryMappedAddress = $result[0];
				$result = $res->xpath("HasEntryPoint");
				$mInfo->IsMain = $result[0];
				$this->LoadedModules[$i] = $mInfo;
			}
		}
		
		$this->Threads = array();
					
		$xmlThreads = $xml->xpath("/MinidumpSummary/Threads/ThreadInfo");
		$rcount = count($xmlThreads);
		if($rcount > 0) {
			
			$i = 0;
			for($i = 0; $i < $rcount; $i++) {
				$res = $xmlThreads[$i];

				$tInfo = new ThreadInfo();

				$result = $res->xpath("ThreadNumber");
				$tInfo->ThreadNumber = $result[0];
				
				$tInfo->Stack = array();
				// load stack entries
				$sresults = $res->xpath("StackEntry");
				$ii = 0;
				$xcount = count($sresults);
				for($ii = 0; $ii < $xcount; $ii++) {
					$sres = $sresults[$ii];

					$sInfo = new StackEntry();
				
					$sresult = $sres->xpath("Depth");
					$sInfo->Depth = $sresult[0];
					$sresult = $sres->xpath("Module");
					$sInfo->Module = $sresult[0];
					$sresult = $sres->xpath("Function");
					$sInfo->Function = $sresult[0];
					$sresult = $sres->xpath("File");
					$sInfo->File = $sresult[0];
					$sresult = $sres->xpath("Line");
					$sInfo->Line = $sresult[0];
					$sresult = $sres->xpath("Address");
					$sInfo->Address = $sresult[0];

					$tInfo->Stack[$ii] = $sInfo;
				}
				$this->Threads[$i] = $tInfo;

			}
			
		}
		return(true);
	}
	
	function LoadXml($xmlString) {
		
		return(false);
	}
}


function getMinidumpStackwalkFile($dumpid, $dfile, $binData, $wantsText) {
	global $cache_minidump_dir;
	global $cache_minidump_text;
	global $cache_minidump_raw;
	global $cache_minidump_xml;
	global $stackwalker_path;
	global $symbols_dir;
	global $cache_dir_mask;

	$should_cache = false;
	if($wantsText) {
		if($cache_minidump_text == 1) {
			$should_cache = true;
		}
	} else {
		if($cache_minidump_raw == 1) {
			$should_cache = true;
		}
	}

	$outStr = '';
	$minidump = NULL;
	$cache_file = NULL;

	if($should_cache) {
		$cache_filename = $dumpid . '_' . $dfile;
		if($wantsText) {
			$cache_filename .= '.txt';
		} else {
			$cache_filename .= '.raw';
		}
		$cache_file = $cache_minidump_dir . '/' . $cache_filename;
		if(file_exists($cache_file)) {
			$fp = fopen($cache_file, 'r');
			if($fp) {
				$outStr = fread($fp, filesize($cache_file));
				fclose($fp);
				return($outStr);
			}
		}
	}

	// if we haven't got an xml string by here, reprocess it and recache it		

	$tmpfname = tempnam("/tmp", "dmp");

	$handle = fopen($tmpfname, "w");
	fwrite($handle, $binData);
	fclose($handle);

	if($wantsText) {
		$cmdpath = "${stackwalker_path}    ${tmpfname} ${symbols_dir}";
	} else {
		$cmdpath = "${stackwalker_path} -m ${tmpfname} ${symbols_dir}";
	}

	exec(escapeshellcmd($cmdpath), &$output);
	header('Content-Type: text/plain');
	$iMax = count($output);
	if($iMax < 1) {
		// get out of here, nothing to send
		return('');
	}

	$outStr = '';
	if($should_cache) {
		if(!file_exists($cache_minidump_dir)) {
			mkdir($cache_minidump_dir, $cache_dir_mask, true);
		}

		$fp = fopen($cache_file, 'w');
		if($fp != false) {
			for($i = 0; $i < $iMax; $i++) {
				$outStr .= $output[$i] . "\n";
				fwrite($fp, $output[$i] . "\n");
			}
			fclose($fp);
		} else {
			for($i = 0; $i < $iMax; $i++) {
				$outStr .= $output[$i] . "\n";
			}
		}
	} else {
		for($i = 0; $i < $iMax; $i++) {
			$outStr .= $output[$i] . "\n";
		}
	}
	unlink($tmpfname);

	return($outStr);
}
?>
