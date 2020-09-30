/**
 * A brief explanation of what this program does.
 * Add a license information here.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include <project.h>

int main(int argc, char *argv[])
{
	(void) argc;

#if defined(_MSC_VER)
	/* Do not wait until program exit to print stderr text. */
	setvbuf(stderr, NULL, _IONBF, 0);
#endif

	puts("Hello");
	fprintf(stderr, "%s %" PRIu8 "!\n", argv[0], CROSS_MODULE_NUMBER);
	return EXIT_SUCCESS;
}
