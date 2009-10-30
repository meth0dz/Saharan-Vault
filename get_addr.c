#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

long read_system_map(char * path, char ** buffer);
void * get_sct_addr(char * buffer, long file_size);

int main(int argc, char ** argv)
{
	char * map_file;
	void * addr;
	long file_bytes;
	if (argc == 2) {
		if (file_bytes = read_system_map(argv[1], &map_file)) {
			if (addr = get_sct_addr(map_file, file_bytes)) {
				printf("%X\n", (long)addr);
				return 0;
			}
		}		
	}
	return 1;
}

long read_system_map(char * path, char ** buffer)
{
	FILE * fp;
	long size = 0;
	if (fp = fopen(path, "r")) {
		if (!fseek(fp, 0, SEEK_END)) {
			if ((size = ftell(fp)) != -1L) {
				rewind(fp);
				if (*buffer = malloc(sizeof(char) * size + 1)) {
					if (fread(*buffer, 1, size, fp) == size) {
						fclose(fp);
						(*buffer)[size] = 0;
						return size;
					}
					free (*buffer);
				}
			}
		}
		fclose(fp);
	}
	return 0;
}

void * get_sct_addr(char * buffer, long file_size)
{
	char addr[15];
	unsigned long number;
	char * loc = strstr(buffer, "sys_call_table");
	while (loc[0] != '\n') loc--;
	loc++;
	sscanf(loc, "%s", addr);
	return (void*)strtoul(addr, NULL, 16);
}
