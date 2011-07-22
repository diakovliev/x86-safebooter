#! /bin/sh

rm ./dsa_*

openssl dsaparam 2048 < /dev/urandom > dsa_param.pem
openssl gendsa dsa_param.pem -out dsa_priv.pem
#openssl dsa -in dsa_priv.pem -pubout -out dsa_pub.pem

openssl dsa -in dsa_priv.pem -text -noout > dsa_keys.txt

cp -f dsa_keys.txt dsa_key.c
sed -i 's#\s*##g'	dsa_key.c
sed -i ':a;N;$!ba;s#:\n#:#g'	dsa_key.c
sed -i 's#^\([^0-9,^:]*\):#unsigned char dsa_\1[] = { 0x#g'	dsa_key.c
sed -i 's#:#,0x#g'	dsa_key.c
sed -i ':a;N;$!ba;s#\n# };\n#g'	dsa_key.c
sed -i ':a;N;$!ba;s#$# };#g'	dsa_key.c
sed -i "s#^.*Private-Key.*\$#/\* Generated at `date` \*/#g"	dsa_key.c
grep dsa_priv dsa_key.c > dsa_pkey.c
sed -i "s#^.*dsa_priv.*\$#/\* Do not modify \*/#g"	dsa_key.c

#echo "foobar" > foo.txt
#sha1sum < foo.txt | awk '{print $1}' > foo.sha1
#openssl dgst -dss1 -sign dsa_priv.pem foo.sha1 > sigfile.bin
#openssl dgst -dss1 -verify dsa_pub.pem -signature sigfile.bin foo.sha1
