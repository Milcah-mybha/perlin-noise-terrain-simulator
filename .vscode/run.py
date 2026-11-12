#!/usr/bin/env python3
import os
import subprocess
import sys

project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

if os.name == 'nt':
    ps_script = os.path.join(project_root, '.vscode', 'run-project.ps1')
    proc = subprocess.run(['powershell', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', ps_script])
    sys.exit(proc.returncode)
else:
    cmd = f'make -C "{project_root}" && "{project_root}/terrain_simulator"'
    proc = subprocess.run(cmd, shell=True)
    sys.exit(proc.returncode)
