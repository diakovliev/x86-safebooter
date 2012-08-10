#! /bin/sh

#--------------------------------------------------------------------
# (C) D.Iakovliev 2011 (daemondzk@gmail.com)
#--------------------------------------------------------------------

TARGET_DIR=.

# Cleanup
rm -f $TARGET_DIR/dsa_*
rm -f $TARGET_DIR/rsa_*

#---------------------------- DSA -----------------------------------
# Generate dsa_*.pem
openssl dsaparam 1024 < /dev/urandom > dsa_param.pem
openssl gendsa dsa_param.pem -out dsa_priv.pem
#openssl dsa -in dsa_priv.pem -pubout -out dsa_pub.pem

# Generate dsa_key.c & dsa_pkey.c
openssl dsa -in dsa_priv.pem -text -noout >					$TARGET_DIR/dsa_key.c
sed -i 's#\s*##g'											$TARGET_DIR/dsa_key.c
sed -i ':a;N;$!ba;s#:\n#:#g'								$TARGET_DIR/dsa_key.c
sed -i 's#^\([^0-9,^:]*\):#unsigned char dsa_\1[] = { 0x#g'	$TARGET_DIR/dsa_key.c
sed -i 's#:#,0x#g'											$TARGET_DIR/dsa_key.c
sed -i ':a;N;$!ba;s#\n# };\n#g'								$TARGET_DIR/dsa_key.c
sed -i ':a;N;$!ba;s#$# };#g'								$TARGET_DIR/dsa_key.c
sed -i "s#^.*Private-Key.*\$#/\* Generated at `date` \*/#g"	$TARGET_DIR/dsa_key.c
grep dsa_priv $TARGET_DIR/dsa_key.c > 						$TARGET_DIR/dsa_pkey.c
sed -i "s#^.*dsa_priv.*\$#/\* Do not modify \*/#g"			$TARGET_DIR/dsa_key.c

echo "unsigned int dsa_pub_size = sizeof(dsa_pub) / sizeof(dsa_pub[0]);"	>> $TARGET_DIR/dsa_key.c 
echo "unsigned int dsa_P_size = sizeof(dsa_P) / sizeof(dsa_P[0]);"			>> $TARGET_DIR/dsa_key.c
echo "unsigned int dsa_Q_size = sizeof(dsa_Q) / sizeof(dsa_Q[0]);"			>> $TARGET_DIR/dsa_key.c
echo "unsigned int dsa_G_size = sizeof(dsa_G) / sizeof(dsa_G[0]);"			>> $TARGET_DIR/dsa_key.c

echo "unsigned int dsa_priv_size = sizeof(dsa_priv) / sizeof(dsa_priv[0]);"	>> $TARGET_DIR/dsa_pkey.c

# Test sign & verify
#echo "foobar" > foo.txt
#sha1sum < foo.txt | awk '{print $1}' > foo.sha1
#openssl dgst -dss1 -sign dsa_priv.pem foo.sha1 > sigfile.bin
#openssl dgst -dss1 -verify dsa_pub.pem -signature sigfile.bin foo.sha1

#---------------------------- RSA -----------------------------------
openssl genrsa -out rsa_priv.pem 1024
openssl rsa -in rsa_priv.pem -text -noout >					$TARGET_DIR/rsa_key.c
sed -i 's#\s*##g'											$TARGET_DIR/rsa_key.c
sed -i ':a;N;$!ba;s#:\n#:#g'								$TARGET_DIR/rsa_key.c
sed -i 's#^\([^:]*\):#unsigned char rsa_\1[] = { 0x#g'		$TARGET_DIR/rsa_key.c
sed -i 's#:#,0x#g'											$TARGET_DIR/rsa_key.c
sed -i ':a;N;$!ba;s#\n# };\n#g'								$TARGET_DIR/rsa_key.c
sed -i ':a;N;$!ba;s#$# };#g'								$TARGET_DIR/rsa_key.c
sed -i "s#^.*Private-Key.*\$#/\* Generated at `date` \*/#g"	$TARGET_DIR/rsa_key.c

echo "/* RSA private key generated at `date` */" > $TARGET_DIR/rsa_priv.c
grep modulus $TARGET_DIR/rsa_key.c >> $TARGET_DIR/rsa_priv.c
grep private $TARGET_DIR/rsa_key.c >> $TARGET_DIR/rsa_priv.c
echo "unsigned int rsa_modulus_size = sizeof(rsa_modulus) / sizeof(rsa_modulus[0]);" >> $TARGET_DIR/rsa_priv.c 
echo "unsigned int rsa_privateExponent_size = sizeof(rsa_privateExponent) / sizeof(rsa_privateExponent[0]);" >> $TARGET_DIR/rsa_priv.c 

echo "/* RSA public key generated at `date` */" > $TARGET_DIR/rsa_pub.c
grep modulus $TARGET_DIR/rsa_key.c >> $TARGET_DIR/rsa_pub.c
grep public $TARGET_DIR/rsa_key.c >> $TARGET_DIR/rsa_pub.c
echo "unsigned int rsa_modulus_size = sizeof(rsa_modulus) / sizeof(rsa_modulus[0]);" >> $TARGET_DIR/rsa_pub.c 
echo "unsigned int rsa_publicExponent_size = sizeof(rsa_publicExponent) / sizeof(rsa_publicExponent[0]);" >> $TARGET_DIR/rsa_pub.c 
