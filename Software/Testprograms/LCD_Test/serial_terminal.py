# -*- coding: utf-8 -*-
"""
Created on Wed Jun 29 00:16:40 2022

@author: Admin
"""

import subprocess
from pathlib import Path

ports = ["COM96", "COM97"]

for port in ports:
    list_files = subprocess.run(["wt.exe", "powershell", "python",
                                 Path.cwd() / "serial_console.py", port])
