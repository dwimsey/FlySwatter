<?xml version="1.0" encoding="UTF-8"?>
<html xsl:version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
	<head>
		<title>Debug and Crash Reports</title>
		<link rel="stylesheet" href="theme.php?type=css" type="text/css"/>
	</head>
	<body>
		<div class="displayTable">
			<table>
				<thead>
					<tr>
						<td colspan="2">Crash and Debug Reports</td>
					</tr>
					<tr>
						<td>Dump ID</td>
						<td>Product</td>
						<td>Application</td>
						<td>Version</td>
						<td>CPU</td>
						<td>OS</td>
						<td>Date Submitted</td>
						<td>Reason</td>
					</tr>
				</thead>
				<tbody>
					<xsl:for-each select="ErrorReports/ReportInfo/.">
						<xsl:sort select="FileTimestamp" data-type="number" order="descending" />
						<tr>
							<td>
								<a>
								<xsl:attribute name="href">reportsummary.php?dumpid=<xsl:value-of select="DumpId"/></xsl:attribute>
								<xsl:value-of select="DumpId"/>
								</a>
							</td>
							<td>
								<xsl:value-of select="ProductName"/>
							</td>
							<td>
								<xsl:value-of select="AppName"/>
							</td>
							<td>
								<xsl:value-of select="AppVersion"/>
							</td>
							<td>
								<xsl:value-of select="CPUCount"/>@<xsl:value-of select="CPUArchitecture"/>
							</td>
							<td>
								<xsl:choose>
									<xsl:when test="OSName = &quot;Windows NT&quot;">
										<xsl:choose>
											<xsl:when test="OSVersion = &quot;6.1.7600&quot;">
												Windows 7
											</xsl:when>
											<xsl:when test="OSVersion = &quot;6.1.7600 Service Pack 1&quot;">
												Windows 7 SP1
											</xsl:when>
											<xsl:when test="OSVersion = &quot;6.1.7600 Service Pack 2&quot;">
												Windows 7 SP2
											</xsl:when>
											<xsl:when test="OSVersion = &quot;6.1.7600 Service Pack 3&quot;">
												Windows 7 SP3
											</xsl:when>
											<xsl:when test="OSVersion = &quot;6.0.7600&quot;">
												Windows Vista
											</xsl:when>
											<xsl:when test="OSVersion = &quot;6.0.6000 Service Pack 1&quot;">
												Windows Vista SP1
											</xsl:when>
											<xsl:when test="OSVersion = &quot;6.0.6000 Service Pack 2&quot;">
												Windows Vista SP2
											</xsl:when>
											<xsl:when test="OSVersion = &quot;5.1.2600&quot;">
												WindowsXP
											</xsl:when>
											<xsl:when test="OSVersion = &quot;5.1.2600 Service Pack 1&quot;">
												WindowsXP SP1
											</xsl:when>
											<xsl:when test="OSVersion = &quot;5.1.2600 Service Pack 2&quot;">
												WindowsXP SP2
											</xsl:when>
											<xsl:when test="OSVersion = &quot;5.1.2600 Service Pack 3&quot;">
												WindowsXP SP3
											</xsl:when>
											<xsl:otherwise>
												<xsl:value-of select="OSName"/> (<xsl:value-of select="OSVersion"/>)
											</xsl:otherwise>
										</xsl:choose>
									</xsl:when>
									<xsl:otherwise>
										<xsl:value-of select="OSName"/> (<xsl:value-of select="OSVersion"/>)
									</xsl:otherwise>
								</xsl:choose>
							</td>
							<td>
								<xsl:value-of select="DateSubmitted"/>
							</td>
							<td>
								<xsl:choose>
									<xsl:when test="Exception = &quot;&quot;">
										<xsl:choose>
											<xsl:when test="Assertion = &quot;&quot;">
												Debug Report
											</xsl:when>
											<xsl:otherwise>
												<xsl:value-of select="Exception"/>
												@ <xsl:value-of select="CrashData"/>
											</xsl:otherwise>
										</xsl:choose>
									</xsl:when>
									<xsl:otherwise>
										<xsl:value-of select="Exception"/>
										@ <xsl:value-of select="CrashData"/>
									</xsl:otherwise>
								</xsl:choose>
							</td>
						</tr>
					</xsl:for-each>
				</tbody>
			</table>
		</div>
	</body>
</html>
