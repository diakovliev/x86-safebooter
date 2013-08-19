.file "xor_key.S"

.global xor_key
.global xor_key_size

.text
xor_key:
.incbin "xor_key"
xor_key_size:
.int . - xor_key
