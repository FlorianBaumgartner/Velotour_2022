###############################################################################
# file    serial_terminal.py
###############################################################################
# brief   Uses Windows 10 Terminal to open specified COM-Ports in multiple tabs
###############################################################################
# author  Florian Baumgartner
# version 1.0
# date    2022-08-02
###############################################################################
# MIT License
#
# Copyright (c) 2022 Crelin - Florian Baumgartner
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell          
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
###############################################################################

import subprocess
from pathlib import Path
from serial_console import Console

COMPARE_VID = 0x239A
COMPARE_PID = 0x80AB
COMPARE_SER = ["0", "1", "2"]

ENABLE_COMP_USB = True
ENABLE_COMP_SER = False

ports = []
for p in Console.getPorts():
    if(ENABLE_COMP_USB and (p.vid != COMPARE_VID or p.pid != COMPARE_PID)):
        continue
    if(ENABLE_COMP_SER and (p.ser not in COMPARE_SER)):
        continue
    ports.append(p.port)
        

for i, port in enumerate(ports):
    list_files = subprocess.run(["wt.exe", "--title", port, "python",
                                 Path.cwd() / "serial_console.py", port])
