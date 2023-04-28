
# Default Configuration

This directory is used to save default logger configurations.

The logger will first look for files under `/usr/share/snaplogger/etc`.
If you have a tool which uses the `snaplogger` library, you can save your
defaults there which means administrators can then update the files
under `/etc` and not affect your defaults.

# Which Files?

To know which files are going to be read from this directory, you
can use the `--logger-configuration-filenames` command line option:

    my-app --logger-configuration-filenames

This gives you a list of the configuration files that can be used
with `snaplogger`:

    Logger common configuration filenames:
     . /usr/share/snaplogger/etc/snaplogger.conf
     . /usr/share/snaplogger/etc/${APPNAME}.d/50-snaplogger.conf
     . /etc/${APPNAME}/logger/snaplogger.conf
     . /etc/${APPNAME}/logger/${APPNAME}.d/50-snaplogger.conf
     . /home/${USER}/.config/${APPNAME}/snaplogger.conf
    Logger application configuration filenames:
     . /usr/share/snaplogger/etc/${APPNAME}.conf
     . /usr/share/snaplogger/etc/${APPNAME}.d/50-${APPNAME}.conf
     . /etc/${APPNAME}/logger/${APPNAME}.conf
     . /etc/${APPNAME}/logger/${APPNAME}.d/50-${APPNAME}.conf
     . /home/${USER}/.config/${APPNAME}/${APPNAME}.conf

## Priority

Note that the `50-...` is the default number. You can add filenames with
any number from `00-...` to `99-...`. This gives you the ability to have
defaults loaded before or after the user expected settings which should
always use the `50-...` filename.

The priority is useful for your application to define defaults that:

1. You want the user to be able to overwrite. Namely, use a priority under 50.
2. You do not want the user to be to overwrite. Namely, use a priority over 50.

Note that a user can also use a priority other than 50, but that is what the
user is expected to use.

This mechanism is also useful for other applications to tweak your application
settings by installing files with lower or higher priorities.

## User Name

The `USER` variable is the name of the currently logged in user. It is
often `root` for daemons.

## Application Name

The `APPNAME` parameter is the name of your application.

See the "How do I implement that in my application?" section below for
details on how a programmer makes this available. Note that in some cases
the `advgetopt` library uses a group name instead of the project name.
In most cases, though, applications are not used in a group so the name
is going to be the same (i.e. in general the path uses the group name
and the `.conf` filename uses the project name).

Here is an example of the `shredlog` tool which comes with the `snaplogger`
libray. The output shows that the path uses `snaplogger` and the tool
name is still `shredlog` (second section shows `[50-]shredlog.conf`):

    Logger common configuration filenames:
     . /usr/share/snaplogger/etc/snaplogger.conf
     . /usr/share/snaplogger/etc/snaplogger.d/50-snaplogger.conf
     . /etc/snaplogger/snaplogger.conf
     . /etc/snaplogger/snaplogger.d/50-snaplogger.conf
     . /home/alexis/.config/snaplogger/snaplogger.conf
    Logger application configuration filenames:
     . /usr/share/snaplogger/etc/shredlog.conf
     . /usr/share/snaplogger/etc/snaplogger.d/50-shredlog.conf
     . /etc/snaplogger/shredlog.conf
     . /etc/snaplogger/snaplogger.d/50-shredlog.conf
     . /home/alexis/.config/snaplogger/shredlog.conf



# How do I implement that in my application?

You need two of the Snap! C++ contrib libraries: `advgetopt` and `snaplogger`.
The first one handles all the options found on the command line, an
environment variable, and configuration files. The second handles the
logger specific options such as the `--logger-configuration-filenames`.

The project name you pass to the `advgetopt` object is what is viewed as
the `APPNAME` above, although if you use the group capability, some of them
make use of the group name instead.

    advgetopt::options_environment const g_options_environment =
    {
        .f_project_name = "<this is ${APPNAME}>",
	...
    };

The snaplogger options are added to your application by properly
initializing the application with the option set to just the
declaration of the environment and then calling the `add_logger_options()`
before finishing the argument parsing process. Finally, you have to
call the `process_logger_options()`, which is the one that finds
out the use of the `--logger-...` options before you start your own
application. Many of these options can also be included in your configuration
file.

Here is an example of such code snippet:

    my_app::my_app(int argc, char * argv[])
        : f_opt(g_options_environment)
    {
        snaplogger::add_logger_options(f_opt);
        f_opt.finish_parsing(argc, argv);
        if(!snaplogger::process_logger_options(f_opt, "/etc/my-app/logger"))
        {
            // exit on any error
            throw advgetopt::getopt_exit("logger options generated an error.", 0);
        }

        ...
    }

_**Note:** The throw on error is not required. For some daemons, you may
prefer to start the daemon no matter what. After all, the error will be
logged so the administrator will eventually see the message one day..._

If you are creating a conglomerate of applications, then you can use a
common `/etc/<app-name>` directory instead of one specific to your
application. That name should be the same as the name in your `f_group_name`
(found in the `advgetopt::options_environment` parameter).

    advgetopt::options_environment const g_options_environment =
    {
	...
        .f_group_name = "app-name",
	...
    };

