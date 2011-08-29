/* Linux boot protocol */
#ifndef LBP_HEADER
#define LBP_HEADER

#include <stdint.h>

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
	LBP_ALL(uint8_t		setup_sects);
	LBP_ALL(uint16_t	root_flags);
	LBP_204(uint32_t	syssize);
	LBP_ALL(uint16_t	ram_size);
	LBP_ALL(uint16_t	vid_mode);
	LBP_ALL(uint16_t	root_dev);
	LBP_ALL(uint16_t	boot_flag);
	LBP_200(uint16_t	jump);
	LBP_200(uint32_t	header);
	LBP_200(uint16_t	version);
	LBP_200(uint32_t	realmode_swtch);
	LBP_200(uint16_t	start_sys_reg);
	LBP_200(uint16_t	kernel_version);
	LBP_200(uint8_t		type_of_loader);
	LBP_200(uint8_t		loadflags);
	LBP_200(uint16_t	setup_move_size);
	LBP_200(uint32_t	code32_start);
	LBP_200(uint32_t	ramdisk_image);
	LBP_200(uint32_t	ramdisk_size);
	LBP_200(uint32_t	bootsect_knudge);
	LBP_201(uint16_t	heap_end_ptr);
	LBP_202(uint8_t		ext_loader_ver);
	LBP_202(uint8_t		ext_loader_type);
	LBP_202(uint32_t	cmd_line_ptr);
	LBP_203(uint32_t	ramdisk_max);
	LBP_205(uint32_t	kernel_alignment);
	LBP_205(uint8_t		relocatable_kernel);
	LBP_210(uint8_t		min_alignment);
	LBP_ALL(uint16_t	unused1);
	LBP_206(uint32_t	cmdline_size);
	LBP_207(uint32_t	hardware_subarch);
	LBP_207(uint64_t	hardware_subarch_data);
	LBP_208(uint32_t	payload_offset);
	LBP_208(uint32_t	payload_length);
	LBP_209(uint64_t	setup_data);
	LBP_210(uint64_t	pref_address);
	LBP_210(uint32_t	init_size);
} kernel_header_t __attribute__((packed));
#pragma pack(pop)
typedef kernel_header_t *kernel_header_p;

#define GET_KERNEL_HEADER_ADDRESS(x) (x+0x01f1)

#endif/*LBP_HEADER*/
