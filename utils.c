/*
 * utils.c
 *
 *  Created on: Feb 11, 2021
 *      Author: salvo
 */
#include "utils.h"

int parseConf(const char *path, struct vdeparms *parms, const int MAX_PARMS) {
	FILE * fp;
	char * line = NULL;
	char * found;
	size_t len = 0;

	fp = fopen(path, "r");
	if (fp == NULL) {
		printf("no file found!\n");
		return -1;
	}
	for (int i=0; i < MAX_PARMS; i++) {
		getline(&line, &len, fp);
		found = strsep(&line,"=");
		//printf("key: %s\t", found);

		found = strsep(&line,"=");

		//printf("value: %s", found);
		if (strlen(found) > 1) {
			found[strcspn(found, "\n")] = 0;
			if (*parms[i].value == NULL)
				*parms[i].value = found;
		}
	}

	fclose(fp);
	if (line)
		free(line);

	return 0;
}
