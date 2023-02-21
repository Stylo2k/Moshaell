#ifndef OPTIONS_H
#define OPTIONS_H

#define DONT_CLEAN_PREV 0
#define CLEAN_PREV 1


#define DONT_INCLUDE_BIN_PATH 0
#define INCLUDE_BIN_PATH 1

void cleanUp();
void newOptions(bool cleanPrevious);
void addOption(char* option);
bool noOptions(bool includeBinPath);
void addBinPathToOptions(char* bin);
char* getBinPath();
int getNumberOfOptions();
char* getArgAt(int index);

#endif