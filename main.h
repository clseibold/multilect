#ifndef MAIN_H
#define MAIN_H

#ifndef __APPLE__
#include <malloc.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#define internal static

#define bool unsigned char
#define true 1
#define false 0

// --- language.c ---

bool isConsonant(char c);
bool isVowel(char c);
void makePlural(char *buffer);
char *numberToText(int n);

// --- ---

typedef struct TermSize {
	int width, height;
} TermSize;

void clrscr();
TermSize getTermSize();

#ifdef _WIN32
#include <conio.h>
#define getch _getch
#define kbhit _kbhit
#else
#include <alloca.h>
#include <unistd.h>
#include <termios.h>
char getch();
char getch_nonblocking();
#endif

typedef enum FileType {
	FT_UNKNOWN, FT_TEXT, FT_MARKDOWN, FT_C, FT_CPP, FT_C_HEADER, FT_GOPHERMAP,
	FT_LUA // Batch and Bash files ?
} FileType;

// --- colors.c ---

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define FOREGROUND_YELLOW FOREGROUND_RED|FOREGROUND_GREEN
#define FOREGROUND_CYAN FOREGROUND_GREEN|FOREGROUND_BLUE
#define FOREGROUND_MAGENTA FOREGROUND_RED|FOREGROUND_BLUE
#define FOREGROUND_WHITE FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE

#define BACKGROUND_YELLOW BACKGROUND_RED|BACKGROUND_GREEN
#define BACKGROUND_CYAN BACKGROUND_GREEN|BACKGROUND_BLUE
#define BACKGROUND_MAGENTA BACKGROUND_RED|BACKGROUND_BLUE
#define BACKGROUND_WHITE BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE

#endif

#define COL_RED "\x1b[31m" // Error
#define COL_GREEN "\x1b[32m" // Prompt, Success
#define COL_YELLOW "\x1b[33m"
#define COL_BLUE "\x1b[34m"
#define COL_MAGENTA "\x1b[35m"
#define COL_CYAN "\x1b[36m" // Information
#define COL_RESET "\x1b[0m" // Input

typedef enum COLOR
{
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_YELLOW,
	COLOR_CYAN,
	COLOR_MAGENTA,
	COLOR_WHITE,
	COLOR_BLACK
} COLOR;

#ifdef _WIN32
HANDLE hConsole;
#endif

void setColor(COLOR foreground);
void resetColor(void);
void colors_printf(COLOR foreground, const char *fmt, ...);
void colors_puts(COLOR foreground, const char *fmt, ...);
void printError(const char *fmt, ...);
void printPrompt(const char *fmt, ...);

// --- Stretchy Buffers (by Sean Barratt) ---

#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#define CLAMP_MAX(x, max) MIN(x, max) // TODO
#define CLAMP_MIN(x, min) MAX(x, min)
#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)

void *xcalloc(size_t num_elems, size_t elem_size);
void *xrealloc(void *prt, size_t num_bytes);
void *xmalloc(size_t num_bytes);
void fatal(const char *fmt, ...);

typedef struct BufHdr {
	size_t len;
	size_t cap;
	char buf[0]; // [0] new in c99
} BufHdr;

#define buf__hdr(b) ((BufHdr *) ((char *) (b) - offsetof(BufHdr, buf)))
#define buf__fits(b, n) (buf_len(b) + (n) <= buf_cap(b))
#define buf__fit(b, n) (buf__fits((b), (n)) ? 0 : ((b) = buf__grow((b), buf_len(b) + (n), sizeof(*(b)))))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_push(b, x) (buf__fit((b), 1), (b)[buf__hdr(b)->len++] = (x))
#define buf_end(b) ((b) + buf_len(b))
#define buf_last(b) ((b) + buf_len(b) - 1)

#define buf_add(b, n) (buf__fit((b), n), buf__hdr(b)->len += n, &(b)[buf__hdr(b)->len - n]) // TODO
#define buf_pop(b) (buf__hdr(b)->len--, &(b)[buf__hdr(b)->len + 1]) // TODO
#define buf_pop_all(b) (buf__hdr(b)->len = 0)

#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)

void *buf__grow(const void *buf, size_t new_len, size_t elem_size);

#endif

// --- parsing.c ---

typedef struct pString {
	char *start;
	char *end;
} pString;

typedef struct lineRange {
	int start;
	int end;
} lineRange;

// Function pointer to function that can run user-code on specific keypresses during
// input (with getInput). If null, the function is not called.
// Return true if keypress should continue to use default action provided by getInput()
typedef bool (*inputKeyCallback)(char, bool isSpecial, char **, int *);

#define INPUT_ESC 27

// ANSI Control Characters
#define INPUT_CTRL_L 12 // Clear Screen
#define INPUT_CTRL_X 24 // Cancel
#define INPUT_CTRL_C 3 // Exit program
#define INPUT_CTRL_O 15

// Special Keys
#ifdef _WIN32

#define INPUT_SPECIAL1 -32
#define INPUT_SPECIAL2 224 // TODO
#define INPUT_LEFT 75
#define INPUT_RIGHT 77
#define INPUT_DELETE 83
#define INPUT_END 79
#define INPUT_HOME 71
#define INPUT_ENDINPUT 26 // Ctrl-Z
#define INPUT_BACKSPACE 8
#else
#define INPUT_SPECIAL1 27
#define INPUT_SPECIAL2 91
#define INPUT_LEFT 68
#define INPUT_RIGHT 67
#define INPUT_DELETE1 51
#define INPUT_DELETE2 126
#define INPUT_END 70
#define INPUT_HOME 72
#define INPUT_ENDINPUT 4 // Ctrl-D
#define INPUT_BACKSPACE 127
#endif

char *skipWhitespace(char *start, char *endBound, bool backwards);
char *skipWord(char *start, char *endBound, bool includeNumbers, bool includeSymbols, bool backwards);
char *skipNumbers(char *start, char *endBound);
char *skipLineNumber(char *start, char *endBound);

char *getInput(bool *canceled, char *inputBuffer, inputKeyCallback callback);
int parsing_getLine(char *line, int max, int trimSpace);
int parsing_getLine_dynamic(char **chars, int trimSpace);
