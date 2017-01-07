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

targets["mtd"] = env.InstallAs("%s/arnold/%s.mtd" % (out_prefix, out_name), "maya/metadata/Obq_Shaders__Core__a4_2_13_3.mtd")
targets["maya_ae"] = env.Install("%s/maya" % out_prefix, "maya/ae")
targets["maya_pr"] = env.Install("%s/maya" % out_prefix, "maya/attrPresets")
targets["maya_do"] = env.Install("%s/maya" % out_prefix, "maya/docs")
targets["maya_ic"] = env.Install("%s/maya" % out_prefix, "maya/icons")
targets["c4d_do"] = env.Install("%s/c4d" % out_prefix, "cinema4d/docs")
targets["c4d_re"] = env.Install("%s/c4d" % out_prefix, "cinema4d/res")
targets["xsi_co"] = env.Install("%s/xsi" % out_prefix, "softimage/compounds")
targets["xsi_sp"] = env.Install("%s/xsi" % out_prefix, "softimage/spdl")

env.Depends(targets["shaders"], targets["mtd"])
env.Depends(targets["shaders"], targets["maya_ae"])
env.Depends(targets["shaders"], targets["maya_pr"])
env.Depends(targets["shaders"], targets["maya_do"])
env.Depends(targets["shaders"], targets["maya_ic"])
env.Depends(targets["shaders"], targets["c4d_do"])
env.Depends(targets["shaders"], targets["c4d_re"])
env.Depends(targets["shaders"], targets["xsi_co"])
env.Depends(targets["shaders"], targets["xsi_sp"])

eco = excons.EcosystemDist(env, "Obq_Shaders.env",
                           {out_name: "/arnold/%s" % excons.EcosystemPlatform(),
                            "mtd":  "/arnold/%s" % excons.EcosystemPlatform(),
                            "maya_ae": "/maya",
                            "maya_pr": "/maya",
                            "maya_do": "/maya",
                            "maya_ic": "/maya",
                            "c4d_do": "/c4d",
                            "c4d_re": "/c4d",
                            "xsi_co": "/xsi",
                            "xsi_sp": "/xsi"},
                           targets=targets,
                           ecoenv={"requires": ["arnold%s+" % arnold.Version()]})

Default(targets["shaders"])
