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
  (KEY EVENTS IN C!!!!!)
  - Clear the screen
    - Press Ctrl + L and see the magic happen (it will clear the screen).
  - Previous commands
    - Press the up arrow key to see the previous commands.
  - Next commands
    - Press the down arrow key to see the next commands.
    - editing these commands is possible.
  - History
    - history command will show you the history of the commands that you have used.
  - Correct Binary highlighting
    - If the binary is found in the path, it will be highlighted in green. Otherwise, it will be highlighted in red.
  - Configuration files to change the prompt (heavily customizable shell, an example is already included in the repository)
    - name_color
    - name_start
    - host_name_color
    - path_color
    - prompt_start
    - prompt_start_color

- Ideas :
  - make a .themis_history where we can see the history of the commands that we have used.
  - make a .themisrc where we can see the configuration of the shell.
    - support aliases
  - support own syntax of a minimal programing language
  - 