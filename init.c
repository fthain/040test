/* 68040 FPU emulation bug tester.
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
#include <sys/mount.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

int main(void) {
	struct stat sbuf;
	char rbuf[1024];
	unsigned int passes = 0;
	unsigned int errors = 0;

	mount("none", "/proc", "proc", MS_MGC_VAL, "");

	int fd = open("/proc/cpuinfo", O_RDONLY);
	if (fd != -1) {
		ssize_t n = read(fd, rbuf, 1022);
		close(fd);

		if (n) {
			rbuf[n] = '\0';
			printf("\n");
			printf(rbuf);
		}
	}

	fd = open("/code", O_RDONLY);
	if (fd == -1) {
		perror("open /code");
		return 1;
	}

	if (fstat(fd, &sbuf)) {
		perror("stat");
		return 1;
	}

	printf("\033[9;%d]", 0); /* disable console blanking */

	printf("\n");
	printf("This program tests for the infamous 68LC040 FPU emulation bug.\n");
	printf("Please reset the computer manually when you've seen enough passes.\n");
	printf("(Errors show up within a few hundred passes on my Mac LC 475. YMMV.)\n");
	printf("Testing will commence in 5 seconds.\n");
	printf("\n");

	sleep(5);

	while (1) {
		int result = 0;
		int dontcare = 0;

		printf("\r%d errors / %d passes", errors, passes);
		fflush(stdout);

		void *addr = mmap(NULL, sbuf.st_size, PROT_READ | PROT_EXEC,
		                  MAP_PRIVATE | MAP_NORESERVE, fd, 0);
		if (addr == (void *) -1) {
			perror("mmap");
			return 1;
		}

		/* call the test code that we mmap'd */

		asm("\t movel %2,%%a2 \n" \
		    "\t jsr   %1@     \n" \
		    "\t movel %%d0,%0 \n" \
		    : "=g" (result) : "a" (addr), "g" (&dontcare) : "d0", "a2");

		if (result != 1999) {
			printf("\n");
			printf("Incorrect floating point result (0x%08lx).\n", result);
			errors++;
		}

		if (munmap(addr, sbuf.st_size) == -1) {
			perror("munmap");
			return 1;
		}

		passes++;
	}
}
