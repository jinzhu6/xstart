#!/usr/bin/python

import os, sys, commands, time

MAX_EXECUTION_TIME = 12 * 60 * 60

def log(s): f = open("watchman.log","a");f.write(s);f.write("\n");f.close();print(s)

dupcheck = commands.getoutput('ps aux | grep watchman.py')
if dupcheck.count("watchman.py") > 3:
    print("Watchman is already running.")
    exit()

if len(sys.argv) < 2: exit()
exe = sys.argv[1]
param = ""
if len(sys.argv) > 2: param = sys.argv[2]
pid = commands.getoutput('pidof %s' % exe)

log("\nWatching %s (%s)." % (exe,pid))
if pid == "": os.system("%s %s &" % (exe, param))

start = time.time()
while True:
    time.sleep(4.0)
    if time.time() - start > MAX_EXECUTION_TIME and MAX_EXECUTION_TIME > 0:
        start = time.time()
        log("Max. execution time reached, stopping and restarting '%s' ..." % (exe))
        os.system("sudo killall " + exe)
        time.sleep(1.0)
        os.system("%s %s &" % (exe, param))

    pid = commands.getoutput('pidof %s' % exe)
    if pid == "":
        log("[%.0fm] Process for '%s' not found, starting now ..." % ((time.time()-start)  / 60.0, exe))
        os.system("%s %s &" % (exe, param))

