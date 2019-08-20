This is based on Ron Aaron's "Magic 8th Ball" - CLI version.
See https://8th-dev.com/forum/index.php/topic,1864.msg10733.html

First is the list of responses. This is just an array.

~~~
{ 'OK
  'Yes
  'Absolutely!
  'Surely
  'Perhaps
  'Could_be
  'Hard_to_say
  'Maybe
  'Unclear
  'Ask_later
  'Down_for_maintenance
  'ABEND_12345
  'No
  'Definitely_not!
  'Leave_me_alone
} 'PROPHESIES const
~~~

To get a random prophecy: get the length of the array, a random
number, and calculate an index based on these to fetch.

~~~
:prophesy (-s)
  PROPHESIES dup a:length n:random n:abs swap mod a:fetch ;
~~~

This finishes the core of the application. The remaining part is
the user interface. RETRO doesn't have GUI bindings, so I'm only
implementing the CLI interface.

My approach is a little different from the original. I split the
input "processing" into a separate word. RETRO doesn't have a
null string, so I left out the check for that. Exit with CTRL+C.

~~~
:process-input (s-)
  s:empty [ 'C'mon,_don't_be_like_that!_Ask_a_question: s:put nl ] s:case
  drop prophesy '\nThe_8th_ball_says:\n\t%s\n\n s:format s:put
                '\nAsk_again_and_you_shall_be_answered:\n s:format s:put ;

:8th-ball-cli (-)
  'Ask_your_question_of_the_8th-ball.__Satisfaction_guaranteed! s:put nl
  repeat
    s:get process-input
  again ;
~~~
