/*
 * UNG's Not GNU
 * 
 * Copyright (c) 2011-2019, Jakob Kaivo <jkk@ung.org>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#define _XOPEN_SOURCE 700
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <unistd.h>

#define BYTES (1 << 0)
#define CHARS (1 << 1)
#define LINES (1 << 2)
#define WORDS (1 << 3)

static uintmax_t total_n = 0;
static uintmax_t total_w = 0;
static uintmax_t total_c = 0;
static wint_t wc_eof = EOF;
static wint_t wc_newline = '\n';

static void flagprint(uintmax_t n, uintmax_t w, uintmax_t c, char *f, int flags)
{
	if (flags == 0) {
		flags = LINES | WORDS | CHARS;
	}

	if (flags & LINES) {
		printf("%ju%s", n, flags ^ LINES ? " " : "");
	}

	if (flags & WORDS) {
		printf("%ju%s", w, flags ^ (LINES | WORDS) ? " " : "");
	}

	if (flags & CHARS || flags & BYTES) {
		printf("%ju", c);
	}

	if (f != NULL) {
		printf(" %s", f);
	}

	printf("\n");
}

static wint_t wc_get_(FILE *f)
{
	return fgetc(f);
}

static int wc_isspace_(wint_t c)
{
	return isspace((int)c);
}

static wint_t (*wc_get)(FILE *) = wc_get_;
static int (*wc_isspace)(wint_t) = wc_isspace_;

static int wc(char *path, int flags)
{
	uintmax_t newlines = 0;
	uintmax_t words = 0;
	uintmax_t charbytes = 0;
	int wasword = 0;

	FILE *f = stdin;

	if (path && strcmp(path, "-") != 0) {
		f = fopen(path, "r");
		if (f == NULL) {
			fprintf(stderr, "wc: %s: %s\n", path, strerror(errno));
			return 1;
		}
	}

	wint_t c;
	while ((c = wc_get(f)) != wc_eof) {
		charbytes++;

		if (wc_isspace(c)) {
			newlines += (c == wc_newline);
			words += !wasword;
			wasword = 1;
		} else {
			wasword = 0;
		}
	}

	if (f != stdin) {
		fclose(f);
	}

	total_n += newlines;
	total_w += words;
	total_c += charbytes;

	flagprint (newlines, words, charbytes, path, flags);
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int flags = 0;
	int total = 0;

	setlocale(LC_ALL, "");
 
	int c;
	while ((c = getopt(argc, argv, "cmlw")) != -1) {
		switch (c) {
		case 'c':
			flags |= BYTES;
			flags &= ~CHARS;
			break;

		case 'm':
			flags |= CHARS;
			flags &= ~BYTES;
			break;

		case 'l':
			flags |= LINES;
			break;

		case 'w':
			flags |= WORDS;
			break;

		default:
			return 1;
		}
	}

	if (argc - optind > 1) {
		total = 1;
	}

	if (flags & CHARS) {
		wc_eof = WEOF;
		wc_get = fgetwc;
		wc_newline = L'\n';
		wc_isspace = iswspace;
	}

	do {
		ret |= wc(argv[optind++], flags);
	} while (optind < argc);
 
	if (total) {
		flagprint(total_n, total_w, total_c, "total", flags);
	}

	return ret;
}
