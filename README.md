
<p align="center">
<img alt="snaplogger" title="Multi-destination and easily extensible logger in C++."
src="https://raw.githubusercontent.com/m2osw/snaplogger/master/doc/snaplogger.png" width="70" height="70"/>
</p>

# Introduction

The `snaplogger` started based on the functionality offered by the
`log.cpp/.h` from the **libsnapwebsites** and various features of the
**log4cplus** library.

The current version allows all our Snap! projects to log errors to
files, the syslog, and the console. An extension allows for the logs
to be sent over the network (on top of the syslog capabilities).

## Features

The following are the main features of this logger:

* Create Log Messages using `std::stringstream`

    Sending log messages uses an `std::stringstream`. When creating a
    `message` object, it can be used just like any `std::basic_ostream<char>`.

    This means all the features supported by `std::basic_ostream` are
    available. You can send hexadecimal (`... << std::hex << ...`), format
    strings, and overload the `<<` operator in order to add more automatic
    transformations of objects to strings just as you do when you want to
    print those objects to `std::cout` or `std::cerr`.

* Severity

    The logs use a severity level. Whenever you create a message you assign
    a severity to it such as DEBUG, INFO, ERROR, etc. We offer
    [26 severity levels](https://github.com/m2osw/snaplogger/blob/9d0d447df9dec4b2bd30f8a65a757cce134838d9/snaplogger/severity.h#L58-L90)
    by default. You can dynamically add more, either in your software
    or in the `severity.conf` file.

    Appenders can be assigned a severity. When a message is logged with
    a lower severity than the appender's, then it gets ignored by that
    appender.

    The severity can also be tweaked directly on the command line.

* Appenders

    A set of classes used to declare the log sinks.

    This library offers the following appenders:

    - buffer
    - console
    - file
    - syslog

    It is very easy to develop your own appender. In most cases you want
    to override the `set_config()` (optional) and the `process_message()`.

    The [eventdispatcher project](https://github.com/m2osw/eventdispatcher/tree/main/snaplogger)
    adds a log service (a server you run separately). This has several
    advantages over the direct appenders offered by the base library.
    The main two are certainly:

    - It can access any files since the service can be running as root
    - It can do work in parallel on a separate computer.

    The eventdispatcher makes use of the base `snaplogger` library which
    is why this server extension is in a separate project.

* Format

    Your log messages can be formatted in many different ways. The
    `snaplogger` library supports many variables supporting parameters
    and functions. All of this is very easily extensible.

    There are many variables supported by default, including displaying
    the timestamp of the message, environment variables, system information
    (i.e. hostname), and all the data from the message being logged.

    Our tests verify a standard one line text message and a JSON message
    to make sure that we support either format. You could also use a CSV
    or XML type of format.

* Diagnostics

    We support several types of diagnostics: map, trace, and nested.

    Maps are actually used to save various parameters such as the program
    name and the program version. You can use as many parameters as you'd
    like to show in your logs.

    You can emit _trace_ logs to have a better idea of where an error occurs.
    The trace logs are written along a log depending on the log severity level.
    The system keeps the last 10 trace logs (you can adjust the number to
    your liking).

    _**WARNING:** The trace logs have nothing to do with the TRACE severity
    level. The level at which trace logs are output depends on the log which
    gets emitted when trace logs are present._

    The nested diagnostics are used to track your location in a stack like
    manner. This is at times referred to as log breadcrumbs.

* Component

    The software allows for filtering the output using a "component" or
    section. When you send a log, you can use an expression such as:

        SNAP_LOG_INFO
            << snaplogger::section(snaplogger::g_secure_component)
            << "My Message Here"
            << SNAP_LOG_SEND;

    This means the log is sent to the appender's output only if the
    appender includes "secure" as one of its components. This allows us
    to very quickly prevent messages from going to the wrong place.

    **Note:** The secure component has a special macro that allows you
    to quickly use that one. For example:

        SNAP_LOG_INFO
            << "My Secure Message"
            << SNAP_LOG_SEND_SECURELY;

    Finally, for the "secure" component, we have a special modifier you can
    use like this:

        SNAP_LOG_INFO
            << snaplogger::secure
            << "This is a secure message"
            << SNAP_LOG_SEND;

    This gives you examples on how to implement your own components.

    You can also make components mutually exclusive. For example, we do
    not allow you to add the "normal" and "secure" components to the same
    message. The network components (see the snaplogger network extension
    in the [eventdispatcher project](https://github.com/m2osw/eventdispatcher/tree/main/snaplogger))
    also include "local"/"remote" and "tcp"/"udp" pairs, which can't be
    used simultaneously.

    You can also create your own components using one of the
    `snaplogger::get_component()` functions.

    **WARNING:** When adding any one component, the default ("normal")
    component is not automatically added. If you want that component
    to be included, you have to do it explicitely:

        SNAP_LOG_INFO
            << snaplogger::section(snaplogger::get_component("electric-fence"))
            << "My message here"
            << SNAP_LOG_SEND;

    In this last example, the only component added to the message is
    "electric-fence". If you also want to have the "normal" component,
    make sure to include it in the list:

        SNAP_LOG_INFO
            << snaplogger::section(snaplogger::get_component("electric-fence"))
            << snaplogger::section(snaplogged::g_normal_component)
            << "My message here"
            << SNAP_LOG_SEND;

    **Note:** You should allocate your components once and then reuse their
    pointers for efficiency. Calling the get_component() function each time
    is considered slow.

    You can print the list of components attached to a message using the
    `${components}` variable in your message format. This can be useful
    while debugging. Probably less useful in production. It can help you
    find out why a message does or does not make it through a given
    appender filter.

    Messages are also filtered from the set of components defined on the
    command line with the `--log-component` option. You can specify the
    name of one or more component in that list. Only messages that have
    at least one component matching that list are sent to the appenders.
    You can reverse the meaning by prepending the exclamation point:

        ... --log-component !debug ...

    In that case, if the `debug` component is found in the message, then
    the message gets dropped immediately.

* Fields

    More and more, log systems want to be able to automatically check various
    values of error messages. This can be tedious if you first have to parse
    the message with regular expression or similar search techniques.

    Our logger allows you to instead add name and value pairs called fields.
    You can see those as dynamic structures. We actually offer to output
    these fields as JSON objects.

    By default the library adds the message unique identifier as the field
    named `id`.

    You can add other (non-dynamic at the moment) fields by adding them to
    the logger instance through the add_default_field() function. It is,
    for example, a good idea to add the name of your service like so:

        snaplogger::logger::get_instance()->add_default_field(
                                                    "service", "<name>");

    Then every time you log a message, that field is added to it. This makes
    it possible to send all your logs from all your services to the same
    database and later find just the logs of one specific service.

    Internally, the Snap! Logger understands the following system names:

        _message
        _timestamp
        _severity
        _filename
        _function_name
        _line

    Any name that starts with an underscore is reserved for the system.

* Stack Trace

    With the help of the libexcept library, `snaplogger` is also
    capable of sending the stacktrace to the log appenders. This
    is done automatically with the `SNAP_LOG_SEND_WITH_STACK_TRACE()`
    macro:

        ...
        catch(std::exception const & e)
        {
            SNAP_LOG_ERROR
                << "Some error happened..."
                << SNAP_LOG_SEND_WITH_STACK_TRACE(e);
        }
        ...

    The stack trace will be sent one line at a time, so this adds many
    lines to your output.

* Bitrate Limit

    By default, all log messages are forwarded to the appender processing
    function (i.e. the function saving the log to a file, sending the log
    over a network, etc.)

    At times, it can be useful to limit the amount of data sent, especially
    when sending logs over a network or saving them to a file to limit the
    amount of I/O used and also limit the amount of disk space used. The
    appenders have a bitrate parameter that can be set to the maximum amount
    of data that the logger is allowed to forward per minute (although the
    amount is defined in "mbps").
    
    Note that at the moment this is based on a per appender basis. It is not
    a system wide limit either (i.e. if you run 20 services, each have its own
    private set of limits).

* Regex Filtering

    You have the ability to add as many file appenders as you'd like. You
    can already filter by Severity and Component and with our Regexp support
    you can further filter by checking whether the log message matches a
    regular expression or not.

* Multi-thread Safe

    The library is multi-thread safe. It can even be used with an
    asynchronous feature so you can send many logs and they are
    processed by a thread. That thread can easily be stopped in case
    you want to `fork()` your application (i.e. threads & `fork()` are
    not compatible).

* advgetopt support

    The logger has a function used to add command line options and another
    to execute them.

    This adds many features to your command line such as the `--debug`
    and `--logger-configuration-filenames` options. You can always add
    more.

    The library also is setup to capture logs from the
    [`advgetopt`](https://github.com/m2osw/advgetopt) and the
    [`cppthread`](https://github.com/m2osw/cppthread)
    libraries (both use the `advgetopt` log feature.)

    Similarly, the library can be setup to capture the `std::clog`
    stream, making it possible to write to your appenders from
    libraries that do not ever reference the snaplogger itself.
    This is done from [snapdev](https://github.com/m2osw/snapdev)
    which is a really low level set of C++ header files used by many
    of the [Snap! C++ environment](https://snapwebsites.org/).

* Ordinal Indicator

    The library comes with the `ordinal_number` class which one can use to
    output numbers followed with the correct ordinal indicator (i.e. as in
    1st, 2nd, 3rd, 4th, etc.).

    This support all 28 languages that make use of such a notation.


# Examples

There is [an `examples` folder in the eventdispatcher project](https://github.com/m2osw/eventdispatcher/tree/main/examples)
where I have simple client & server implementations showing how the
`snaplogger` and [`advgetopt`](https://github.com/m2osw/advgetopt)
libraries are used in a more complete environment.


# Reasons Behind Having Our Own Library

We want to be in control of the capabilities of our logger and we want
to be able to use it in many different situations such as beyond of
`fork()`, to generate different formats, etc.

All of our capabilities are just not readily available in other loggers.


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
* alert ([eventdispatcher project](https://github.com/m2osw/eventdispatcher/tree/main/snaplogger))
* tcp ([eventdispatcher project](https://github.com/m2osw/eventdispatcher/tree/main/snaplogger))
* udp ([eventdispatcher project](https://github.com/m2osw/eventdispatcher/tree/main/snaplogger))

**Note:** the last three are part of the `eventdispatcher` project which
          offers network related appenders.

## Configuration Files

You configure the `snaplogger` library via a set of configuration files.

By default, the files are searched under `/etc/<project-name>/logger/...`
and `/usr/share/<project-name>/logger/...` where your `<project-name >`
is defined through the [`advgetopt`](https://github.com/m2osw/advgetopt)
options.

The `advgetopt` makes it easy for configuration files to be defined in
many other places. Your project may or may not offer such. Yet it is
easy to also support configuration files under the user configuration
directory (`~/.config/<project-name>/logger/...`) and the project home
folder (`/var/log/<project-name>/logger/...`).

To list all the possible locations, use the following command line:

    my-tool --logger-configuration-filenames

More information about configuration files can be found in the
[`advgetopt`](https://github.com/m2osw/advgetopt).

The configuration files include parameters that setup the logger.
These are described in the following list (incomplete at the moment).

### Appender

The configuration has some global settings and then a list of appenders.

An appender is a name and then a set of parameters:

* Type
* Severity
* Component
* Filter
* Format
* No-Repeat
* ...

These are described below.

Here is an example with a console and a file appender:

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
    path=/var/log/snapwebsites
    filename=firewall
    maximum_size=10Mb
    on_overflow=rotate
    lock=false
    fallback_appenders=console, syslog
    secure=true

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

The filename must can be a full path with an extension:

    filename=/var/log/my-app/strange-errors.txt

If no slashes are found, then the `path` parameter is prepended, so the
following has the same effect:

    filename=strange-errors.txt
    path=/var/log/my-app

Similarly, if `filename` does not include an extension, the default ".log"
is appended. In the following case, the file wil be in the same location,
but it will use the ".log" extension:

    filename=strange-errors
    path=/var/log/my-app

When using files for the output, the files can grow very quickly if the
process derails (i.e. there is an issue and it tries again and again
generating many log messages non-stop). For this reason, we have a
size limiter set at 10Mb by default. You can change this parameter
as follow:

    maximum_size=25Mb

If you set the `maximum_size` parameter to zero, then the limit is
removed and no checks are performed. If the maximum size is
reached before logrotate has time to rotate a file, the snaplogger
will applied the specified action:

    on_overflow=rotate

The supported actions are:

* skip -- simply ignore the file appender until logrotate does its job
* fatal -- throw the `snaplogger::fatal_error`
* rotate -- do a _simple rotation_ of the current file with the next
* logrotate -- not yet (properly) implemented
* `<other>` -- anything else is taken as the "skip", although it may be
               an error in this version it allows multiple versions of
               the snaplogger to run on a system

The simple _simple rotateion_ is equivalent to:

    mv -f my-app.log my-app.log.1
    echo "message" > my-app.log

A form of logrotate without the compression or more than one backup.
Note that the existing `my-app.log.1` is overwritten (somewhat on purpose).

    fallback_to_console=true
    fallback_to_syslog=true
    fallback_appenders=console,syslog,tcp,some_file
    fallback_only=false
    severity_considered_an_error=WARNING

The fallback feature allows for messages to be sent to another appender if
the normal appender is failing (i.e. disk is full, network is not reachable,
etc.)

The file type supports a `fallback_to_console` and `fallback_to_syslog`.
These are implemented directly within the file appender. They do not
require another appender. The console has priority. Also, if the severity
is equal or larger than `severity_considered_an_error`, then it sends the
message to stderr instead of stdout.

The `fallback_appenders` is a list of appender names. This parameter works
in all appenders. The name must be specified as defined in the `[...]` name.
There is no limit to the number of fallbacks although in most cases you
probably do not want more than one or two. It is also suggested that you
use `fallback_only=true` for those definitions since it would always be
sent to that appender anyway.

So the fallback order in the file appender is:

1. try the console (stderr or stdout--internally implemented)
2. try the syslog (internally implemented)
3. try each appender as defined in the `fallback_appenders` parameter

Note that to avoid (1) and (2), just keep the corresponding variables
set to false, which is the default.

#### Additional Appenders

We have additional appenders in other projects. There are in these projects
because that way they can make use of projects that depend on the snaplogger.

* Network Appenders

    The eventdispatcher project includes the TCP, UDP, and alert appenders.

    The TCP allows for the logs to be sent via TCP to a remote log daemon.

    The UDP allows for the logs to be sent via UDP to a remote log daemon.
    This type should use a bitrate parameter to avoid sending too many
    messages and lose many of them.

    The Alert allows for the logs to be sent via TCP to a remote daemon
    after being filtered in various ways and only if it happens a given
    number of time (unless marked as an alert log, in which case, by default,
    it always gets sent).

### Severity

By default the severity is set to INFORMATION.

This means anything below that severity, such as DEBUG or NOTICE, gets ignored.
Anything equal or above that severity gets included.

The severity can be different for each appender.

We support the following levels:

* `ALL`
* `TRACE`
* `NOISY`
* `DEBUG`
* `NOTICE`
* `UNIMPORTANT`
* `VERBOSE`
* `CONFIGURATION`
* `CONFIGURATION_WARNING`
* `INFORMATION`
* `IMPORTANT`
* `MINOR`
* `TODO`
* `DEPRECATED`
* `WARNING`
* `MAJOR`
* `RECOVERAL_ERROR`
* `ERROR`
* `NOISY_ERROR`
* `SEVERE`
* `EXCEPTION`
* `CRITICAL`
* `ALERT`
* `EMERGENCY`
* `FATAL`
* `OFF`

The severity can also be changed dynamically (along the way, from command line
arguments, etc.). Here is an example for a configuration file:

    [log]
    severity=DEBUG

### Component

Each entry can be segregated in a specific component. This gives your
application the ability to send the log to a specific component.

    [emergency]
    components=hardware,heat,vibration,radiation

In Snap!, we make use of the `normal` and `secure` components. The `secure`
logs are better protected (more constrained `chmod`, sub-folder, etc.)
than the logs sent via the `normal` component. `snaplogger` defines all
of the following components:

    g_as2js_component
    g_cppthread_component
    g_clog_component
    g_debug_component
    g_normal_component
    g_secure_component
    g_self_component
    g_banner_component
    g_not_implemented_component

The `as2js` component is used to mark messages coming from the as2js library.

The `cppthread` component is used to mark messages coming from the cppthread
library.

The `clog` component is used to mark messages captured from the `std::clog`
stream.

The `self` component is used by the `snaplogger` library. So any log
generated by the library will be marked by at least that one component.
That does not apply to the `as2js`, `cppthread`, and `clog` streams which
are handled by `snaplogger` but those logs are not considered as being
generated by `snaplogger` itself.

The `banner` component is used whenever sending banner like logs. This
allows you to easily filter out banners (especially if your project
generates many lines like a large logo in ASCII art).

There are many parts of our Snap! C++ environment which is not yet
implemented, so `not_implemented` component is often used by logs that
mention hitting a line of code that is expected to do more in the future.

The `debug`, `normal`, and `secure` are basic defaults that are used to
mark messages are being used for debugging purposes, _normal_ logs,
and secure logs. The secure logs are expected to be saved in secured
files (files with permissions and ownership prevent most users from
accessing them without using sudo or a similar Unix feature).

The [eventdispatcher project](https://github.com/m2osw/eventdispatcher/tree/main/snaplogger)
defines a `snaplogger` network extension (to efficiently send the logs
over the network). This extension includes several new components:

    g_network_component;
    g_daemon_component;
    g_remote_component;
    g_local_component;
    g_tcp_component;
    g_udp_component;

The `daemon` component is similar to the `self` component. It indicates
that the `snaplogger` daemon generated that log.

The `remote` component is added to clearly mark logs that come from
another computer,

The `local` component is the opposite of the `remote` component. It
marks messages that were created locally.

The `tcp` and `udp` components are used by the respective appender.
They are also mutually exclusive.

Others can be added. Internally, the name is forced to lowercase. So `Normal`
is the same as `normal`.

Technically, when no component is specified in a log, `normal` is assumed.
Otherwise, the appender must include that component or the log message is
ignored. (i.e. that log will not make it to that appender output if none
of its components match the ones in the log message being processed.)

### Bitrate

At times, a process may end up sending way too many logs at once. This may be
fine for a console (the console will force a slow down of the incoming data)
but for some other appenders, this may be a killer (i.e. you end up filling
up your drive, you have so much to send over the network that it kills all
client connections, etc.)

So we offer a speed which is measured in `mbps` (mega bits per second). We
use that measurement because many  people are familiar with it because
networks generally use it.

    [file]
    # Limit to about 122Kb of log per second
    bitrate=1

1 mbps, as shown in this example, represents 1,000 messages of a little
over 100 characters (mainly assuming ASCII characters) per second. The
parameter supports decimal numbers (i.e. 0.1 for one tenth of that number).

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
    ${severity[:format=alpha|number|systemd]}
                                severity level description, in color if allowed
                                `systemd` outputs a systemd log priority tag
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
    ${diagnostic:trace[=count]} the current set of traces (the last 'count'
                                ones if larger)

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
    ${time:hour=24|12}          hour number; (0 to 23 or 1 to 12; 24 is implied
                                if not specified)
    ${time:minute}              minute number (0 to 59)
    ${time:second}              second number (0 to 60)
    ${time:millisecond[=leadingzeroes]}
                                millisecond number (0 to 999)
    ${time:macrosecond[=leadingzeroes]}
                                macrosecond number (0 to 999999)
    ${time:nanosecond[=leadingzeroes]}
                                nanosecond number (0 to 999999999)
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

The variable name can be followed by a colon and parameters. Many
parameters can be assigned a value. We offer three types of parameters:

* Direct parameters which transform what the variable returns. For
  example, the `${time:hour}` outputs the hour of the message timestamp.
* Actual parameters, such as the `align` parameter. This one takes a
  value which is one of those three words: "left", "center", or
  "right". That alignment is held in the formatter until we
  are done or another `align=...` happens.
* Finally, we have the parameters that represent functions. Those further
  format the input data. For example, you can write the PID in your logs.
  You may want to pad the PID with zeroes as in:

        ${pid:padding='0':align=right:exact-width=5}

  This way the output of the PID will always look like it is 5 digits.

To format the `${time:nanosecond}` output and only show the milliseconds
(i.e. first 3 digits), you can use the following:

    ${time}.${time:nanosecond:padding='0':exact_width=9:align=left:max_width=3}

Note that we now offer the `${time:millisecond}` instead, which makes it faster
and much easier to use. That being said, you probably still want to align with
padding:

    ${time}.${time:millisecond:padding='0':exact_width=3}

#### Fields

The messages can be assigned fields which are viewed as user defined
variables. These can be output in the format with the `${field:...}`
and `${fields:...}` formatting entries.

Messages are automatically assigned a unique identifier named `"id"`.
This field can be output like so:

    ${field:name=id}

It is customary to include the identifier either at the beginning or
the end of your messages. By default, though, it is not output.

To better support JSON formatting, we also offer a way to output
all the fields in a list like so:

    ${fields:format=json:json=object}

The JSON format supports three objects `json=start_comma`,
`json=end_comma`, and `json=object`. The `start_comma` option means
that if any field is output, then a comma is added before the first
value. `end_comma` means that a comma gets added at the end, no matter
what. `object` means that the values are defined within `{...}`. This
gives you the ability to add these fields after or before your own
fields. Here is an example using the `object` feature:

    {"id":"123","color":"blue"}

In general, you want to create a JSON object with the message, some
other fields such as the location (filename, line number), and
then add the fields as JSON like so:

    {"message":"${message}","filename":"${filename}","line":${line},"fields":${fields:format=json:json=object}}

We also offer a Shell format:

    ${fields:format=shell}

This option means the fields are about as a list of Shell variables:

    id=123
    material=cheese
    ...

The implementation of the field variable allows for fields that are
managed by the `message` object. The means some fields are available to
you even when you don't define them as fields. These are:

* `_message` -- the raw message
* `_timestamp` -- the time when the `message` object was created
* `_severity` -- the severity of the message
* `_filename` -- the file in which the message was created
* `_function_name` -- the function in which the message was created
* `_line` -- the line number on which the message was created

These parameters are the same as the ones found in the `message` object.

**WARNING:** All fields are ignored when comparing two messages to know
whether they repeat. This makes for a slightly faster test and allows us
to have the `"id"` field which change each time a message is created.

#### Extending Formats

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

Since you may need your own parameter values to last between calls, we allow
for saving them in a map of strings. Here is how we save the one character
for the padding feature:

    d.set_param(std::string("padding"), pad);

Functions receive the `value` of the variable which is reachable with
the `get_value()` function. Note that it returns a writable reference
for faster modifications. For example appending is very easy, however,
prepending still requires a `set_value()` call:

    d.get_value() += " add at the end";
    d.set_value(padding + d.get_value());

So the snaplogger message format is extremely extensible.

#### Default Format

The default format:

    ${date} ${time} ${hostname} ${progname}[${pid}]: ${severity}: ...
          ... ${message:escape:max_width=1000} ...
          ... (in function \"${function}()\") ...
          ... (${basename}:${line})

If no format is defined for an appender then this format is used. The "..."
is just to show it continues on the following line.

#### Systemd Severity Format

The severity variable includes a `systemd` format which allows you to output
the severity of a log message as a systemd tag priority.

The systemd environment offers a feature to allow the output to std::cout
or std::cerr to be sent to the journalctl (default) or syslog. In both cases,
the tag is defined as the syslog level written between angle brackets. For
example, an error tag looks like this:

    <3>

The tag will be transformed by to a priority before the log is sent to
log files.

You want to use this feature with your `[console]` entry. You can include
the snaplogger severity as a name as well:

    ${severity:format=systemd}${message} (${severity})

This will output the tag at the start and the severity exact name at the end.

**NOTE:**

The severity priority in syslog is limited to 8 levels (3 bits).

Our NOTICE severity level is below INFORMATION. To output a NOTICE to syslog,
you want to use a MINOR severity level instead.

#### JSON Format

To support the JSON format you need to be careful with double quotes.
They need to be escaped. (TODO: finish example below)

    {"date":"${date}",

### No Repeat

The library has the ability to avoid sending the last few messages more
than once. The setting defines the number of messages that are kept by
the appenders and that will not be repeated.

    [file]
    no_repeat=5

This parameter is defined in appenders because that way the formatting
was already applied and we don't compare the string format but really
the final message before sending it for final processing (i.e. write to
console or disk). That being said, we still ignore all date related
formats for this test.

The parameter is currently limited to `NO_REPEAT_MAXIMUM` (100 at time
of writing). So any one of up to the last `NO_REPEAT_MAXIMUM` messages
will be dropped if repeated. To set the parameter to the maximum whatever
it currently is, you can use the following:

    no_repeat=maximum
      -- or --
    no_repeat=max

By default the parameter is set to `off` (0) meaning that the `no-repeat`
feature is not active (all messages can be repeated).

    no_repeat=off
      -- or --
    no_repeat=0

There is also a `default` special value which is set to `NO_REPEAT_DEFAULT`
(10 at time of writing):

    no_repeat=default


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

            SNAP_LOG_INFO
                << "message"
                << SNAP_LOG_SEND;
        }
    }

When you use the `${diagnostic}` format variable, it outputs "sec-1/sub-A".
This gives you a very good idea of which block the log appears in.
(The filename and line number should be enough but with the nested
diagnostic, it's even easier).

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
            SNAP_LOG_INFO
                << "message"
                << SNAP_LOG_SEND;
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


# Debugging Factories

The library checks the `APPENDER_FACTORY_DEBUG` flag. If set, it will display
the name of each appender being registered. This is particularly helpful when
you are trying to use plugins.

You may also want to test with the `--list-appenders` command.


# Properly Deleting Log Files

Whenever you create a packge for your project and the purge function is used
to completely remove your project, you should delete all the corresponding log
files along everything else. In your `postrm` script, you want to delete
secure logs and logs with user data such as IP addresses, anything that
includes _compromising_ data, using the `shredlog` command.

Here is an example to remove the Snap! Firewall secure logs:

    shredlog -fu /var/log/snapwebsites/snapfirewall.log*

Similarly, the `logrotate` tool needs to be used with the shredding feature:

    /var/log/snapwebsites/secure/snapfirewall.log {
        ...
        shred
        ...
    }

This way your files will properly be shredded.

## Shredding and SSD Devices

Note that SSD drives are likely to write new data at a new address. In
other words, you are not overwritting anything. If you are using SSD
drives, then do not use any kind of shredding feature. This means you
are left with encryption (see below) if you want to make your logs safe.

**Note:** The `shredlog` command can be used on SSD. See usage for details.

## Do I Really Need Shredding Files?

If you are in full control of the hardware, you probably do not need to
spend any time shredding anything. If you use a VPS or even a dedicated
server at a data center where you are not the only user, then you do
want to use the `shredlog` command. You may lose your VPS which then gets
assigned to another user which could possibly find a way to read your
data (it _should_ not be possible for them to do that, but...)

## Is `shred` Working on my File System/Device?

Chances are it will work as expected, but some types of file systems often
do not overwrite existing data. Maybe it writes new data at new locations
in a sequencial manner (as far as the drive is concerned). It could also
be be that some data ends up in the journal instead of the file itself.
`shred` uses `sync` by default, but all file system do not respect that
command 100%.

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

**Note:** This is one reason why we created `shredlog`. Our tool can
do a straight forward `rm \<file>` or use `shred <file>` depending on
whether your hard drive is an HDD or an SSD. The `--auto` command line
argument is there for this reason. Also you can force the `rm ...` if
you know your system is using SSD drives (this is actually the default
at this time since 99.9% of data centers offer SSD and not HDD).

## Instead of Shredding

Another solution is to use an encrypted drive. For logs, though, it can
be a good idea to send them over the wire (i.e. use the [#logserver])
and encrypt the drive where you write the logs on that [#logserver]
computer.

Encryption makes things a little slower, but when taking advantage of
newer hardware it is still pretty fast. From various sources, it looks
like the impact is only around +0.25%. In other words, the costs are
probably worth looking into using ext4 encryption. Note that this is
true even with SSD drives. So with HDD drives, you are even less likely
to notice the difference. Note also that encrypted data take more space
on disk. One sector is going to be more than the usual 512 bytes of data.

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
see the `man mount` manual pages for details). This should work on any
computer, but a [#logserver] will get even more logs and thus having
a separate partition makes even more sense in that situation.

## What About Other Files?

The shredding mechanism can be used with any file. Obviously, it makes the
delete _very slow_, so it is recommended to only use this technique against
files that are likely to require it, such as your secure logs.

Two other places where I use the technique:

1. Configuration files where I have important data such as encryption keys,
   passwords, and various _top-secret_ IP addresses.
2. Database files, for example, a Cassandra node is going to use
   `/var/lib/cassandra/data` for your database. You could encrypt/shred
   these files.

In other words, any file that may include sensitive data (personal
information) should be shred to piece.


# TRIM on SSD

For servers specifically setup to receive logs, you may want to consider
using the TRIM feature of _ext4_, _btrfs_, and _fat_ (as of Linux 4.4).

If you are using a VPS from a system such as DigitalOcean, then you do
not have access to the low level setup of the drive so you can't do
anything about the TRIM feature. Not only that, you are likely to be
using an HDD simulator, so the TRIM option is not going to be supported.

If you own your own hardware, then this feature makes sense for you to
use. It tells the SSD drive about all the sectors that are not used
anymore and allows the garbage collector in the SSD controller to better
know which chip to use next.

To TRIM feature is turned on by using the `discard` option when mounting
a drive. For example:

    # <file system> <mount point>   <type>  <options>         <dump>  <pass>
    /dev/sdc1       /mnt/ssd        ext4    defaults,discard  0       2


# License

The project is covered by the
[GPL 3.0 license](https://github.com/m2osw/snaplogger/blob/master/LICENSE.txt).


# Bugs

Submit bug reports and patches on
[github](https://github.com/m2osw/snaplogger/issues).


_This file is part of the [snapcpp project](https://snapwebsites.org/)._

vim: ts=4 sw=4 et
