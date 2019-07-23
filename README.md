
<p align="center">
<img alt="advgetopt" title="Advance getopt, a C++ library to handle your command line options and configuration files seamlessly."
src="https://raw.githubusercontent.com/m2osw/snaplogger/master/doc/snaplogger.png" width="70" height="70"/>
</p>

# Introduction

The snaplogger is based on our `log.cpp/.h` from the libsnapwebsites.

It allows all our Snap! projects to log errors to file, syslog, console,
and ultimately through a network connection (although this is part of
the snaplog, because the snapcommunicator depends on the logger.)

# Reasons Behind Having Our Own Library

We want to be in control of the capabilities of our logger and we want
to be able to use it in our `snap_child` objects in a thread. Many
capabilities which are not readily available in log4cplus.

# API

## Appenders

An appender is an object that allows the log to be _appended_.

This is the same terminology used by log4cplus.

We have a C++ base class which allows you to create your own appenders.
These get registered and then the user can make use of those by naming
them in their configuration files.

We offer the following appenders:

* File
* Console
* Syslog

A network appender will be available through snaplog. We want to have
access to snapnetwork but snapnetwork depends on snaplogger so we have
to have another project to log to a remote log service.

## Configuration Files

You configure snaplogger via a set of configuration files.

These files are found under `/etc/logger` by default, but the `snapwebsites`
environment checks for these files under `/etc/snapwebsites/logger`.

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
    format=${date}: ${level}: ${message}
    asynchronous=true

    [console]
    type=console
    level=DEBUG

    [file]
    type=file
    level=INFORMATION
    file.name=firewall
    file.lock=false

**Note:** An appender defined in the main logger.conf file can be turned
off by setting its level to OFF in your other configuration files. For
example:

    # In /etc/logger/logger.conf
    [console]
    type=console
    level=TRACE
    console.color=true
    
    # In /etc/logger/my-app.conf
    [console]
    level=OFF

**Note:** The name in the section (`[console]`) does not need to be the
name of the appender type. This allows you to have multiple appenders
of the same type. For example, you may want to write to an `all.log` file:

    [all]
    type=file
    file.name=all
    file.lock=true

### Type

The type is actually the name of the appender.

The base supports the following appenders:

* file
* console
* syslog

Other types can be added by creating a class derived from the snaplogger
Appender class. For example, the snaplog offers the "network" appender.

#### Type=File

Write the logs to a file. The file must be named unless your entry is an
override of another entry of the same name.

    file.name=database

The filename must not include any slashes or extension (periods are not
accepted). This way we are in control of those characters. Especially,
other parameters define where the files go:

    file.path=/var/log/snapwebsites

### Level

By default the level is set to INFORMATION.

This means anything below that level gets ignored. Anything equal or above
that level gets included.

The level can be different for each component and each appender.

We support the following levels:

* TRACE
* DEBUG
* NOTICE
* UNIMPORTANT
* INFORMATION
* IMPORTANT
* MINOR
* WARNING
* MAJOR
* ERROR
* CRITICAL
* EMERGENCY
* FATAL
* OFF

The level can also be changed dynamically (along the way, from command line
arguments, etc.)

    [log]
    level=DEBUG

### Component

Each entry can be segregated in a specific component. This gives your
application the ability to send the log to a specific component.

In Snap!, we make use of the NORMAL and SECURE components. The SECURE
logs are better protected (more constratined chmod, sub-folder, etc.)
than the NORMAL component.

Others can be added.

Technically, when no component is specified in a log, NORMAL is assumed.
Otherwise, the appender must include that component or it will be ignored.
(i.e. that log will not make it to that appender if none of its components
match the ones in the log being processed.)

### Format

The format string defines how the output will look like in the logs.
It supports a set of variables which are written as in:

    ${<name>[:<param>[=<value>]:...]}

This syntax instead of %\<letter> was chosen because it's much more versatile.
The following are parameters supported internally:

    # Functions
    ${...:prepend='c'}          make exact_width, max_width, min_width prepend
                                'c' as required
    ${...:append='c'}           make exact_width, max_width, min_width append
                                'c' as required
    ${...:exact_width=n}        set width of parameter to 'n', pad/truncate
    ${...:max_width=n}          truncate if width is more than 'n'
    ${...:min_width=n}          pad if width is more than 'n'
    ${...:lower}                transform to lowercase
    ${...:upper}                transform to uppercase
    ${...:caps}                 transform to capitalized words
    ${...:escape[=format]}      escape various characters such as quotes

    # Logger
    ${level}                    level description, in color if allowed
    ${message}                  the log message
    ${diagnostic}               the whole current log diagnostic data
    ${diagnostic:nested[=level]} the current nested diagnostic data; if level
                                is specified, show at most that many levels
    ${diagnostic:map[=key]}     the current map diagnostic data; if key is
                                given limit the diagnostic to that key

    # System
    ${hostname}                 name of the computer
    ${hostbyname}               name of the computer
    ${domainname}               name of the domain
    ${pid}                      PID of the running program
    ${tid}                      identifier of the running thread
    ${thread_name}              name o the running thread
    ${uid}                      the UID
    ${username}                 the name of the user (getuid() to string)
    ${gid}                      the GID
    ${groupname}                the name of the group (getgid() to string)
    ${env:name=n}               insert the environment variable $n

    # User Data (when available)
    ${progname}                 name of the running program
    ${filename}                 name of the file where the log occurred
    ${basename}                 only write the basename of the file
    ${path}                     only write the path to the file
    ${function}                 name of function where the log occurred
    ${line}                     line number where the log occurred

    # Date/Time
    ${date}                     year/month/day, year includes century
    ${date:day_of_week_name}    name of day of week, English
    ${date:day_of_week}         day of week in numbers
    ${date:year_week}           year week (1 to 53)
    ${date:year_day}            year day (1 to 366)
    ${date:month_name}          name of month, English
    ${date:month}               month number (1 to 12)
    ${date:year}                year as a number (i.e. "2020")
    ${time}                     hour:minute:second in 24 hour
    ${time:hour=24|12}          hour number; (0 to 23 or 1 to 12)
    ${time:minute}              minute number (0 to 59)
    ${time:second}              second number (0 to 60)
    ${time:nanosecond}          microsecond number (0 to 999999999)
    ${time:unix}                seconds since Jan 1, 1970 (Unix timestamp)
    ${time:meridiem}            AM or PM according to locale
    ${time:offset}              amount of nanoseconds since logger started
    ${locale:day_of_week}       name of day of week (letters), using locale
    ${locale:month_name}        name of month, using locale
    ${locale:date}              date formatted according to locale
    ${locale:time}              time formatted according to locale
    ${locale:date_and_time}     date time formatted according to locale
    ${locale:timezone}          timezone name
    ${locale:timezone_offset}   timezone as +hhmm or -hhmm

See [log4cplus supported formats](http://log4cplus.sourceforge.net/docs/html/classlog4cplus_1_1PatternLayout.html).

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

In this case, the `"state"` key changes depening on various parameters
of yours. It would be complicated for you to maintain all the statuses
of your app. for when a log message is to be generated (and you could
be calling all sorts of sub-functions that have no clue of said state!)

With the `map_diagnostic` you change the diagnostic message as you change
your program state. Much more practical. There is no limit to the number
of keps or the message (although you certainly want to limit both.)

When you use the `${diagnostic}`, all the keys are written by default
(i.e. `state=normal,size=33,...`) You can setup your logger's format
to only print certain keys. For example:

    ${diagnostic:map=state}

only outputs `"normal"` or `"this happened!"`.

# License

The source is covered by the MIT license. The debian folder is covered
by the GPL 2.0.


# Bugs

Submit bug reports and patches on
[github](https://github.com/m2osw/snaplogger/issues).


_This file is part of the [snapcpp project](https://snapwebsites.org/)._

vim: ts=4 sw=4 et
