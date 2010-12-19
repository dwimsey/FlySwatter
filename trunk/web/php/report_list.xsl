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
						<td>Date Submitted</td>
					</tr>
				</thead>
				<tbody>
					<xsl:for-each select="ErrorReports/ReportInfo/.">
						<xsl:sort select="SubmissionTimestamp" data-type="number" order="descending" />
						<tr>
							<td>
								<a>
								<xsl:attribute name="href">reportsummary.php?dumpid=<xsl:value-of select="DumpId"/></xsl:attribute>
								<xsl:value-of select="DumpId"/>
								</a>
							</td>
							<td>
								<xsl:value-of select="DateSubmitted"/>
							</td>
						</tr>
					</xsl:for-each>
				</tbody>
			</table>
		</div>
	</body>
</html>
