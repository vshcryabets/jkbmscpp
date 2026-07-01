Import("env")

sources = ["+<JkBmsController.cpp>", "+<JkBmsFrames.cpp>", "+<JkBmsTools.cpp>"]
env.Replace(SRC_FILTER=sources)
