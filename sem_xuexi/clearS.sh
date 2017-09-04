#########################################################################
# File Name: clearS.sh
# Author: abc2514671
# mail: abc2514671@163.com
# Created Time: Sun 20 Aug 2017 04:27:08 PM CST
#########################################################################
#!/bin/bash
echo `ipcrm -M 256`
echo `ipcrm -M 257`
echo `ipcrm -S 256`
echo `ipcrm -S 257`
echo `ipcrm -S 258`
