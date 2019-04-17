This is a tiny wrapper over the `mail` application to allow
sending emails from within RETRO.

~~~
:mail:send (sss-)
  swap 'mail_-s_"%s"_%s s:format file:W unix:popen ;
  swap [ over file:write ] s:for-each unix:pclose ;
~~~

# Send a Message

```
(Body      'This_is_a_test\n\nSending_mail_via_RETRO s:format
(Subject   'Test
(Recipient 'crc@forthworks.com
mail:send
```
