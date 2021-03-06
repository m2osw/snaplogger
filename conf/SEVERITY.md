
# Log Severity

The `snaplogger` library let you add severity levels to your applications.
This is done by adding files in the `/usr/share/snaplogger` and
`/etc/snaplogger` directories.

**Note:** The severity options use an internal `advgetopt` declaration so
you can't just define your severities under your own application folder.

# Format

The format used to declare a severity is what we call the INI format.
You enter the name of the severity between square brackets and then
a list of variable with a value below that.

Here is an example for the `information` level:

    [information]
    level=50
    aliases=info
    description=information
    default=true
    styles=green

The `snaplogger` library supports aliases, a long description, and styles.
See the next sections for additional information about the available
parameters.

## Severity Level

Each severity has a level which indicates whether to print that value or
not. The level is limited to numbers between 0 and 255.

0 is specifically used to say "all severities will be printed out".

255 is specifically used to say "no severities will be printed out".

Two different severities cannot make use of the same level. If you want to
do that, use the `aliases` parameter instead.

## Aliases

Each severity is given a name such as `error`. No two severities can make
use of the same name or alias.

The `aliases=...` parameter can be used to give multiple names to the same
severity level. To enter more than one name, separate them by commas.

When reading the .ini file, the names can be duplicated. The first instance
is kept. Further instances are ignored.

## Description

The description field is used to better define the severity level. Often we
use abbreviations for the level name so the description can be used to have
a more complete definition. It can include spaces, punctuations, etc.

## Mark as Default

The `default=true` parameter can be used to define a default severity.
This severity can be determined by doing a `logger->get_default_severity()`.
It is used by the message when no severity is specified on construction.
You can use the special `SNAP_LOG_DEFAULT` macro to make use of the default
severity.

The internal default severity is the `SEVERITY_INFORMATION` level.

## Styles

When printing out messages, it is possible to change their basic styles.

The basic styles available are colors such as green or red and font styles
like bold and italic. What is supported depends on the appender that you
use. For instance, no style is used for the file appender. The console
appender, on the other hand, supports a long list of styles (72 are supported
at time of writing). Even the console may not use its styling capability
if it can't detect that the output stream is a TTY.

The following lists the styles available in the console appender:

    bg-black
    bg-blue
    bg-bright-black
    bg-bright-blue
    bg-bright-cyan
    bg-bright-green
    bg-bright-magenta
    bg-bright-red
    bg-bright-white
    bg-bright-yellow
    bg-cyan
    bg-green
    bg-orange
    bg-magenta
    bg-red
    bg-white
    bg-yellow
    black
    blink-off
    blue
    bold
    bright-black
    bright-blue
    bright-cyan
    bright-green
    bright-magenta
    bright-red
    bright-white
    bright-yellow
    conceal
    crossed-out
    cyan
    default-font
    default-background-color
    default-foreground-color
    double-underline
    encircled
    faint
    font0
    font1
    font2
    font3
    font4
    font5
    font6
    font7
    font8
    font9
    font10
    fraktur
    framed
    green
    inverse-off
    italic
    magenta
    normal
    not-crossed-out
    not-framed
    not-italic
    not-overline
    orange
    overline
    rapid-blink
    red
    reveal
    reverse-video
    slow-blink
    standout
    underline
    underline-off
    white
    yellow

