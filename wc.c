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

enum {
	WC_CHARS = 1 << 0,
	WC_LINES = 1 << 1,
	WC_WORDS = 1 << 2,
};

static wint_t wc_get_(FILE *f);
static int wc_isspace_(wint_t c);

static uintmax_t wc_total_n = 0;
static uintmax_t wc_total_w = 0;
static uintmax_t wc_total_c = 0;

static wint_t wc_eof = EOF;
static wint_t wc_newline = '\n';
static wint_t (*wc_get)(FILE *) = wc_get_;
static int (*wc_isspace)(wint_t) = wc_isspace_;

static void wc_print(uintmax_t n, uintmax_t w, uintmax_t c, char *f, int flags)
{
	if (flags == 0) {
		flags = WC_LINES | WC_WORDS | WC_CHARS;
	}

	flags &= (WC_LINES | WC_WORDS | WC_CHARS);

	if (flags & WC_LINES) {
		flags ^= WC_LINES;
		printf("%ju%s", n, flags ? " " : "");
	}

	if (flags & WC_WORDS) {
		flags ^= WC_WORDS;
		printf("%ju%s", w, flags ? " " : "");
	}

	if (flags & WC_CHARS) {
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

static int wc(char *path, int flags)
{
	uintmax_t newlines = 0;
	uintmax_t words = 0;
	uintmax_t charbytes = 0;
	int wasspace = 1;

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
			words += !wasspace;
			wasspace = 1;
		} else {
			wasspace = 0;
		}
	}

	if (f != stdin) {
		fclose(f);
	}

	wc_total_n += newlines;
	wc_total_w += words;
	wc_total_c += charbytes;

	wc_print(newlines, words, charbytes, path, flags);
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
		case 'c':	/* count bytes */
			flags |= WC_CHARS;
			wc_eof = EOF;
			wc_get = wc_get_;
			wc_newline = '\n';
			wc_isspace = wc_isspace_;
			break;

		case 'm':	/* count characters */
			flags |= WC_CHARS;
			wc_eof = WEOF;
			wc_get = fgetwc;
			wc_newline = L'\n';
			wc_isspace = iswspace;
			break;

		case 'l':	/* count lines */
			flags |= WC_LINES;
			break;

		case 'w':	/* count words */
			flags |= WC_WORDS;
			break;

		default:
			return 1;
		}
	}

	if (argc - optind > 1) {
		total = 1;
	}

	do {
		ret |= wc(argv[optind++], flags);
	} while (optind < argc);
 
	if (total) {
		wc_print(wc_total_n, wc_total_w, wc_total_c, "total", flags);
	}

	return ret;
}
