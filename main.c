#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "main.h"

// -----

#ifdef _WIN32

void clrscr() {
	system("cls");
}

TermSize getTermSize() {
	// TODO, for Windows
	return { 0, 0 };
}

#else

#include <unistd.h> // _getch
#include <termios.h> // _getch
char getch() {
	char buf=0;
	struct termios old={0};
	fflush(stdout);
	if (tcgetattr(0, &old) < 0)
		perror("tcsetattr()");
	old.c_lflag&=~ICANON;
	old.c_lflag&=~ECHO;
	old.c_cc[VMIN]=1;
	old.c_cc[VTIME]=0;
	if (tcsetattr(0, TCSANOW, &old) < 0)
		perror("tcsetattr ICANON");
	if (read(0, &buf, 1) < 0)
		perror("read()");
	old.c_lflag|=ICANON;
	old.c_lflag|=ECHO;
	if (tcsetattr(0, TCSADRAIN, &old) < 0)
		perror("tcsetattr ~ICANON");
	return buf;
}

char getch_nonblocking() {
	char buf = 0;
	struct termios old = {0};
	
	fflush(stdout);
	
	if (tcgetattr(0, &old) < 0)
		perror("tcsetattr()");
	
	old.c_lflag&=~ICANON;
	old.c_lflag&=~ECHO;
	old.c_cc[VMIN] = 0;
	old.c_cc[VTIME] = 0;
	
	if (tcsetattr(0, TCSANOW, &old) < 0)
		perror("tcsetattr ICANON");
	if (read(0, &buf, 1) < 0)
		perror("read()");
	
	old.c_lflag|=ICANON;
	old.c_lflag|=ECHO;
	
	if (tcsetattr(0, TCSADRAIN, &old) < 0)
		perror("tcsetattr ~ICANON");
	return buf;
}

void clrscr() {
	printf("\e[1;H\e[2J");
}

#include <sys/ioctl.h>
TermSize getTermSize() {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	TermSize size = { w.ws_col, w.ws_row };
	return size;
}

#endif

typedef enum LineType {
	BLANK, PARAGRAPH, HEADING, UL, OL, CODE, TABLE, BLOCKQUOTE, HRULE, OTHER
} LineType;

typedef struct Line {
	char *chars;
	LineType type;
	unsigned int nest;
} Line;

char char_backward(Line *lines, unsigned int *currentLine, unsigned int *currentIndex, int amt) {
	unsigned int backwardIndex = *currentIndex - amt;
	
	if (backwardIndex >= 0) {
		(*currentIndex) = backwardIndex;
		return lines[*currentLine].chars[backwardIndex];
	} else {
		do {
			(*currentLine) -= 1;
			backwardIndex += buf_len(lines[*currentLine].chars);
		} while (backwardIndex < 0);
		
		(*currentIndex) = backwardIndex;
		return lines[*currentLine].chars[backwardIndex];
	}
}

char char_forward(Line *lines, unsigned int *currentLine, unsigned int *currentIndex, int amt) {
	unsigned int forwardIndex = (*currentIndex) + amt;
	
	while (forwardIndex >= buf_len(lines[*currentLine].chars)) {
		forwardIndex -= buf_len(lines[*currentLine].chars);
		(*currentLine) += 1;
	}
	
	(*currentIndex) = forwardIndex;
	return lines[*currentLine].chars[forwardIndex];
}

void printIndent(unsigned int indent) {
	for (int i = 0; i < indent; i++) {
		printf(" ");
	}
}

void printHRule(unsigned int contentWidth, unsigned int indent) {
	printIndent(indent);
	
	for (int i = 0; i < contentWidth; i++)
		printf("-");
	printf("\n");
}

void printTOC(unsigned int *headingLines, Line *lines, unsigned int contentWidth, unsigned int indent) {
	printHRule(contentWidth, indent);
	printIndent(indent);
	printf("Table of Contents:\n");
	
	unsigned int index[6] = {0};
	
	for (int i = 0; i < buf_len(headingLines); i++) {
		int line = headingLines[i];
		index[lines[line].nest]++;
		
		// Print indentation for centering content in terminal
		printIndent(indent);
		
		// Print indentation for nest
		for (int i = 0; i < lines[line].nest; i++) {
			printf("    ");
		}
		
		// Print index of heading within nest/scope
		printf("%d. ", index[lines[line].nest]);
		
		// Print the heading
		printf("%.*s", buf_len(lines[line].chars), lines[line].chars);
		
		// If nest 0 (Main Title), insert line below
/*		if (lines[line].nest == 0) {
			for (int i = 0; i <= buf_len(lines[line].chars) + 1; i++)
				printf("-");
			printf("\n");
		}
		
		// Print new line to separate from content underneath
		printf("\n");*/
	}
	
	printHRule(contentWidth, indent);
}

void printParagraph(Line *lines, int lineStart, int lineEnd, unsigned int contentWidth, unsigned int indent);

void printBullet(Line *lines, unsigned int line, unsigned int contentWidth, unsigned int indent) {
	printIndent(indent);
	
	printf("\u2022 ");
	unsigned int col = 2;
	
	char *lineChars = lines[line].chars;
	for (int i = 0; i < buf_len(lineChars); i++) {
		char c = lineChars[i];
		
		++col;
		printf("%c", c);
		
		if (col >= contentWidth && i + 1 < buf_len(lineChars)) {
			printf("\n");
			printIndent(indent);
			printf("  "); // To align-up with start of text of bullet
			col = 0;
		}
	}
}

void printFile(Line *lines, unsigned int contentWidth, unsigned int indent) {
	int typeSectionStartLine = 0;
	for (unsigned int i = 0; i < buf_len(lines); i++) {
		if (i != 0 && lines[i].type != lines[i - 1].type) {
			typeSectionStartLine = i;
		}
		
		if (lines[i].type == PARAGRAPH && lines[i + 1].type != PARAGRAPH) {
			printParagraph(lines, typeSectionStartLine, i, contentWidth, indent);
		} else if (lines[i].type == PARAGRAPH) {
			; // Skip paragraphs until end (when it will then print)
		} else if (lines[i].type == UL) {
			printBullet(lines, i, contentWidth, indent);
		} else if (lines[i].type == HEADING) {
			printf("\n");
			printIndent(indent);
			colors_printf(COLOR_YELLOW, "%.*s", buf_len(lines[i].chars), lines[i].chars);
			printf("\n");
		} else {
			printIndent(indent);
			printf("%.*s", buf_len(lines[i].chars), lines[i].chars);
		}
	}
}

void printParagraph(Line *lines, int lineStart, int lineEnd, unsigned int contentWidth, unsigned int indent) {
	bool bold = false;
	bool italics = false;
	bool strikethrough = false;
	bool code = false;
	char *current = NULL;
	
	unsigned int col = 0;
	printIndent(indent);
	for (int line = lineStart; line <= lineEnd; line++) {
		char *lineChars = lines[line].chars;
		
		for (unsigned int i = 0; i < buf_len(lineChars); i++) {
			char c = lineChars[i];
			
			if (c == '*' && (i + 1 == buf_len(lineChars) - 1 || lineChars[i + 1] != '*') && (i - 1 >= 0 || lineChars[i - 1] != '*'))
				italics = !italics;
			else if (c == '*' && (i + 1 != buf_len(lineChars) - 1 || lineChars[i + 1] == '*') && (i - 1 >= 0 || lineChars[i - 1] != '*'))
				bold = !bold;
			else if (c == '`') code = !code;
			else if ((c == ' ' || c == '\t') && (i + 1 < buf_len(lineChars) && lineChars[i + 1] != ' ' && lineChars[i + 1] != '\t' && lineChars[i + 1] != '\n' && lineChars[i + 1] != '\r')) {
				// Word wrapping
				// Search for next whitespace
				unsigned int length = 1;
				unsigned int currentIndex = i;
				unsigned int currentLine = line;
				char temp_c;
				while ((temp_c = char_forward(lines, &currentLine, &currentIndex, 1)) != ' ' && temp_c != '\t') {
					if (currentLine > lineEnd) break;
					if (temp_c != '\n' && temp_c != '\r') ++length;
				}
				--length;
				
				if (col + length >= contentWidth) {
					printf("$\n");
					printIndent(indent);
					col = 0;
				} else if (col == 0) {
					continue;
				} else {
					printf("%c", c);
					++col;
				}
			}
			else if (c == '\n' || c == '\r') {
				printf(" ");
				++col;
			} else if (c == '<' && lineChars[i + 1] == 'b' && lineChars[i + 2] == 'r' && lineChars[i + 3] == '>') { // TODO: Check that i + 1 is within bounds first
				printf("\n");
				printIndent(indent);
				col = 0;
				i += 3;
				// Skip new line so that line doesn't start with space
				if (lineChars[i + 1] == '\n') i++;
			} else {
				++col;
				if (bold) printf("\e[1m");
				if (code) printf("\e[2;36m");
				printf("%c", c);
				printf("\e[0m");
			}
			
			if (col >= contentWidth && i + 1 < buf_len(lineChars)) {
				printf("^\n");
				printIndent(indent);
				col = 0;
			}
		}
	}
	printf("\n");
}

int main(int argc, char **argv) {
	char *input;
	if (argc <= 1) {
		printf("Enter a markdown file to open: ");
		
		bool canceled = false;
		input = getInput(&canceled, NULL, NULL);
		if (*buf_last(input) == '\n')
			buf_pop(input);
		if (*buf_last(input) == '\r')
			buf_pop(input);
		
		buf_push(input, '\0');
	} else { // Open given file
		input = argv[1];
	}
		
	// Open the file
	
	FILE *fp;
	fp = fopen(input, "rb");
	if (fp == NULL) {
		printf("Error: File could not be read. errno = %d\n", errno);
		return(1);
	}
	
	// Put file into char buffer
	Line *lines = NULL;
	char *chars = NULL;
	unsigned int line = 0;
	unsigned int *headingLines = NULL;
	
	char c;
	bool beginning = true;
	unsigned int currentLineNest = 0; // Starts at 0 as first nest level
	LineType currentLineType = PARAGRAPH;
	
	while ((c = fgetc(fp)) != EOF) {
		if (c == '\n') {
			buf_push(chars, '\n');
			if (buf_len(chars) <= 1) currentLineType = BLANK;
			buf_push(lines, ((Line) { chars, currentLineType, currentLineNest }));
			if (currentLineType == HEADING) buf_push(headingLines, line);
			++line;
			chars = NULL;
			beginning = true;
			currentLineNest = 0;
			currentLineType = PARAGRAPH;
		} else if (c == '\r') {
			c = fgetc(fp);
			if (c == EOF) {
				buf_push(chars, '\n');
				if (buf_len(chars) <= 1) currentLineType = BLANK;
				buf_push(lines, ((Line) { chars, currentLineType, currentLineNest }));
				if (currentLineType == HEADING) buf_push(headingLines, line);
				++line;
				chars = NULL;
				break;
			} else if (c == '\n') {
				buf_push(chars, '\n');
				if (buf_len(chars) <= 1) currentLineType = BLANK;
				buf_push(lines, ((Line) { chars, currentLineType, currentLineNest }));
				if (currentLineType == HEADING) buf_push(headingLines, line);
				++line;
				chars = NULL;
			} else if (c == '\r') {
				buf_push(chars, '\n');
				if (buf_len(chars) <= 1) currentLineType = BLANK;
				buf_push(lines, ((Line) { chars, currentLineType, currentLineNest }));
				if (currentLineType == HEADING) buf_push(headingLines, line);
				++line;
				chars = NULL;
				buf_push(chars, '\n');
				buf_push(lines, ((Line) { chars, PARAGRAPH, 0 }));
				if (currentLineType == HEADING) buf_push(headingLines, line);
				++line;
				chars = NULL;
			} else {
				buf_push(chars, '\n');
				if (buf_len(chars) <= 1) currentLineType = BLANK;
				buf_push(lines, ((Line) { chars, currentLineType, currentLineNest }));
				if (currentLineType == HEADING) buf_push(headingLines, line);
				++line;
				chars = NULL;
				buf_push(chars, c);
			}
			beginning = true;
			currentLineNest = 0;
			currentLineType = PARAGRAPH;
		} else {
			if (beginning) {
				beginning = false;
				
				if (c >= '0' && c <= '9') { // Ordered List
					unsigned int index = 0;
					while ((c = fgetc(fp) >= '0') && c <= '9') index++;
					if (c == '.') {
						currentLineType = OL;
						// Skip whitespace
						while ((c = fgetc(fp) == ' ') || c == '\t') ;
						fseek(fp, -1, SEEK_CUR);
						continue;
					} else {
						fseek(fp, -1 * index, SEEK_CUR);
					}
				} else if (c == '#') {
					unsigned int nest = 0;
					while ((c = fgetc(fp) == '#')) nest++;
					fseek(fp, -1, SEEK_CUR);
					while ((c = fgetc(fp) == ' ') || c == '\t') ;
					fseek(fp, -1, SEEK_CUR);
					currentLineType = HEADING;
					currentLineNest = nest;
					continue;
				} /* else if (c == ' ' || c == '\t') { // Check for blank line
					unsigned int index = 0;
					// Skip all whitespace
					while ((c = fgetc(fp) == ' ') || c == '\t') ++index;
					if (c == '\r' || c == '\n' || c == EOF) // Blank Line
						currentLineType = BLANK;
					fseek(fp, -1 * index, SEEK_CUR); // Go back to beginning
				}*/
				else if (c == '*' || c == '-' || c == '+') {
					if ((c = fgetc(fp)) == ' ') {
						currentLineType = UL;
						while ((c = fgetc(fp) == ' ') || c == '\t') ;
						fseek(fp, -1, SEEK_CUR);
						continue;
					} else {
						fseek(fp, -1, SEEK_CUR);
					}
				} else if (c == '[') {
					// Hack so that multilect works slightly better for vf1
					// text/plain handler, for now...
					currentLineType = OTHER;
				}
			}
			
			buf_push(chars, c);
		}
	}
	
	fclose(fp);
	
	TermSize terminalSize = getTermSize();
	unsigned int contentWidth = 72;
	if (contentWidth > terminalSize.width) contentWidth = terminalSize.width - 2;
	printf("Content Width: %d\n", contentWidth);
	
	printf("%d, %d\n", terminalSize.width, terminalSize.height);
	
	printTOC(headingLines, lines, contentWidth, (terminalSize.width - contentWidth) / 2);
	
	printf("\n");
	printFile(lines, contentWidth, (terminalSize.width - contentWidth) / 2);
	
	// Look for paragraphs (denoted by test with blank line above and below, or 
	// a heading above/below) and insert places in dynamic array.
	return(0);
}

