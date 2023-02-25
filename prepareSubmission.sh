# zip all files in the current dir including the .sheeshrc , except for tests .vscode .gitignore .git *.sh
rm -f assignment.zip
zip -r assignment.zip ./* -x "tests/*" -x ".vscode/*" -x ".gitignore" -x ".git/*" -x "*.sh" -x "*.zip"