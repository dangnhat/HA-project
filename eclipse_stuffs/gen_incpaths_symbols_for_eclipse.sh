#!/bin/bash
# This file generate .xml file in eclipse format which contains all folders 
# holding header files in RIOT (except boards)
#

################################################################################
# Configuration variables
#	folders contain include paths
RIOT_ROOT=`pwd`/../../RIOT
RIOT_BOARD=mboard-1
HA_LIBS=`pwd`/../libs

#	defined symbols
DEFINED_SYMS="USE_STDPERIPH_DRIVER"

#	output_filename
OUTFILE="incpaths_symbols.xml"
################################################################################

# Temp file
TEMP_FILE=/tmp/elipse_gen_tmp.txt

# Eclipse xml headers and trailers
XML_HEADER="<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
CDT_HEADER="<cdtprojectproperties>"
CDT_TRAILER="</cdtprojectproperties>"
SECTION_TRAILER="</section>"
LANG_TRAILER="</language>"

SECTIONS=("<section name=\"org.eclipse.cdt.internal.ui.wizards.settingswizards.IncludePaths\">" \
"<section name=\"org.eclipse.cdt.internal.ui.wizards.settingswizards.Macros\">")

INCLUDE_LANGS=("<language name=\"Assembly Source File\">" \
"<language name=\"C Source File\">" \
"<language name=\"C++ Source File\">" \
"<language name=\"Object File\">")
INCLUDE_HEADER="<includepath>"
INCLUDE_TRAILER="</includepath>"

MACRO_LANG=("<language name=\"Assembly Source File\">" \
"<language name=\"C Source File\">" \
"<language name=\"C++ Source File\">" \
"<language name=\"Object File\">")

MACRO_HEADER="<macro>"
MACRO_TRAILER="</macro>"
NAME_HEADER="<name>"
NAME_TRAILER="</name>"
VALUE_HEADER="<value/>"

# main program
RIOT_ABS_ROOT=$(readlink -e $RIOT_ROOT)
HA_LIBS_ABS_ROOT=$(readlink -e $HA_LIBS)

clear
echo "*** Eclipse include paths and defined symbols xml file generator ***"
echo "* Author: Pham Huu Dang Nhat, Ho Chi Minh Univeristy of Technology *"
echo "* Version: 1.0, Date: 12-Oct-2014                                  *"
echo "********************************************************************"
echo
echo "*** Configured parameters ***"
echo "	HA libs folder: $HA_LIBS_ABS_ROOT"
echo "	RIOT root folder: $RIOT_ABS_ROOT"
echo "	RIOT board: $RIOT_BOARD"
echo "	Defined symbols: $DEFINED_SYMS"
echo "	Output file: $OUTFILE"
echo
echo "Please verify above parameters before continue. Correct? (Y/n)"
while true; do
	read REP
	if [ $REP = "Y" -o $REP = "y" ]; then
		break
	fi
	
	if [ $REP = "n" -o $REP = "N" ]; then
		exit 0
	fi
done

# Put all folders which contain header file(s) in a temp file.
touch $TEMP_FILE
echo
echo "*** Finding header files in ... ***"

echo "	$HA_LIBS_ABS_ROOT"
find $HA_LIBS_ABS_ROOT -name "*.h" -printf "%h\n"|sort -u >> $TEMP_FILE

echo "	$RIOT_ABS_ROOT (except boards and tests)"
echo "	$RIOT_ABS_ROOT/boards/$RIOT_BOARD"
find $RIOT_ABS_ROOT -type d \( -path */boards -o -path */tests \) \
-prune -o -type f -name '*.h' |sed 's#\(.*\)/.*#\1#' |sort -u >> $TEMP_FILE
find $RIOT_ABS_ROOT/boards/$RIOT_BOARD -name "*.h" -printf "%h\n"|sort -u >> $TEMP_FILE

echo "" >> $TEMP_FILE

echo
echo "*** Converting to xml ***"
echo

rm -f $OUTFILE
touch $OUTFILE

# echo file header
echo $XML_HEADER >> $OUTFILE
echo $CDT_HEADER >> $OUTFILE

for SEC in "${SECTIONS[@]}";
do
	# echo section header
	echo $SEC >> $OUTFILE
	
	if [ "$SEC" = '<section name="org.eclipse.cdt.internal.ui.wizards.settingswizards.IncludePaths">' ]; then
		for LANG in "${INCLUDE_LANGS[@]}";
		do
			# echo language header
			echo $LANG >> $OUTFILE
			
			if [ "$LANG" != '<language name="Object File">' ]; then
				# echo include paths
				for LINE in $(cat $TEMP_FILE);
				do
					echo "${INCLUDE_HEADER}${LINE}${INCLUDE_TRAILER}" >> $OUTFILE
				done
			fi
			
			# echo language trailer
			echo "" >> $OUTFILE
			echo $LANG_TRAILER >> $OUTFILE
		done
	fi
	
	if [ "$SEC" = '<section name="org.eclipse.cdt.internal.ui.wizards.settingswizards.Macros">' ]; then
		for LANG in "${MACRO_LANG[@]}";
		do
			# echo language header
			echo $LANG >> $OUTFILE
			
			if [ "$LANG" != '<language name="Object File">' ]; then
				for LINE in $DEFINED_SYMS; do
					# echo macro header
					echo $MACRO_HEADER >> $OUTFILE
				
					# echo line
					echo "${NAME_HEADER}${LINE}${NAME_TRAILER}${VALUE_HEADER}" >> $OUTFILE
			
					# echo macro trailer
					echo $MACRO_TRAILER >> $OUTFILE
				done
			fi
			
			# echo language trailer
			echo "" >> $OUTFILE
			echo $LANG_TRAILER >> $OUTFILE			
		done
	fi
	
	# echo section trailer
	echo "" >> $OUTFILE
	echo $SECTION_TRAILER >> $OUTFILE
	
done

#echo file trailer
echo $CDT_TRAILER >> $OUTFILE

echo "Done!"

rm -f $TEMP_FILE
