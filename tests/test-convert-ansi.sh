#!/bin/sh -e
#
# This script is an example on how one can test the ANSI converter

append_intro() {
	if test -z "$INTRO"
	then
		INTRO="\033[$1"
	else
		INTRO="${INTRO};$1"
	fi
}

usage() {
	echo "Usage: `basename $0` [--opt ...] [state ...]"
	echo
	echo "where --opt is one or more of the following options:" 
	echo "  -b | --br             add <br/> along \\n"
	echo "       --convert        do the conversion (opposite of --show)"
	echo "  -h | --help           show this help screen"
	echo "       --hide-style     do not show the HTML styles"
	echo "  -B | --no-br          do not add <br/> along \\n"
	echo "  -O | --optimize       optimize the HTML (i.e. <b> instead of <span class=\"ansi-bold\">)"
	echo "  -s | --show           show the effect in your console (no conversion)"
	echo "       --show-hex       show what is output in hex & ASCII (no conversion)"
	echo "       --show-style     do show the HTML styles (default)"
	echo "       --text <arg>     use <arg> as the text instead of the default: ${TEXT}"
	echo "       --use-classes    do not optimize the HTML"
	echo
	echo "where 'state' is one or more of the following:"
	echo "  background-palette <index>   set background color to the specified palette color (0 to 255)"
	echo "  background-rgb <r> <g> <b>   set background color to the specified RGB color"
	echo "  black-foreground             set foreground color to black"
	echo "  black-background             set background color to black"
	echo "  blue-foreground              set foreground color to blue"
	echo "  blue-background              set background color to blue"
	echo "  bright-blue-foreground       set foreground color to bright blue"
	echo "  bright-blue-background       set background color to bright blue"
	echo "  bright-cyan-foreground       set foreground color to bright cyan"
	echo "  bright-cyan-background       set background color to bright cyan"
	echo "  bright-green-foreground      set foreground color to bright green"
	echo "  bright-green-background      set background color to bright green"
	echo "  bright-magenta-foreground    set foreground color to bright magenta"
	echo "  bright-magenta-background    set background color to bright magenta"
	echo "  bright-red-foreground        set foreground color to bright red"
	echo "  bright-red-background        set background color to bright red"
	echo "  bright-white-foreground      set foreground color to bright white"
	echo "  bright-white-background      set background color to bright white"
	echo "  bright-yellow-foreground     set foreground color to bright yellow"
	echo "  bright-yellow-background     set background color to bright yellow"
	echo "  bold                         turn on bold mode (replaces normal intensity & light)"
	echo "  conceal                      prevent output of normal text"
	echo "  cross-out                    turn on cross out mode (strike through)"
	echo "  cyan-foreground              set foreground color to cyan"
	echo "  cyan-background              set background color to cyan"
	echo "  default-foreground           set foreground color to its default color"
	echo "  default-background           set background color to its default color"
	echo "  default-underline            set underline color to its default color"
	echo "  double-underline             turn on double underline mode (note: in HTML this also turns on double strike through and overline)"
	echo "  fast-blink                   turn on fast blinking mode"
	echo "  foreground-palette <index>   set foreground color to the specified palette color (0 to 255)"
	echo "  foreground-rgb <r> <g> <b>   set foreground color to the specified RGB color"
	echo "  gray-foreground              set foreground color to gray (also referenced as bright black)"
	echo "  gray-background              set background color to gray (also referenced as bright black)"
	echo "  green-foreground             set foreground color to green"
	echo "  green-background             set background color to green"
	echo "  inverse                      turn on inverse mode (swap foreground & background colors)"
	echo "  italic                       turn on italic mode"
	echo "  light                        turn on light mode (replaces normal intensity & bold)"
	echo "  magenta-foreground           set foreground color to magenta"
	echo "  magenta-background           set background color to magenta"
	echo "  normal                       reset all modes, colors, etc. to normal"
	echo "  normal-intensity             turn off bold or light mode"
	echo "  not-cross-out                turn off cross out mode (no strike through)"
	echo "  not-italic                   turn off italic mode"
	echo "  not-overline                 turn off overline mode"
	echo "  not-proportional             turn off proportional mode"
	echo "  not-superscript              turn off superscript and subscript"
	echo "  not-subscript                turn off superscript and subscript"
	echo "  not-underline                turn off underline mode"
	echo "  overline                     turn on overline mode"
	echo "  positive                     turn off inverse mode"
	echo "  proportional                 turn on proportional mode"
	echo "  red-foreground               set foreground color to red"
	echo "  red-background               set background color to red"
	echo "  slow-blink                   turn on slow blink mode"
	echo "  steady                       turn off the fast/slow blink modes"
	echo "  superscript                  turn on superscript mode"
	echo "  subscript                    turn on subscript mode"
	echo "  underline                    turn on underline mode"
	echo "  underline-palette <index>    set underline color to the specified palette color (0 to 255)"
	echo "  underline-rgb <r> <g> <b>    set underline color to the specified RGB color"
	echo "  visible                      turn off the conceal mode (text appears again)"
	echo "  white-foreground             set foreground color to white"
	echo "  white-background             set background color to white"
	echo "  yellow-foreground            set foreground color to yellow"
	echo "  yellow-background            set background color to yellow"
}

# Make an easy to use command line instead of escape codes
#
BR=""
INTRO=""
TEXT="test"
SHOW=convert
OUTPUT_STYLE="--output-style-tag"
OPTIMIZE=""
while test -n "${1}"
do
	P="${1}"
	shift
	case "${P}" in
	"black-foreground")		append_intro 30;;
	"black-background")		append_intro 40;;
	"blue-foreground")		append_intro 34;;
	"blue-background")		append_intro 44;;
	"bright-blue-foreground")	append_intro 94;;
	"bright-blue-background")	append_intro 104;;
	"bright-cyan-foreground")	append_intro 96;;
	"bright-cyan-background")	append_intro 106;;
	"bright-green-foreground")	append_intro 92;;
	"bright-green-background")	append_intro 102;;
	"bright-magenta-foreground")	append_intro 95;;
	"bright-magenta-background")	append_intro 105;;
	"bright-red-foreground")	append_intro 91;;
	"bright-red-background")	append_intro 101;;
	"bright-white-foreground")	append_intro 97;;
	"bright-white-background")	append_intro 107;;
	"bright-yellow-foreground")	append_intro 93;;
	"bright-yellow-background")	append_intro 103;;
	"bold")				append_intro 1;;
	"conceal")			append_intro 8;;
	"cross-out")			append_intro 9;;
	"cyan-foreground")		append_intro 36;;
	"cyan-background")		append_intro 46;;
	"default-foreground")		append_intro 39;;
	"default-background")		append_intro 49;;
	"default-underline")		append_intro 59;;
	"double-underline")		append_intro 21;;
	"fast-blink")			append_intro 6;;
	"gray-foreground")		append_intro 90;;
	"gray-background")		append_intro 100;;
	"green-foreground")		append_intro 32;;
	"green-background")		append_intro 42;;
	"inverse")			append_intro 7;;
	"italic")			append_intro 3;;
	"light")			append_intro 2;;
	"magenta-foreground")		append_intro 35;;
	"magenta-background")		append_intro 45;;
	"normal")			append_intro 0;;
	"normal-intensity")		append_intro 22;;
	"not-cross-out")		append_intro 29;;
	"not-italic")			append_intro 23;;
	"not-overline")			append_intro 55;;
	"not-proportional")		append_intro 50;;
	"not-superscript")		append_intro 75;;
	"not-subscript")		append_intro 75;;
	"not-underline")		append_intro 24;;
	"overline")			append_intro 53;;
	"positive")			append_intro 27;;
	"proportional")			append_intro 26;;
	"red-foreground")		append_intro 31;;
	"red-background")		append_intro 41;;
	"slow-blink")			append_intro 5;;
	"steady")			append_intro 25;;
	"superscript")			append_intro 73;;
	"subscript")			append_intro 74;;
	"underline")			append_intro 4;;
	"visible")			append_intro 28;;
	"white-foreground")		append_intro 37;;
	"white-background")		append_intro 47;;
	"yellow-foreground")		append_intro 33;;
	"yellow-background")		append_intro 43;;

	"background-palette")
		append_intro 48
		append_intro 5
		append_intro "${1}"
		shift
		;;

	"background-rgb")
		append_intro 48
		append_intro 2
		append_intro "${1}"
		append_intro "${2}"
		append_intro "${3}"
		shift 3
		;;

	"foreground-palette")
		append_intro 38
		append_intro 5
		append_intro "${1}"
		shift
		;;

	"foreground-rgb")
		append_intro 38
		append_intro 2
		append_intro "${1}"
		append_intro "${2}"
		append_intro "${3}"
		shift 3
		;;

	"underline-palette")
		append_intro 58
		append_intro 5
		append_intro "${1}"
		shift
		;;

	"underline-rgb")
		append_intro 58
		append_intro 2
		append_intro "${1}"
		append_intro "${2}"
		append_intro "${3}"
		shift 3
		;;

	"-b"|"--br")
		BR=""
		;;

	"--convert")
		SHOW=convert
		;;

	"-h"|"--help")
		usage | less
		exit 1
		;;

	"--hide-style")
		OUTPUT_STYLE=""
		;;

	"-B"|"--no-br")
		BR="--no-br"
		;;

	"-O"|"--optimize")
		OPTIMIZE="--optimize"
		;;

	"-s"|"--show")
		SHOW=show
		;;

	"--show-hex")
		SHOW=hex
		;;

	"--show-style")
		OUTPUT_STYLE="--output-style-tag"
		;;

	"--text")
		TEXT="${1}"
		shift
		;;

	"--use-classes")
		OPTIMIZE=""
		;;

	*)
		echo "error: unknown option \"${P}\"."
		exit 1
		;;

	esac
done

case "${SHOW}" in
"hex")
	echo "${INTRO}m${TEXT}\033[m" | od -tx1a
	;;

"show")
	echo "${INTRO}m${TEXT}\033[m"
	;;

"convert")
	echo "${INTRO}m${TEXT}\033[m" | ../../BUILD/Debug/contrib/snaplogger/tools/convert-ansi "${OUTPUT_STYLE}" "${OPTIMIZE}" "${BR}" && echo
	;;

esac

