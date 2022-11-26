
* Verify that data passed by app. is valid UTF-8 (especially the message)
  If some bytes do not represent a valid UTF-8 character, replace with '?'.
  We should be able to use a function in libutf8 for that purpose.

* Change how the variables are discovered in our messages

  The best (I think) would be to have sections of the message clearly marked
  as including variables and others not (i.e. by default the message itself
  should probably not include variables).

  I had a problem with ipload which has variables which may include an
  advgetopt variable (i.e. `knock = ${knock_ports}`). Writing that value
  in log message was captured by the "here is a variable". Because no
  variable type named `"knock_ports"` exists, it would just throw an
  exception. The result was that the app. would break early.

  The current solution is: if the variable (type) does not exist then we just
  ignore the introducer and print the variable as is in the output message.
  However, parsing a variable is complex and can still generate many errors.
  Although of course you have to have a variable which matches a snaplogger
  variable type (such as "if").

  I think that if the parsing fails at any step we should not throw but just
  ignore that as being a variable. I could also help debugging in case you
  misstype a variable.

* Review the `field` extension

** Type

   Right now we allow for fields of type string (which value is a string).

   The fields should be given a type and be integers, floating points as
   well as strings (maybe some other types too? like a date & bool & size?)

** ${fields}

   The library allows for system fields such as `"_timestamp"`. But these are
   totally missed by the ${fields} at the moment.

** Dynamic Fields

   Whenever a message gets created, we could call a user defined callback
   to generate fields dynamically. For example, you could have a flag that
   you want to report just at the time the log message is created. You could,
   of course, add something such as `... << (flag ? "true" : "false") ...`
   to your `SNAP_LOG_ERROR << ...` statement. But if that flag is generic
   enough, it may be much more practical to handle it through a callback.

** Conditional Fields

   Often, the format includes a field which is set only at times. It would
   be great to display that field only when actually set. There are other
   conditions too; for example when ${tid} == ${pid} then we could avoid
   showing the ${tid} altogether.

   The condition would require an if-like statement. It is often written with
   a tag in an XML environment. We could have something like this:

       ${if:<expr>}...${endif}
       ${if:<expr>}...${else}...${endif}

   The feature would have to support multiple level to make use very useful.
   We need to determine what `<expr>` would need to be. For sure, we want to
   output certain strings only if a given field is not empty. So such a test
   needs to be available.

   We may want to get the `snap_expr` class in a separate project first so
   we can use it for the `<expr>`.


* Allow for a set of default formats to be defined in snaplogger directly.

  Especially, I want to be able to add just `.../${tid}...` when a process
  runs with threads. This allows us to have the thread identifier when a
  log goes out. It's not particularly good to have a such format in all the
  other project that want that field. Although this one could also be
  resolved with the ${if...} like so:

      ...${if:${tid}!=${pid}&&${tid}!=""}/${tid}${endif}...


* Look at making the removal of the banner conditional per appender

  Right now it's all or nothing, yet I think that when running a command
  line you may still want to get the banner in the files (assuming you have
  write permissions).

  I think I can add a tag to prevent the banner dynamically (i.e. if the tag
  is set, then in the `[console]` definition, the message does not get
  printed.


* Document all the fields of the existing appenders.


* Actually implement the snaploggerchecker tool.


* Severities have the `mark_as_registered()` function and the constructor
  which I think should be private and only the `logger_private` object
  would be able to access them. This way it would be much safer. But I
  have to determine whether this is how the severities are expected to
  be limited.


* Look at RFC5424 to see whether we could also support sending logs to
  syslog directly (and support many more features than just a level and
  a message) -- this may be something for the snaplogger/network
  found in eventdispatcher since it requires UDP and such.

  For `syslog` messages, we can use (I think) the
  [`librelp`](https://www.rsyslog.com/librelp-1-1-0/) and setup the
  system to forward messages to our service.

  Then the syslog.d/... setup looks like this:

      *.* @remote-host:3333

  The port number and "remote-host" have to be chosen so things actually work.
  The default UDP port is 514, there is no relp default port in rsyslog. In
  our /etc/services we have the following:

      syslog          514/udp
      syslog-tls      6514/tcp


* Allow a form of subcription to the logs

  This should be a pubsub type of a thing. However, the logger is below the
  eventdispatcher so it has no network abilities at all (to avoid code
  duplication--see the snaplogger network extension in eventdispatcher).
  (The snapcommunicator will have a pubsub, but I'm not so sure I'd want to
  use that because this would be way too much traffic!)

  However, since we can implement any type of appender, we can have one which
  is a simple callback or have a pubsub appender in the logserver project
  which will be used to send all the logs here and there. It may be smart
  to implement that feature only in the `server daemon` since that one is
  remote and thus you do not eat on local resources (i.e. you can then have
  log dedicated machines which allow for subscriptions.)

  The subscription can then be used by a tool which would replace fail2ban.
  (i.e. a form of extension to the snapwatchdog service)


* Added support for sealing (encrypting) the logs

  When dealing with logs on the very computer that created them, we want to
  make sure attackers can't temper with them. Although we offer a network
  extension (see the eventdispatcher project for said extension), most of
  our logs stop on the very computer where they are created.

  The Seal feature would allow for protection of these logs. They at least
  can't be tempered with (they can still be deleted, though). The journald
  system already implements this feature. I have a PDF document about how
  this is achieved.

      doc/secure-logging-seekable-sequential-key-generators-2013-397.pdf

