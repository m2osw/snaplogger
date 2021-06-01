#!/bin/bash
#
# Sample script to run make without having to retype the long path each time
# This will work if you built the environment using our ~/bin/build-snap script

FULL_VERSION=`dpkg-parsechangelog --show-field Version | sed -e 's/~.*//' -e 's/\(^[0-9]\+\.[0-9]\+\.[0-9]\+.[0-9]\+\).*/\1/'`
DOC_VERSION=`echo ${FULL_VERSION} | sed -e 's/\(^[0-9]\+\.[0-9]\+\).*/\1/'`
SOURCE=`pwd`
PROJECT=`basename ${SOURCE}`
PROCESSORS=`nproc`
PARENTDIR=`dirname ${SOURCE}`
PARENTNAME=`basename ${PARENTDIR}`
if test "${PARENTNAME}" != "contrib"
then
	CONTRIBDIR=""
	BUILD="../BUILD"
else
	CONTRIBDIR="contrib/"
	BUILD="../../BUILD"
fi
TYPE="Debug"
TARGET=
LESS=false
DOCS=false
TESTS=""

while test -n "${1}"
do
	case ${1} in
	"-d"|"--documentation")
		shift
		DOCS=true
		;;

	"-b"|"--debug")
		shift
		TYPE="Debug"
		;;
		
	"-h"|"--help")
		echo "Usage: ${0} [-opts]"
		echo "where -opts are:"
		echo "  -d | --documentation   Delete the documentation so it gets rebuilt."
		echo "  -b | --debug           Build the Debug version."
		echo "  -h | --help            Print out this help screen."
		echo "  -i | --install         Install once built."
		echo "  -l | --less            Force output through less."
		echo "  -p | --processors      Change the number of processors."
		echo "  -r | --release         Build the Release version."
		echo "  -s | --sanitize        Build the Sanitized version."
		echo "  -t | --test            Build and then run the tests."
		;;

	"-i"|"--install")
		shift
		TARGET=install
		;;

	"-l"|"--less")
		shift
		LESS=true
		;;

	"-p"|"--processors")
		shift
		PROCESSORS="${1}"
		shift
		;;

	"-r"|"--release")
		shift
		TYPE="Release"
		;;

	"-s"|"--sanitize")
		shift
		TYPE="Sanitize"
		;;

	"-t"|"--test")
		shift
		while [[ "${1}" != "" && "${1}" != "-"* ]]
		do
			TESTS="${TESTS} ${1}"
			shift
		done
		;;

	*)
		echo "error: unknown command line option \"${1}\"."
		exit 1
		;;

	esac
done


OUTPUT="${BUILD}/${TYPE}/${CONTRIBDIR}${PROJECT}"


if ${DOCS}
then
	rm -rf ${OUTPUT}/doc/${PROJECT}-doc-${DOC_VERSION}.tar.gz
fi

if ${LESS}
then
	make -j${PROCESSORS} -C ${OUTPUT} ${TARGET} 2>&1 | less -SR
	RESULT=${?}
else
	make -j${PROCESSORS} -C ${OUTPUT} ${TARGET}
	RESULT=${?}
fi

if [[ ${RESULT} = 0 ]]
then
	if test -n ${TESTS}
	then
		if ${LESS}
		then
			${OUTPUT}/tests/unittest --progress ${TESTS} 2>&1 | less -SR
		else
			${OUTPUT}/tests/unittest --progress ${TESTS}
		fi
	else
		echo
		echo "Success."
	fi
else
	echo
	echo "Build failed."
fi

