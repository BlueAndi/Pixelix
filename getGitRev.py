# MIT License
# 
# Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import subprocess
import json

Import("env")

def getGitRevisionHash():
    return subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode('ascii').strip()

def getGitRevisionShortHash():
    return subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).decode('ascii').strip()

def getCurrentGitBranch():
    return subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).decode('ascii').strip()

def anyLocalChange():
    anyLocalChange = False
    result = subprocess.check_output(['git', 'status', '--porcelain', '--untracked-files=no']).decode('ascii').strip()

    if (0 < len(result)):
        anyLocalChange = True

    return anyLocalChange

def anyUnpushedCommit(upstream):
    anyUnpushedCommit = False
    result = ""

    try:
        result = subprocess.check_output(['git', 'cherry', '-v', upstream], stderr=subprocess.STDOUT).decode('ascii').strip()
    except:
        pass

    if (0 < len(result)):
        anyUnpushedCommit = True

    return anyUnpushedCommit

def getVersion():
    version = "vX.X.X"
    branchName = getCurrentGitBranch()

    try:
        with open("version.json") as jsonFile:
            data = json.load(jsonFile)
            version = data["version"]
            revision = data["revision"]

        if (revision != getGitRevisionHash()):
            version = version + ":dev"

        if (True == anyUnpushedCommit("origin/" + branchName)):
            version = version + ":uc"

        if (True == anyLocalChange()):
            version = version + ":lc"

        if (branchName != "master"):
            version = version + ":b"

    except:
        pass

    return version

softwareVersion         = "-DSW_VERSION=" + getVersion()
softwareRevision        = "-DSW_REV=" + getGitRevisionHash()
softwareRevisionShort   = "-DSW_REV_SHORT=" + getGitRevisionShortHash()
softwareBranch          = "-DSW_BRANCH=" + getCurrentGitBranch()

env.Append(BUILD_FLAGS=[ softwareVersion, softwareRevision, softwareRevisionShort, softwareBranch ])

print("Software version       : " + getVersion())
print("Software revision long : " + getGitRevisionHash())
print("Software revision short: " + getGitRevisionShortHash())
print("Software branch        : " + getCurrentGitBranch())
