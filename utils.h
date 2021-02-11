/*
 * utils.h
 *
 *  Created on: Feb 11, 2021
 *      Author: salvo
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <libvdeplug_mod.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int parseConf(const char *path, struct vdeparms *parms, const int MAX_PARMS);

#endif /* UTILS_H_ */
