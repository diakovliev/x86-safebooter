.file "mbr_xor_key.S"

.global mbr_xor_key
.global mbr_xor_key_size

.text
mbr_xor_key:
.incbin "mbr_xor_key"
mbr_xor_key_size:
.int . - mbr_xor_key
