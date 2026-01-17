# gameinput2xinput
This library partially reimplements gameinput using xinput.

# Installation with wine
- Copy gameinput2xinput.dll to the same directory as the game's main executable, then rename it to gameinput.dll.
- add the following launch option to the game's properties: `WINEDLLOVERRIDES="gameinput=n,b" %command%`

# Tested games
- Stalker 2 version 1.0.1: working
- Kingdom Come: Deliverance II
