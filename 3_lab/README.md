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

For this labs bonuses you dont have to run the shell in experimental mode.

First of all run :
```bash
./shell -h
```
to see the help menu

glob pattern matching
```bash
ls *.c
cat *.h
idfk whocares*
```

read code from file (with the -f flag)
```bash
./shell -f <filename>
```

read code from string (just like in bash) with the -c flag
```bash
./shell -c <string>
```

Custom exit codes
```bash
./shell
echo a
echo b
exit 69
```
Will exit with code 69

Custom rc and live updates of rc files using the source command
```bash
./shell
source <filename>
```

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
echo hello # this is also a comment (not yet supported)
```


- Ideas :
  - make a .sheesh_history where we can see the history of the commands that we have used. 
  - support aliases
  - support own syntax of a minimal programming language
  - show similar commands (if the command is not found)
  - CTRL + R to search for a command
  - show a dimmed version of the command that is being typed (from the history)