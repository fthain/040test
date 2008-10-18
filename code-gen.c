/* 68040 FPU emulation bug code generator.
 * Copyright (c) 2008 Finn Thain
 * fthain@telegraphics.com.au

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Generate some machine code with an unimplemented FPU op placed
 * across a page boundary. fint is unimplemented on 68040 and (of course)
 * 68LC040 CPUs. Precede it with a write, since there's supposedly
 * a bug on the 68EC040 triggered that way, according to
 * http://lists.debian.org/debian-68k/2000/08/msg00076.html
 * The motorola errata doesn't mention needing a write, 
 * http://www.freescale.com/files/microcontrollers/doc/errata/MC68040DE_D.txt
 * but this could easily be one and the same bug.
 */

unsigned int hexchr(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + c - 'a';
}

void hexcpy(unsigned char *dst, char *src) {
	while (*src) {
		int d = 0;
		int i = 4;

		do {
			d <<= 4;
			d |= hexchr(*src++);
		} while (--i && *src);
		*dst++ = d >> 8;
		*dst++ = d;
	}
}

char *part1 =
	"4e56fffc"      /* linkw %fp,#-4              */
	"2d7cc49a4000"  /* movel #-996524032,%fp@(-4) */
	"fffc"
	"f22e4400fffc"; /* fmoves %fp@(-4),%fp0       */

char *part2 =
	"34bc0000"      /* movew #0,%a2@              */
	"f2000003"      /* fintrzx %fp0,%fp0          */
	"f2000018"      /* fabsx %fp0,%fp0            */
	"f23c4422443f"  /* fadds #765,%fp0            */
	"4000"
	"f2006000"      /* fmovel %fp0,%d0            */
	"4e5e"          /* unlk %fp                   */
	"4e75";         /* rts                        */

#define PAGE_SZ 8192
#define N_PAGES 17

int main(void) {
	int i;
	unsigned char *p, *q;

	p = malloc(PAGE_SZ * N_PAGES);
	if (!p)
		return 1;

	q = p;
	for (i = 0; i < PAGE_SZ * N_PAGES / 2; i++) {
		*q++ = 0x4e; *q++ = 0x71; /* nop */
	}

	q = p;
	hexcpy(q, part1);

	q += PAGE_SZ * (N_PAGES - 1);
	q -= 6;
	hexcpy(q, part2);

	fwrite(p, PAGE_SZ * N_PAGES, 1, stdout);
	return 0;
}
