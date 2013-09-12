COLOR 	= printf "\x1b[$1m"
BLACK	= $(call COLOR,30)
RED		= $(call COLOR,31)
GREEN	= $(call COLOR,32)
BROWN	= $(call COLOR,33)
BLUE	= $(call COLOR,34)
MAGENTA	= $(call COLOR,35)
CYAN	= $(call COLOR,34)
LGRAY	= $(call COLOR,36)

NORMAL	= printf "\x1b[0m"
REVERSE	= printf "\x1b[7m"

