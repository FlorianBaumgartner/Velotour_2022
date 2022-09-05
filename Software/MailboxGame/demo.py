# -*- coding: utf-8 -*-
"""
Created on Mon Sep  5 14:25:31 2022

@author: Admin
"""


import subprocess
import os


use_tabs_console = False
compare_vid_pid_console = True
compare_Serial = False
usb_vid = 0x239A
usb_pid = 0x80AB
usb_serial = 0

command = ["python", "serial_terminal.py"]
command += [str(use_tabs_console), str(compare_vid_pid_console), str(compare_Serial), str(usb_vid), str(usb_pid), str(usb_serial)]
CREATE_NO_WINDOW = 0x08000000
process = os.popen(" ".join(command))
#subprocess.Popen(command, close_fds=True, creationflags=CREATE_NO_WINDOW, shell = True)


