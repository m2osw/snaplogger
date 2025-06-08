// Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
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

/** \file
 * \brief Utilities function implementations.
 *
 * This file includes implementations of utility functions.
 */


// self
//
#include    "utils.h"


// advgetopt
//
#include    <advgetopt/utils.h>


// snapdev
//
#include    <snapdev/join_strings.h>


// C++
//
#include    <fstream>


// C
//
#include    <limits.h>
#include    <sys/sysmacros.h>



namespace snaplogger
{



namespace
{



struct device_type_t
{
    bool        f_type = false;
    int         f_errno = 0;
};

typedef std::map<int, device_type_t>        device_type_cache_t;

device_type_cache_t             g_device_type_cache = device_type_cache_t();



} // no name namespace



/** \brief Determine whether the specified file is on a HDD device.
 *
 * The shredlog likes to know whether a file is or not on a rotational 
 * device. The tool does not want to shred files on SSD devices because
 * it is never useful (the likelihood that shred works on an SSD is 0%).
 *
 * This function returns true if the drive is marked as rotational.
 *
 * The function returns false if the drive is not rotational or the
 * determination of the drive type failed in which case errno is set
 * to the last error.
 *
 * The errno errors returned are the same as what stat() and realpath()
 * may return or the ENODATA which means the function could not find
 * the 'rotational' information for that file.
 *
 * \warning
 * At this time, all types of VPS misrepresent the type of drives they
 * are offering. In most cases, all those drives are always marked as
 * rotational instead of whatever the original drive really is.
 *
 * \param[in] filename  The file for which you want to determine the
 * location type.
 *
 * \return true if the file is found to be on a rotational drive.
 */
bool is_rotational(std::string const & filename)
{
    struct stat s;
    if(stat(filename.c_str(), &s) != 0)
    {
        return false;
    }

    return is_rotational(s);
}


/** \brief Determine whether the stat structure references a rotational device.
 *
 * The shredlog likes to know whether a file is or not on a rotational 
 * device. The tool does not want to shred files on SSD devices because
 * it is never useful (the likelihood that shred works on an SSD is 0%).
 *
 * This function returns true if the drive is marked as rotational.
 *
 * The function returns false if the drive is not rotational or the
 * determination of the drive type failed, in which case errno is set
 * to the last error.
 *
 * The errno errors returned are the same as what realpath() function and
 * it may also return the ENODATA error number, which means the function
 * could not find the 'rotational' information for that file.
 *
 * \warning
 * At this time, all types of VPS misrepresent the type of drives they
 * are offering. In most cases, all those drives are always marked as
 * rotational instead of whatever the original drive really is.
 *
 * \note
 * Of note, the results get cached once unless the destination device
 * file is not found (the softlink under `/sys/dev/block/<major>:<minor>`
 * does not point to a valid directory). At this time we do not offer
 * a way to reset the cache.
 *
 * \note
 * Only the `st_dev` field of parameter \p s is used.
 *
 * \param[in] s  The stat structure of file for which you want to
 * determine the location type.
 *
 * \return true if the file is found to be on a rotational drive.
 */
bool is_rotational(struct stat & s)
{
    auto const it(g_device_type_cache.find(s.st_dev));
    if(it != g_device_type_cache.end())
    {
        errno = it->second.f_errno;
        return it->second.f_type;
    }

    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        device_type_t const type = {
            .f_type = false,
            .f_errno = ENODATA,
        };
#pragma GCC diagnostic pop
        g_device_type_cache[s.st_dev] = type;
    }

    std::string dev_path("/sys/dev/block/");
    dev_path += std::to_string(::major(s.st_dev));
    dev_path += ":";
    dev_path += std::to_string(::minor(s.st_dev));
    char device_path[PATH_MAX + 1];
    if(realpath(dev_path.c_str(), device_path) == nullptr)
    {
        if(errno == ENOENT)
        {
            // if the destination file does not exist, it might later
            // so do not cache that information
            //
            g_device_type_cache.erase(g_device_type_cache.find(s.st_dev));
        }
        else
        {
            g_device_type_cache[s.st_dev].f_errno = errno;
        }
        return false;
    }
    device_path[PATH_MAX] = '\0'; // extra security

    advgetopt::string_list_t segments;
    advgetopt::split_string(device_path, segments, { "/" });
    while(segments.size() > 3)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wrestrict"
        std::string const path(
                          "/"
                        + snapdev::join_strings(segments, "/")
                        + "/queue/rotational");
#pragma GCC diagnostic pop
        std::ifstream in;
        in.open(path);
        if(in.is_open())
        {
            char line[32];
            in.getline(line, sizeof(line));
            g_device_type_cache[s.st_dev].f_type = std::atoi(line) != 0;
            g_device_type_cache[s.st_dev].f_errno = 0;
            errno = 0;
            return g_device_type_cache[s.st_dev].f_type;
        }
        segments.pop_back();
    }

    errno = ENODATA;
    return false;
}



} // snaplogger namespace
// vim: ts=4 sw=4 et
