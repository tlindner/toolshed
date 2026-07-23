/********************************************************************
 * os9setdate.c - File system date/time set utility for OS-9
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cocotypes.h>
#include <os9path.h>
#include <toolshed.h>

#define MAX_IMAGES 2048

/* Help message */
static char const *const helpMessage[] = {
	"Syntax: setdate {[<opts>]} <image> [<image2> ...] [<YYMMDDhhmm>]]\n",
	"Usage:  Set date and time for all files/dirs and LSN0 on OS-9 disk images.\n",
	"Options:\n",
	"     -q        quiet mode (suppress output)\n",
	"     -t        touch mode (use current host system date/time)\n",
	NULL
};

/* Reads an FD sector at the specified LSN using read_lsn, updates fd_dat, and writes back using write_lsn */
static error_code update_fd_timestamp(os9_path_id path, int lsn, u_char date[5])
{
	fd_stats fd;

	/* Read FD sector using library read_lsn */
	if (read_lsn(path, lsn, &fd) != path->bps)
	{
		return EOS_SE;
	}

	/* Update creation/modification timestamp */
	memcpy(&(fd.fd_dat), date, 5);
	memcpy(&(fd.fd_creat), date, 3);
	
	/* Write updated FD sector using library write_lsn */
	if (write_lsn(path, lsn, &fd) != path->bps)
	{
		return EOS_WRITE;
	}

	return 0;
}

static error_code update_directory_timestamps(os9_path_id path, char *pathname, u_char date[5])
{
	error_code ec = 0;
	os9_dir_entry dentry;

	/* Reset directory path position to byte 0 */
	_os9_seek(path, 0, SEEK_SET);

	while (1)
	{
		/* Call _os9_read passing pointer to size */
		ec = _os9_readdir(path, &dentry);

		/* Exit on error or EOF (size returned as 0) */
		if (ec != 0)
		{
			break;
		}

		/* Skip empty directory entries */
		if (dentry.name[0] == 0)
		{
			continue;
		}

		/* Skip "." (single '.' with high-bit set: 0x2E | 0x80 = 0xAE) */
		if (dentry.name[0] == ('.' | 0x80))
		{
			continue;
		}

		/* Skip ".." (first char 0x2E, second char '.' with high-bit set: 0xAE) */
		if (dentry.name[0] == '.' && dentry.name[1] == ('.' | 0x80))
		{
			continue;
		}

		/* Extract target LSN using tool macro int3() */
		int child_lsn = int3(dentry.lsn);

		/* 1. Update timestamp in the child's File Descriptor block */
		ec = update_fd_timestamp(path, child_lsn, date);
		if (ec != 0)
		{
			break;
		}

		/* 2. Check if child is a directory; if so, open and recurse */
		u_char clean_name[D_NAMELEN];
		_os9_ncpy_name(dentry, clean_name, sizeof(clean_name));

		/* Construct child pathlist relative to raw root path */
		char sub_pathlist[512];
		snprintf(sub_pathlist, sizeof(sub_pathlist), "%s/%s", pathname, clean_name);

		os9_path_id sub_path;
		ec = _os9_open(&sub_path, sub_pathlist, FAM_READ | FAM_WRITE | FAM_DIR);
		if (ec == 0)
		{
			ec = update_directory_timestamps(sub_path, sub_pathlist, date);
			_os9_close(sub_path);
			if (ec != 0)
			{
				break;
			}
		}
	}

	/* Treat EOF cleanly */
	if (ec == EOS_EOF)
	{
		ec = 0;
	}

	return ec;
}

/* Helper to process a single image file */
static error_code process_image(char *argv0, char *imagePath, u_char date[5], int quiet)
{
	char rawPath[512];
	char *commaPos;

	/* Prepare path in raw mode format: strip after comma, append ,@ */
	snprintf(rawPath, sizeof(rawPath), "%s", imagePath);
	commaPos = strchr(rawPath, ',');
	if (commaPos != NULL)
	{
		*commaPos = '\0';
	}
	strncat(rawPath, ",@", sizeof(rawPath) - strlen(rawPath) - 1);

	os9_path_id rootPath;
	error_code ec = _os9_open(&rootPath, rawPath, FAM_READ | FAM_WRITE);
	if (ec != 0)
	{
		fprintf(stderr, "%s: error %d opening image '%s': %s\n",
			argv0, ec, imagePath, TSReportError(ec));
		return (ec);
	}

	/* 1. Update LSN0 (Disk Descriptor Sector) creation date using read_lsn and write_lsn */
	memcpy(&(rootPath->lsn0->dd_dat), date, 5);
	if (write_lsn(rootPath, 0, rootPath->lsn0) != rootPath->bps)
	{
		return EOS_SE;
	}

	/* 2. Update root directory's own File Descriptor LSN timestamp */
	ec = update_fd_timestamp(rootPath, int3(rootPath->lsn0->dd_dir), date);
	if (ec != 0 && quiet == 0)
	{
		fprintf(stderr, "%s: warning updating root dir FD on '%s': %s\n",
			argv0, imagePath, TSReportError(ec));
	}

	_os9_close(rootPath);

	/* 3. Recursively traverse all files/directories and update their FDs */
	os9_path_id rootDirPath;
	/* Prepare path in raw mode format: strip after comma, append , */
	snprintf(rawPath, sizeof(rawPath), "%s", imagePath);
	commaPos = strchr(rawPath, ',');
	if (commaPos != NULL)
	{
		*commaPos = '\0';
	}
	strncat(rawPath, ",", sizeof(rawPath) - strlen(rawPath) - 1);
	ec = _os9_open(&rootDirPath, rawPath, FAM_READ | FAM_WRITE | FAM_DIR);
	if (ec != 0)
	{
		fprintf(stderr, "%s: error %d opening image '%s': %s\n",
			argv0, ec, imagePath, TSReportError(ec));
		return (ec);
	}

	ec = update_directory_timestamps(rootDirPath, rawPath, date);
	if (ec != 0)
	{
		fprintf(stderr, "%s: error updating file tree on '%s': %s\n",
			argv0, imagePath, TSReportError(ec));
	}

	_os9_close(rootDirPath);

	if (quiet == 0 && ec == 0)
	{
		printf("%s: set date/time to %02d/%02d/%02d %02d:%02d\n",
			imagePath, date[0], date[1], date[2], date[3], date[4]);
	}

	return ec;
}

int os9setdate(int argc, char *argv[])
{
	char *imagePaths[MAX_IMAGES];
	int imageCount = 0;
	char *dateStr = NULL;
	u_char date[5] = {0, 0, 0, 0, 0};
	int quiet = 0;
	int useCurrentTime = 0;
	int i;

	/* Walk command line for options and arguments */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			char *p;
			for (p = &argv[i][1]; *p != '\0'; p++)
			{
				switch (*p)
				{
				case 'q':
					quiet = 1;
					break;
				case 't':
					useCurrentTime = 1;
					break;
				case 'h':
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
		else
		{
			/* Positional argument: distinguish date/time strings from image paths if -t is not set */
			if (i == (argc-1) && !useCurrentTime )
			{
				dateStr = argv[i];
			}
			else
			{
				if (imageCount >= MAX_IMAGES)
				{
					fprintf(stderr, "%s: error, maximum number of images (%d) exceeded\n", argv[0], MAX_IMAGES);
					return (1);
				}
				imagePaths[imageCount++] = argv[i];
			}
		}
	}

	if (imageCount == 0)
	{
		show_help(helpMessage);
		return (0);
	}

	/* Determine target date and time values */
	if (useCurrentTime)
	{
		time_t now = time(NULL);
		struct tm *tm_now = localtime(&now);

		date[0] = (u_char)(tm_now->tm_year % 100);
		date[1] = (u_char)(tm_now->tm_mon + 1);
		date[2] = (u_char)tm_now->tm_mday;
		date[3] = (u_char)tm_now->tm_hour;
		date[4] = (u_char)tm_now->tm_min;
	}
	else if (dateStr != NULL)
	{
		int year = 0, month = 0, day = 0, hour = 0, min = 0;
		if (sscanf(dateStr, "%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &min) != 5)
		{
			fprintf(stderr, "%s: invalid date format '%s'\n", argv[0], dateStr);
			return (1);
		}

		date[0] = (u_char)year;
		date[1] = (u_char)month;
		date[2] = (u_char)day;
		date[3] = (u_char)hour;
		date[4] = (u_char)min;
	}
	else
	{
		show_help(helpMessage);
		return (0);
	}

	/* Process all collected image files */
	error_code last_ec = 0;
	for (i = 0; i < imageCount; i++)
	{
		error_code ec = process_image(argv[0], imagePaths[i], date, quiet);
		if (ec != 0)
		{
			last_ec = ec;
		}
	}

	return (last_ec);
}
