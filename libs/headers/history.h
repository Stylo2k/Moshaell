#ifndef HISTORY_H
#define HISTORY_H

void newHistory(bool cleanPrevious);
void addToHistory(char* command);
int getNumberOfHistoryCommands();
bool anyHistory();
char* getMostRecent();
char** getHistoryAt(int index);
char** getArgsOfMostRecent();
char** getArgsOfHistoryAt(int index);

#endif