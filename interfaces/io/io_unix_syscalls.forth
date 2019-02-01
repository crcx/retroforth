# System Interaction

The `unix:` namespace contains words for interacting with the
host operating system.

~~~
{{
  'io:UnixSyscall var
  :identify
    @io:UnixSyscall n:zero? [
      #8 io:scan-for dup n:negative?
      [ drop 'IO_DEVICE_TYPE_0008_NOT_FOUND s:put nl ]
      [ !io:UnixSyscall ] choose ] if ;
  ---reveal---
  :io:unix-syscall identify @io:UnixSyscall io:invoke ;
}}
~~~

`unix:system` runs another application using the system shell
and returns after execution is completed.

~~~
:unix:system (s-)    #0 io:unix-syscall ;
~~~

`unix:fork` forks the current process and returns a process
identifier.

~~~
:unix:fork   (-n)    #1 io:unix-syscall ;
~~~

This group is used to execute a new process in place of the
current one. These take a program and optionally 1-3 arguments.
They map to the execl() system call.

Example:

    '/usr/bin/cal '2 '2019 unix:exec2

~~~
:unix:exec0  (s-)    #2 io:unix-syscall ;
:unix:exec1  (ss-)   #3 io:unix-syscall ;
:unix:exec2  (sss-)  #4 io:unix-syscall ;
:unix:exec3  (ssss-) #5 io:unix-syscall ;
~~~

`unix:exit` takes a return code and exits RRE, returning the
specified code.

~~~
:unix:exit   (n-)    #6 io:unix-syscall ;
~~~

`unix:getpid` returns the current process identifier.

~~~
:unix:getpid (-n)    #7 io:unix-syscall ;
~~~

`unix:wait` waits for a child process to complete. This maps to
the wait() system call.

~~~
:unix:wait   (-n)    #8 io:unix-syscall ;
~~~

`unix:kill` terminates a process. Takes a process and a signal
to send.

~~~
:unix:kill (nn-)  #9 io:unix-syscall ;
~~~

The next two words allow opening and closing pipes. The first,
`unix:popen` takes the name of a program and a file mode and
returns a file handle usable with words in the `file:` namespace.
The second, `unix:pclose` closes the pipe.

~~~
:unix:popen (sn-n) #10 io:unix-syscall ;
:unix:pclose (n-) #11 io:unix-syscall ;
~~~

~~~
:unix:write (sh-) [ dup s:length ] dip #12 io:unix-syscall ;
~~~

`unix:chdir` changes the current working directory to the
specified one.

~~~
:unix:chdir (s-) #13 io:unix-syscall ;
~~~

~~~
:unix:getenv (sa-) #14 io:unix-syscall ;
:unix:putenv (s-)  #15 io:unix-syscall ;
~~~

`unix:sleep` pauses execution for the specified number of
seconds.

~~~
:unix:sleep (n-) #16 io:unix-syscall ;
~~~

~~~
:unix:io:n:put (n-) #17 io:unix-syscall ;
:unix:io:s:put (s-) #18 io:unix-syscall ;
~~~

~~~
:unix:get-cwd (-s)
  'pwd file:R unix:popen
    dup file:read-line s:trim swap
  unix:pclose
  #0 sys:argv s:length + '/ s:append ;
~~~

~~~
:unix:count-files-in-cwd (-n)
  'ls_-1_|_wc_-l  file:R unix:popen
    dup file:read-line s:trim s:to-number swap
  unix:pclose ;
~~~

~~~
:unix:for-each-file (q-)
  'ls_-1_-p file:R unix:popen
  unix:count-files-in-cwd
  [ [ file:read-line s:temp over call ] sip ] times
  unix:pclose drop ;
~~~


