#include "../common.h"


static Pipeline *pipeline = NULL;


Command *newCommand(char* name, char** args, int argCount) {
    Command *cmd = malloc(sizeof(Command));
    cmd->name = strdup(name);
    cmd->path = findBinary(name);
    // copy the args
    cmd->args = malloc(sizeof(char*) * (argCount + 2));
    cmd->args[0] = strdup(name);
    free(name);
    int i = 1;
    while (i < argCount) {
        cmd->args[i] = strdup(args[i]);
        i++;
    }
    cmd->args[i] = NULL;
    cmd->argc = argCount;
    cmd->in = STDIN_FILENO;
    cmd->out = STDOUT_FILENO;
    cmd->isBuiltIn = false;
    cmd->err = STDERR_FILENO;
    cmd->next = NULL;
    return cmd;
}

void assertPipeline() {
    if (!pipeline) {
        pipeline = malloc(sizeof(Pipeline));
        pipeline->commands = NULL;
        pipeline->command_count = 0;
    }
}

void addCommandToPipeline(Command *cmd) {
    assertPipeline();
    // add the command to the pipeline which is a linked list
    if (pipeline->commands == NULL) {
        pipeline->commands = cmd;
    } else {
        Command *current = pipeline->commands;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = cmd;
    }
    pipeline->command_count++;
}

void addCommandToPipelineWithArgs(char* name, char** args, int argCount) {
    Command *cmd = newCommand(name, args, argCount);
    addCommandToPipeline(cmd);
}


Command* getCommand(int index) {
    assertPipeline();
    if (index > pipeline->command_count) {
        return NULL;
    }
    Command *current = pipeline->commands;
    int i = 0;
    while (i < index) {
        current = current->next;
        i++;
    }
    return current;
}


void resetPipeline() {
    if (pipeline) {
        Command *current = pipeline->commands;
        while (current != NULL) {
            Command *next = current->next;
            free(current->name);
            free(current->path);
            free(current->args[0]);
            int i = 1;
            while (i < current->argc) {
                free(current->args[i]);
                i++;
            }
            free(current->args);
            free(current);
            current = next;
        }
        free(pipeline);
        pipeline = NULL;
    }
}

Command* getFirstCommand() {
    assertPipeline();
    return pipeline->commands;
}

Command* getLastCommand() {
    assertPipeline();
    Command *current = pipeline->commands;
    while (current->next != NULL) {
        current = current->next;
    }
    return current;
}


Command* getCommandAt(int index) {
    assertPipeline();
    if (index > pipeline->command_count) {
        return NULL;
    }
    Command *current = pipeline->commands;
    int i = 0;
    while (i < index) {
        current = current->next;
        i++;
    }
    return current;
}


void configureInput(Command *cmd, int fd) {
    if (!cmd) {
        return;
    }
    if (cmd->in != STDIN_FILENO) {
        close(cmd->in);
    }
    cmd->in = fd;
}

void configureOutput(Command *cmd, int fd) {
    if(!cmd) {
        return;
    }
    if (cmd->out != STDOUT_FILENO) {
        close(cmd->out);
    }
    cmd->out = fd;
}

void addBuiltInToPipelineWithArgs(char* name, char** args, int argCount) {
    Command *cmd = newCommand(name, args, argCount);
    cmd->isBuiltIn = true;
    addCommandToPipeline(cmd);
}

int getCommandCount() {
    assertPipeline();
    return pipeline->command_count;
}
