#! /bin/sh
#
# Generating initramfs source file from the directory content
# (c)D.Iakovliev 2011
#

# hardlinks is not supported

print_usage() {
	echo "Usage: $0 dir [uid] [gid]"
	exit 1
}

dev_major() {
	echo $((`stat -c "0x%t" $1`))
}

dev_minor() {
	echo $((`stat -c "0x%T" $1`))
}

permissions() {
	perm=`find $1 -printf %#m`
	perm=${perm:0:4}
	echo $perm
}

slink_target() {
	echo "`find $1 -printf %l`"
}

[ ! -z "$1" ] || print_usage || exit 1
DIR=$1
OUID=0
OGID=0
[ -z "$2" ] || OUID=$2
[ -z "$3" ] || OGID=$3

for node in `find $DIR` ; do

	[ "$node" == "$DIR" ] && continue

	nodename=${node##$DIR}

	perm=`permissions $node`

	# devices
	if [ -b $node ] || [ -c $node ]; then
		
		indexes=$(echo "`dev_major $node` `dev_minor $node`")

		[ -b $node ] && echo "nod $nodename $perm $OUID $OGID b $indexes" && continue
		[ -c $node ] && echo "nod $nodename $perm $OUID $OGID c $indexes" && continue
	fi

	# directories
	[ -d $node ] && echo "dir $nodename $perm $OUID $OGID" && continue

	# symlinks
	if [ -h $node ]; then

		target=`slink_target $node`

		echo "slink $nodename $target $perm $OUID $OGID" && continue
	fi

	# files
	[ -f $node ] && echo "file $nodename $node $perm $OUID $OGID" && continue

done

