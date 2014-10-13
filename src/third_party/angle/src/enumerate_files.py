import fnmatch
import os
import sys

rootdirs = [ ]
filetypes = [ ]
excludes = [ ]

foundTypesArg = False
foundExcludesArg = False
for i in range(1, len(sys.argv)):
    arg = sys.argv[i]
    if arg == "-types":
        foundTypesArg = True
        continue
    elif arg == "-exclude":
        foundExcludesArg = True
    
    if foundTypesArg:
        filetypes.append(arg)
    elif foundExcludesArg:
        excludes.append(arg)
    else:
        rootdirs.append(arg)

for rootdir in rootdirs:
    for root, dirnames, filenames in os.walk(rootdir):
        for file in filenames:
            for type in filetypes:
                if fnmatch.fnmatchcase(file, type):
                    fullPath = os.path.join(root, file).replace("\\", "/")
                    if not fullPath in excludes:
                      print fullPath
                    break
