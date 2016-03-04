#!/usr/bin/env python3


import ast
import difflib
import glob
import subprocess as sub
import sys
from sys import stdout, stderr


TEMP = "/tmp/28d8vjsdfu239ur0z89fd8f23pq2038h8vc"

devnull = open("/dev/null", "w")


def main(asm_name):
    fail = []

    for testname in sorted(glob.glob("test-lex/*")):
        stdout.write(testname + "\n")
        with open(testname) as src:
            asm = sub.Popen(
                (asm_name,), stdin=src, stdout=sub.PIPE, stderr=sub.STDOUT)
            out, _ = asm.communicate()
            out = out.decode('utf-8')
            if asm.returncode != 0:
                stderr.write("TEST FAILED: Can't lex\n")
                stderr.write("Output:\n")
                for line in out.splitlines():
                    stderr.write("    " + line + "\n")
                fail.append(testname)

    for testname in sorted(glob.glob("test-no-lex/*")):
        stdout.write(testname + "\n")
        with open(testname) as src:
            asm = sub.Popen(
                (asm_name,), stdin=src, stdout=sub.PIPE, stderr=sub.STDOUT)
            out, _ = asm.communicate()
            out = out.decode('utf-8')
            if asm.returncode == 0:
                stderr.write("TEST FAILED: Lex should fail\n")
                stderr.write("Output:\n")
                for line in out.splitlines():
                    stderr.write("    " + line + "\n")
                fail.append(testname)


    if fail:
        stdout.write("FAILED: " + ", ".join(fail) + "\n")
    else:
        stdout.write("PASSED\n")


if __name__ == '__main__':
    if len(sys.argv) != 2:
        stderr.write("Usage:  test.py ./asm\n")
        exit(1)
    main(sys.argv[1])
