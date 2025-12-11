/********************************************************************
 * os9dump.c - OS-9 dump utility
 *
 * $Id$
 ********************************************************************/

#include "util.h"
#include "cocopath.h"
#include "cocotypes.h"
#include "toolshed.h"
#include "cococonv.h"

static int decbBinary;

#define BUFFSIZ	256

static int do_dump(char **argv, char *file, enum dumpformat format);

/* Help message */
static char const *const helpMessage[] = {
	"Syntax: dump {[<opts>]} {<file> [<...>]} {[<opts>]}\n",
	"Usage:  Display the contents of a file in hexadecimal.\n",
	"Options:\n",
	"     -a    dump output in assembler format (hex)\n",
	"     -b    dump output in assembler format (binary)\n",
	"     -c    don't display ASCII character data\n",
	"     -e    dump output in C format\n",
	"     -t    don't display header\n",
	"     -h    don't display header (DEPRECATED)\n",
	"     -l    don't display line label/count\n",
	"     -z    decode DECB binary\n",
	NULL
};

int os9dump(int argc, char **argv)
{
	static enum dumpformat format;
	error_code ec = 0;
	char *p = NULL;
	int i;

	format = FMT_STANDARD;
	displayASCII = 1;
	displayHeader = 1;
	displayLabel = 1;
	dumpchunk = 16;
	decbBinary = 0;

	if (argv[1] == NULL)
	{
		show_help(helpMessage);
		return (0);
	}
	/* walk command line for options */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			for (p = &argv[i][1]; *p != '\0'; p++)
			{
				switch (*p)
				{
				case 'a':
					format = FMT_ASMHEX;
					dumpchunk = 8;
					break;

				case 'b':
					format = FMT_ASMBIN;
					dumpchunk = 1;
					break;

				case 'c':
					displayASCII = 0;
					break;

				case 'e':
					format = FMT_C;
					dumpchunk = 8;
					break;

				case 'h':
					fprintf(stderr,
						"%s: warning: option -h is deprecated, use -t\n",
						argv[0]);
					/* fallthrough */
				case 't':
					displayHeader = 0;
					break;

				case 'l':
					displayLabel = 0;
					break;

				case 'z':
					decbBinary = 1;
					break;

				case '?':
					show_help(helpMessage);
					return (0);

				default:
					fprintf(stderr,
						"%s: unknown option '%c'\n",
						argv[0], *p);
					return (0);
				}
			}
		}
	}

	/* walk command line for pathnames */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			continue;
		}
		else
		{
			p = argv[i];
		}

		ec = do_dump(argv, p, format);

		if (ec != 0)
		{
			fprintf(stderr, "%s: error %d opening '%s'\n",
				argv[0], ec, p);
			return (ec);
		}
	}

	return (0);
}

static int do_dump(char **argv, char *file, enum dumpformat format)
{
	error_code ec = 0;
	u_char buffer[BUFFSIZ], *tot_buffer;
	u_int tot_bytes = 0;
	coco_path_id path;

	/* 1. Open a path to the file. */
	ec = _coco_open(&path, file, FAM_READ);

	if (ec != 0)
	{
		ec = _coco_open(&path, file, FAM_DIR | FAM_READ);

		if (ec != 0)
		{
			fprintf(stderr, "%s: cannot open file\n", argv[0]);
			return (ec);
		}
	}

	if (format == FMT_C)
	{
		printf("{");
	}

	/* 2. Determine file size by reading until error */

	while (1)
	{
		u_int num_bytes;

		num_bytes = BUFFSIZ;
		ec = _coco_read(path, buffer, &num_bytes);
		tot_bytes += num_bytes;
		if (ec != 0)
		{
			break;
		}
	}

	/* 2. Rewind and dump the file */

	_coco_seek(path, 0, SEEK_SET);

	tot_buffer = malloc(tot_bytes);

	if (tot_buffer != 0)
	{

		ec = _coco_read(path, tot_buffer, &tot_bytes);

		if (decbBinary)
		{
			dumpDECB(tot_buffer, tot_bytes, format);
		}
		else
		{
			dump_header(format);
			dump(tot_buffer, 0, tot_bytes, format);
		}

		free(tot_buffer);
	}
	else
	{
		fprintf(stderr, "No memory to open: %s.\n", file);
	}

	if (format == FMT_C)
	{
		printf("\n}");
	}

	printf("\n");

	ec = _coco_close(path);

	return ec;
}

