/*
 *  part.h
 */
#ifndef PART_HEADER
#define PART_HEADER

#include <linux/blkdev_part_hook.h>

#define MAX_PARTS_NAME	15
#define MAX_PARTS 		6

int parse_partitions_info(const char *parts);

int partition(struct parsed_partitions *state);

#endif/*PART_HEADER*/

