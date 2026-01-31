// Copyright (c) 2006-2025  Made to Order Software Corp.  All Rights Reserved
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

// self
//
#include    "catch_main.h"


// snaplogger
//
#include    <snaplogger/console_appender.h>
//#include    <snaplogger/exception.h>
//#include    <snaplogger/format.h>
#include    <snaplogger/logger.h>
#include    <snaplogger/map_diagnostic.h>
//#include    <snaplogger/message.h>
//#include    <snaplogger/severity.h>
//#include    <snaplogger/version.h>


// cppthread
//
#include    <cppthread/runner.h>
#include    <cppthread/thread.h>


// snapdev
//
#include    <snapdev/ostream_to_buf.h>


// C
//
//#include    <unistd.h>
//#include    <netdb.h>
//#include    <sys/param.h>


namespace
{



// a thread so we can create a pipe and read the other side so the write
// does not get stuck
//
class pipe_reader
    : public cppthread::runner
{
public:
    typedef std::shared_ptr<pipe_reader>        pointer_t;


    pipe_reader(int pipe)
        : runner("pipe-reader-runner")
        , f_pipe(pipe)
    {
    }


    virtual void enter() override
    {
        // avoid the cppthread "entering" message
    }


    virtual void run() override
    {
        for(;;)
        {
            char buf[256];
            ssize_t n(read(f_pipe, buf, sizeof(buf)));
            if(n < 0)
            {
                return;
            }
//std::cout << "--- received [" << std::string(buf, n) << "]" << std::endl;
            f_received += std::string(buf, n);
        }
    }


    virtual void leave(cppthread::leave_status_t status) override
    {
        // avoid the cppthread "leaving" message
        //
        snapdev::NOT_USED(status);
    }


    std::string const & get_received() const
    {
        return f_received;
    }

private:
    int         f_pipe = -1;
    std::string f_received = std::string();
};



}




CATCH_TEST_CASE("console_appender", "[appender]")
{
    CATCH_START_SECTION("console_appender: create")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "one-console");

        snaplogger::appender::pointer_t console(snaplogger::create_appender("console", "test-console"));
        CATCH_REQUIRE(console != nullptr);
        CATCH_REQUIRE(console->get_type() == "console");
        CATCH_REQUIRE(console->get_name() == "test-console");

        snaplogger::console_appender::pointer_t console_appender(std::dynamic_pointer_cast<snaplogger::console_appender>(console));
        CATCH_REQUIRE(console_appender != nullptr);

        CATCH_REQUIRE_FALSE(console_appender->get_force_style());
        console_appender->set_force_style(true);
        CATCH_REQUIRE(console_appender->get_force_style());
        console_appender->set_force_style(false);
        CATCH_REQUIRE_FALSE(console_appender->get_force_style());

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::appender::vector_t appenders(l->get_appenders());
        CATCH_REQUIRE(appenders.size() == 0);
        CATCH_REQUIRE_FALSE(l->has_appender("console"));
        CATCH_REQUIRE(l->get_appender("test-console") == nullptr);
        l->add_appender(console);
        CATCH_REQUIRE(l->has_appender("console"));
        CATCH_REQUIRE(l->get_appender("test-console") != nullptr);
        appenders = l->get_appenders();
        CATCH_REQUIRE(appenders.size() == 1);

        // trying to add another console "works" but the existing
        // appender (the "original") is kept and the second instance is
        // totally ignored
        //
        snaplogger::appender::pointer_t second_console(snaplogger::create_appender("console", "second-console"));
        CATCH_REQUIRE(second_console != nullptr);
        CATCH_REQUIRE(second_console->get_type() == "console");
        CATCH_REQUIRE(second_console->get_name() == "second-console");
        l->add_appender(second_console);
        appenders = l->get_appenders();
        CATCH_REQUIRE(appenders.size() == 1); // still 1!
        CATCH_REQUIRE(appenders[0] == console); // original, "second_console" was ignored
        CATCH_REQUIRE(console->get_name() == "test-console"); // still the original console!

        l->reset();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("console_appender: default console + create user console")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "user-console");

        // by default, output is sent to stderr; replace that file
        // with a pipe so we can capture anything that goes there
        //
        // Note: unfortunately, it has to be done early on because the
        //       fd is read at the time the console appender gets initialized
        //       the first time and our pipe needs to be ready by then
        //       this means we may capture other error messages in our
        //       `pc` thread runner
        //
        int fifo[2]; // 0 -- read, 1 -- write
        int const r(pipe(fifo));
        CATCH_REQUIRE(r == 0);

        FILE * fout(fdopen(fifo[1], "w"));
        std::swap(*fout, *stderr);

        pipe_reader::pointer_t pc(std::make_shared<pipe_reader>(fifo[0]));
        cppthread::thread t("pipe-reader", pc.get());
        CATCH_REQUIRE(t.start());

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::appender::vector_t appenders(l->get_appenders());
        CATCH_REQUIRE(appenders.size() == 0);

        // the default console is named "console"
        //
        snaplogger::appender::pointer_t console(l->add_console_appender());
        CATCH_REQUIRE(console != nullptr);
        CATCH_REQUIRE(console->get_type() == "console");
        CATCH_REQUIRE(console->get_name() == "console");
        CATCH_REQUIRE(std::dynamic_pointer_cast<snaplogger::console_appender>(console) != nullptr);
        CATCH_REQUIRE(std::dynamic_pointer_cast<snaplogger::console_appender>(console)->get_output_stream() == "stderr");

        CATCH_REQUIRE(l->has_appender("console"));
        CATCH_REQUIRE(l->get_appender("console") == console);
        appenders = l->get_appenders();
        CATCH_REQUIRE(appenders.size() == 1);
        CATCH_REQUIRE(appenders[0] == console);

        // trying to add another console "works" but the existing
        // appender (the "original") is kept and the second instance is
        // totally ignored except in this very special case where the
        // original was named "console"--here the original gets renamed
        // to the new console "user" name
        //
        snaplogger::appender::pointer_t second_console(snaplogger::create_appender("console", "user-console"));
        CATCH_REQUIRE(second_console != nullptr);
        CATCH_REQUIRE(second_console->get_type() == "console");
        CATCH_REQUIRE(second_console->get_name() == "user-console");
        CATCH_REQUIRE(std::dynamic_pointer_cast<snaplogger::console_appender>(second_console) != nullptr);
        CATCH_REQUIRE(std::dynamic_pointer_cast<snaplogger::console_appender>(second_console)->get_output_stream() == "stderr");
        l->add_appender(second_console);
        appenders = l->get_appenders();
        CATCH_REQUIRE(appenders.size() == 1); // still 1!
        CATCH_REQUIRE(appenders[0] == console); // original, "second_console" was ignored
        CATCH_REQUIRE(console->get_name() == "user-console"); // except that the name was updated

        SNAP_LOG_ERROR << "Test the console." << SNAP_LOG_SEND;

        std::swap(*fout, *stderr);
        fclose(fout);
        close(fifo[0]); // only close fifo[0], fifo[1] was managed by FILE* in fout
        t.stop();

        // we expect the default format to include the name of the
        // appender and the message above and the severity
        //
        std::string const & received(pc->get_received());
        CATCH_REQUIRE(received.find("Test the console.") != std::string::npos);
        CATCH_REQUIRE(received.find(console->get_name()) != std::string::npos);
        CATCH_REQUIRE(received.find("error") != std::string::npos);

        l->reset();
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
