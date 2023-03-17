#ifndef READ_RC_H
#define READ_RC_H


char* getPromptStart();

void getNameColor();
void getHostNameColor();
void getPathColor();
void getPromptStartColor();
void resetColor();
char* getNameStart();
char* getRcFilePath();
void readSpecificRcFile(char* path);
char* getCommandFromAlias(char* alias);
void addAlias(char* alias, char* command);

#endif
