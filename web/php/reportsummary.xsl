<?xml version="1.0" encoding="UTF-8"?>
<html xsl:version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
<xsl:variable name="aand"><![CDATA[&]]></xsl:variable>
	<head>
		<title>Crash report for <xsl:value-of select="/UploadedDebugReport/Parameters/AppName"/> v<xsl:value-of select="/UploadedDebugReport/Parameters/AppVersion"/></title>
		<link rel="stylesheet" href="theme.php?type=css" type="text/css"/>
	</head>
	<body>
		<script type="text/javascript"><![CDATA[
			function ShowStackWalk(targetObj, dmpId, dmpFileIndex) {
				if (window.XMLHttpRequest) {// code for IE7+, Firefox, Chrome, Opera, Safari
					xmlhttp=new XMLHttpRequest();
				} else {// code for IE6, IE5
					xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
				}
				// this works around the problem of including the amperasand in the stylesheet itself
				var amper = String.fromCharCode(38);
				var dumpUrl = 'minidump.php?dumpid=' + dmpId + amper + 'dfile=' + dmpFileIndex + amper + 'type=html';
				xmlhttp.onreadystatechange=function() {
					if(xmlhttp.readyState == 4) {
						if(xmlhttp.status == 200) {
							targetObj.innerHTML = xmlhttp.responseText;
						} else {
							targetObj.innerHTML = "Error: Status: " + xmlhttp.status;
						}
					} else {
						targetObj.innerHTML = 'Loading (' + xmlhttp.readyState + ')' + dumpUrl + ' ...';
					}
				}
				xmlhttp.open('GET', dumpUrl, true);
				xmlhttp.send();
			};
		]]></script>
		<div class="displayTable">
			<table>
				<thead>
					<tr>
						<td colspan="4" width="100%" align="left">Report Parameters - <xsl:value-of select="/UploadedDebugReport/Parameters/AppName"/> v<xsl:value-of select="/UploadedDebugReport/Parameters/AppVersion"/> ( <a style="color:white;font-size:10"><xsl:attribute name="href">reportsummary.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;dfile=0&amp;type=xml</xsl:attribute>Download</a> )</td>
						<td colspan="2" align="right"><nobr>Crash Id: <xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/></nobr></td>
					</tr>
				</thead>
				<tbody>
				<xsl:for-each select="UploadedDebugReport/Parameters/*">
					<xsl:sort select="name(.)"/>
					<xsl:choose>
						<xsl:when test="name(.) = &quot;AppName&quot;">
						<tr>
							<td width="20"><nobr>Application Name:</nobr></td><td align="left" colspan="5"><xsl:value-of select="."/></td>
						</tr>
						</xsl:when>
						<xsl:when test="name(.) = &quot;AppGuid&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;ProductName&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;ProductGuid&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;SubmissionTimestamp&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;AppVersion&quot;">
						<tr>
							<td>Version:</td><td colspan="5"><xsl:value-of select="."/></td>
						</tr>
						</xsl:when>
						<xsl:when test="name(.) = &quot;AppBuildId&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CompanyName&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CompanyLegalName&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CompanyShortName&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;FlyTrapReportURL&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;FlyTrapVersion&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;FlyTrapCrashId&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;FlyTrapBuildFlags&quot;">
						</xsl:when>

						<xsl:when test="name(.) = &quot;CrashReport_Title&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CrashReport_WelcomeMessage&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CrashReport_Info1Message&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CrashReport_Info2Message&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CrashReport_Info3Message&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CrashReport_Info1Button&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CrashReport_Info2Button&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CrashReport_Info3Button&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CrashReport_DontAskCheckbox&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CrashReport_SendButton&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;CrashReport_CancelButton&quot;">
						</xsl:when>

						<xsl:when test="name(.) = &quot;DebugReport_Title&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;DebugReport_WelcomeMessage&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;DebugReport_Info1Message&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;DebugReport_Info2Message&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;DebugReport_Info3Message&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;DebugReport_Info1Button&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;DebugReport_Info2Button&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;DebugReport_Info3Button&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;DebugReport_DontAskCheckbox&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;DebugReport_SendButton&quot;">
						</xsl:when>
						<xsl:when test="name(.) = &quot;DebugReport_CancelButton&quot;">
						</xsl:when>

						<xsl:when test="name(.) = &quot;AttachedFiles&quot;">
						</xsl:when>
						<xsl:when test="starts-with(name(.), &quot;AttachedFile_&quot;)">
						</xsl:when>
						<xsl:when test="name(.) = &quot;AttachedRegKeys&quot;">
						</xsl:when>
						<xsl:when test="starts-with(name(.), &quot;AttachedRegKey_&quot;)">
						</xsl:when>

						<!-- Remove these in the future, they are only for early release compatibility -->
						<xsl:when test="name(.) = &quot;FlyTrap_AttachFiles&quot;">
						</xsl:when>
						<xsl:when test="starts-with(name(.), &quot;FlyTrap_AttachFiles_&quot;)">
						</xsl:when>
						<xsl:when test="name(.) = &quot;FlyTrap_AttachRegKeys&quot;">
						</xsl:when>
						<xsl:when test="starts-with(name(.), &quot;FlyTrap_AttachRegKeys_&quot;)">
						</xsl:when>

						<xsl:when test="name(.) = &quot;ptime&quot;">
					<tr>
						<td>Application Uptime (milliseconds since start)</td><td colspan="5"><xsl:value-of select="."/></td>
					</tr>
						</xsl:when>
						<xsl:otherwise>
					<tr>
						<td><xsl:value-of select="name(.)"/></td><td colspan="5"><xsl:value-of select="."/></td>
					</tr>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:for-each>
				</tbody>
			</table>
		</div>


  		<div style="background-color:white"><br/></div>


		<div class="displayTable">
			<table>
				<thead>
				<tr>
					<td colspan="4">Dump File</td>
				</tr>
				<tr>
					<td>Filename</td>
					<td>Size</td>
					<td>Mime Type</td>
					<td>Dump Summary</td>
				</tr>
				</thead>
				<tbody>
			    <xsl:for-each select="UploadedDebugReport/Files/*">
			    	<tr>
						<td valign="top">
							<a>
								<xsl:attribute name="href">minidump.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;dfile=0&amp;type=bin</xsl:attribute>
								<span style="font-weight:bold"><xsl:value-of select="Name"/></span>
							</a>
						</td>
						<td valign="top"><xsl:value-of select="Size"/></td>
						<td valign="top"><xsl:value-of select="Type"/></td>
						<td valign="top" xml:space="preserve">
							<!-- <div style="color:Blue">
								<xsl:attribute name="onclick">javascript:ShowStackWalk(this, '<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>', 0);</xsl:attribute>
								Show Stack Trace
							</div> -->
							<a><xsl:attribute name="href">minidump.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;dfile=0&amp;type=html</xsl:attribute>HTML</a>
							<a><xsl:attribute name="href">minidump.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;dfile=0&amp;type=text</xsl:attribute>Text</a> 
							<a><xsl:attribute name="href">minidump.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;dfile=0&amp;type=xml</xsl:attribute> XML</a> 
							<a><xsl:attribute name="href">minidump.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;dfile=0&amp;type=raw</xsl:attribute>Raw</a> 
							<a><xsl:attribute name="href">minidump.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;dfile=0&amp;type=json</xsl:attribute>JSON</a>
						</td>
			    	</tr>
        		</xsl:for-each>
				</tbody>
			</table>
		</div>

  		<div style="background-color:white"><br/></div>

		<div class="displayTable">
			<table>
				<thead>
					<tr>
						<td colspan="3">Attached Files</td>
					</tr>
					<tr>
						<td width="20">#</td>
						<td>File Path</td>
						<td><nobr>File Size</nobr></td>
					</tr>
				</thead>
				<tbody>
				<xsl:for-each select="UploadedDebugReport/Parameters/*">
					<tr>
						<xsl:if test="starts-with(name(.), &quot;AttachedFile_&quot;)">
							<td>
								<xsl:value-of select="substring-after(name(.), &quot;AttachedFile_&quot;)"/>
							</td>
							<td>
								<xsl:choose>
									<xsl:when test="substring-before(substring-after(., &quot;;&quot;), &quot;;&quot;) = &quot;-2&quot;">
										<xsl:value-of select="substring-before(., &quot;;&quot;)"/>
									</xsl:when>
									<xsl:when test="substring-before(substring-after(., &quot;;&quot;), &quot;;&quot;) = &quot;0&quot;">
										<xsl:value-of select="substring-before(., &quot;;&quot;)"/>
									</xsl:when>
									<xsl:otherwise>
								<a>
									<xsl:attribute name="href">reportsummary.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;afile=<xsl:value-of select="substring-after(name(.), &quot;AttachedFile_&quot;)"/></xsl:attribute>
								<xsl:value-of select="substring-before(., &quot;;&quot;)"/>
								</a>
									</xsl:otherwise>
								</xsl:choose>
							</td>
							<td>
								<xsl:choose>
									<xsl:when test="substring-before(substring-after(., &quot;;&quot;), &quot;;&quot;) = &quot;-2&quot;">
										<nobr><span class="itemnotfound">File Not Found</span></nobr>
									</xsl:when>
									<xsl:when test="substring-before(substring-after(., &quot;;&quot;), &quot;;&quot;) = &quot;0&quot;">
										<nobr><font class	="itemempty">0</font></nobr>
									</xsl:when>
									<xsl:otherwise>
										<nobr><xsl:value-of select="substring-before(substring-after(., &quot;;&quot;), &quot;;&quot;)"/></nobr>
									</xsl:otherwise>
								</xsl:choose>
							</td>
						</xsl:if>
						<!-- This handles old style file attachment names -->
						<xsl:if test="starts-with(name(.), &quot;FlyTrap_AttachFiles_&quot;)">
							<td>
								<xsl:value-of select="substring-after(name(.), &quot;FlyTrap_AttachFiles_&quot;)"/>
							</td>
							<td>
								<xsl:choose>
									<xsl:when test="substring-before(substring-after(., &quot;;&quot;), &quot;;&quot;) = &quot;-2&quot;">
										<xsl:value-of select="substring-before(., &quot;;&quot;)"/>
									</xsl:when>
									<xsl:when test="substring-before(substring-after(., &quot;;&quot;), &quot;;&quot;) = &quot;0&quot;">
										<xsl:value-of select="substring-before(., &quot;;&quot;)"/>
									</xsl:when>
									<xsl:otherwise>
								<a>
									<xsl:attribute name="href">reportsummary.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;afile=<xsl:value-of select="substring-after(name(.), &quot;FlyTrap_AttachFile_&quot;)"/></xsl:attribute>
								<xsl:value-of select="substring-before(., &quot;;&quot;)"/>
								</a>
									</xsl:otherwise>
								</xsl:choose>
							</td>
							<td>
								<xsl:choose>
									<xsl:when test="substring-before(substring-after(., &quot;;&quot;), &quot;;&quot;) = &quot;-2&quot;">
										<nobr><span class="itemnotfound">File Not Found</span></nobr>
									</xsl:when>
									<xsl:when test="substring-before(substring-after(., &quot;;&quot;), &quot;;&quot;) = &quot;0&quot;">
										<nobr><span class="itemempty">0</span></nobr>
									</xsl:when>
									<xsl:otherwise>
										<nobr><xsl:value-of select="substring-before(substring-after(., &quot;;&quot;), &quot;;&quot;)"/></nobr>
									</xsl:otherwise>
								</xsl:choose>
							</td>
						</xsl:if>
					</tr>
				</xsl:for-each>
				</tbody>
			</table>
		</div>

  		<div style="background-color:white"><br/></div>

				<div class="displayTable">
			<table>
				<thead>
				<tr>
					<td colspan="2">Attached Registry Keys</td>
				</tr>
				<tr xml:space="preserve">
					<td width="20">#</td>
					<td>Base Path</td>
					<td></td>
				</tr>
				</thead>
				<tbody>
				<xsl:for-each select="UploadedDebugReport/Parameters/*">
					<tr>
						<xsl:if test="starts-with(name(.), &quot;AttachedRegKey_&quot;)">
							<td>
								<xsl:value-of select="substring-after(name(.), &quot;AttachedRegKey_&quot;)"/>
							</td>
							<td><xsl:value-of select="substring-before(., &quot;;&quot;)"/></td>
							<td>
								<xsl:choose>
									<xsl:when test="contains(substring-after(., &quot;;&quot;), &quot;!!Error: Could not open key: 2&quot;)">
										<span class="itemnotfound">
											Registry key not found.
										</span>
									</xsl:when>
									<xsl:otherwise>
									<span xml:space="preserve"><a><xsl:attribute name="href">reportsummary.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;rfile=<xsl:value-of select="substring-after(name(.), &quot;FlyTrap_AttachRegKeys_&quot;)"/>&amp;mode=view</xsl:attribute>View</a> - <a><xsl:attribute name="href">reportsummary.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;rfile=<xsl:value-of select="substring-after(name(.), &quot;AttachedRegKey_&quot;)"/></xsl:attribute>Download</a></span>
									</xsl:otherwise>
								</xsl:choose>
							</td>

						</xsl:if>
						<!-- This handles old style registry attachment names -->
						<xsl:if test="starts-with(name(.), &quot;FlyTrap_AttachRegKeys_&quot;)">
							<td>
								<xsl:value-of select="substring-after(name(.), &quot;FlyTrap_AttachRegKeys_&quot;)"/>
							</td>
							<td><xsl:value-of select="substring-before(., &quot;;&quot;)"/></td>
							<td>
								<xsl:choose>
									<xsl:when test="contains(substring-after(., &quot;;&quot;), &quot;!!Error: Could not open key: 2&quot;)">
										<span class="itemnotfound">
											Registry key not found.
										</span>
									</xsl:when>
									<xsl:otherwise>
									<span xml:space="preserve"><a><xsl:attribute name="href">reportsummary.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;rfile=<xsl:value-of select="substring-after(name(.), &quot;FlyTrap_AttachRegKeys_&quot;)"/>&amp;mode=view</xsl:attribute>View</a> - <a><xsl:attribute name="href">reportsummary.php?dumpid=<xsl:value-of select="/UploadedDebugReport/Parameters/FlyTrapCrashId"/>&amp;rfile=<xsl:value-of select="substring-after(name(.), &quot;FlyTrap_AttachRegKeys_&quot;)"/></xsl:attribute>Download</a></span>
									</xsl:otherwise>
								</xsl:choose>
							</td>
						</xsl:if>
					</tr>
				</xsl:for-each>
				</tbody>
			</table>
		</div>
		<div>
		<a href="index.php">Crash Index</a>
		</div>

	</body>
</html>
