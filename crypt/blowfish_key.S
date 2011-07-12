.file "key.S"

.global blowfish_key
.global blowfish_key_size

.text
blowfish_key:
.incbin "blowfish_key"
blowfish_key_size:
.int . - blowfish_key


