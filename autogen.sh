#!/bin/sh

# Regenerate all necessary files...
aclocal &&
autoconf &&
autoheader &&
automake
if [ $? -ne 0 ]; then
echo "Error: Failed to regenerate Makefiles and configure scripts."
fi
