#include "main.h"

bool isConsonant(char c) {
	char n = c;
	
	if (n > 90) n -= 32;
	
	switch (n) {
		case 'B':
		case 'C':
		case 'D':
		case 'F':
		case 'G':
		case 'H':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		return true;
		break;
		default:
		return false;
	}
}

bool isVowel(char c) {
	char n = c;
	
	if (n > 90) n -= 32;
	
	switch (n) {
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
		return true;
		break;
		default:
		return false;
	}
}

void makePlural(char *buffer) {
	// Manual check for irregulars
	
	switch (buffer[buf_len(buffer) - 1]) { // Switch on last character
		case 's':
		case 'o':
		{
			// Add es
			buf_push(buffer, 'e');
			buf_push(buffer, 's');
		} break;
		case 'h':
		{
			// Check second to last for c or s (ch, sh)
			if (buffer[buf_len(buffer) - 2] == 'c' || buffer[buf_len(buffer) - 2] == 's') {
				buf_push(buffer, 'e');
				buf_push(buffer, 's');
			}
		} break;
		case 'z':
		{
			buf_push(buffer, 'z');
			buf_push(buffer, 'e');
			buf_push(buffer, 's');
		} break;
		case 'f':
		{
			buf_pop(buffer);
			buf_push(buffer, 'v');
			buf_push(buffer, 'e');
			buf_push(buffer, 's');
		} break;
		case 'e':
		{
			if (buffer[buf_len(buffer) - 2] == 'f') {
				buf_pop(buffer);
				buf_pop(buffer);
				buf_push(buffer, 'v');
				buf_push(buffer, 'e');
				buf_push(buffer, 's');
			} else {
				buf_push(buffer, 's');
			}
		} break;
		case 'x':
		{
			if (buffer[buf_len(buffer) - 2] == 'e') {
				// ex -> ices
				buf_pop(buffer);
				buf_pop(buffer);
				buf_push(buffer, 'i');
				buf_push(buffer, 'c');
				buf_push(buffer, 'e');
				buf_push(buffer, 's');
			} else {
				// x -> xes
				buf_push(buffer, 'e');
				buf_push(buffer, 's');
			}
		} break;
		case 'y':
		{
			if (isConsonant(buffer[buf_len(buffer) - 2])) {
				// cons + y -> cons + ies
				buf_pop(buffer);
				buf_push(buffer, 'i');
				buf_push(buffer, 'e');
				buf_push(buffer, 's');
			} else {
				// vowel + y -> vowel + ys
				buf_push(buffer, 's');
			}
		} break;
		default:
		{
			buf_push(buffer, 's');
		} break;
	}
}

char *numberToText(int n) {
	char *buffer = NULL;
	
	if (n >= 100) {
		// Keep as number chars
		int numOfDigits = 0;
		int x = n;
		while (x > 0) {
			numOfDigits++;
			x = x / 10;
		}
		
		if (x == 0) numOfDigits = 1;
		
		char *s = buf_add(buffer, numOfDigits);
		snprintf(s, numOfDigits, "%d", n);
	} else {
		unsigned char ones = 0;
		unsigned char tens = 0;
		
		ones = n % 10;
		tens = n / 10;
		
		// Handle 10 - 19, the teens
		if (tens == 1) {
			switch (ones) {
				case 0:
				{
					buf_push(buffer, 't');
					buf_push(buffer, 'e');
					buf_push(buffer, 'n');
				} break;
				case 1:
				{
					buf_push(buffer, 'e');
					buf_push(buffer, 'l');
					buf_push(buffer, 'e');
					buf_push(buffer, 'v');
					buf_push(buffer, 'e');
					buf_push(buffer, 'n');
				} break;
				case 2:
				{
					buf_push(buffer, 't');
					buf_push(buffer, 'w');
					buf_push(buffer, 'e');
					buf_push(buffer, 'l');
					buf_push(buffer, 'v');
					buf_push(buffer, 'e');
				} break;
				default:
				{
					if (ones == 3) {
						buf_push(buffer, 't');
						buf_push(buffer, 'h');
						buf_push(buffer, 'i');
						buf_push(buffer, 'r');
					} else if (ones == 4) {
						buf_push(buffer, 'f');
						buf_push(buffer, 'o');
						buf_push(buffer, 'u');
						buf_push(buffer, 'r');
					} else if (ones == 5) {
						buf_push(buffer, 'f');
						buf_push(buffer, 'i');
						buf_push(buffer, 'f');
					} else if (ones == 6) {
						buf_push(buffer, 's');
						buf_push(buffer, 'i');
						buf_push(buffer, 'x');
					} else if (ones == 7) {
						buf_push(buffer, 's');
						buf_push(buffer, 'e');
						buf_push(buffer, 'v');
						buf_push(buffer, 'e');
						buf_push(buffer, 'n');
					} else if (ones == 8) {
						buf_push(buffer, 'e');
						buf_push(buffer, 'i');
						buf_push(buffer, 'g');
						buf_push(buffer, 'h');
					} else if (ones == 9) {
						buf_push(buffer, 'n');
						buf_push(buffer, 'i');
						buf_push(buffer, 'n');
						buf_push(buffer, 'e');
					}
					
					buf_push(buffer, 't');
					buf_push(buffer, 'e');
					buf_push(buffer, 'e');
					buf_push(buffer, 'n');
				} break;
			}
		} else if (tens == 0 || tens > 1) {
			if (tens > 1) {
				switch (tens) {
					case 2:
					{
						buf_push(buffer, 't');
						buf_push(buffer, 'w');
						buf_push(buffer, 'e');
						buf_push(buffer, 'n');
						buf_push(buffer, 't');
						buf_push(buffer, 'y');
					} break;
					case 3:
					{
						buf_push(buffer, 't');
						buf_push(buffer, 'h');
						buf_push(buffer, 'i');
						buf_push(buffer, 'r');
						buf_push(buffer, 't');
						buf_push(buffer, 'y');
					} break;
					case 4:
					{
						buf_push(buffer, 'f');
						buf_push(buffer, 'o');
						buf_push(buffer, 'u');
						buf_push(buffer, 'r');
						buf_push(buffer, 't');
						buf_push(buffer, 'y');
					} break;
					case 5:
					{
						buf_push(buffer, 'f');
						buf_push(buffer, 'i');
						buf_push(buffer, 'f');
						buf_push(buffer, 't');
						buf_push(buffer, 'y');
					} break;
					case 6:
					{
						buf_push(buffer, 's');
						buf_push(buffer, 'i');
						buf_push(buffer, 'x');
						buf_push(buffer, 't');
						buf_push(buffer, 'y');
					} break;
					case 7:
					{
						buf_push(buffer, 's');
						buf_push(buffer, 'e');
						buf_push(buffer, 'v');
						buf_push(buffer, 'e');
						buf_push(buffer, 'n');
						buf_push(buffer, 't');
						buf_push(buffer, 'y');
					} break;
					case 8:
					{
						buf_push(buffer, 'e');
						buf_push(buffer, 'i');
						buf_push(buffer, 'g');
						buf_push(buffer, 'h');
						buf_push(buffer, 't');
						buf_push(buffer, 'y');
					} break;
					case 9:
					{
						buf_push(buffer, 'n');
						buf_push(buffer, 'i');
						buf_push(buffer, 'n');
						buf_push(buffer, 'e');
						buf_push(buffer, 't');
						buf_push(buffer, 'y');
					} break;
				}
				
				if (ones != 0) {
					buf_push(buffer, '-');
				}
			}
	
			switch (ones) {
				case 0:
				{
					if (tens == 0) {
						buf_push(buffer, 'z');
						buf_push(buffer, 'e');
						buf_push(buffer, 'r');
						buf_push(buffer, 'o');
					}
				} break;
				case 1:
				{
					buf_push(buffer, 'o');
					buf_push(buffer, 'n');
					buf_push(buffer, 'e');
				} break;
				case 2:
				{
					buf_push(buffer, 't');
					buf_push(buffer, 'w');
					buf_push(buffer, 'o');
				} break;
				case 3:
				{
					buf_push(buffer, 't');
					buf_push(buffer, 'h');
					buf_push(buffer, 'r');
					buf_push(buffer, 'e');
					buf_push(buffer, 'e');
				} break;
				case 4:
				{
					buf_push(buffer, 'f');
					buf_push(buffer, 'o');
					buf_push(buffer, 'u');
					buf_push(buffer, 'r');
				} break;
				case 5:
				{
					buf_push(buffer, 'f');
					buf_push(buffer, 'i');
					buf_push(buffer, 'v');
					buf_push(buffer, 'e');
				} break;
				case 6:
				{
					buf_push(buffer, 's');
					buf_push(buffer, 'i');
					buf_push(buffer, 'x');
				} break;
				case 7:
				{
					buf_push(buffer, 's');
					buf_push(buffer, 'e');
					buf_push(buffer, 'v');
					buf_push(buffer, 'e');
					buf_push(buffer, 'n');
				} break;
				case 8:
				{
					buf_push(buffer, 'e');
					buf_push(buffer, 'i');
					buf_push(buffer, 'g');
					buf_push(buffer, 'h');
					buf_push(buffer, 't');
				} break;
				case 9:
				{
					buf_push(buffer, 'n');
					buf_push(buffer, 'i');
					buf_push(buffer, 'n');
					buf_push(buffer, 'e');
				} break;
			}
		}
	}
	
	return buffer;
}

