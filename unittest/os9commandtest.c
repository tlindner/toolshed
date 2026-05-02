/*
 * ALWAYS BE TESTING!!!
 *
 * ALWAYS BE WRITING MORE TESTS!!!
 *
 * THIS WORK IS **NEVER** DONE!!!
 */

#include "tinytest.h"
#include <toolshed.h>

void test_os9_command_format()
{
	error_code ec;
	native_path_id nativepath;
	u_int size;

	ec = system("../os9/os9 format -e test.dsk -l65000 > /dev/null 2>&1");
	ASSERT_EQUALS(0, ec);
	
	ec = _native_open(&nativepath, "test.dsk", FAM_READ);
	ASSERT_EQUALS(0, ec);

	ec = _native_gs_size(nativepath, &size);	
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(65000*256, size);

	ec = _native_close(nativepath);
	ASSERT_EQUALS(0, ec);
	
	ec = _os9_makdir("test.dsk,/CMDS");
	ASSERT_EQUALS(0, ec);
}

void test_os9_command_copy_metadata()
{
	error_code ec;
	struct stat native_stat;
	os9_path_id os9path;
	fd_stats os9_fstat;
	int os9_fstat_size = sizeof(os9_fstat);
	fd_stats converted_native_fstat = {0};
 
	/* Create a file on the disk image with known content */
	ec = _os9_create(&os9path, "test.dsk,meta.txt",
	                 FAM_READ | FAM_WRITE, FAP_READ | FAP_WRITE);
	ASSERT_EQUALS(0, ec);
	char *buf = "hello";
	u_int size = strlen(buf);
	ec = _os9_write(os9path, buf, &size);
	ASSERT_EQUALS(0, ec);
	ec = _os9_close(os9path);
	ASSERT_EQUALS(0, ec);
 
	/* Set the OS9 file's modification date to a fixed date in the 1980s:
	 * January 15, 1987 at 10:30 (OS9 fd_dat: year, month, day, hour, min).
	 * January is chosen to avoid DST ambiguity in the mktime round-trip. */
	ec = _os9_open(&os9path, "test.dsk,meta.txt", FAM_READ | FAM_WRITE);
	ASSERT_EQUALS(0, ec);
	ec = _os9_gs_fd(os9path, os9_fstat_size, &os9_fstat);
	ASSERT_EQUALS(0, ec);
	os9_fstat.fd_dat[0] = 87;  /* year: 1987 (years since 1900) */
	os9_fstat.fd_dat[1] = 1;   /* month: January */
	os9_fstat.fd_dat[2] = 15;  /* day */
	os9_fstat.fd_dat[3] = 10;  /* hour */
	os9_fstat.fd_dat[4] = 30;  /* minute */
	ec = _os9_ss_fd(os9path, os9_fstat_size, &os9_fstat);
	ASSERT_EQUALS(0, ec);
	ec = _os9_close(os9path);
	ASSERT_EQUALS(0, ec);
 
	/* Copy from OS9 image to native filesystem */
	ec = system("../os9/os9 copy test.dsk,meta.txt meta_out.txt > /dev/null 2>&1");
	ASSERT_EQUALS(0, ec);
 
	/* Read the OS9 file's modification time */
	ec = _os9_open(&os9path, "test.dsk,meta.txt", FAM_READ);
	ASSERT_EQUALS(0, ec);
	ec = _os9_gs_fd(os9path, os9_fstat_size, &os9_fstat);
	ASSERT_EQUALS(0, ec);
	ec = _os9_close(os9path);
	ASSERT_EQUALS(0, ec);
 
	/* Check that the native file's mtime matches the OS9 last_modified_time */
	ec = stat("meta_out.txt", &native_stat);
	ASSERT_EQUALS(0, ec);
	UnixToOS9Time(native_stat.st_mtime, (char *) converted_native_fstat.fd_dat);
	
// 	fprintf(stderr, "os9_fstat.fd_dat: %d %d %d %d %d\n", os9_fstat.fd_dat[0], os9_fstat.fd_dat[1],os9_fstat.fd_dat[2], os9_fstat.fd_dat[3], os9_fstat.fd_dat[4]);
// 	fprintf(stderr, "converted_native_fstat.fd_dat: %d %d %d %d %d\n", converted_native_fstat.fd_dat[0], converted_native_fstat.fd_dat[1],converted_native_fstat.fd_dat[2], converted_native_fstat.fd_dat[3], converted_native_fstat.fd_dat[4]);
	
	ASSERT_MEM_EQUALS(os9_fstat.fd_dat, converted_native_fstat.fd_dat, 5);
 
	remove("meta_out.txt");
}

int main()
{
	remove("test.dsk");
	RUN(test_os9_command_format);
	RUN(test_os9_command_copy_metadata);

	return TEST_REPORT();
}
