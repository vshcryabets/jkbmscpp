Import("env")
from os.path import join, realpath

sources = ["+<JkBmsController.cpp>", "+<JkBmsFrames.cpp>", "+<JkBmsTools.cpp>"]


env.Replace(SRC_FILTER=sources)
