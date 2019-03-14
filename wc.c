/*
 * UNG's Not GNU
 * 
 * Copyright (c) 2011, Jakob Kaivo <jakob@kaivo.net>
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

#include <stdio.h>
#include <ctype.h>
#include <getopt.h>

const char *wc_desc = "word, line, and byte or character count";
const char *wc_inv  = "wc [-c|-m] [-lw] [file...]";

#define BYTES 1 << 0
#define CHARS 1 << 1
#define LINES 1 << 2
#define WORDS 1 << 3

static unsigned long total_n = 0;
static unsigned long total_w = 0;
static unsigned long total_c = 0;

int flagprint (unsigned long n, unsigned long w, unsigned long c, char *f,
  int flags)
{
  if (flags == 0) flags = LINES | WORDS | CHARS;

  if (flags & LINES)
    printf ("%lu%s", n, flags ^ LINES ? " " : "");
  if (flags & WORDS)
    printf ("%lu%s", w, flags ^ (LINES | WORDS) ? " " : "");
  if (flags & CHARS || flags & BYTES)
    printf ("%lu", c);
  // FIXME: What an ass-pain
  if (f != NULL)
    printf ("%s%s", flags & CHARS || flags & BYTES ? " " : "", f);
  printf ("\n");
}

int wc (FILE *f, int flags, char *name)
{
  unsigned long newlines = 0;
  unsigned long words = 0;
  unsigned long charbytes = 0;
  int wasword = 0;
  int n, i;
  char buf[BUFSIZ];

  while (!feof(f)) {
    n = fread (buf, sizeof(char), BUFSIZ, f);

    if (flags & CHARS) {
      charbytes += n; // FIXME: wrong, convert to wc_type or mb_char
    } else {
      charbytes += n;
    }

    for (i = 0; i < n; i++) {
      if (buf[i] == '\n') {
        newlines++;
        if (wasword == 0) {
          wasword = 1;
          words++;
        }
      } else if (wasword == 0 && isspace(buf[i])) {
        wasword = 1;
        words++;
      } else {
        wasword = 0;
      }
    }
  }

  total_n += newlines;
  total_w += words;
  total_c += charbytes;

  flagprint (newlines, words, charbytes, name, flags);
}

int
main(int argc, char **argv)
{
  int flags = 0;
  int total = 0;
 
  int c;
  while ((c = getopt (argc, argv, ":cmlw")) != -1) {
    switch (c) {
      case 'c':
        if (flags & CHARS) { return 1; }
        flags |= BYTES;
        break;
      case 'm':
        if (flags & BYTES) { return 1; }
	flags |= CHARS;
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

  if (argc - optind > 1)
    total = 1;

  if (optind >= argc) {
    wc (stdin, flags, NULL);
  } else while (optind < argc) {
    FILE *in = fopen (argv[optind], "r");
    wc (in, flags, argv[optind]);
    fclose (in);
    optind++;
  }
 
  if (total) {
    flagprint (total_n, total_w, total_c, "total", flags);
  }

  return 0;
}
