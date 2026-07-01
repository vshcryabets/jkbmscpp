#!/usr/bin/python3

import subprocess
import sys

if "--prepare" in sys.argv:
    subprocess.run(
        ["cmake", "-B", "./build", "-DCMAKE_BUILD_TYPE=Release", "-S", "./samples/simpleBleSample"],
        check=True
    )
if "--build" in sys.argv:
    subprocess.run(
        ["cmake", "--build", "./build"],
        check=True
    )
if "--test" in sys.argv:
    subprocess.run(
        ["ctest"],
        cwd="./build",
        check=True
    )
