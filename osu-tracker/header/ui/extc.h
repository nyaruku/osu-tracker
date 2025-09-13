#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

// Function to calculate the length of a null-terminated char*
size_t custom_strlen(const char* str) {
    size_t length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

// Function to concatenate multiple null-terminated char* strings
char* custom_strcat(size_t numStrings, ...) {
    // Calculate the total length of the concatenated string
    size_t totalLen = 0;
    va_list args;
    va_start(args, numStrings);
    for (size_t i = 0; i < numStrings; i++) {
        const char* currentString = va_arg(args, const char*);
        totalLen += custom_strlen(currentString);
    }
    va_end(args);

    // Allocate memory for the result string
    char* result = (char*)malloc(totalLen + 1); // +1 for the null terminator
    if (!result) return NULL;

    // Copy the contents of each string to the result
    size_t currentIndex = 0;
    va_start(args, numStrings);
    for (size_t i = 0; i < numStrings; i++) {
        const char* currentString = va_arg(args, const char*);
        size_t currentLen = custom_strlen(currentString);
        for (size_t j = 0; j < currentLen; j++) {
            result[currentIndex++] = currentString[j];
        }
    }
    va_end(args);

    // Null-terminate the result string
    result[totalLen] = '\0';

    return result;
}

char* custom_strcat_static(size_t numStrings, ...) {
    static char buffer[1024];  // fixed max size
    size_t totalLen = 0;

    va_list args;
    va_start(args, numStrings);

    // clear buffer first
    buffer[0] = '\0';

    for (size_t i = 0; i < numStrings; i++) {
        const char* s = va_arg(args, const char*);
        size_t len = strlen(s);

        if (totalLen + len >= sizeof(buffer) - 1) {
            // truncate if no space
            len = sizeof(buffer) - 1 - totalLen;
        }

        strncat(buffer, s, len);
        totalLen += len;
    }
    va_end(args);

    return buffer;  // valid until next call
}

const char* intToConst(int i) {
    static char buf[32];
    snprintf(buf, sizeof(buf), "%d", i);
    return buf;
}

const char* floatToConst(float f) {
    static char buf[32];
    snprintf(buf, sizeof(buf), "%f", f);
    return buf;
}

// Function to remove a substring from a string and return it as const char*
const char* removeSubstring(const char* inputString, const char* substringToRemove) {
    const char* startPos = strstr(inputString, substringToRemove);

    if (startPos != NULL) {
        size_t substringLength = strlen(substringToRemove);
        size_t inputStringLength = strlen(inputString);

        // Calculate the length of the modified string
        size_t modifiedStringLength = inputStringLength - substringLength;

        // Create a new char* buffer to hold the modified string
        char* modifiedString = (char*)malloc(modifiedStringLength + 1);
        if (!modifiedString) return NULL;

        // Copy the part before the substring
        size_t prefixLength = startPos - inputString;
        strncpy(modifiedString, inputString, prefixLength);
        modifiedString[prefixLength] = '\0';

        // Copy the part after the substring
        strcat(modifiedString, startPos + substringLength);

        return modifiedString;
    }

    // If the substring is not found, return the original inputString
    return inputString;
}

// Function to check if a line starts with a specific substring
bool lineStartsWith(const char* line, const char* prefix) {
    return strncmp(line, prefix, strlen(prefix)) == 0;
}

void ext_TextColor(struct nk_context* ctx, int r, int g, int b) {
    ctx->style.text.color.r = r;
    ctx->style.text.color.g = g;
    ctx->style.text.color.b = b;
}

void ext_BG(struct nk_context* ctx, int r, int g, int b) {
    ctx->style.window.fixed_background.data.color.r = r;
    ctx->style.window.fixed_background.data.color.g = g;
    ctx->style.window.fixed_background.data.color.b = b;
}

const char* bool2str(bool b) {
    return b ? "true" : "false";
}