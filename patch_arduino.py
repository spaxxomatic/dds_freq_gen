Import("env")
import os
from shutil import copyfile
# global build environment
#print dir(env)

# project build environment (is used source files in "src" folder)
print ("Patching Arduino with the  9-bit hwserial")
#print dir(env)
files = ['HardwareSerial.h','HardwareSerial_private.h', 'HardwareSerial.cpp']
source_folder = os.path.join('serial_9_bit_lib','HardwareSerial9bit')
target_folder = os.path.join('temp','packages','framework-arduinoavr','cores','arduino')
if not os.path.exists(target_folder):
    print "Something is wrong in your setup, the arduino framework folder %s is not there"%fp
for f in files:
    fp = os.path.join(source_folder, f)
    dest = os.path.join(target_folder, f)
    if os.path.exists(fp):
        cmd = 'diff -q %s %s > /dev/null'%(fp, dest) #check if files are the same
        ret = os.system(cmd)
        if ret == 0:
            print "File already patched: %s"%dest
        else:
            print "Patching %s"%dest
            copyfile(fp, dest)
    else:
        print "Could not find patch file %s"%fp
        exit(1)

#env.Execute("cp serial_9_bit_lib/HardwareSerial9bit/* temp/packages/framework-arduinoavr/cores/arduino")


