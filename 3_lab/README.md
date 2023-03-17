# OperatingSystems

You can run our shell with the bonus stuff implemented by running the following command:
 
```bash
make
./shell -e
```


# Lab 1:
- Extras :
  - cd command
    - regular cd command
    - Using cd without any arguments will take you to your home directory.
  - prompt
    - The home directory is indicated by a tilde (~) as the first character of the prompt.
    - The current working directory is indicated by the characters after the tilde.
    - The username is indicated by the characters before the @ sign.
    - The hostname is indicated by the characters between the @ sign and the colon.
### (KEY EVENTS IN C!!!!!)
  - Clear the screen
    - Press `Ctrl + L` and see the magic happen (it will clear the screen).
  - Previous commands
    - Press the up arrow key to see the previous commands.
  - Next commands
    - Press the down arrow key to see the next commands.
    - editing these commands is possible.
  - History with `history` command
    - history command will show you the history of the commands that you have used.
  - Correct Binary highlighting
    - If the binary is found in the path, it will be highlighted in green. Otherwise, it will be highlighted in red.
  - Configuration files to change the prompt (heavily customizable shell, an example is already included in the repository. You will have to do
   `mv sheeshrc .sheeshrc` to make it work.)
    - name_color
    - name_start
    - host_name_color
    - path_color
    - prompt_start
    - prompt_start_color

# New Bonuses for lab 3:



First of all run :
```bash
./shell -h
```
to see the help menu

Append to a file with the >> operator
```bash
./shell
echo a > file.txt
cat file.txt
a
echo hello >> file.txt
cat file.txt
a
hello
```

glob pattern matching
```bash
ls *.c
cat *.h
idfk whocares*
```

read code from file (with the -f flag)
```bash
./shell -f shellScript.sh
```

or without the -f or --file flag
```bash
./shell <filename>
```

read code from string (just like in bash) with the -c/--code flag
```bash
./shell -c "status; echo a; echo b; exit 33"
```

Custom exit codes
```bash
./shell
exit 69
```
Will exit with code 69

Custom rc and live updates of rc files using the source command
```bash
./shell
source <filename>
```

This means that when you have made any changes to the rc file, you can just run the source command and the changes will be applied to the shell.
If you have sourced a file in the current shell session, then that file is going to be sourced.

Redirection of stdout and stderr
```bash
./shell
make 2> error.txt # will redirect stderr to error.txt
make 1> output.txt # will redirect stdout to output.txt
```

Added the ability to have comments in the shell, which gives us the ability to have comments inside your shell scripts
```bash
./shell
# this is a comment
echo hello # this is also a comment
```

Added the ability to have multiline comments in the shell, which gives us the ability to have multiline comments inside your shell scripts
```bash
./shell
'''
this is a multiline comment
ls
pwd
'''
echo hello
```

Aliases support with the alias command inside the RC file
```bash
cat .sheeshrc
alias ls="ls -l"
ls
total 444
-rw-r--r-- 1 mohammad mohammad     12 Mar 17 16:19 1.in
-rwxr-xr-x 1 mohammad mohammad  33426 Mar 17 16:19 a.out
-rw-r--r-- 1 mohammad mohammad  20814 Mar 15 15:37 assignment.zip
drwxr-xr-x 1 mohammad mohammad    156 Mar 17 19:11 build
-rw-r--r-- 1 mohammad mohammad    643 Mar 13 14:20 common.h
-rw-r--r-- 1 mohammad mohammad      3 Mar 17 16:39 idk
-rw-r--r-- 1 mohammad mohammad  59384 Mar 17 19:11 lexer.c
-rw-r--r-- 1 mohammad mohammad    193 Mar 13 14:20 lexer.h
drwxr-xr-x 1 mohammad mohammad    166 Mar 17 16:19 libs
-rw-r--r-- 1 mohammad mohammad   1047 Mar 13 14:20 Makefile
-rw-r--r-- 1 mohammad mohammad     12 Mar 17 16:19 new
-rw-r--r-- 1 mohammad mohammad  58608 Mar 17 19:11 parser.c
-rw-r--r-- 1 mohammad mohammad   3335 Mar 17 19:11 parser.tab.h
-rw-r--r-- 1 mohammad mohammad  13524 Mar 17 18:30 parser.y
-rw-r--r-- 1 mohammad mohammad    244 Mar 14 14:29 prepareSubmission.sh
-rw-r--r-- 1 mohammad mohammad   3180 Mar 17 19:13 README.md
-rwxr-xr-x 1 mohammad mohammad 186696 Mar 17 19:11 shell
-rw-r--r-- 1 mohammad mohammad   8721 Mar 17 19:11 shell.l
-rw-r--r-- 1 mohammad mohammad     45 Mar 17 16:26 shellScript.sh
-rw-r--r-- 1 mohammad mohammad    969 Mar 17 16:19 stderr
drwxr-xr-x 1 mohammad mohammad      0 Mar 14 18:18 tests
-rw-r--r-- 1 mohammad mohammad    412 Mar 13 14:20 types.h
```

Alias support with the alias command inside the shell
```bash
./shell
alias lm ls -l
lm
total 444
-rw-r--r-- 1 mohammad mohammad     12 Mar 17 16:19 1.in
-rwxr-xr-x 1 mohammad mohammad  33426 Mar 17 16:19 a.out
-rw-r--r-- 1 mohammad mohammad  20814 Mar 15 15:37 assignment.zip
drwxr-xr-x 1 mohammad mohammad    156 Mar 17 19:11 build
-rw-r--r-- 1 mohammad mohammad    643 Mar 13 14:20 common.h
-rw-r--r-- 1 mohammad mohammad      3 Mar 17 16:39 idk
-rw-r--r-- 1 mohammad mohammad  59384 Mar 17 19:11 lexer.c
-rw-r--r-- 1 mohammad mohammad    193 Mar 13 14:20 lexer.h
drwxr-xr-x 1 mohammad mohammad    166 Mar 17 16:19 libs
-rw-r--r-- 1 mohammad mohammad   1047 Mar 13 14:20 Makefile
-rw-r--r-- 1 mohammad mohammad     12 Mar 17 16:19 new
-rw-r--r-- 1 mohammad mohammad  58608 Mar 17 19:11 parser.c
-rw-r--r-- 1 mohammad mohammad   3335 Mar 17 19:11 parser.tab.h
-rw-r--r-- 1 mohammad mohammad  13524 Mar 17 18:30 parser.y
-rw-r--r-- 1 mohammad mohammad    244 Mar 14 14:29 prepareSubmission.sh
-rw-r--r-- 1 mohammad mohammad   3180 Mar 17 19:13 README.md
-rwxr-xr-x 1 mohammad mohammad 186696 Mar 17 19:11 shell
-rw-r--r-- 1 mohammad mohammad   8721 Mar 17 19:11 shell.l
-rw-r--r-- 1 mohammad mohammad     45 Mar 17 16:26 shellScript.sh
-rw-r--r-- 1 mohammad mohammad    969 Mar 17 16:19 stderr
drwxr-xr-x 1 mohammad mohammad      0 Mar 14 18:18 tests
-rw-r--r-- 1 mohammad mohammad    412 Mar 13 14:20 types.h
```

Added the ability to have a history of the commands that you have used in the shell inside the .sheesh_history file
```bash
./shell
echo hello
hello
cat .sheesh_history
echo hello
```


- Ideas :
  - support own syntax of a minimal programming language
  - show similar commands (if the command is not found)
  - CTRL + R to search for a command
  - show a dimmed version of the command that is being typed (from the history)