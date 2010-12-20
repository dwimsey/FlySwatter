<?xml version="1.0" encoding="UTF-8"?>
<html xsl:version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
	<head>
		<title>Minidump Summary</title>
		<link rel="stylesheet" href="theme.php?type=css" type="text/css"/>
	</head>
	<body>
		<div class="displayTable">
			<table>
				<thead>
					<tr>
						<td align="left">General Information</td>
						<td style="width=100%" colspan="3"></td>
					</tr>
				</thead>
				<tbody>
					<tr>
						<td align="right">Operating System<span style="color:blue">:</span></td>
						<td><xsl:value-of select ="/MinidumpSummary/OS/Name"/></td>
						<td align="right">Processor Configuration<span style="color:blue">:</span></td>
						<td><xsl:value-of select ="/MinidumpSummary/CPU/Architecture"/><span style="color:blue"> x </span><xsl:value-of select ="/MinidumpSummary/CPU/Count"/></td>
					</tr>
					<tr>
						<td align="right">Version<span style="color:blue">:</span></td>
						<td align="left"><xsl:value-of select ="/MinidumpSummary/OS/Version"/></td>
						<td align="right">Model<span style="color:blue">:</span></td>
						<td align="left"><xsl:value-of select ="/MinidumpSummary/CPU/Model"/></td>
					</tr>
				</tbody>
			</table>
		</div>
		<br/>
		<div class="displayTable">
			<table>
				<thead>
				<tr>
					<td colspan="2">Threads</td>
				</tr>
				<tr>
					<td width="3%">Thread</td>
					<td><nobr>Stack Trace</nobr></td>
				</tr>
				</thead>
				<tbody>
			    <xsl:for-each select="/MinidumpSummary/Threads/ThreadInfo">
			    	<tr>
			    	<xsl:choose>
			    		<xsl:when test="HasEntryPoint = &quot;1&quot;">
						</xsl:when>
						<xsl:otherwise>
							<td valign="top"><nobr>#<xsl:value-of select="ThreadNumber"/></nobr></td>
							<td valign="top">
								<table width="100%">
									<thead>
										<tr>
											<td>Depth</td>
											<td>Module</td>
											<td>Function</td>
											<td>File</td>
											<td>Line</td>
											<td>Address</td>
										</tr>
									</thead>
									<tbody>
								<xsl:for-each select="StackEntry">
										<tr>
											<td><xsl:value-of select="Depth"/></td>
											<td><xsl:value-of select="Module"/></td>
											<td><nobr><xsl:value-of select="Function"/></nobr></td>
											<td><xsl:value-of select="substring-after(File, &quot;\&quot;)"/></td>
											<td><xsl:value-of select="Line"/></td>
											<td><xsl:value-of select="Address"/></td>
										</tr>
								</xsl:for-each>
									</tbody>
								</table>	
							</td>
						</xsl:otherwise>
					</xsl:choose>
			    	</tr>
					<tr><td colspan="2"><hr/></td></tr>
        		</xsl:for-each>
				</tbody>
			</table>
		</div>
		<br/>
		<div class="displayTable">
			<table>
				<thead>
				<tr>
					<td colspan="6">Modules</td>
				</tr>
				<tr>
					<td>Filename</td>
					<td>Version</td>
					<td>PDB File</td>
					<td>Symbol Id</td>
					<td>BaseAddress</td>
					<td>UpperAddress</td>
				</tr>
				</thead>
				<tbody>
			    <xsl:for-each select="/MinidumpSummary/ModuleInfo">
			    	<tr>
			    	<xsl:choose><xsl:when test="HasEntryPoint = &quot;1&quot;">
			    		<td valign="top" class="hilightedrow"><xsl:value-of select="Name"/></td>
						<td valign="top" class="hilightedrow"><xsl:value-of select="Version"/></td>
						<td valign="top" class="hilightedrow"><xsl:value-of select="PDBFile"/></td>
						<td valign="top" class="hilightedrow"><xsl:value-of select="Checksum"/></td>
						<td valign="top" class="hilightedrow"><xsl:value-of select="LowAddress"/></td>
						<td valign="top" class="hilightedrow"><xsl:value-of select="HighAddress"/></td>
					</xsl:when><xsl:otherwise>
						<td valign="top"><xsl:value-of select="Name"/></td>
						<td valign="top"><xsl:value-of select="Version"/></td>
						<td valign="top"><xsl:value-of select="PDBFile"/></td>
						<td valign="top"><xsl:value-of select="Checksum"/></td>
						<td valign="top"><xsl:value-of select="LowAddress"/></td>
						<td valign="top"><xsl:value-of select="HighAddress"/></td>
					</xsl:otherwise></xsl:choose>
			    	</tr>
        		</xsl:for-each>
				</tbody>
			</table>
		</div>
	</body>
</html>

