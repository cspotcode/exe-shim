#include "Windows.h"
#include <shellapi.h>
#include <stdio.h>
#include <assert.h>
// #include <iostream>
// #include <stdlib.h>
#pragma comment(lib, "shell32.lib")

#define debug1(x) /* noop */
#define debug2(x, y) /* noop */

const boolean false = 0;
const boolean true = 1;
typedef LPSTR string;

struct ParseStringResult {
    string str;
    string next;
};

/** Allocate a new string of the given length */
string newString(int length) {
    return malloc(sizeof(char) * (length + 1));
}

struct ParseStringResult parseArgument(string commandLine) {
    int commandLineLength = strlen(commandLine);
    boolean inQuote = false;
    boolean prevWasBackslash = false;
    string acc = malloc(sizeof(char) * (commandLineLength + 1));
    int accI = 0;
    char c = '\0';
    int i;
    for(i = 0; i < commandLineLength; i++) {
        c = commandLine[i];
        // Terminate at a space
        if(!inQuote && c == ' ') {
            break;
        }
        if(c == '"') {
            if(prevWasBackslash) {
                acc[accI++] = c;
                prevWasBackslash = false;
            } else {
                inQuote = !inQuote;
            }
            continue;
        }
        if(c == '\\') {
            if(prevWasBackslash) {
                acc[accI++] = c;
                prevWasBackslash = false;
                continue;
            } else {
                prevWasBackslash = true;
                continue;
            }
        }
        if(prevWasBackslash) {
            acc[accI++] = '\\';
        }
        prevWasBackslash = false;
        acc[accI++] = c;
    }
    if(prevWasBackslash == true) {
        acc[accI++] = c;
    }
    acc[accI] = 0;
    struct ParseStringResult ret;
    ret.str = acc;
    ret.next = commandLine + i;
    return ret;
}

/**
 * return pointer to the first occurrence of a character in a string, or a pointer to the end
 * if it's not found.
 */
string firstOccurrenceOf(string input, char find) {
    string s = input;
    for(; ; s++) {
        if(*s == 0) return s;
        if(*s == find) return s;
    }
}

/**
 * return pointer to the last occurrence of a character in a string, or a pointer to the end
 * if it's not found
 */
string lastOccurrenceOf(string input, char find) {
    string s = input;
    string found = 0;
    for(; ; s++) {
        if(*s == 0) break;
        if(*s == find) found = s;
    }
    if(found == 0) return s;
    return found;
}

struct ParseStringResult readTill(string s, char till) {
    string end = firstOccurrenceOf(s, till);
    string retStr = newString(end - s);
    strncpy(retStr, s, end - s);
    retStr[end - s] = 0;
    struct ParseStringResult ret;
    ret.str = retStr;
    ret.next = end;
    return ret;
}

string scanPast(string start, string skipChars) {
    int numChars = strlen(skipChars);
    string cursor;
    for(cursor = start; ; cursor++) {
        char c = *cursor;
        if(c == 0) return cursor;
        boolean foundSkippableChar = false;
        for(int ci = 0; ci < numChars; ci++) {
            if(c == skipChars[ci]) {
                foundSkippableChar = true;
                break;
            }
        }
        if(!foundSkippableChar) {
            return cursor;
        }
    }
}


/**
 * Read an entire file into a string
 */
string readFile(string path) {
    string buffer = 0;
    long length;
    FILE *f = fopen(path, "rb");
    if(f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length + sizeof(char));
        if(buffer) {
            fread(buffer, 1, length, f);
            buffer[length] = 0;
        }
        fclose(f);
    }
    return buffer;
}

/** malloc and return a new string that is concatenation of two input strings */
string strconcat3(string a, string b, string c) {
    int aLength = strlen(a);
    int bLength = strlen(b);
    int cLength = strlen(c);
    string ret = malloc(sizeof(char) * (aLength + bLength + cLength + 1));
    strcpy(ret, a);
    strcpy(ret + aLength, b);
    strcpy(ret + aLength + bLength, c);
    return ret;
}

string strconcat2(string a, string b) {
    return strconcat3(a, b, "");
}

string encodeArgument(string arg) {
    boolean willBeWrapped = false;
    for(string s = arg; *s != 0; s++) {
        char c = *s;
        if(c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '"') {
            willBeWrapped = true;
            break;
        }
    }
    // Long enough to escape every character, plus wrapping quotes
    string output = newString(strlen(arg) * 2 + 2);
    string outputCursor = output;
    if(willBeWrapped) {
        *(outputCursor++) = '"';
    }
    int backslashCount = 0;
    // Escape double-quotes and preceding backslashes
    // If necessary, escape trailing backslashes
    for(string argCursor = arg; ; argCursor++) {
        char c = *argCursor;
        debug2("%c", c);
        if(c == '\\') {
            backslashCount++;
        } else if(c == '"') {
            // escape all backslashes preceding doublequote
            for(; backslashCount; backslashCount--) {
                *(outputCursor++) = '\\';
                *(outputCursor++) = '\\';
            }
            *(outputCursor++) = '\\';
            *(outputCursor++) = '"';
        } else if(c == 0) {
            if(willBeWrapped) {
                // escape trailing backslashes
                for(; backslashCount; backslashCount--) {
                    *(outputCursor++) = '\\';
                    *(outputCursor++) = '\\';
                }
                *(outputCursor++) = '"';
            } else {
                for(; backslashCount; backslashCount--) {
                    *(outputCursor++) = '\\';
                }
            }
            *outputCursor = 0;
            break;
        } else {
            for(; backslashCount; backslashCount--) {
                *(outputCursor++) = '\\';
            }
            *(outputCursor++) = c;
        }
    }
    return output;
}

int main(int argc, char* argv[]) {
    string lpCmdLine = GetCommandLine();

// int CALLBACK WinMain(
//   _In_ HINSTANCE hInstance,
//   _In_ HINSTANCE hPrevInstance,
//   _In_ LPSTR     lpCmdLine,
//   _In_ int       nCmdShow
// ) {
    // build absolute path to this shim.exe
    struct ParseStringResult parsedBinPath = parseArgument(lpCmdLine);

    string arguments = parsedBinPath.next;

    debug2("This binary: %s\n", parsedBinPath.str);
    debug2("Extra args: %s\n", arguments);

    // Build path to config file (absolute path to this shim, with extension replaced by .config)
    string configDirectory = newString(strlen(parsedBinPath.str) + 7);
    strcpy(configDirectory, parsedBinPath.str);
    string lastDot = lastOccurrenceOf(configDirectory, '.');
    assert(*lastDot != 0);
    strcpy(lastDot, ".config");

    debug2("Path to config file: %s\n", configDirectory);

    string target = NULL;
    string argumentsPrefix = NULL;

    string configFile = readFile(configDirectory);
    debug2("config file contents: [[[\n%s\n]]]\n", configFile);
    string readIndex = configFile;
    // Read null-terminated strings from config file
    while(true) {
        readIndex = scanPast(readIndex, "\n");
        struct ParseStringResult parseKey = readTill(readIndex, '=');
        readIndex = scanPast(parseKey.next, "=");
        struct ParseStringResult parseValue = readTill(readIndex, '\n');
        readIndex = scanPast(parseValue.next, "\n");
        if(strcmp(parseKey.str, "target") == 0) {
            target = parseValue.str;
        }
        if(strcmp(parseKey.str, "argumentsPrefix") == 0) {
            argumentsPrefix = parseValue.str;
        }
        if(parseKey.str[0] == 0) {
            break;
        }
    }

    debug2("target: %s\n", target);
    debug2("argumentsPrefix: %s\n", argumentsPrefix);

    // Build path to interpreter in local directory
    // Check if it exists

    // Remove all occurrences of .JS; from PATHEXT

    // For each directory in PATH
        // If interpreter name includes extension, for ""
        // For each extension in PATHEXT
            // build full path
            // check if interpreter exists

    // Concat config file args and runtime args
    string allArgs = strconcat3(argumentsPrefix, " ", arguments);
    string fullCommandLine = strconcat3(encodeArgument(target), " ", allArgs);

    debug2("fullCommandLine: %s\n", fullCommandLine);

    // Using found absolute path of interpreter:
    // SHELLEXECUTEINFO ShExecInfo = {0};
    // ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    // ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    // ShExecInfo.hwnd = NULL;
    // ShExecInfo.lpVerb = NULL;
    // ShExecInfo.lpFile = target;
    // ShExecInfo.lpParameters = allArgs;
    // ShExecInfo.lpDirectory = NULL;
    // ShExecInfo.nShow = SW_SHOW;
    // ShExecInfo.hInstApp = NULL;
    // ShellExecuteEx(&ShExecInfo);

    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInformation;
    ZeroMemory( &startupInfo, sizeof(startupInfo) );
    startupInfo.cb = sizeof(startupInfo);
    ZeroMemory( &processInformation, sizeof(processInformation) );
    if(!CreateProcessA(
        NULL,
        fullCommandLine,
        NULL, // lpProcessAttributes
        NULL, // lpThreadAttributes
        true, // bInheritHandles
        0, // dwCreationFlags
        NULL, // lpEnvironment
        NULL, // lpCurrentDirectory
        &startupInfo, // lpStartupInfo
        &processInformation // lpProcessInformation
    )) {
        // TODO handle failure
        GetLastError();
    }

    // wait for process to exit
    WaitForSingleObject(processInformation.hProcess, INFINITE);
    DWORD exitCode;
    boolean r = GetExitCodeProcess(processInformation.hProcess, &exitCode);
    CloseHandle(processInformation.hProcess);
    CloseHandle(processInformation.hThread);
    if(r) {
        // success: return status of spawned process
        debug1("Got exit code of process\n");
        return exitCode;
    } else {
        debug1("Error getting process exit code\n");
        // TODO handle failure
    }
}

struct StrBuilder {
    int nextI;
    string buffer;
};

struct StrBuilder* createBuilder(int maxLength) {
    struct StrBuilder * ret = malloc(sizeof(struct StrBuilder));
    ret->nextI = 0;
    ret->buffer = malloc(sizeof(char) * (maxLength + 1));
    return ret;
}
