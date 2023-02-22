# zip all files in the current dir, except for tests .vscode .gitignore .git *.sh
rm -f assignment.zip
zip -r assignment.zip ./* -x "*.sh" "*.git*" "*.vscode*" "tests/*" "ignore/*" "shell"