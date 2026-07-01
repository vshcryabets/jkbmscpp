Import("env")
from os.path import join, realpath

sources = ["+<JkBmsController.cpp>", "+<JkBmsFrames.cpp>", "+<JkBmsTools.cpp>"]
print("Sources: ", sources)
env.Replace(SRC_FILTER=sources)
