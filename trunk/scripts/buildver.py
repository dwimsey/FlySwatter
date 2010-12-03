#!/usr/bin/env python
debugging = 1
import sys		# System/posix methods
import os		# OS Info methods
import re		# Regular Expressions
import subprocess	# For POpen calls
import time		# For sleep

def appendVB6Property(outputarray, propertyname, proptype, propvalue):
	outputarray.append("Public Property Get " + propertyname + "() As " + proptype)
	outputarray.append("    " + propertyname + " = " + propvalue)
	outputarray.append("End Property")
	return

argc = len(sys.argv)

# Set default repository info
cvs_project_branch = "NONE"
cvs_project_tag = "HEAD"
cvs_cur_buildcount = 0
cvs_cur_filecount = 0
cvs_root = ""
cvs_repository = ""
cvs_ignores = []

startup_cwd = os.getcwd()

if debugging > 2:
	print "Startup directory is: %s" % startup_cwd

# Setup the defaults and validate command line args
if argc == 1:
	work_path = "."
	buildinfo_file = ".buildinfo"
elif argc == 2:
	work_path = sys.argv[1]
	buildinfo_file = os.path.join(work_path, ".buildinfo")
elif argc == 3:
	work_path = sys.argv[1]
	buildinfo_file = sys.argv[2]
else:
	print "usage: buildver.py [buildinfo file] [working directory]"
	exit()

tn = time.gmtime(None)	# Take note of the time

# END Setup the defaults and validate command line args

# Generate normalized paths

normalized_work_path = os.path.normpath(os.path.abspath(work_path))
os.chdir(normalized_work_path)
normalized_buildinfo_file = os.path.normpath(os.path.abspath(buildinfo_file))

if debugging > 2:
	print "     Input Working directory: %s" % work_path
	print "Normailzed Working directory: %s" % normalized_work_path
	print "             Input Info file: %s" % buildinfo_file
	print "        Normalized Info file: %s" % normalized_buildinfo_file
# END Generate normalized paths


if debugging > 0:
	print "Generating version include files in %s using rules from %s" % (normalized_work_path, normalized_buildinfo_file)
# Read .buildinfo file

output_type = ""
output_file = ""
try:
	bifh = open(buildinfo_file)
	try:
		cvs_project_branch = bifh.readline().strip()
		major_version = int(bifh.readline().strip())
		minor_version = int(bifh.readline().strip())
		patchlevel = int(bifh.readline().strip())
		output_type = bifh.readline().strip()
		output_file = bifh.readline().strip()
		output_prefix = bifh.readline().strip()
		ignore_paths = bifh.readline().strip()
	finally:
		bifh.close()

except IOError, (errno, strerror):
	print "Could not parse buildinfo file: %s: I/O error(%d): %s" % (normalized_buildinfo_file, errno, strerror)
	exit(255)

except Exception, ex:
	print "Could not parse buildinfo file: %s: unknown error: %s" % (normalized_buildinfo_file, ex)
	exit(255)

if debugging > 1:
	print "Build Information:"
	print "Major Version: %d" % major_version
	print "Minor Version: %d" % minor_version
	print "Patch Level: %d" % patchlevel
# END .buildinfo file


# Determines output types and files
output_types = output_type.lower().split(",")
output_files = output_file.lower().split(",")

argc = len(sys.argv)
if len(output_types) == len(output_files):
	if debugging > 4:
		print "Output file count: %d" % len(output_types)
else:
	print "Output types and output file counts are not equal, please correct \'.buildinfo\' and rerun.  Types: %d  Files: %d" % (len(output_types), len(output_files))
	exit(255)

for otype in output_types:
	if debugging > 4:
		print "Output type input: ", otype
	
	dstr = ""
	if otype == 'c':
		dstr = "Output file type: C: "
	elif otype == 'nsis':
		dstr = "Output file type: NSIS: "
	elif otype == 'nsis_productversion':
		dstr = "Output file type: NSIS Product Version: "
	elif otype == 'vb6class':
		dstr = "Output file type: VisualBasic 6 Class File: "
	elif otype == 'console':
		dstr = "Output file type: Console: "
	elif otype == 'xml':
		dstr = "Output file type: XML: "
	elif otype == 'dgs':
		dstr = "Output file type: DGS Package: "
	else:
		print "Unsupported output file type: %s" % otype
		exit(255)

	if debugging > 3:
		print dstr
		dstr = ""

# END Determines output types and files


rcs_workdir = normalized_work_path + "/.svn"
if os.path.exists(rcs_workdir):
	if debugging > 4:
		print "Working with Subversion"

# Find a SVN binary

	svn_paths = []
	svn_paths.append("/usr/local/bin/svnversion")
	svn_paths.append("/usr/bin/svnversion")
	svn_paths.append("/opt/bin/svnversion")
	svn_paths.append("C:\\Program Files\\Utilities\\svnversion.exe")
	svn_paths.append("C:\\Program Files\\Subversion\\svnversion.exe")
	svn_paths.append("C:\\Program Files\\Subversion\\bin\\svnversion.exe")

	svn_binary = ""
	
	for tpath in svn_paths:
		if os.path.exists(tpath):
			svn_binary = tpath
			break
	
	if svn_binary == "":
		print "Could not find svnversion binary?"
		exit(255)
	
	if debugging > 3:
		print "SVN Binary is located at: %s" % svn_binary

	
# END Find a SVN binary

	cvs_project_branch = "NONE"
	cvs_project_tag = ""
	svn_rev_flags = ""
	svn_mixed_base_rev = 0

	svn_version_status = subprocess.Popen([svn_binary], stdout=subprocess.PIPE).communicate()[0].splitlines()
	svnversion_output = svn_version_status[0]

	try:
		cvs_cur_buildcount = int(svnversion_output)
	except Exception, ex:
		print "This does not appear to be a clean revision, generating psuedo version information ..."
		base_rev = 0
		cur_rev = ""
		cur_flags = ""
		for c in svnversion_output:
			if c.isdigit():
				cur_rev += c
			else:
				if c == ':':
					# This is the first part of a mixed revision,
					# store the first part
					base_rev = int(cur_rev)
					# Clear cur_rev so it can find the high revision number
					cur_rev = ""
				else:
					# This is a flag if its not a colon or a number
					
					# Is this the first flag?
					cur_flags += c

		cvs_cur_buildcount = int(cur_rev)
		if base_rev > 0:
			cvs_project_tag = str(base_rev) + ":" + str(cvs_cur_buildcount) + cur_flags
		else:
			cvs_project_tag = str(cur_flags)
			
	
	cvs_cur_filecount = 0
	cvs_root = ""
	cvs_repository = ""
	cvs_ignores = []
	cvs_files_str = ""
	
	if debugging > 3:
		print "SVN revision: %s" % cvs_cur_buildcount

else:

#rcs_workdir = normalized_work_path + "/CVS"
#if os.path.exists(rcs_workdir):
#	if debugging > 4:
#		print "Working with CVS log parser"

# START Get CVS Version Info

# Parse ignores, if we have any cvs_ignores
	input_ignores = ignore_paths.split(";")
	for ign in input_ignores:
		if debugging > 4:
			print "Ignore: %s" % ign
	
		if os.path.exists(ign):
			if debugging > 3:
				print "Ignore Valid: %s" % ign
	
			cvs_ignores.append(ign)
		else:
			if ign == "":
				print "Blank ignore entry found."
			else:
				if debugging > 1:
					print "Ignore not found, adding anyway: %s" % ign
	
				cvs_ignores.append(ign)
	
# END Parse ignores, if we have any
	
# Find a CVS binary
	
	cvs_paths = []
	cvs_paths.append("/usr/local/bin/cvsnt")
	cvs_paths.append("/usr/bin/cvsnt")
	cvs_paths.append("/bin/cvsnt")
	cvs_paths.append("/usr/local/bin/cvs")
	cvs_paths.append("/usr/bin/cvs")
	cvs_paths.append("/bin/cvs")
	cvs_paths.append("C:\\Program Files\\GNU\\WinCvs 1.3\\CVSNT\\cvs.exe")
	cvs_paths.append("C:\\Program Files\\GNU\\WinCvs 1.3\\CVS\\cvs.exe")
	cvs_paths.append("C:\\Program Files\\GNU\\CVSNT\\cvs.exe")
	cvs_paths.append("C:\\Program Files\\GNU\\CVS\\cvs.exe")
	cvs_paths.append("C:\\Program Files\\CVSNT\\cvs.exe")
	cvs_paths.append("C:\\Program Files\\CVS\\cvs.exe")
	
	cvs_binary = ""
	
	for tpath in cvs_paths:
		if os.path.exists(tpath):
			cvs_binary = tpath
			break
	
	if cvs_binary == "":
		print "Could not find cvs binary?"
		exit(255)
	
	if debugging > 3:
		print "CVS Binary is located at: %s" % cvs_binary
	
# END Find a CVS binary
	
	
# Everything is prepaired to run CVS and get the log output
	if debugging > 0:
		print "Retreiving version information ..."
	
	cvs_status = subprocess.Popen([cvs_binary, "-q", "status"], stdout=subprocess.PIPE).communicate()[0].splitlines()
	
	if debugging > 254:
		print "CVS Status Output:"
		print cvs_status
	
	if len(cvs_status) < 5:
		# cvs status returned too little data, probably couldn't find the binary, error out anyway
		print "cvs status returned < 5 lines, this probably isnt' valid, aborting."
		exit(255)
	
	cvs_cur_file = ""
	cvs_cur_file_rev = ""
	cvs_cur_file_tag = "HEAD"
	cvs_files_str = ""
	
	vc = 0
	# Cycle the lines of status and parse out the revision and tag information for each file
	for nline in cvs_status:
		dstr = ""
		line = nline.strip().replace("\t", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ")
	
		m = re.match("File: (.*) Status:", line)
		if m:
			# this line contains our filename
			cvs_cur_file = m.group(1)
			if debugging > 4:
				print "VC Info for: %s" % cvs_cur_file
	
	
		m = re.search("Repository revision: ([0-9.]+) (.*)$", line)
		if m:
			cvs_cur_file_rev = m.group(1)
			cvs_cur_file_fullpath = m.group(2)
			vers = cvs_cur_file_rev.strip().split(".")
			vc = 0
			for rv in vers:
				vc += int(rv)
	
			if debugging > 3:
				print "Got Revision Info: %s: %s" % (cvs_cur_file_fullpath, cvs_cur_file_rev)
		
	
		m = re.search("Sticky Tag: (.*)", line)
		if m:
			ctag = m.group(1).strip()
			if ctag.lower() == "(none)":
				cvs_cur_file_tag = "HEAD"
			else:
				cvs_cur_file_tag = ctag.split(" ")[0]
	
			if debugging > 4:
				print "CVS File Tag: %s: %s" % (cvs_cur_file_fullpath, cvs_cur_file_tag)
	
		m = re.search("Sticky Date:", line)
		if m:
			if cvs_cur_file.lower() == ".buildinfo" and cvs_project_tag == "HEAD":
				cvs_project_tag = cvs_cur_file_tag
			else:
				# Check to see if this file should be ignored
				should_ignore = 0
				for this_ignore in cvs_ignores:
					if debugging > 5:
						print "Checking ignore: %s: %s" % (this_ignore, cvs_cur_file_fullpath)
					mm = re.search(this_ignore, cvs_cur_file_fullpath)
					if mm:
						if debugging > 4:
							print "Ignoring %s" % (this_ignore)
						should_ignore = 1
						break
	
				if should_ignore == 0:
					cvs_cur_buildcount += vc
					if cvs_cur_filecount == 0:
						pass
						cvs_files_str = cvs_cur_file
						cvs_files_str += ": "
						cvs_files_str +=  cvs_cur_file_rev
					else:
						pass
						cvs_files_str += "\\r\\n"
						cvs_files_str += cvs_cur_file
						cvs_files_str += ": "
						cvs_files_str += cvs_cur_file_rev
	
					cvs_cur_filecount += 1
					dstr = "Included file: "
					dstr += cvs_cur_file
				else:
					dstr = "Ignored file: "
					dstr +=  cvs_cur_file
	
				if debugging > 4:
					print dstr
					dstr = ""
	
			
			cvs_cur_file = ""
			cvs_cur_file_rev = ""
			cvs_cur_file_branch = ""
			cvs_cur_file_tag = ""
			vc = 0
	
	if debugging > 1:
		print " Done"

# END Get CVS Version Info

# Write output files, checking for changes before updating existing files
cvs_files_str = cvs_files_str.strip()
output_file_index = 0
output_files_updated = 0
for otype in output_types:

	ofile = output_files[output_file_index]
	output_file_index += 1  # increment now so if we call next to skip part of the loop things will still work right
	output_text = []
	if debugging > 3:
		print "Output type: ", otype

	dotype = otype.lower()
	if dotype == 'console':
		output_text.append( "     CVS Root: " + cvs_root + " Repository: " + cvs_repository)
		output_text.append( "       Branch: " + cvs_project_branch + "\tTag: " + cvs_project_tag + "\tVersion: " + major_version + "." + minor_version + "\tBuild: " + cvs_cur_buildcount)
		output_text.append( "        Files: " + cvs_cur_filecount)
		output_text.append( "  Output Type: " + otype)
		output_text.append( "  Output File: " + ofile)
		output_text.append( "Output Prefix: " + output_prefix)
	elif dotype == 'c':
		output_text.append( "// Autogenerated by buildver.py (Revision: 2.0)  Copyright 2005-20010, David Wimsey.  http://www.notresponsible.org/")
		output_text.append("#define " + output_prefix + "VERSION_MAJOR\t" + str(major_version))
		output_text.append("#define " + output_prefix + "VERSION_MINOR\t" + str(minor_version))
		output_text.append("#define " + output_prefix + "VERSION_PATCH\t" + str(patchlevel))
		output_text.append("#define " + output_prefix + "VERSION_BUILD\t" + str(cvs_cur_buildcount))
		output_text.append("#define " + output_prefix + "STRPRIVATEBUILD\t\"" + cvs_project_branch + "\"")
		output_text.append("#define " + output_prefix + "STRSPECIALBUILD\t\"" + cvs_project_tag + "\"")
		#output_text.append("#define " + output_prefix + "VERSION_STRINGS\t\"" + cvs_files_str + "\"")
		output_text.append("#define " + output_prefix + "BUILD_UTIME\t\"" + str(tn) + "\"")

		bt = time.strftime("%a %b %d %H:%M:%S %Y", tn)
		st = time.strftime("%H:%M:%S", tn)
		dt = time.strftime("%a %b %d, %Y", tn)
		output_text.append("#define " + output_prefix + "VERSION_BUILDTIME\t\"" + bt + "\"")
		output_text.append("#define " + output_prefix + "STRBUILDTIME\t\"Built at " + st + " on " + dt + "\"")
		output_text.append("#define " + output_prefix + "STRPRODUCTVERSION \"" + str(major_version) + "." + str(minor_version) + "." + str(patchlevel) + "." + str(cvs_cur_buildcount) + "\\0\"")
		output_text.append("#define " + output_prefix + "STRFILEVERSION \"" + str(major_version) + "." + str(minor_version) + "." + str(patchlevel) + "." + str(cvs_cur_buildcount) + "\\0\"")
		output_text.append("#define " + output_prefix + "PRODUCTVERSION " + str(major_version) + ", " + str(minor_version) + ", " + str(patchlevel) + ", " + str(cvs_cur_buildcount))
		output_text.append("#define " + output_prefix + "FILEVERSION " + str(major_version) + ", " + str(minor_version) + ", " + str(patchlevel) + ", " + str(cvs_cur_buildcount))

	elif dotype == 'nsis':
		output_text.append("; Autogenerated by buildver.py (2.0)  Copyright 2005-2010, David Wimsey.  http://www.notresponsible.org/")
		output_text.append("")

		output_text.append("VIAddVersionKey /LANG=${LANG_ENGLISH} \"FileVersion\" \"" + str(major_version) + "." + str(minor_version) + "." + str(patchlevel) + "." + str(cvs_cur_buildcount) + "\"")
		output_text.append("VIAddVersionKey /LANG=${LANG_ENGLISH} \"PrivateBuild\" \"" + cvs_project_branch + "\"")
		output_text.append("VIAddVersionKey /LANG=${LANG_ENGLISH} \"SpecialBuild\" \"" + cvs_project_tag + "\"")
		st = time.strftime("%H:%M:%S", tn)
		dt = time.strftime("%a %b %d, %Y", tn)
		output_text.append("VIAddVersionKey /LANG=${LANG_ENGLISH} \"BuildTime\" \"Built at " + st + " on " + dt + "\"")

	elif dotype == 'nsis_productversion':
		output_text.append("; Autogenerated by buildver.py (2.0)  Copyright 2005-2010, David Wimsey.  http://www.notresponsible.org/")
		output_text.append("")
		output_text.append("VIProductVersion \"" + str(major_version) + "." + str( minor_version) + "." + str(patchlevel) + "." + str(cvs_cur_buildcount) + "\"")
		output_text.append("")
		output_text.append("Function InitializeProductVersionTable")
		output_text.append("\tVar /GLOBAL \"ProductProductVersion\"")
		output_text.append("\tVar /GLOBAL \"ProductFileVersion\"")
		output_text.append("\tVar /GLOBAL \"ProductPrivateBuild\"")
		output_text.append("\tVar /GLOBAL \"ProductSpecialBuild\"")
		output_text.append("\tVar /GLOBAL \"ProductBuildTime\"")
		output_text.append("\tVar /GLOBAL \"ProductMajorVersion\"")
		output_text.append("\tVar /GLOBAL \"ProductMinorVersion\"")
		output_text.append("\tVar /GLOBAL \"ProductPatchLevel\"")
		output_text.append("\tVar /GLOBAL \"ProductBuildNumber\"")

		output_text.append("\tStrCpy $ProductProductVersion \"" + str(major_version) + "." + str(minor_version) + "." + str(patchlevel) + "." + str(cvs_cur_buildcount) + "\"")
		output_text.append("\tStrCpy $ProductFileVersion \"" + str(major_version) + "." + str(minor_version) + "." + str(patchlevel) + "." + str(cvs_cur_buildcount) + "\"")
		output_text.append("\tStrCpy $ProductPrivateBuild \"" + cvs_project_branch + "\"")
		output_text.append("\tStrCpy $ProductSpecialBuild \"" + cvs_project_tag + "\"")
		st = time.strftime("%H:%M:%S", tn)
		dt = time.strftime("%a %b %d, %Y", tn)
		output_text.append("\tStrCpy $ProductBuildTime \"Built at " + st + " on " + dt + "\"")
		output_text.append("\tStrCpy $ProductMajorVersion \"" + str(major_version) + "\"")
		output_text.append("\tStrCpy $ProductMinorVersion \"" + str(minor_version) + "\"")
		output_text.append("\tStrCpy $ProductPatchLevel \"" + str(patchlevel) + "\"")
		output_text.append("\tStrCpy $ProductBuildNumber \"" + str(cvs_cur_buildcount) + "\"")
		output_text.append("FunctionEnd")

	elif dotype == 'vb6class':
		output_text.append("VERSION 1.0 CLASS")
		output_text.append("BEGIN")
		output_text.append("MultiUse = -1  'True")
		output_text.append("Persistable = 0  'NotPersistable")
		output_text.append("DataBindingBehavior = 0  'vbNone")
		output_text.append("DataSourceBehavior  = 0  'vbNone")
		output_text.append("MTSTransactionMode  = 0  'NotAnMTSObject")
		output_text.append("END")
		output_text.append("Attribute VB_Name = \"" + output_prefix + "\"")
		output_text.append("Attribute VB_GlobalNameSpace = False")
		output_text.append("Attribute VB_Creatable = False")
		output_text.append("Attribute VB_PredeclaredId = False")
		output_text.append("Attribute VB_Exposed = False")

		output_text.append("' Autogenerated by buildver.py (2.0)  Copyright 2005-2010, David Wimsey.  http://www.notresponsible.org/")
		output_text.append("'")
		output_text.append("'")
		appendVB6Property(output_text, "MajorVersion", "Long", str(major_version))
		appendVB6Property(output_text, "MinorVersion", "Long", str(minor_version))
		appendVB6Property(output_text, "PatchLevel", "Long", str(patchlevel))
		appendVB6Property(output_text, "BuildNumber", "Long", str(cvs_cur_buildcount))
		appendVB6Property(output_text, "Version", "String", "\"" + str(major_version) + "." + str(minor_version) + "." + str(patchlevel) + "." + str(cvs_cur_buildcount) + "\"")

		appendVB6Property(output_text, "PrivateBuild", "String", "\"" + cvs_project_branch + "\"")
		appendVB6Property(output_text, "SpecialBuild", "String", "\"" + cvs_project_tag + "\"")
		#appendVB6Property(output_text, "Files", "String", "\"" + cvs_files_str + "\"")
		appendVB6Property(output_text, "Timestamp", "String", "\"" + str(tn) + "\"")

		bt = time.strftime("%a %b %d %H:%M:%S %Y", tn)
		st = time.strftime("%H:%M:%S", tn)
		dt = time.strftime("%a %b %d, %Y", tn)
		appendVB6Property(output_text, "BuildDate", "String", "\"" + bt + "\"")
		appendVB6Property(output_text, "BornOn", "String", "\"Built at " + st + " on " + dt + "\"")

	elif dotype == 'xml':
		output_text.append("<?xml version=\"1.0\" encoding=\"utf-8\" ?>")
		output_text.append("<VersionInfo>")
		output_text.append("\t<FileMajorVersion>" + str(major_version) + "</FileMajorVersion>")
		output_text.append("\t<FileMinorVersion>" + str(minor_version) + "</FileMinorVersion>")
		output_text.append("\t<FilePatchLevel>" + str(patchlevel) + "</FilePatchLevel>")
		output_text.append("\t<FileBuildNumber>" + str(cvs_cur_buildcount) + "</FileBuildNumber>")
		output_text.append("\t<ProductMajorVersion>" + str(major_version) + "</ProductMajorVersion>")
		output_text.append("\t<ProductMinorVersion>" + str(minor_version) + "</ProductMinorVersion>")
		output_text.append("\t<ProductPatchLevel>" + str(patchlevel) + "</ProductPatchLevel>")
		output_text.append("\t<ProductBuildNumber>" + str(cvs_cur_buildcount) + "</ProductBuildNumber>")
		output_text.append("</VersionInfo>")
		output_text.append("")

	elif dotype == 'dgs':
		output_text.append("<?xml version=\"1.0\" encoding=\"utf-8\" ?>")
		output_text.append("<DGSPackage>")
		output_text.append("\t<DGSVariables>")
		output_text.append("\t\t<DGSVariable name=\"FileMajorVersion\">" + str(major_version) + "</DGSVariable>")
		output_text.append("\t\t<DGSVariable name=\"FileMinorVersion\">" + str(minor_version) + "</DGSVariable>")
		output_text.append("\t\t<DGSVariable name=\"FilePatchLevel\">" + str(patchlevel) + "</DGSVariable>")
		output_text.append("\t\t<DGSVariable name=\"FileBuildNumber\">" + str(cvs_cur_buildcount) + "</DGSVariable>")
		output_text.append("\t\t<DGSVariable name=\"ProductMajorVersion\">" + str(major_version) + "</DGSVariable>")
		output_text.append("\t\t<DGSVariable name=\"ProductMinorVersion\">" + str(minor_version) + "</DGSVariable>")
		output_text.append("\t\t<DGSVariable name=\"ProductPatchLevel\">" + str(patchlevel) + "</DGSVariable>")
		output_text.append("\t\t<DGSVariable name=\"ProductBuildNumber\">" + str(cvs_cur_buildcount) + "</DGSVariable>")
		output_text.append("\t</DGSVariables>")
		output_text.append("\t<DGSFiles>")
		output_text.append("\t</DGSFiles>")
		output_text.append("</DGSPackage>")
		output_text.append("")
	else:
		print "Unexpected output type: ", otype


        
	output_filename = ofile + ".new"
	try:
		output = open(output_filename, "wb")
	except Exception, ex:
		print "Could not open output file: %s.new: %s" % ( ofile, str(ex))
		exit(255)

	try:
		for olline in output_text:
			output.write(olline + "\r\n")

	finally:
		output.close()


	old_text = []
	new_text = []

	try:
		orig_file = open(ofile, "rb")
		orig_file.close()
	except:
		if debugging > 0:
			print "No existing version information, updating " + ofile + ".  Build: " + str(cvs_cur_buildcount)

		try:
			os.unlink(ofile)
		except:
			pass
		os.rename(ofile + ".new", ofile)
		output_files_updated += 1
		continue

	lcount = 0
	oofile = open(ofile)
	for nline in oofile.readlines():
		old_text.append(str(nline))
		lcount += 1
	oofile.close()
	lcount = 0

	try:
		nofile = open(ofile + ".new")

		try:
			for nline in nofile.readlines():
				new_text.append(str(nline))
				lcount += 1


		except Exception, ex:
			print "Error reading new version information. Aborting: ", str(ex)
			exit(255)

		nofile.close()

	except Exception, ex:
		print "Error opening new version information. Aborting: ", str(ex)
		exit(255)


	if len(old_text) != len(new_text):
		if debugging > 0:
			print "Updating " + ofile + "  Build: " + str(cvs_cur_buildcount)

		try:
			os.unlink(ofile)
		except:
			pass
		os.rename(ofile + ".new", ofile)
		output_files_updated += 1
		continue

	file_renamed = 0
	ccount = 0
	lcount = len(new_text)

	while ccount < lcount:
		oline = old_text[ccount]
		nline = new_text[ccount]

		if oline != nline:
			m = re.search("TIME", oline)
			m2 = re.search("TIME", nline)
			if m and m2:
				pass
			else:
				mm = re.search("BuildTime", oline)
				mm2 = re.search("BuildTime", nline)
				if mm and mm2:
					pass
				else:
					print "Changed line (Old): ", oline
					print "             (New): ", nline
					print "Updating " + ofile + ". Build: " + str(cvs_cur_buildcount)
					try:
						os.unlink(ofile)
					except:
						pass
					os.rename(ofile + ".new", ofile)
					file_renamed = 1
					output_files_updated += 1
					ccount = lcount

		ccount += 1


	if file_renamed == 0:
		if debugging > 3:
			print "Removing temporary output: ", ofile, ".new"

		os.unlink(ofile + ".new")


if debugging > 0:
	if output_files_updated == 0:
		print "Version information is up to date.  Build: %d" % cvs_cur_buildcount
	else:
		print "Version information updated.  %d files changed to build %d" % (output_files_updated, cvs_cur_buildcount)

# END Write output files, checking for changes before updating existing files



# Final cleanup
os.chdir(startup_cwd)
exit(0)	# Explicitly return successful exit
