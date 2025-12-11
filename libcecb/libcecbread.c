/********************************************************************
 * libcebcread.c - Cassette BASIC read routines
 *
 * $Id$
 ********************************************************************/

#include "cecbpath.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

error_code _cecb_read(cecb_path_id path, void *buffer, u_int * size)
{
	error_code ec = 0;
	u_int requested_bytes;
	char *b = (char *) buffer;

	/* 1. Check the mode. */

	if ((path->mode & FAM_READ) == 0)
	{
		return EOS_FNA;
	}


	/* 2. Treat raw path differently. */

	if (path->israw == 1)
	{
		return EOS_FNA;
	}

	requested_bytes = *size;
	*size = 0;

	if ((path->eof_flag == 1) && (path->current_pointer == path->block_length))
		return EOS_EOF;

	if ((path->eof_flag == 0) && (path->current_pointer == path->block_length))
		ec = _cecb_read_next_block(path);

	while (requested_bytes > 0)
	{
		if ((path->eof_flag == 1)
		    && (path->current_pointer == path->block_length))
			break;

		/* Fulfill request from buffer */
		if (path->current_pointer < path->block_length)
		{
			size_t copy_bytes;

			copy_bytes =
				MIN((path->block_length - path->current_pointer),
				    requested_bytes);

			memcpy(b + (*size),
			       &(path->data[path->current_pointer]),
			       copy_bytes);

			path->current_pointer += copy_bytes;
			*size += copy_bytes;
			requested_bytes -= copy_bytes;
			path->filepos += copy_bytes;
		}

		/* if buffer empty, get new block */
		if (path->current_pointer == path->block_length)
		{
			ec = _cecb_read_next_block(path);
			path->current_pointer = 0;

			if (ec != 0)
				return ec;

			if (path->block_type == 0xff)
				path->eof_flag = 1;	/* End of file */
		}
	}

	return ec;
}

error_code _cecb_readln(cecb_path_id path, void *buffer, u_int * size)
{
	error_code ec = 0;
	char *current;
	u_int read_size;
	u_int requested_bytes;

	current = buffer;
	requested_bytes = *size;
	*size = 0;

	while (requested_bytes > 0)
	{
		read_size = 1;
		ec = _cecb_read(path, current, &read_size);

		if (ec != 0)
			return ec;

		if (read_size != 1)
			break;

		current++;
		*size += 1;
		requested_bytes -= 1;

		if (*current == 0x0d)
			break;
	}

	return ec;
}

error_code _cecb_read_next_dir_entry(cecb_path_id path)
{
	error_code ec = 0;
// 	unsigned char data[256], calc_crc;
// 	unsigned char block_type, block_length;

	while (ec == 0)
	{
		ec = _cecb_read_next_block(path);

		if ((ec == EOS_CRC) || (ec == 0))
		{
			if ((path->block_type == 0)
			    && (path->block_length == sizeof(cecb_dir_entry)))
			{
				memcpy(&(path->dir_entry), path->data,
				       sizeof(cecb_dir_entry));
				path->current_pointer = path->block_length;	
				break;
			}
		}
	}

	return ec;
}

error_code _cecb_ncpy_name(cecb_dir_entry e, u_char * name, size_t len)
{
	error_code ec = 0;
	u_char c_name[9];

	DECBStringToCString(e.filename, (u_char *) " ", c_name);
	c_name[8] = 0;

	strncpy((char *) name, (const char *) c_name, len);

	return ec;

}

error_code _cecb_read_next_block(cecb_path_id path)
{
	error_code ec = 0;
	unsigned char find_block;
// 	unsigned char checksum_ck;
	int i;

	find_block = 0;

	while (find_block != 0x3c)
	{
		unsigned char newbit;

		find_block >>= 1;

		ec = _cecb_read_bits(path, 1, &newbit);

		if (ec != 0)
			return ec;

		find_block |= (unsigned short) newbit;
	}

	if (path->tape_type == WAV)
	{
		path->wav_cb_start = path->wav_current_sample;
	}
	else
	{
		path->cas_cb_start_byte = path->cas_current_byte;
		path->cas_cb_start_bit = path->cas_current_bit;
	}

	ec = _cecb_read_bits(path, 8, &(path->block_type));
	ec = _cecb_read_bits(path, 8, &(path->block_length));

	path->calc_cksum = path->block_type + path->block_length;

// 	fprintf( stderr, "Length: %d\n", path->block_length );

	for (i = 0; i < path->block_length; i++)
	{
		ec = _cecb_read_bits(path, 8, &(path->data[i]));
		path->calc_cksum += path->data[i];
// 		fprintf( stderr, "c: %02x (%c), i: %d\n", path->data[i], isprint(path->data[i]) ? path->data[i] : '.', i );
	}
	
	path->current_pointer = 0;
	ec = _cecb_read_bits(path, 8, &(path->embed_cksum));

	if (path->calc_cksum != path->embed_cksum)
		ec = EOS_CRC;

	return ec;
}

error_code _cecb_read_bits(cecb_path_id path, int count,
			   unsigned char *result)
{
	error_code ec;

	if ((path->tape_type == CAS) || (path->tape_type == C10))
		ec = _cecb_read_bits_cas(path, count, result);
	else if (path->tape_type == WAV)
		ec = _cecb_read_bits_wav(path, count, result);
	else
		ec = EOS_IA;

	return ec;
}
