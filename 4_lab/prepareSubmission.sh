# zip all files in the current dir including the .sheeshrc , except for tests .vscode .gitignore .git *.sh *.in
rm -f assignment.zip
rm shell idk
zip -r assignment.zip ./* -x "tests/*" -x ".vscode/*" -x ".gitignore" -x ".git/*" -x "*.sh" -x "*.zip" -x "*.in" -x "shell" -x "idk"
