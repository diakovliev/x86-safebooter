/*
 *  part.c
 *
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "part.h"

struct partition_info_s {
	int idx;
	int start;
	int size;
	char name[MAX_PARTS_NAME+1];
};

struct partition_info_s *partitions_info(void)
{
	static struct partition_info_s partitions[MAX_PARTS];
	return partitions;
}

int parse_partitions_info(const char *parts)
{
	char buffer[MAX_PARTS_NAME+1];
	char *buffer_ptr = buffer;
	char *parts_copy = NULL;
	char *ptr = NULL;
	int index = 0;
	struct partition_info_s *partitions = partitions_info();

	parts_copy = kstrdup(parts,GFP_KERNEL);

	while ((parts_copy != NULL) && ((ptr = strsep(&parts_copy, ",")) != NULL))
	{
		if (index >= MAX_PARTS)
		{
			printk(KERN_INFO "custom_part::reach the limit of the partitions count (parts: %s), allowed maximum %d partitions, rest will be ignored\n", parts, MAX_PARTS);
			++index;
			break;
		}

#ifdef __MODULE_DEBUG__
		printk(KERN_INFO "custom_part::sep[%d]: %s\n", index, ptr);
#endif/*__MODULE_DEBUG__*/
		memset(buffer, 0, MAX_PARTS_NAME+1);		
		buffer_ptr = buffer;

		partitions[index].idx = index + 1;

		while (*ptr)
		{
			if(*ptr == '@')
			{
				partitions[index].start = simple_strtol(buffer, NULL, 0);
				buffer_ptr = buffer;
				memset(buffer, 0, MAX_PARTS_NAME+1);
			}
			else
			if(*ptr == '(')
			{
				partitions[index].size = simple_strtol(buffer, NULL, 0);
				buffer_ptr = buffer;
				memset(buffer, 0, MAX_PARTS_NAME+1);
			}
			else
			if(*ptr == ')')
			{
				strncpy(partitions[index].name,buffer,MAX_PARTS_NAME);
				buffer_ptr = buffer;
				memset(buffer, 0, MAX_PARTS_NAME+1);
			}
			else
			if (buffer_ptr - buffer <= MAX_PARTS_NAME)
			{
				*buffer_ptr = *ptr;
				buffer_ptr++;
			}
			++ptr;
		}

#ifdef __MODULE_DEBUG__
		printk(KERN_INFO "custom_part::parser: [%s] %d - %d\n",  partitions[index].name,
							partitions[index].start, 
							partitions[index].size);
#endif/*__MODULE_DEBUG__*/

		++index;
	}

	if (parts_copy != NULL) 
	{
		kfree(parts_copy);
	}

	return index ? index : -EINVAL;
}

int partition(struct parsed_partitions *state)
{
	int i;

	struct partition_info_s *partitions = partitions_info();

	for (i = 0; i < MAX_PARTS; ++i)
	{
		if (partitions[i].idx == 0) break;

		put_partition(state, partitions[i].idx, 
							partitions[i].start, 
							partitions[i].size );
#ifdef __MODULE_DEBUG__
		printk(KERN_INFO "[%s] %d - %d\n",  partitions[i].name,
							partitions[i].start, 
							partitions[i].start + partitions[i].size - 1 );
#endif/*__MODULE_DEBUG__*/
	}

	strlcat(state->pp_buf, "\nMake partitions done\n", PAGE_SIZE);

	return 1;
}

