#!/bin/sh
#********************************************************
#	be/util/mkarchive.sh								*
#********************************************************
#	Droids												*
#	(c) 2000, FRS & RM  								*
#********************************************************
#   Modifiable par :       [ ] FRS         [X] RM       *
#********************************************************
#	Creation d'une distrib PowerPulsar
#********************************************************

PREFIX="(distrib)/PowerPulsar_081_"
SUFFIX=".zip"
NDAT=`date +%d%m%Y`

SRC="PowerPulsar.0.8"

if [ -f "$PREFIX$NDAT$SUFFIX" ]
then
	NDAT=`date +%d%m%Y_%Hh%Mm%S`
fi

NAME="$PREFIX$NDAT$SUFFIX"
echo "Archiving to $NAME"

ZOPT="-9 -y"

cd ..

zip $ZOPT $SRC/$NAME \
		$SRC/PowerPulsar.R5.x86 $SRC/config*.pp $SRC/PowerPulsar.txt \
		$SRC/add-ons/* $SRC/add-ons/*/* \
		$SRC/documentation.html $SRC/documentation/* $SRC/documentation/*/* \
		$SRC/img/distrib_*.jpg \
		$SRC/img_pulsar/* \
		$SRC/pulsar_addon_src/*.[ch]* $SRC/pulsar_addon_src/beos_r4/*.[ch]* \
		$SRC/sample_addons/*/*.[ch]* $SRC/sample_addons/*/*.rsrc \
		$SRC/sample_addons/*/*.proj*ppc $SRC/sample_addons/*/*.proj*86 \
		$SRC/sample_addons/*/makefile

echo Done !
