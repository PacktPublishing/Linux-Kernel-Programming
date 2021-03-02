#!/bin/bash
# color.sh
# ***************************************************************
# * This program is part of the source code released for the book
# *  "Linux Kernel Programming"
# *  (c) Author: Kaiwan N Billimoria
# *  Publisher:  Packt
# *  GitHub repository:
# *  https://github.com/PacktPublishing/Linux-Kernel-Programming
# *
# * From: Ch 6 : Kernel and Memory Management Internals Essentials
# ****************************************************************
# * Brief Description:
# [K]ASLR = [Kernel] Address Space Layout Randomization
# This script provides basic color support
# ****************************************************************
# Ref: https://stackoverflow.com/questions/5947742/how-to-change-the-output-color-of-echo-in-linux
# [Ans by Drew Noakes]
# Useful Ref! https://i.stack.imgur.com/a2S4s.png

#--- Foreground Colors
fg_black() { tput setaf 0 
}
fg_red() { tput setaf 1
}
fg_orange() { tput setaf 202
}
fg_green() { tput setaf 22 
}
fg_purple() { tput setaf 53 
}
fg_yellow() { tput setaf 3
}
fg_blue() { tput setaf 4
}
fg_magenta() { tput setaf 5
}
fg_cyan() { tput setaf 6
}
fg_white() { tput setaf 7
}
fg_grey() { tput setaf 8
}
 
#--- Background Colors
bg_white() { tput setab 15
}
bg_gray() { tput setab 250
}
bg_red() { tput setab 1
}
bg_green() { tput setab 2
}
bg_yellow() { tput setab 3
}
bg_blue() { tput setab 4
}
bg_cyan() { tput setab 6
}

#  Reset text attributes to normal without clearing screen.
color_reset()
{ 
   tput sgr0 
} 

# Try me!
test_256()
{
for i in $(seq 0 255)
do
  tput setab $i
  printf '%03d ' $i
done
color_reset
}
