
<p align="center">
<img alt="snaplogger" title="Multi-destination and easily extensible logger in C++."
src="https://raw.githubusercontent.com/m2osw/snaplogger/master/doc/snaplogger.png" width="70" height="70"/>
</p>

# Introduction

The snaplogger started based on the functionality offered by the
`log.cpp/.h` from the **libsnapwebsites** and various features of the
**log4cplus** library.

The current version allows all our Snap! projects to log errors to
files, the syslog, and the console.

## Features

The following are the main features of this logger:

* Messages using `std::stringstream`

    Sending log messages uses an `std::stringstream`. When creating a
    `message` class, it can be used just like any `std::basic_ostream<char>`.
    
    This means all the features supported by `std::basic_ostream` are
    available. You can send hexadecimal (`... << std::hex << ...`), format
    strings, and overload the `<<` operator in order to add more automatic
    transformation of objects to strings just as you do when you want to
    print those objects to `std::cout` or `std::cerr`.

* Severity

    The logs use a severity level. Whenever you create a message you assign
    a severity to it such as DEBUG, INFO, ERROR, etc. We offer 18 severity
    levels by default. You can dynamically add more, either in your software
    or in the severity.conf file.

    Appenders can be assigned a severity. When a message is logged with
    a lower severity than the appender's, then it gets ignored by that
    appender.

    The severity can also be tweaked directly on the command line.

* Appenders

    A set of classes used to declare the log sinks.
    
    The library offers the following appenders:

    - buffer
    - console
    - file
    - syslog
    
    It is very easy to develop your own appender. In most cases you want
    to override the `set_config()` (optional) and the `process_message()`.

    The  project will add a log service. This has several
    advantages over the direct appenders offered by the base library. The
    main two are certainly:
    
    - It can access any files since the service can be running as root
    - It can do work in parallel on a separate computer.
    
    The [#eventdispatcher] makes use of the base snaplogger library which
    is why this server extension is in a separate project.

* Format

    Your log messages can be formatted in many different ways. The
    `snaplogger` library supports many variables which support parameters
    and functions. All of this is very easily extensible.
    
    There are many variables supported by default, including displaying
    the timestamp of the message, environment variables, system information
    (i.e. hostname), and all the data from the message being logged.
    
    Our tests verifies a standard one line text message and a JSON message
    to make sure that we support either format. You could also use a CSV
    type of format.

* Diagnostics

    We support two types of diagnostics: map and nested.

    Maps are actually used to save various parameters such as the program
    name and the program version. You can use as many parameters as you'd
    like to show in your logs.

    The nested diagnostics are used to track your location in a stack like
    manner.

* Component

    The software allows for filtering the output using a "component" or
    section. When you send a log, you can use an expression such as:

        SNAP_LOG_INFO
            << snaplogger::section(snaplogger::secure_component)
            << "My Message Here"
            << SNAP_LOG_END;

    This means the log will only be sent to the appender's output if the
    appender includes "secure" as one of its components. This allows us
    to very quickly prevent messages from going to the wrong place.

    **Note:** The secure component has special options that allows you
    to quickly use that one. For example:

        SNAP_LOG_INFO
            << "My Secure Message"
            << SNAP_LOG_END_SECURELY;

    Also we have a special modifier you can use like this:

        SNAP_LOG_INFO
            << snaplogger::secure
            << SNAP_LOG_END;

    This gives you examples on how to implement your own flags.

* Regex Filtering

    You have the ability to add as many file appenders as you'd like. You
    can already filter by Severity and Component and with our Regexp support
    you can further filter by checking whether the log message matches a
    regular expression or not.

* Multi-thread Safe

    The library is multi-thread safe. It can even be used with an
    asynchronous feature so you can send many logs and they are
    processed by a thread. That thread can easily be stopped in case
    you want to `fork()` your application.

* advgetopt support

    The logger has a function used to add command line options and another
    to execute them.

    This adds many features to your command line such as the `--debug`
    and `--logger-configuration-filenames` options. You can always add
    more.

    The library also is setup to capture logs from the `advgetopt` and
    the cppthread libraries (both use the `advgetopt` log feature.)


# Reasons Behind Having Our Own Library

We want to be in control of the capabilities of our logger and we want
to be able to use it in our `snap_child` objects in a thread. Many
capabilities which are not readily available in `log4cplus`.

# API

## Appenders

An appender is an object that allows the log messages to be _appended_.

This is the same terminology used by `log4cplus`.

We have a C++ base class which allows you to create your own appenders.
These get registered and then the user can make use of those by naming
them in their configuration files.

In the base library, We offer the following appenders:

* buffer
* console
* file
* syslog

A network appender (service) will be available through [#logserver]. We
want to have access to [#eventdispatcher] but [#eventdispatcher] depends
on [#logserver] so we have to have another project to log to a remote
server.

## Configuration Files

You configure `snaplogger` via a set of configuration files.

These files are found under `/etc/snaplogger` by default, but the
`snapwebsites` environment checks for these files under
`/etc/snapwebsites/logger`.

The configuration files include parameters that setup the log. These
are:

### Appender

The configuration has some global settings and then a list of appenders.

An appender is a name and then a set of parameters:

* Type
* Level
* Component
* Format

These are described below.

    # Global settings
    format=${date} ${time}: ${severity}: ${message}
    asynchronous=true
    severity=WARNING

    [console-output]
    type=console
    severity=DEBUG

    [log-to-file]
    type=file
    severity=INFORMATION
    filename=firewall
    lock=false

**Note:** An appender defined in the main logger.conf file can be turned
off by setting its severity to OFF in your other configuration files. For
example:

    # In /etc/snaplogger/logger.conf
    [console]
    type=console
    severity=TRACE
    style=orange
    force_style=true
    
    # In /etc/snaplogger/my-app.conf
    [console]
    severity=OFF

**Note:** The name in the section (`[console]`) does not need to be the
name of the appender type. This allows you to have multiple appenders
of the same type. For example, you may want to write to an `all.log` file:

    [all]
    type=file
    filename=all
    lock=true

### Type

The type is often the name of the appender.

The base supports the following types (appenders):

* buffer
* file
* console
* syslog

Other types can be added by creating a class derived from the snaplogger
Appender class. For example, the `logserver` offers the "network" appender.

The type is mandatory, but if the `type=...` field is not specified,
the system tries with the name of the section. So the following
means we get a file:

    [file]
    filename=carrots

Remember that to create multiple entries of the same type, you must use
different section names. Reusing the same name is useful only to overwrite
parameters of a previous section definition. Some type of appenders
(i.e. syslog) do not allow duplication, so you can only have one of those.

#### type=file

Write the logs to a file. The file must be named unless your section is an
override of another section of the same name which already has a filename.

    filename=database

The filename must not include any slashes or extension (periods are not
accepted). This way we are in control of those characters. Especially,
other parameters define where the files go:

    path=/var/log/snapwebsites

### Severity

By default the severity is set to INFORMATION.

This means anything below that severity, such as DEBUG or NOTICE, gets ignored.
Anything equal or above that severity gets included.

The severity can be different for each appender.

We support the following levels:

* `ALL`
* `TRACE`
* `DEBUG`
* `NOTICE`
* `UNIMPORTANT`
* `INFORMATION`
* `IMPORTANT`
* `MINOR`
* `DEPRECATED`
* `WARNING`
* `MAJOR`
* `RECOVERAL_ERROR`
* `ERROR`
* `CRITICAL`
* `ALERT`
* `EMERGENCY`
* `FATAL`
* `OFF`

The severity can also be changed dynamically (along the way, from command line
arguments, etc.)

    [log]
    severity=DEBUG

### Component

Each entry can be segregated in a specific component. This gives your
application the ability to send the log to a specific component.

In Snap!, we make use of the `NORMAL` and `SECURE` components. The `SECURE`
logs are better protected (more constrained `chmod`, sub-folder, etc.)
than the `NORMAL` component.

Others can be added.

Technically, when no component is specified in a log, `NORMAL` is assumed.
Otherwise, the appender must include that component or it will be ignored.
(i.e. that log will not make it to that appender if none of its components
match the ones in the log being processed.)

### Filter

You can filter the message using a regular expression. It is strongly
advised that you use this feature only when necessary and possibly not
in production.

    [overflows]
    type=file
    filename=overflows.log
    filter=/overflow|too large/i

The slashes are optional if you do not need to use any flags.

The supported flags are:

* `a` -- use awk regex
* `b` -- use basic regex
* `c` -- use locale to compare
* `e` -- test egrep regex
* `g` -- test grep regex
* `i` -- test case insensitively
* `j` -- use ECMAScript regex
* `x` -- use extended POSIX regex (this is the default)

The `a`, `b`, `e`, `g`, `j`, and `x` are mutually exclusive, only one
of them can be used in your list of flags. We use `x` by default when
you do not explicitly specify a type.

### Format

The format string defines how the output will look like in the logs.
It supports a set of variables which are written as in:

    ${<name>[:<param>[=<value>]:...]}

This syntax, instead of %\<letter>, was chosen because it's much more
versatile. The following are parameters supported internally:

    # Functions
    ${...:align=left|right}     truncate keeps left or right size, padding
    ${...:padding=character}    use 'character' for padding
    ${...:exact_width=n}        set width of parameter to 'n', pad/truncate
    ${...:max_width=n}          truncate if width is more than 'n'
    ${...:min_width=n}          pad if width is less than 'n'
    ${...:lower}                transform to lowercase
    ${...:upper}                transform to uppercase
    ${...:caps}                 transform to capitalized words
    ${...:escape[=characters]}  escape various characters such as quotes

    # Logger
    ${severity[:format=alpha|number]}
                                severity level description, in color if allowed
    ${message}                  the log message
    ${project_name}             name of the project
                                (equivalent to ${diagnostic:map=project_name})
    ${progname}                 name of the running program
                                (equivalent to ${diagnostic:map=progname})
    ${version}                  show version of running program
                                (equivalent to ${diagnostic:map=version})
    ${build_date}               show build date of running program
                                (equivalent to ${diagnostic:map=build_date})
    ${build_time}               show build time of running program
                                (equivalent to ${diagnostic:map=build_time})
    ${filename}                 name of the file where the log occurred
    ${basename}                 only write the basename of the file
    ${path}                     only write the path to the file
    ${function}                 name of function where the log occurred
    ${line}                     line number where the log occurred
    ${diagnostic}               the whole current log diagnostic data
    ${diagnostic:nested[=depth]} the current nested diagnostic data; if depth
                                is specified, show at most that many messages
    ${diagnostic:map[=key]}     the current map diagnostic data; if key is
                                given limit the diagnostic to that key

    # System
    ${hostname}                 name of the computer
    ${hostbyname:name=hostname} name of a computer referenced by name or IP
    ${domainname}               name of the domain
    ${pid[:running]}            PID of the running program
    ${pgid[:running]}           GID of the running program
    ${tid[:running]}            identifier of the running thread
    ${thread_name}              name of the running thread
                                (equivalent to ${diagnostic:map=thread_name})

    # User
    ${uid}                      the UID
    ${username}                 the name of the user (getuid() to string)
    ${gid}                      the GID
    ${groupname}                the name of the group (getgid() to string)

    # Environment
    ${env:name=n}               insert the environment variable $n

    # Date/Time
    ${date}                     year/month/day, year includes century
    ${date:day}                 day of month as a number (1 to 31)
    ${date:day_of_week_name}    name of day of week, English
    ${date:day_of_week}         day of week as a number
    ${date:year_week}           year week (1 to 53)
    ${date:year_day}            year day (1 to 366)
    ${date:month_name}          name of month, English
    ${date:month}               month as a number (1 to 12)
    ${date:year}                year as a number (i.e. "2020")
    ${time}                     hour:minute:second in 24 hour
    ${time:hour=24|12}          hour number; (0 to 23 or 1 to 12)
    ${time:minute}              minute number (0 to 59)
    ${time:second}              second number (0 to 60)
    ${time:nanosecond}          nanosecond number (0 to 999999999)
    ${time:unix}                seconds since Jan 1, 1970 (Unix timestamp)
    ${time:meridiem}            AM or PM (right now according to locale...)
    ${time:offset}              amount of nanoseconds since logger started
    ${time:process}             amount of nanoseconds consumed by process
    ${time:thread}              amount of nanoseconds consumed by thread
    ${time:process_ms}          time consumed by process as H:M:S.999
    ${time:thread_ms}           time consumed by thread as H:M:S.999
    ${locale}                   date time formatted according to locale
    ${locale:day_of_week_name}  name of day of week (letters), using locale
    ${locale:month_name}        name of month, using locale
    ${locale:date}              date formatted according to locale
    ${locale:time}              time formatted according to locale
    ${locale:meridiem}          AM or PM according to locale
    ${locale:timezone}          timezone name
    ${locale:timezone_offset}   timezone as +hhmm or -hhmm

The library supports an advanced way of formatting messages using
variables as in:

    ${time}

The variable name can be followed by a colon and a parameter. Many
parameters can be assigned a value. We offer three types of parameters.
Direct parameters which transform what the variable returns. For
example, the `${time:hour}` outputs the hour of the message timestamp.
Actual parameters, such as the `align` parameter. This one takes a
value which is one of those three words: "left", "center", or
"right". That alignment is held in the formatter until we
are done or another `align=...` happens. Finally, we have
the parameters that represent functions. Those further format
the input data. For example, you can write the PID in your logs.
You may want to pad the PID with zeroes as in:

    ${pid:padding='0':align=right:exact-width=5}

This way the output of the PID will always look like it is 5 digits.

Formats are extensible. We use the following macro for that purpose:

    DEFINE_LOGGER_VARIABLE(date)
    {
        ...the process_value() function...
    }

Parameters make use of the following macro:

    DECLARE_FUNCTION(padding)
    {
        ...the apply() function...
    }

Since you may have parameters of your own, we allow for any parameter to
be defined with a map of strings. Here is how we save the one character
for the padding feature;

    d.set_param(std::string("padding"), pad);

Functions receive the `value` of the variable which is reachable with
the `get_value()` function. Note that it returns a writable reference
for faster modifications. For example appending is very easy, however,
prepending still requires a `set_value()` call:

    d.get_value() += " add at the end";
    d.set_value(padding + d.get_value());

So our format is extremely extensible.

#### Default Format

The default format:

    ${date} ${time} ${hostname} ${progname}[${pid}]: ${severity}: ...
          ... ${message:escape:max_width=1000} ...
          ... (in function \"${function}()\") ...
          ... (${basename}:${line})

If no format is defined for an appender then this format is used. The "..."
is just to show it continues on the following line.

#### JSON Format

To support the JSON format you need to be careful with double quotes.
They need to be escaped.

    {"date":"${date}",


## Diagnostic Features

Like log4cplus, we support diagnostic extensions (Nested Diagnostic Context
or NDC and Mapped Diagnostic Context).

To use the Nested Diagnostic Context create `snaplogger::nested_diagnostic`
object in your blocks. That extra output can can added to your logs using the
`${diagnostic}` variable.

    {
        snaplogger::nested_diagnostic section1("sec-1");

        {
            snaplogger::nested_diagnostic sub_sectionA("sub-A");

            SNAP_LOG_INFO("message");
        }
    }

When you call the `SNAP_LOG_INFO()` in this example, the `${diagnostic}`
variable outputs "sec-1/sub-A". This gives you a very good idea of which
block the log appears in (The filename and line number should be enough
but with the nested diagnostic, it's even easier.)

The mapped diagnostic is useful when you have one block with multiple
states that change over time. You can reflect the current state in
the map.

    {
        snaplogger::map_diagnostic diag;
        diag("state", "normal");

        for(;;)
        {
            ...
            if(this_happened)
            {
                diag("state", "this happened!");
            }
            else
            {
                diag("state", "normal");
            }
            ...
            SNAP_LOG_INFO("message");
        }
    }

In this case, the `"state"` key changes depending on various parameters
of yours. It would be complicated for you to maintain all the statuses
of your app. for when a log message is to be generated (and you could
be calling all sorts of sub-functions that have no clue of said state!)

With the `map_diagnostic` you change the diagnostic message as you change
your program state. Much more practical. There is no limit to the number
of maps or the message (although you certainly want to limit both.)

When you use the `${diagnostic}`, all the keys are written by default
(i.e. `state=normal,size=33,...`) You can setup your logger's format
to only print certain keys. For example:

    ${diagnostic:map=state}

only outputs `"normal"` or `"this happened!"`.


# Properly Deleting Log Files

Whenever you create a packge for your project and the purge function is used
to completely remove your project, you should delete all the corresponding log
files along everything else. In your `postrm` script, you want to delete
secure logs and logs with user data such as IP addresses using the shred
command. Anything that includes _compromising_ data.

Here is an example to remove the Snap! Firewall secure logs:

    shred -fu /var/log/snapwebsites/snapfirewall.log*

Similarly, the `logrotate` tool needs to be used with the shredding feature:

    /var/log/snapwebsites/secure/snapfirewall.log {
        ...
        shred
        ...
    }

This way your files will properly be shredded.

## Shredding and SSD

Note that SSD drives are likely to write new data at a new address. In
other words, you are not overwritting anything. If you are using SSD
drives, then do not use any kind of shredding. This means you are left
with encryption (see below) if you want to make your logs safe.

## Do I Really Need Shredding Files?

If you are in full control of the hardware, you probably do not need to
spend any time shredding anything. If you use a VPS or even a dedicated
server at a data center which you are not the only user, then you do
want to use the `shred` command. You may lose your VPS which then gets
assigned to another user which could possibly find a way to read your
data (it _should_ not be possible for them to do that, but...)

## Is `shred` Working on my File System/Device?

Chances are it will work as expected, but some types of file systems often
do not overwrite existing systems. Maybe it writes new data at new locations
in a sequencial manner. It could also be be that some data ends up in the
journal instead of the file itself. Shred uses `sync` by default, but all
file system do not respect that command 100%.

Please check the docs with `man shred`.

Also you may be interested by `apt-get install secure-delete` and then
check out `srm`. This tool has the advantage of deleting a whole tree of
folders without the need for a `find ...` command to find all the files
to delete. It has the same drawbacks as `shred` in link with the file
system not overwriting the existing sectors or using a journal.

Finally, the device you are using will itself have a controller which
may or may not overwrite the data on the same sector. Good controllers
for SSDs are very likely to write blocks of data to new addresses so
the writes remain fast (otherwise a read + re-write cycle would always
be required).

## Instead of Shredding

Another solution, which we do not recommand because it is slower, is to use
an encrypted drive. For logs, though, it can be a good idea to send them
over the wire (i.e. use the [#logserver]) and encrypt the drive where you
write the logs on that [#logserver] computer.

Encryption makes things slower, but when taking advantage of newer hardware
it is still pretty fast. From various sources, it looks like the impact is
only around +0.25%. In other words, the costs are probably worth looking
into using ext4 encryption. Note that this is true even with SSD drives.
So with HDD drives, you are even less likely to notice the difference.
Note also that encrypted data take more space on disk. 1 sector is going
to be more than the usual 512 bytes of data.

You may also want to look into how it works: i.e. if you reboot, do you
have to enter a passphrase before the computer can actually work? This can
be rather complicated on a large live system. Again, you may want to limit
the encryption to partitions with data that require it (i.e. maybe `/var/log`
and `/var/lib/cassandra/data`). I have no clue how that works on a VPS which
auto-formats your drive and even less how you can enter your password to
unlock the encryption on a reboot. Make sure to try your services and
properly document the matter.

**Side Note:** If you have a separate computer, you may conside having
the `/var/log` folder on a separate partition and make it non-executable
(i.e. do not use the `defaults` options and do not include the `exec` flag,
see the `man mount` manual page for details.) This should work on any
computer, but a [#logserver] will get even more logs and thus having
a separate partition makes even more sense in that situation.

## What About Other Files?

The shredding mechanism can be used with any file. Obviously, it makes the
delete very slow, so it is recommended to only use this technique against
files that are likely to require it, such as your logs.

Two other places where I use the technique:

1. Configuration files where I have important data such as encryption keys,
   passwords, and various _top-secret_ IP addresses.
2. Database files, for example, a Cassandra node is going to use
   `/var/lib/cassandra/data` for your database. You could encrypt/shred
   these files.


# TRIM on SSD

For servers specifically setup to receive logs, you may want to consider
using the TRIM feature of _ext4_, _btrfs_, and _fat_ (as of Linux 4.4).

If you are using a VPS from a system such as DigitalOcean, then you do
not have access to the low level setup of the drive so you can't do
anything about the TRIM feature.

If you own your own hardware, then this feature makes sense for you to
use. It tells the SSD drive about all the sectors that are not used
anymore and allows the SSD drive garbage collector to better know which
chip to use next.

When mounting a drive, make sure to include the `discard` option. For
example:

    # <file system> <mount point>   <type>  <options>         <dump>  <pass>
    /dev/sdc1       /mnt/ssd        ext4    defaults,discard  0       2


# License

The project is covered by the
[GPL 2.0 license](https://github.com/m2osw/snaplogger/blob/master/LICENSE.txt).


# Bugs

Submit bug reports and patches on
[github](https://github.com/m2osw/snaplogger/issues).


_This file is part of the [snapcpp project](https://snapwebsites.org/)._

vim: ts=4 sw=4 et
