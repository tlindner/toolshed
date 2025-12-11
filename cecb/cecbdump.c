/********************************************************************
 * cecbdump.c - Wrapper for os9dump() utility for Cassette BASIC
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cocotypes.h>
#include <toolshed.h>
#include <cecbpath.h>

static int do_cecb_dump(char **argv, char *file);

/* Help message */
static char const *const helpMessage[] = {
	"Syntax: dump {[<opts>]} {<file> [<...>]} {[<opts>]}\n",
	"Usage:  Display detailed information about a file.\n",
	"Options:\n",
	NULL
};


int cecbdump(int argc, char *argv[])
{
	int i;
	char *p = NULL;
	error_code ec = 0;

	if (argv[1] == NULL)
	{
		show_help(helpMessage);
		return (0);
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

		ec = do_cecb_dump(argv, p);

		if (ec != 0)
		{
			fprintf(stderr, "%s: error %d opening '%s'\n",
				argv[0], ec, p);
			return (ec);
		}
	}

	return (0);
}

static int do_cecb_dump(char **argv, char *file)
{
	error_code ec;
	coco_path_id path;
	_path_type path_type;
// 	unsigned char calc_crc;

	ec = _coco_open(&path, file, FAM_READ);

	if(ec == 0)
	{
		ec = _coco_gs_pathtype(path, &path_type);
	
		if (ec != 0)
		{
			return ec;
		}
	
		if (path_type == CECB)
		{
// 			format = FMT_STANDARD;
			displayASCII = 1;
			displayHeader = 1;
			displayLabel = 1;
			dumpchunk = 16;

			cecb_path_id cecb_path = path->path.cecb;
			
			while(ec == 0 || ec == EOS_CRC)
			{
				ec = _cecb_read_next_block(cecb_path);
				
				if( cecb_path->tape_type == WAV)
				{
					printf("Start sample:   %ld\n", cecb_path->wav_cb_start );
				}
				else
				{
					printf("Start byte:     %ld, bit: %d\n", cecb_path->cas_cb_start_byte, cecb_path->cas_cb_start_bit );
				}
				
				printf( "Block type:     %3d ($%02x)\n", cecb_path->block_type, cecb_path->block_type );
				printf( "Block length:   %3d ($%02x)\n", cecb_path->block_length, cecb_path->block_length );
				printf( "Embedded cksum: %3d ($%02x)\n", cecb_path->embed_cksum, cecb_path->embed_cksum );
				printf( "Calc cksum:     %3d ($%02x)\n", cecb_path->calc_cksum, cecb_path->calc_cksum );
				
				dump(cecb_path->data, 0, cecb_path->block_length, FMT_STANDARD);
				printf("\n\n");
			}
		}
		else
		{
			fprintf( stderr, "Dump in this mode requires a cassette file.\n");
			ec = 0;
		}
		
		ec = _coco_close(path);
	}

	return (ec);
}