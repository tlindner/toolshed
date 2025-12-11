/********************************************************************
 * $Id$
 ********************************************************************/

#include "toolshed.h"
#include "cocotypes.h"

int displayASCII;
int displayHeader;
int displayLabel;
u_int dumpchunk;

void dumpDECB(u_char * buffer, size_t num_bytes,
		     enum dumpformat format)
{
	int length;
	u_int address;
	int count = 0;

	while (count < num_bytes)
	{
		if (buffer[count] == 0)
		{
			count++;
			length = buffer[count++] << 8;
			length += buffer[count++];
			address = buffer[count++] << 8;
			address += buffer[count++];

			dump_header(format);
			dump(&(buffer[count]), address, length, format);
			printf("\n");
			count += length;
		}
		else if (buffer[count] == 0xff)
		{
			count++;
			length = buffer[count++] << 8;
			length += buffer[count++];
			address = buffer[count++] << 8;
			address += buffer[count++];

			printf("\nExecution address: $%04X\n", address);

			int remain = num_bytes - count;

			if (remain > 0)
			{
				printf("%d bytes extra at end of file.\n",
				       remain);
			}
			break;
		}
		else
		{
			printf("Aborting: expected 0 or 255 for block header\n");
			break;
		}
	}
}

void dump(u_char * buffer, u_int offset, size_t num_bytes,
		 enum dumpformat format)
{
	u_int i, j;

	for (i = 0, j = 0; i < num_bytes; i += dumpchunk)
	{
		if (j > 255)
		{
			dump_header(format);
			j = 0;
		}
		if (format == FMT_STANDARD)
		{
			if (displayLabel == 1)
			{
				printf("\n%08x  ", offset);
			}
			else
			{
				printf("\n");
			}
		}
		else if (format == FMT_ASMHEX || format == FMT_ASMBIN)
		{
			if (displayLabel == 1)
			{
				printf("\nL%04X    fcb   ", offset);
			}
			else
			{
				printf("\n         fcb   ");
			}
		}
		else if (format == FMT_C)
		{
			if (displayLabel == 1)
			{
				printf("\n   /* offset = %08X */ ", offset);
			}
			else
			{
				printf("\n   ");
			}
		}

		if (num_bytes - i > dumpchunk)
		{
			dump_line(&buffer[i], dumpchunk, format);
		}
		else
		{
			dump_line(&buffer[i], num_bytes - i, format);
		}

		offset += dumpchunk;

		j += dumpchunk;

	}

	return;
}

void dump_line(u_char * buffer, u_int count, enum dumpformat format)
{
	int i;
	int carry = 0;

	if (count % 2 != 0)
	{
		count--;
		carry = 1;
	}
	for (i = 0; i < count; i += 2)
	{
		switch (format)
		{
		case FMT_STANDARD:
			printf("%02x%02x ", buffer[i], buffer[i + 1]);
			break;

		case FMT_ASMHEX:
			if (i == count - 2 && carry == 0)
			{
				printf("$%02X,$%02X", buffer[i],
				       buffer[i + 1]);
			}
			else
			{
				printf("$%02X,$%02X,", buffer[i],
				       buffer[i + 1]);
			}
			break;

		case FMT_ASMBIN:
			if (i == count - 2 && carry == 0)
			{
				printf("%%%s,%%%s", binary(buffer[i]),
				       binary(buffer[i + 1]));
			}
			else
			{
				printf("%%%s,%%%s,", binary(buffer[i]),
				       binary(buffer[i + 1]));
			}
			break;

		case FMT_C:
			{
				printf("0x%02X,0x%02X,", buffer[i],
				       buffer[i + 1]);
			}
			break;
		}
	}

	if (carry == 1)
	{
		switch (format)
		{
		case FMT_STANDARD:
			printf("%02x", buffer[i]);
			break;

		case FMT_ASMHEX:
			printf("$%02X", buffer[i]);
			break;

		case FMT_ASMBIN:
			printf("%%%s", binary(buffer[i]));
			break;

		case FMT_C:
			printf("0x%02X,", buffer[i]);
			break;
		}
		count++;
	}
	if (displayASCII == 1)
	{
		/* make spaces available if last line is not full */
		i = (dumpchunk - count);

		if (format == FMT_ASMHEX)
		{
			printf("   ");
		}
		if (i % 2 != 0)
		{
			switch (format)
			{
			case FMT_ASMHEX:
				printf("     ");
				break;

			case FMT_C:
				printf("     ");
				break;

			default:
				printf("   ");
				break;
			}
		}
		i /= 2;

		while (i--)
		{
			if (format == FMT_ASMHEX)
			{
				printf("        ");
			}
			else if (format == FMT_C)
			{
				printf("          ");
			}
			else
			{
				printf("     ");
			}
		}

		if (format == FMT_C)
		{
			printf("  // ");
		}

		/* print character dump on right side */
		for (i = 0; i < count; i++)
		{
			if (buffer[i] >= 32 && buffer[i] < 127)
			{
				printf("%c", buffer[i]);
			}
			else if (buffer[i] >= 128 + 32
				 && buffer[i] <= 128 + 'z')
			{
				printf("%c", buffer[i] - 128);
			}
			else
			{
				printf(".");
			}
		}
	}
	return;
}

void dump_header(enum dumpformat format)
{
	if (format == FMT_STANDARD && displayHeader == 1)
	{
		printf("\n\n");
		if (displayLabel)
		{
			printf("  Addr    ");
		}
		printf(" 0 1  2 3  4 5  6 7  8 9  A B  C D  E F");
		if (displayASCII == 1)
		{
			printf(" 0 2 4 6 8 A C E");
		}
		printf("\n");

		if (displayLabel)
		{
			printf("--------  ");
		}
		printf("---- ---- ---- ---- ---- ---- ---- ----");
		if (displayASCII == 1)
		{
			printf(" ----------------");
		}
	}
	return;
}

char *binary(char s)
{
	static char buffer[9] =
		{ '0', '0', '0', '0', '0', '0', '0', '0', '\0' };
	int i;

	for (i = 0; i < 8; i++)
	{
		int x = s & (1 << (7 - i));

		if (x != 0)
		{
			buffer[i] = '1';
		}
		else
		{
			buffer[i] = '0';
		}
	}

	return (buffer);
}
