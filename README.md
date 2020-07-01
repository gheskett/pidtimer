# pidtimer

*Written by Gregory Heskett (gheskett)*

This is meant to be a command line application with the focus of terminating any number of processes after a certain length of time.
An example of a useful application for this would include a music player that plays music or sounds overnight but needs to be terminated by morning.

This application can also be used to open/execute files with arguments after the set amount of time.
An example of this could be to play an obnoxious audio file as an alarm.


Usage: `pidtimer.exe <[days]:[hours]:[minutes]:[seconds].[milliseconds]> [OPTIONAL ARGS] ...`

OPTIONAL ARGUMENTS:
 - `-k`, `--kill   <PID>`
 - `-o`, `--open   <file path + cmd args>`  (Use escape characters for quotes!)

USAGE EXAMPLES:
 - `pidtimer.exe 3:07:42:13.962 --kill 3479`
 - `pidtimer.exe 420:69 -k 42069 -o "\"funky music.mp3\"" -k 19573`
 - `pidtimer.exe 1:30:00 -o "send_message.exe \"This is a message string!\""`
