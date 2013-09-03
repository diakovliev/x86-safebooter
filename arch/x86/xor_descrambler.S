/* %ax - buffer */
/* %bx - buffer_size */
xor_descramble_memory:
    mov   %eax, mbr_xor_buffer_ptr
    mov   %eax, mbr_xor_buffer_stored_ptr
    mov   %ebx, mbr_xor_buffer_size
	mov   $mbr_xor_key, %eax
	mov   %eax, mbr_xor_key_ptr
	mov   %eax, mbr_xor_key_stored_ptr
    jmp   check_buffer_bounds

xor_loop:
 	mov   mbr_xor_buffer_ptr,%eax
 	xor   %ecx,%ecx
    movb  (%eax), %cl
 	mov   mbr_xor_key_ptr,%eax
 	xor   %ebx,%ebx
    movb  (%eax), %bl

	movb  %cl,mbr_xor_b_next
	xor   mbr_xor_b,%bl

    xor   %bl, %cl
 	mov   mbr_xor_buffer_ptr,%eax
    movb  %cl, (%eax)

	movb  mbr_xor_b_next,%cl
	movb  %cl,mbr_xor_b

    incl  mbr_xor_key_ptr

    mov   mbr_xor_key_size, %eax
    dec   %eax
    add   mbr_xor_key_stored_ptr, %eax
    mov   mbr_xor_key_ptr, %ebx
    cmp   %ebx, %eax
    jae   inc_buffer_ptr

	mov   mbr_xor_key_stored_ptr, %eax
	mov   %eax, mbr_xor_key_ptr

inc_buffer_ptr:
    incl  mbr_xor_buffer_ptr

check_buffer_bounds:
    mov   mbr_xor_buffer_stored_ptr, %eax
    add   mbr_xor_buffer_size, %eax
    mov   mbr_xor_buffer_ptr, %ebx
    cmp   %ebx, %eax
	ja xor_loop
    ret

fail:
	cli
	hlt

/* Variables */
mbr_xor_key_ptr:			.int 0x00000000
mbr_xor_key_stored_ptr:		.int 0x00000000
mbr_xor_buffer_ptr:			.int 0x00000000
mbr_xor_buffer_stored_ptr: 	.int 0x00000000
mbr_xor_buffer_size:		.int 0x00000000
mbr_xor_b:					.byte 0x00
mbr_xor_b_next:				.byte 0x00
mbr_xor_key: 				.incbin "../../mbr_xor_key"
mbr_xor_key_size: 			.int . - mbr_xor_key

