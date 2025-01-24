#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <Arduino.h>
#include <strings.h>

#define MAX_COMMANDS 10
#define MAX_COMMAND_LENGTH 20
#define MAX_ARGS 5
#define MAX_DESCRIPTION_LENGTH 64

class CommandProcessor {
public:
    enum ArgType {
        TypeString,
        TypeInt,
        TypeFloat
    };

    typedef String (*CommandFunction)(int argc, char* argv[]);

    struct Command {
        char name[MAX_COMMAND_LENGTH];
        CommandFunction function;
        char description[MAX_DESCRIPTION_LENGTH];
        ArgType argTypes[MAX_ARGS];
        int argCount;
    };

    CommandProcessor() : lastStatus("No commands processed yet"), lastResult(""), debugMode(false), lastStatusIsError(false) {}

    void setDebugMode(bool debug) {
        debugMode = debug;
    }

    void registerCommand(const char* name, CommandFunction func, const char* description, int argCount, ...) {
        if (commandCount < MAX_COMMANDS) {
            Command& cmd = commands[commandCount++];
            strncpy(cmd.name, name, MAX_COMMAND_LENGTH);
            cmd.function = func;
            strncpy(cmd.description, description, MAX_DESCRIPTION_LENGTH);
            cmd.argCount = argCount;

            va_list args;
            va_start(args, argCount);
            for (int i = 0; i < argCount && i < MAX_ARGS; i++) {
                cmd.argTypes[i] = static_cast<ArgType>(va_arg(args, int));
            }
            va_end(args);
        } else {
            setStatus("Error: Maximum number of commands reached", true);
        }
    }

    bool processCommand(const char* input) {
        char command[MAX_COMMAND_LENGTH];
        char* args[MAX_ARGS];
        int argc = 0;
        setStatus("OK", true);

        char* token = strtok((char*)input, " ");
        if (token != NULL) {
            strncpy(command, token, MAX_COMMAND_LENGTH);
            command[MAX_COMMAND_LENGTH - 1] = '\0';  // Ensure null-termination
            while ((token = strtok(NULL, " ")) != NULL && argc < MAX_ARGS) {
                args[argc++] = token;
            }
        }

        // Check for help command
        if (strcasecmp(command, "help") == 0 || strcasecmp(command, "?") == 0) {
            listAllCommands();
            return true;
        }

        for (int i = 0; i < commandCount; i++) {
            if (strcasecmp(commands[i].name, command) == 0) {
                if (validateArgs(argc, args, commands[i].argCount, commands[i].argTypes)) {
                    lastResult = commands[i].function(argc, args);
                    String statusMsg = "Executed command: " + String(command);
                    for (int j = 0; j < argc; j++) {
                        statusMsg += " " + String(args[j]);
                    }
                    setStatus(statusMsg);
                    return true;
                }
                return false;
            }
        }

        setStatus("Error: Unknown command '" + String(command) + "'", true);
        return false;
    }

    void listAllCommands() {
        String commandList = "Available commands:\n";
        for (int i = 0; i < commandCount; i++) {
            commandList += String(commands[i].name) + ": " + String(commands[i].description) + "\n";
        }
        commandList += "\n";
        lastResult = commandList;
        setStatus(commandList);
    }

    void loop() {
        if (Serial.available()) {
            String input = Serial.readStringUntil('\n');
            if (processCommand(input.c_str())) {
              Serial.print(lastResult);
            } else {
              Serial.print(lastStatus);
            }
        }
    }

    String getLastStatus() const {
        return lastStatus;
    }

    String getLastResult() const {
        return lastResult;
    }

    bool isLastStatusError() const {
        return lastStatusIsError;
    }

private:
    Command commands[MAX_COMMANDS];
    int commandCount = 0;
    String lastStatus;
    String lastResult;
    bool debugMode;
    bool lastStatusIsError;

    void setStatus(const String& status, bool isError = false) {
        lastStatus = status;
        lastStatusIsError = isError;
        if (debugMode) {
            Serial.println(status);
        }
    }

    bool validateArgs(int argc, char* argv[], int expectedCount, ArgType* expectedTypes) {
        if (argc != expectedCount) {
            String errorMsg = "Error: Expected " + String(expectedCount) + " arguments, but got " + String(argc);
            errorMsg += ". Expected types: ";
            for (int i = 0; i < expectedCount; i++) {
                errorMsg += getTypeName(expectedTypes[i]) + " ";
            }
            errorMsg += ". Received: ";
            for (int i = 0; i < argc; i++) {
                errorMsg += String(argv[i]) + " ";
            }
            setStatus(errorMsg, true);
            return false;
        }

        for (int i = 0; i < expectedCount; i++) {
            if (!validateArg(argv[i], expectedTypes[i])) {
                String errorMsg = "Error: Argument " + String(i + 1) + " has incorrect type. Expected " + 
                                  getTypeName(expectedTypes[i]) + ", got '" + String(argv[i]) + "'";
                setStatus(errorMsg, true);
                return false;
            }
        }

        return true;
    }

    static bool validateArg(const char* arg, ArgType type) {
        switch (type) {
            case TypeString:
                return true;
            case TypeInt:
                return isInteger(arg);
            case TypeFloat:
                return isFloat(arg);
            default:
                return false;
        }
    }

    static bool isInteger(const char* str) {
        if (*str == '-') str++;  // Skip negative sign if present
        if (*str == '\0') return false;  // Empty string is not an integer
        while (*str) {
            if (*str < '0' || *str > '9') return false;
            str++;
        }
        return true;
    }

    static bool isFloat(const char* str) {
        bool hasDecimal = false;
        if (*str == '-') str++;  // Skip negative sign if present
        if (*str == '\0') return false;  // Empty string is not a float

        while (*str) {
            if (*str == '.') {
                if (hasDecimal) return false;  // More than one decimal point
                hasDecimal = true;
            } else if (*str < '0' || *str > '9') {
                return false;  // Non-digit character (except '.')
            }
            str++;
        }
        return true;
    }

    static String getTypeName(ArgType type) {
        switch (type) {
            case TypeString: return "String";
            case TypeInt: return "Integer";
            case TypeFloat: return "Float";
            default: return "Unknown";
        }
    }
};

#endif // COMMAND_PROCESSOR_H