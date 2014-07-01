/*
 *	Project: Custom part
 *
 *	File: custom_part.c
 *	Author: daemondzk@gmail.com
 *	Created:
 *
 *	Description:
 *  
 *  File contains driver main implementeation
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>

#include "part.h"

static char *parts = "";
module_param(parts, charp, 0000);
MODULE_PARM_DESC(parts, "Partitions table");

static int custom_part_init(void) 
{
	int result = 0;

#ifdef __MODULE_DEBUG__
	printk(KERN_INFO "custom_part: module loaded\n");

	printk(KERN_INFO "custom_part::parts: %s\n", parts);
#endif/*__MODULE_DEBUG__*/
	result = parse_partitions_info(parts) > 0 ? 0 : -EINVAL;
	if (!result)
	{
		setup_partition_hook(partition);
	}
	
	return result;
}

static void custom_part_exit(void) 
{
	setup_partition_hook(NULL);

#ifdef __MODULE_DEBUG__
	printk(KERN_INFO "custom_part: module exit\n");
#endif/*__MODULE_DEBUG__*/
}

MODULE_LICENSE("GPL");
module_init(custom_part_init);
module_exit(custom_part_exit);

