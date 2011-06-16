/* Linux boot protocol */
#ifndef LBP_HEADER
#define LBP_HEADER

#include "loader_types.h"

/* Load linux kernel */
#define KERNEL_HDRS 0x53726448

#define LBP_ALL(x) x
#define LBP_200(x) x
#define LBP_201(x) x
#define LBP_202(x) x
#define LBP_203(x) x
#define LBP_204(x) x
#define LBP_205(x) x
#define LBP_206(x) x
#define LBP_207(x) x
#define LBP_208(x) x
#define LBP_209(x) x
#define LBP_210(x) x

#pragma pack(push,1)
typedef struct kernel_header_s {
	LBP_ALL(byte_t	setup_sects);
	LBP_ALL(word_t	root_flags);
	LBP_204(dword_t	syssize);
	LBP_ALL(word_t	ram_size);
	LBP_ALL(word_t	vid_mode);
	LBP_ALL(word_t	root_dev);
	LBP_ALL(word_t	boot_flag);
	LBP_200(word_t	jump);
	LBP_200(dword_t	header);
	LBP_200(word_t	version);
	LBP_200(dword_t	realmode_swtch);
	LBP_200(word_t	start_sys_reg);
	LBP_200(word_t	kernel_version);
	LBP_200(byte_t	type_of_loader);
	LBP_200(byte_t	loadflags);
	LBP_200(word_t	setup_move_size);
	LBP_200(dword_t	code32_start);
	LBP_200(dword_t	ramdisk_image);
	LBP_200(dword_t	ramdisk_size);
	LBP_200(dword_t	bootsect_knudge);
	LBP_201(word_t	heap_end_ptr);
	LBP_202(byte_t	ext_loader_ver);
	LBP_202(byte_t	ext_loader_type);
	LBP_202(dword_t	cmd_line_ptr);
	LBP_203(dword_t	ramdisk_max);
	LBP_205(dword_t	kernel_alignment);
	LBP_205(byte_t	relocatable_kernel);
	LBP_210(byte_t	min_alignment);
	LBP_ALL(word_t	unused1);
	LBP_206(dword_t	cmdline_size);
	LBP_207(dword_t	hardware_subarch);
	LBP_207(quad_t	hardware_subarch_data);
	LBP_208(dword_t	payload_offset);
	LBP_208(dword_t	payload_length);
	LBP_209(quad_t	setup_data);
	LBP_210(quad_t	pref_address);
	LBP_210(dword_t	init_size);
} kernel_header_t __attribute__((packed));
#pragma pack(pop)
typedef kernel_header_t *kernel_header_p;

#define GET_KERNEL_HEADER_ADDRESS(x) (x+0x01f1)

#endif/*LBP_HEADER*/
