Import("env", "projenv")
import os

env.ProcessUnFlags("-DVECT_TAB_ADDR")
env.Append(CPPDEFINES=("VECT_TAB_ADDR", 0x123456789))

def after_build(source, target, env):
    print "Copying hex file to target folder"
    if not os.path.isdir("target"):
        env.Execute("mkdir target | echo $BUILD_DIR")
    env.Execute("echo ----- copy hex to target folder ---- ")
    env.Execute("cp $BUILD_DIR/${PROGNAME}.hex target")

def after_build2(source, target, env):
    env.Execute("echo ----- all done ---- ")
    print "All done"

print "Current build targets", map(str, BUILD_TARGETS)

#env.AddPreAction("upload", before_upload)
#env.AddPostAction("upload", after_upload)
env.AddPostAction("size", after_build2)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", after_build)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", after_build2)
env.AddPostAction("buildprog", after_build2)
