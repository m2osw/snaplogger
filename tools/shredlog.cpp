// Copyright (c) 2013-2024  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/snaplogger
// contact@m2osw.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// snaplogger
//
#include    <snaplogger/logger.h>
#include    <snaplogger/options.h>
#include    <snaplogger/version.h>


// advgetopt
//
#include    <advgetopt/exception.h>
#include    <advgetopt/options.h>
#include    <advgetopt/utils.h>


// cppthread
//
#include    <cppthread/log.h>


// snapdev
//
#include    <snapdev/glob_to_list.h>
#include    <snapdev/stringize.h>


// last include
//
#include    <snapdev/poison.h>





/** \file
 * \brief Tool to shred log and other files.
 *
 * This tool is used to shred files (by calling `shred -uf <filename>` for
 * exampe.)
 *
 * The shredlog tool can be used to shred and delete files and directories.
 *
 * It can be used to work recursively (`--recursive`) so an entire tree
 * can be destroyed in one go.
 *
 * The default mode (`--auto`) checks each file for its device. If that
 * device is an HDD, then the `shred` tool is used to first overwrite that
 * file's data and then delete it (the deletion happens if you use the
 * `--unlink` option along the `--auto`).
 *
 * We offer several other modes, especially, the auto-detection of HDD or
 * SSD drives fails badly on any VPS (the ones you run on your computer
 * at home and at data centers like DigialOcean). I think this is because
 * by default the systems that create virtualize computers have fake
 * drives for HDD and thus they _say_ that the drives are rotational.
 * Hopefully, this will be fixed at some point so the virtual system can
 * detect the original drive type and replicate that in the virtual
 * environment.
 *
 * The different modes:
 *
 * \li `--auto` -- let shredlog detect what to do; if the drive is an HDD,
 * then just shred the file; if the drive is an SSD, delete the file.
 *
 * \li `--delete` -- always delete the file (`rm \<filename>`)
 *
 * \li `--shred` -- always shred the file (`shred \<filename>`)
 *
 * \li `--unlink` -- in shred mode, also delete the file once done with the
 * shredding (so `shred -u \<filename>`)
 *
 * So `--unlink` flag can be used with `--auto`, `--delete`, and `--shred`.
 * With `--delete` it is somewhat redundant.
 *
 * Our tool supports all the command line options like `shred`.
 *
 * \li `--exact` -- do not round the file size to an exact number of blocks
 * which means you may not overwrite some of the data appearing after
 * the end (in the last block); this is useful on file where on APPEND
 * is used (i.e. logs, journals) and encrypted files which anyway are
 * going to use such and _break_ the encryption stream
 *
 * \li `--force` -- force the shredding and/or deletion of files. If the
 * file does not exist, remain silent.
 *
 * \li `--iterations=\<count>` -- the number of passes to shred the file;
 * this defaults to 3; the more passes the more likely the original data
 * will not be recoverable
 *
 * \li `--random-source` -- specify a filename to read random data from
 * (i.e. `/dev/random`)
 *
 * \li `--recursive` -- work recursively through directories; otherwise
 * a directory is deleted with `rmdir ...` unless `--force` is used and
 * a directory gets deleted with `rm -rf ...` (i.e. no shredding will
 * happen on the files inside the directory unless `--recursive` is
 * specified!)
 *
 * \li `--remove=\<unlink|wipe|wipesync>` -- specify how `shred` will delete
 * a file; `unlink` -- just call `unlink()`; `wipe` -- first truncate the
 * file, rename it multiple times, then `unlink()` it; `wipesync` -- like
 * `wipe` but use `sync` wherever possible (i.e. so each modification gets
 * commited on disk; otherwise it will only happen in memory)
 *
 * \li `--size=\<count[KMG]>` -- specify the maximum number of bytes to
 * shred; you can specify K, M, or G to change the number in kilo, mega,
 * and giga bytes; this size is the maximum, if the file is smaller, it
 * does not get enlarged
 *
 * \li `--verbose` -- show what is happening
 *
 * \li `--zero` -- before truncating/deleting the file, make one more pass
 * writing all zeroes to the file; this allows for hiding the fact that
 * the file was shredded and possibly the type of random source
 *
 * \warning
 * Note that by default the `shredlog` command is used in `auto` mode
 * which means that the files get deleted if on an SSD drive and only
 * shredded on an HDD drive (i.e. in the latter the file remains on
 * disk, only its data was overwriten with random data.)
 * \warning
 * It is strongly suggested that you always use the `--unlink` command
 * line option. We may actually change this behavior in later versions
 * where the `--unlink` gets removed and the behavior will be to always
 * delete everything in the end with a possibility of shredding in
 * between. In that case, we may want to rename the tool `rmlog`.
 */

namespace
{


advgetopt::option const g_options[] =
{
    // OPTIONS
    //
    advgetopt::define_option(
          advgetopt::Name("exact")
        , advgetopt::ShortName('x')
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("do not round file sizes up to the next full block; this is the default for non-regular files.")
    ),
    advgetopt::define_option(
          advgetopt::Name("ignore-fail-on-non-empty")
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("ignore the \"directory not empty\" error.")
    ),
    advgetopt::define_option(
          advgetopt::Name("force")
        , advgetopt::ShortName('f')
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("change permission to allow writting if necessary.")
    ),
    advgetopt::define_option(
          advgetopt::Name("iterations")
        , advgetopt::ShortName('n')
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::DefaultValue("3")
        , advgetopt::Help("overwrite this number of times instead of the default.")
    ),
    advgetopt::define_option(
          advgetopt::Name("mode")
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("select shreding mode: auto, delete, shred, both.")
    ),
    advgetopt::define_option(
          advgetopt::Name("random-source")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("get random bytes from specified file.")
    ),
    advgetopt::define_option(
          advgetopt::Name("recursive")
        , advgetopt::ShortName('r')
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("shred/remove directories and their contents recursively.")
    ),
    advgetopt::define_option(
          advgetopt::Name("remove")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::DefaultValue("wipesync")
        , advgetopt::Help("specify how to delete: \"unlink\", \"wipe\", or \"wipesync\".")
    ),
    advgetopt::define_option(
          advgetopt::Name("size")
        , advgetopt::ShortName('s')
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("shred this many bytes (suffixes like K, M, G accepted).")
    ),
    advgetopt::define_option(
          advgetopt::Name("verbose")
        , advgetopt::ShortName('v')
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("show progress.")
    ),
    advgetopt::define_option(
          advgetopt::Name("zero")
        , advgetopt::ShortName('z')
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("add a final overwrite with zeros to hide shredding.")
    ),

    // FILENAMES/DIRECTORIES
    //
    advgetopt::define_option(
          advgetopt::Name("--")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_NONE
                    , advgetopt::GETOPT_FLAG_MULTIPLE
                    , advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
    ),

    // END
    //
    advgetopt::end_options()
};



char const * g_configuration_directories[] =
{
    "/etc/snaplogger",
    nullptr
};


advgetopt::group_description const g_group_descriptions[] =
{
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
        , advgetopt::GroupName("command")
        , advgetopt::GroupDescription("Commands:")
    ),
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
        , advgetopt::GroupName("option")
        , advgetopt::GroupDescription("Options:")
    ),
    advgetopt::end_groups()
};


// until we have C++20, remove warnings this way
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "snaplogger",
    .f_group_name = nullptr,
    .f_options = g_options,
    .f_options_files_directory = nullptr,
    .f_environment_variable_name = "SHREDLOG",
    .f_environment_variable_intro = nullptr,
    .f_section_variables_name = nullptr,
    .f_configuration_files = nullptr,
    .f_configuration_filename = "shredlog.conf",
    .f_configuration_directories = g_configuration_directories,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [--<opt>] <config-name> ...\n"
                     "where --<opt> is one or more of:",
    .f_help_footer = "%c",
    .f_version = SNAPLOGGER_VERSION_STRING,
    .f_license = "GNU GPL v2",
    .f_copyright = "Copyright (c) 2013-"
                   SNAPDEV_STRINGIZE(UTC_BUILD_YEAR)
                   " by Made to Order Software Corporation -- All Rights Reserved",
    .f_build_date = UTC_BUILD_DATE,
    .f_build_time = UTC_BUILD_TIME,
    .f_groups = g_group_descriptions
};
#pragma GCC diagnostic pop







class tool
{
public:
    enum class select_t
    {
        SELECT_AUTO,
        SELECT_DELETE,
        SELECT_SHRED,
        SELECT_BOTH
    };

    int                             init(int argc, char * argv[]);
    int                             execute();

private:
    int                             process(std::string const & filename);
    bool                            get_disk_type(struct stat & s);

    advgetopt::getopt::pointer_t    f_opt = advgetopt::getopt::pointer_t();
    select_t                        f_select = select_t::SELECT_AUTO;
    int                             f_found_file = 0;
    bool                            f_verbose = false;
    bool                            f_ignore_fail_on_non_empty = false;
    bool                            f_force = false;
};



int tool::init(int argc, char * argv[])
{
    f_opt = std::make_shared<advgetopt::getopt>(g_options_environment);

    snaplogger::add_logger_options(*f_opt);

    f_opt->finish_parsing(argc, argv);

    if(!snaplogger::process_logger_options(
                  *f_opt
                , "/etc/snaplogger/logger"
                , std::cout
                , false))
    {
        // exit on any error
        //
        throw advgetopt::getopt_exit("logger options generated an error.", 0);
    }

    return 0;
}


int tool::execute()
{
    f_verbose = f_opt->is_defined("verbose");
    f_ignore_fail_on_non_empty = f_opt->is_defined("ignore-fail-on-non-empty");
    f_force = f_opt->is_defined("force");

    std::string const mode(f_opt->get_string("mode"));
    if(mode == "auto")
    {
        f_select = select_t::SELECT_AUTO;
    }
    else if(mode == "delete")
    {
        f_select = select_t::SELECT_DELETE;
    }
    else if(f_opt->is_defined("shred"))
    {
        f_select = select_t::SELECT_SHRED;
    }
    else if(f_opt->is_defined("both"))
    {
        f_select = select_t::SELECT_BOTH;
    }
    else
    {
        SNAP_LOG_FATAL
            << "mode \""
            << mode
            << "\" is  unknown. Try one of \"auto\", \"delete\", \"shred\", or \"both\"."
            << SNAP_LOG_SEND;
        return 1;
    }

    int result(0);
    std::size_t const max(f_opt->size("--"));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        int const r(process(f_opt->get_string("--", idx)));
        if(result == 0
        && r != 0)
        {
            result = r;
        }
    }

    if(f_verbose && f_found_file > 0)
    {
        std::cout << "shredded " << f_found_file << " files.\n";
    }

    return result;
}


int tool::process(std::string const & filename)
{
    struct stat s;
    if(stat(filename.c_str(), &s) != 0)
    {
        if(!f_force
        || errno != ENOENT)
        {
            int const e(errno);
            SNAP_LOG_ERROR
                    << "could not retrieve meta data of \""
                    << filename
                    << "\" (errno: "
                    << std::to_string(e)
                    << " -- "
                    << strerror(e)
                    << ")."
                    << SNAP_LOG_SEND;
            return 1;
        }
        return 0;
    }

    int result(0);

    if(S_ISDIR(s.st_mode))
    {
        // this is a directory, ignore unless we have --recursive
        //
        if(!f_opt->is_defined("recursive"))
        {
            SNAP_LOG_RECOVERABLE_ERROR
                    << "\""
                    << filename
                    << "\" is a directory; ignored (use --recursive to ${progname} sub-directories)."
                    << SNAP_LOG_SEND;
            return 0;
        }

        std::string const pattern(filename + "/*");
        snapdev::glob_to_list<std::list<std::string>> dir;
        if(!dir.read_path<
                  snapdev::glob_to_list_flag_t::GLOB_FLAG_IGNORE_ERRORS
                , snapdev::glob_to_list_flag_t::GLOB_FLAG_PERIOD
                , snapdev::glob_to_list_flag_t::GLOB_FLAG_EMPTY>(pattern))
        {
            SNAP_LOG_ERROR
                    << "an error occurred while reading directory \""
                    << filename
                    << "\": "
                    << dir.get_last_error_message()
                    << SNAP_LOG_SEND;
            return 1;
        }
        for(auto const & f : dir)
        {
            if(process(f) != 0)
            {
                result = 1;
            }
        }

        if(f_force)
        {
            std::string rmrf("rm --force --recursive ");
            rmrf += filename;
            if(f_verbose)
            {
                std::cout << rmrf << std::endl;
            }
            if(system(rmrf.c_str()) != 0)
            {
                // note: should we stay quiet if `result` is already not zero?
                //
                int const e(errno);
                SNAP_LOG_ERROR
                    << "could not delete directory \""
                    << filename
                    << "\" (errno: "
                    << std::to_string(e)
                    << " -- "
                    << strerror(e)
                    << ")."
                    << SNAP_LOG_SEND;
                return 1;
            }
        }
        else
        {
            if(f_verbose)
            {
                std::cout << "rmdir " << filename << std::endl;
            }
            if(rmdir(filename.c_str()) != 0)
            {
                if(errno != ENOENT
                && (!f_ignore_fail_on_non_empty || errno != ENOTEMPTY))
                {
                    int const e(errno);
                    SNAP_LOG_ERROR
                        << "could not delete directory \""
                        << filename
                        << "\" (errno: "
                        << std::to_string(e)
                        << " -- "
                        << strerror(e)
                        << ")."
                        << SNAP_LOG_SEND;
                    return 1;
                }
            }
        }
    }
    else
    {
        ++f_found_file;

        // this is a file, shred and/or unlink as specified
        //
        select_t select(f_select);

        if(select == select_t::SELECT_AUTO)
        {
            if(snaplogger::is_rotational(s))
            {
                select = select_t::SELECT_BOTH;
            }
            else
            {
                // just do a delete on SSD drives
                //
                select = select_t::SELECT_DELETE;
            }
        }

        if(select == select_t::SELECT_DELETE)
        {
            if(f_verbose)
            {
                std::cout << "rm " << filename << "\n";
            }
            if(unlink(filename.c_str()) != 0)
            {
                if(!f_force
                || errno != ENOENT)
                {
                    int const e(errno);
                    SNAP_LOG_ERROR
                        << "could not delete file \""
                        << filename
                        << "\" (errno: "
                        << e
                        << " -- "
                        << strerror(e)
                        << ")."
                        << SNAP_LOG_SEND;
                    return 1;
                }
            }
            return 0;
        }

        std::string shred("/usr/bin/shred ");
        if(f_force)
        {
            shred += "--force ";
        }
        if(f_opt->is_defined("iterations"))
        {
            shred += "--iterations ";
            shred += std::to_string(f_opt->get_long("iterations"));
            shred += " ";
        }
        if(f_opt->is_defined("random-source"))
        {
            shred += "--random-source ";
            shred += f_opt->get_long("random-source");
            shred += " ";
        }
        if(f_opt->is_defined("remove"))
        {
            shred += "--remove ";
            shred += f_opt->get_long("remove");
            shred += " ";
        }
        if(f_opt->is_defined("size"))
        {
            shred += "--size ";
            shred += f_opt->get_long("size");
            shred += " ";
        }
        if(f_verbose)
        {
            shred += "--verbose ";
        }
        if(f_opt->is_defined("exact"))
        {
            shred += "--exact ";
        }
        if(f_opt->is_defined("zero"))
        {
            shred += "--zero ";
        }
        if(select == select_t::SELECT_BOTH)
        {
            shred += "-u ";
        }
        shred += filename;

        if(f_verbose)
        {
            std::cout << shred << "\n";
        }
        if(system(shred.c_str()) != 0)
        {
            int const e(errno);
            SNAP_LOG_ERROR
                    << "could not shred file \""
                    << filename
                    << "\" (errno: "
                    << std::to_string(e)
                    << " -- "
                    << strerror(e)
                    << ")."
                    << SNAP_LOG_SEND;
            return 1;
        }
    }

    return result;
}




}
// no name namespace





int main(int argc, char * argv[])
{
    try
    {
        tool t;
        if(t.init(argc, argv) != 0)
        {
            return 0;
        }

        return t.execute();
    }
    catch(advgetopt::getopt_exit const &)
    {
        return 0;
    }
    catch(std::exception const & e)
    {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
}


// vim: ts=4 sw=4 et
