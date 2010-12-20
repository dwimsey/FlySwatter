<?
if($_GET['type'] == 'css') {
	header("Content-Type: text/css");
?>
.hilightedRow {
	color:				Black;
	background-color:	#e4e4ff;
	padding:			4px;
}
.displayTable {
	background-color:	#3A5099;
	color:				White;
	padding:			2px
}

.displayTable table {
	width:				100%;
	padding:			0px;
	border-spacing:		0px;
	border:				0px;
}

.displayTable table thead tr:first-child {
	background-color:	#3A5099;
	color:				white;
	padding:			4px
}
		
.displayTable table thead tr {
	background-color:	white;
	color:				black;
	padding:			4px;
}
		
.displayTable table tbody tr {
	background-color:	white;
	color:				black;
	padding:			4px;
}

.itemnotfound {
	color:				red;
}

<?
}
?>

