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

#endif
