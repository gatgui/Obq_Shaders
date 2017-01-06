import re
import sys
import glob
import excons
from excons.tools import maya
from excons.tools import arnold


env = excons.MakeBaseEnv()

Obq_version = "3.16"

if sys.platform == "win32":
   env.Append(CPPDEFINES=["_CRT_SECURE_NO_WARNINGS"])
   # Get rid of:
   # - nameless struct/union warning (4201)
   # - unreferenced formal parameter (4100)
   # - local variable is initialized but not referenced (4189)
   # - conditional expression is constant (4127)
   env.Append(CPPFLAGS=" /wd4201 /wd4100 /wd4189 /wd4127")
else:
   env.Append(CCFLAGS=" -fmessage-length=0")
   env.Append(CPPFLAGS=" -Wno-unused-parameter -Wno-unused-variable -Wno-unused")
   if re.match("^linux", sys.platform):
      env.Append(LINKFLAGS=" -Wl,--no-undefined")


A, M, m, p = arnold.Version(asString=False)
if A < 4 or (A == 4 and (M < 2 or (M == 2 and m < 13 or (m == 13 and p < 3)))):
   print("Arnold 4.2.13.3 at least required")
   sys.exit(1)

out_name = "Obq_Shaders__Core__a%s" % arnold.Version().replace(".", "_")

prjs = [
   {"name": out_name,
    "alias": "shaders",
    "type": "dynamicmodule",
    "prefix": "arnold",
    "ext": arnold.PluginExt(),
    "incdirs": ["glm", "src", "src/pbrt"],
    "srcs": filter(lambda x: "Simbiont" not in x, glob.glob("src/*.cpp")) +
            glob.glob("src/pbrt/*.cpp") +
            ["src/kettle/Grid2DAS.cpp", "src/kettle/KettleBaker.cpp"],
    "custom": [arnold.Require]
   }
]

targets = excons.DeclareTargets(env, prjs)

out_prefix = excons.OutputBaseDirectory()
env.Depends(targets["shaders"], env.InstallAs("%s/arnold/%s.mtd" % (out_prefix, out_name), "maya/metadata/Obq_Shaders__Core__a4_2_13_3.mtd"))
env.Depends(targets["shaders"], env.Install("%s/maya" % out_prefix, "maya/ae"))
env.Depends(targets["shaders"], env.Install("%s/maya" % out_prefix, "maya/attrPresets"))
env.Depends(targets["shaders"], env.Install("%s/maya" % out_prefix, "maya/docs"))
env.Depends(targets["shaders"], env.Install("%s/maya" % out_prefix, "maya/icons"))
env.Depends(targets["shaders"], env.Install("%s/c4d" % out_prefix, "cinema4d/docs"))
env.Depends(targets["shaders"], env.Install("%s/c4d" % out_prefix, "cinema4d/res"))

