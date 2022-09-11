# MavShell
# Project Overview
This shell works similiar to bourne shell (bash), c-shell (csh) or korn shell (ksh). This program (msh.c) accepts commands, forks a child process and execute those commands. It works like csh or bash, which will run and accept commands until the user exits the shell.

# How to Run
Commands to run:
./msh.c or gcc msh.c
a.out



## Commands List to Try
| Command | Description |
| ----------- | ----------- |
| ls | list the files or folders inside the directory. |
| cd | Changes the directory to the provided filename. |
| pidhistory | prints the last 15 commands spawned by the user. |
| mkdir | creates a new directory. |
| history | command prints the last 15 user inputs. |
| !n | where n is the number between 0 and 14 which will re-run the nth command. |
| ls | Lists the directory contents. Supports listing "." and ".."|
| Error | Prints command not in history or command not found. |
| exit | Terminates the program. |
