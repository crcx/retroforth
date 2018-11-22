# RETRO

This is a set of extensions for RRE.

# Console Input

~~~
:c:get (-c) as{ 'liii.... i #1 d }as ;
~~~

---------------------------------------------------------------

---------------------------------------------------------------

# Scripting: Command Line Arguments

~~~
:sys:argc (-n) `-6100 ;
:sys:argv (n-s) s:empty swap `-6101 ;
~~~

# System Interaction

The `unix:` namespace contains words for interacting with the
host operating system.

`unix:system` runs another application using the system shell
and returns after execution is completed.

~~~
:unix:system (s-)    #-8000 `-6300 ;
~~~

`unix:fork` forks the current process and returns a process
identifier.

~~~
:unix:fork   (-n)    #-8001 `-6300 ;
~~~

`unix:exit` takes a return code and exits RRE, returning the
specified code.

~~~
:unix:exit   (n-)    #-8002 `-6300 ;
~~~

`unix:getpid` returns the current process identifier.

~~~
:unix:getpid (-n)    #-8003 `-6300 ;
~~~

This group is used to execute a new process in place of the
current one. These take a program and optionally 1-3 arguments.
They map to the execl() system call.

Example:

    '/usr/bin/cal '2 '2019 unix:exec2

~~~
:unix:exec0  (s-)    #-8004 `-6300 ;
:unix:exec1  (ss-)   #-8005 `-6300 ;
:unix:exec2  (sss-)  #-8006 `-6300 ;
:unix:exec3  (ssss-) #-8007 `-6300 ;
~~~

`unix:wait` waits for a child process to complete. This maps to
the wait() system call.

~~~
:unix:wait   (-n)    #-8008 `-6300 ;
~~~

`unix:kill` terminates a process. Takes a process and a signal
to send.

~~~
:unix:kill (nn-)  #-8009 `-6300 ;
~~~

The next two words allow opening and closing pipes. The first,
`unix:popen` takes the name of a program and a file mode and
returns a file handle usable with words in the `file:` namespace.
The second, `unix:pclose` closes the pipe.

~~~
:unix:popen (sn-n) #-8010 `-6300 ;
:unix:pclose (n-) #-8011 `-6300 ;
~~~

~~~
:unix:write (sh-) [ dup s:length ] dip #-8012 `-6300 ;
~~~

`unix:chdir` changes the current working directory to the
specified one.

~~~
:unix:chdir (s-) #-8013 `-6300 ;
~~~

~~~
:unix:getenv (sa-) #-8014 `-6300 ;
:unix:putenv (s-)  #-8015 `-6300 ;
~~~

`unix:sleep` pauses execution for the specified number of
seconds.

~~~
:unix:sleep (n-) #-8016 `-6300 ;
~~~

---------------------------------------------------------------

~~~
:unix:io:n:put (n-) #-8100 `-6300 ;
:unix:io:s:put (s-) #-8101 `-6300 ;
~~~

# Interactive Listener

~~~
'NoEcho var

{{
  :version (-)    @Version #100 /mod n:put $. c:put n:put ;
  :eol?    (c-f)  [ ASCII:CR eq? ] [ ASCII:LF eq? ] [ ASCII:SPACE eq? ] tri or or ;
  :valid?  (s-sf) dup s:length n:-zero? ;
  :ok      (-)    @NoEcho not 0; drop compiling? [ nl 'Ok_ s:put ] -if ;
  :check-eof (c-c) dup [ #-1 eq? ] [ #4 eq? ] bi or [ 'bye d:lookup d:xt fetch call ] if ;
  :check-bs  (c-c) dup [ #8 eq? ] [ #127 eq? ] bi or [ buffer:get buffer:get drop-pair ] if ;
  :s:get      (-s) [ #1025 buffer:set
                     [ c:get dup buffer:add check-eof check-bs eol? ] until
                      buffer:start s:chop ] buffer:preserve ;
---reveal---
  :banner  (-)    @NoEcho not 0; drop
                  'RETRO_12_(rx- s:put version $) c:put nl
                  EOM n:put '_MAX,_TIB_@_1025,_Heap_@_ s:put here n:put nl ;
  :bye     (-)    #0 unix:exit ;
  :listen  (-)
    ok repeat s:get valid? [ interpret ok ] [ drop ] choose again ;
}}
~~~

~~~
:include (s-) `-9999 ;
~~~

~~~
{{
  :gather (c-)
    dup [ #8 eq? ] [ #127 eq? ] bi or [ drop ] [ buffer:add ] choose ;
  :cycle (q-qc)  repeat c:get dup-pair swap call not 0; drop gather again ;
---reveal---
  :parse-until (q-s)
    [ s:empty buffer:set cycle drop-pair buffer:start ] buffer:preserve ;
}}

:s:get (-s) [ [ ASCII:LF eq? ] [ ASCII:CR eq? ] bi or ] parse-until ;
~~~

