/*
 * ALWAYS BE TESTING!!!
 *
 * ALWAYS BE WRITING MORE TESTS!!!
 *
 * THIS WORK IS **NEVER** DONE!!!
 */

#include "tinytest.h"
#include <toolshed.h>

#define TEST_STRING "Line1\nLine2\n"
#define TEST_PROGRAM "10 PRINT \"TEST \\xff\"\n20 GOTO 10\n"
#define LARGE_BUFFER 2048
char *test_string = TEST_STRING;
unsigned int test_string_size = sizeof TEST_STRING;
char *basic_program = TEST_PROGRAM;
unsigned int test_program_size = sizeof TEST_PROGRAM;

#define LARGE_FILE_SIZE 1000
char *large_file;
unsigned int large_file_size;

void test_cecb_bulkerase()
{
	error_code ec;

	// test creation of a non-existent wave file
	ec = _cecb_bulkerase("test.wav", 44100, 8, 1.0);
	ASSERT_EQUALS(0, ec);

	// test creation of a non-existent cas file
	ec = _cecb_bulkerase("test.cas", 0, 0, 0);
	ASSERT_EQUALS(0, ec);

	// test creation of a non-existent c10 file
	ec = _cecb_bulkerase("test.c10", 0, 0, 0);
	ASSERT_EQUALS(0, ec);
}

void test_cecb_create()
{
	cecb_path_id p_wav;
	cecb_path_id p_cas;
	cecb_path_id p_c10;
	error_code ec;
	unsigned int size;

	// Open file written in previous written
	ec = _cecb_create(&p_wav, "test.wav,FILE", FAM_READ | FAM_WRITE, 1,
			  255, 0, 0x55, 0x65);
	ASSERT_EQUALS(0, ec);

	// Open file written in previous written
	ec = _cecb_create(&p_cas, "test.cas,FILE", FAM_READ | FAM_WRITE, 1,
			  255, 0, 0x55, 0x65);
	ASSERT_EQUALS(0, ec);

	// Open file written in previous written
	ec = _cecb_create(&p_c10, "test.c10,FILE", FAM_READ | FAM_WRITE, 1,
			  255, 0, 0x55, 0x65);
	ASSERT_EQUALS(0, ec);

	// Write data to file
	size = test_string_size;
	ec = _cecb_write(p_wav, test_string, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(test_string_size, size);

	// Write data to file
	size = test_string_size;
	ec = _cecb_write(p_cas, test_string, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(test_string_size, size);

	// Write data to file
	size = test_string_size;
	ec = _cecb_write(p_c10, test_string, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(test_string_size, size);

	ec = _cecb_close(p_wav);
	ASSERT_EQUALS(0, ec);

	ec = _cecb_close(p_cas);
	ASSERT_EQUALS(0, ec);

	ec = _cecb_close(p_c10);
	ASSERT_EQUALS(0, ec);

	// Open file written in previous written
	ec = _cecb_create(&p_wav, "test.wav,LGFILE", FAM_READ | FAM_WRITE, 1,
			  255, 0, 0x55, 0x65);
	ASSERT_EQUALS(0, ec);

	// Open file written in previous written
	ec = _cecb_create(&p_cas, "test.cas,LGFILE", FAM_READ | FAM_WRITE, 1,
			  255, 0, 0x55, 0x65);
	ASSERT_EQUALS(0, ec);

	// Open file written in previous written
	ec = _cecb_create(&p_c10, "test.c10,LGFILE", FAM_READ | FAM_WRITE, 1,
			  255, 0, 0x55, 0x65);
	ASSERT_EQUALS(0, ec);
	
	// Write data to file
	size = large_file_size;
	ec = _cecb_write(p_wav, large_file, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(large_file_size, size);

	// Write data to file
	size = large_file_size;
	ec = _cecb_write(p_cas, large_file, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(large_file_size, size);

	// Write data to file
	size = large_file_size;
	ec = _cecb_write(p_c10, large_file, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(large_file_size, size);

	ec = _cecb_close(p_wav);
	ASSERT_EQUALS(0, ec);

	ec = _cecb_close(p_cas);
	ASSERT_EQUALS(0, ec);

	ec = _cecb_close(p_c10);
	ASSERT_EQUALS(0, ec);
}

void test_cecb_read()
{
	cecb_path_id p_wav;
	cecb_path_id p_cas;
	cecb_path_id p_c10;
	error_code ec;
	char buffer[LARGE_BUFFER];
	unsigned int size;

	// Opening in write mode is not supported
	ec = _cecb_open(&p_wav, "DoesNoteExit.wav,FILE",
			FAM_READ | FAM_WRITE);
	ASSERT_EQUALS(EOS_IC, ec);

	// Opening in write mode is not supported
	ec = _cecb_open(&p_wav, "DoesNoteExit.wav,FILE", FAM_READ);
	ASSERT_EQUALS(EOS_BPNAM, ec);

	// Opening existing file
	ec = _cecb_open(&p_cas, "DoesNoteExit.cas,FILE", FAM_READ);
	ASSERT_EQUALS(EOS_BPNAM, ec);

	// Opening existing file
	ec = _cecb_open(&p_wav, "test.wav,FILE", FAM_READ);
	ASSERT_EQUALS(0, ec);

	// Opening existing file
	ec = _cecb_open(&p_cas, "test.cas,FILE", FAM_READ);
	ASSERT_EQUALS(0, ec);

	// Opening existing file
	ec = _cecb_open(&p_c10, "test.c10,FILE", FAM_READ);
	ASSERT_EQUALS(0, ec);

	// Read file into buffer
	size = LARGE_BUFFER;
	ec = _cecb_read(p_wav, buffer, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(test_string_size, size);
	ASSERT_STRING_EQUALS(test_string, buffer);

	// Read file into buffer
	size = LARGE_BUFFER;
	buffer[0] = 0;
	ec = _cecb_read(p_cas, buffer, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(test_string_size, size);
	ASSERT_STRING_EQUALS(test_string, buffer);

	// Read file into buffer
	size = LARGE_BUFFER;
	buffer[0] = 0;
	ec = _cecb_read(p_c10, buffer, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(test_string_size, size);
	ASSERT_STRING_EQUALS(test_string, buffer);

	ec = _cecb_close(p_wav);
	ASSERT_EQUALS(0, ec);

	ec = _cecb_close(p_cas);
	ASSERT_EQUALS(0, ec);

	ec = _cecb_close(p_c10);
	ASSERT_EQUALS(0, ec);
	
	// Opening large file
	ec = _cecb_open(&p_wav, "test.wav,LGFILE", FAM_READ);
	ASSERT_EQUALS(0, ec);

	// Read file into buffer
	memset(buffer, 0, LARGE_BUFFER);
	size = LARGE_BUFFER;
	ec = _cecb_read(p_wav, buffer, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(LARGE_FILE_SIZE, size);
	ASSERT_MEM_EQUALS(large_file, buffer, size);

	ec = _cecb_close(p_wav);
	ASSERT_EQUALS(0, ec);
	
	// Opening large file
	ec = _cecb_open(&p_cas, "test.cas,LGFILE", FAM_READ);
	ASSERT_EQUALS(0, ec);

	// Read file into buffer
	memset(buffer, 0, LARGE_BUFFER);
	size = LARGE_BUFFER;
	ec = _cecb_read(p_cas, buffer, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(LARGE_FILE_SIZE, size);
	ASSERT_MEM_EQUALS(large_file, buffer, size);

	ec = _cecb_close(p_cas);
	ASSERT_EQUALS(0, ec);
	
	// Opening large file
	ec = _cecb_open(&p_c10, "test.c10,LGFILE", FAM_READ);
	ASSERT_EQUALS(0, ec);

	// Read file into buffer
	memset(buffer, 0, LARGE_BUFFER);
	size = LARGE_BUFFER;
	ec = _cecb_read(p_c10, buffer, &size);
	ASSERT_EQUALS(0, ec);
	ASSERT_EQUALS(LARGE_FILE_SIZE, size);
	ASSERT_MEM_EQUALS(large_file, buffer, size);

	ec = _cecb_close(p_c10);
	ASSERT_EQUALS(0, ec);
}

void test_cecb_token()
{
	error_code ec;
	unsigned char *ent_outbuf;
	u_int ent_out_size;

	char *det_outbuf;
	u_int det_out_size;

	ec = _cecb_entoken((unsigned char *)basic_program, test_program_size, &ent_outbuf, &ent_out_size, 0);
	ASSERT_EQUALS(0, ec);

	ec = _cecb_detoken(ent_outbuf, ent_out_size, &det_outbuf, &det_out_size);
	ASSERT_EQUALS(0, ec);
	
	ASSERT_EQUALS(test_program_size, det_out_size-2);

	ec = memcmp(basic_program, det_outbuf, test_program_size-1);
	ASSERT_EQUALS(0, ec);

	free(ent_outbuf);
	free(det_outbuf);
}

int main()
{
	remove("test.c10");
	remove("test.cas");
	remove("test.wav");

	// create large buffer of data
	large_file = malloc(LARGE_FILE_SIZE);
	if(!large_file)
	{
		fprintf( stderr, "Failure to allocate buffer.\n");
		return -1;
	}
	large_file_size = LARGE_FILE_SIZE;
	for(int i=0; i<LARGE_FILE_SIZE; i++)
	{
		large_file[i] = i;
	}

	RUN(test_cecb_bulkerase);
	RUN(test_cecb_create);
	RUN(test_cecb_read);
	RUN(test_cecb_token);

	free(large_file);
	remove("test.c10");
	remove("test.cas");
	remove("test.wav");

	return TEST_REPORT();
}
