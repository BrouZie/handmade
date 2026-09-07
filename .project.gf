[executable]
path=build/handmadehero
arguments=
ask_directory=0

[vim]
enabled=0

[commands]
Compile=shell make compile
Rebuild and run=shell make compile;file build/handmadehero;b main;run&

[shortcuts]
Ctrl+R=gf-command Rebuild and run
