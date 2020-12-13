import subprocess
import json

Import("env")

def getGitRevisionHash():
    return subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode('ascii').strip()

def getGitRevisionShortHash():
    return subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).decode('ascii').strip()

def anyLocalChange():
    anyLocalChange = False
    result = subprocess.check_output(['git', 'status', '--porcelain', '--untracked-files=no']).decode('ascii').strip()

    if (0 < len(result)):
        anyLocalChange = True

    return anyLocalChange

def anyUnpushedCommit():
    anyUnpushedCommit = False
    result = subprocess.check_output(['git', 'cherry', '-v']).decode('ascii').strip()

    if (0 < len(result)):
        anyUnpushedCommit = True

    return anyUnpushedCommit

def getVersion():
    version = "vX.X.X"

    try:
        with open("version.json") as jsonFile:
            data = json.load(jsonFile)
            version = data["version"]

        if (True == anyUnpushedCommit()):
            version = version + ":uc"

        if (True == anyLocalChange()):
            version = version + ":lc"
    except:
        pass

    return version

softwareVersion         = "-DSW_VERSION=" + getVersion()
softwareRevision        = "-DSW_REV=" + getGitRevisionHash()
softwareRevisionShort   = "-DSW_REV_SHORT=" + getGitRevisionShortHash()

env.Append(BUILD_FLAGS=[ softwareVersion, softwareRevision, softwareRevisionShort ])

print("Software version       : " + getVersion())
print("Software revision long : " + getGitRevisionHash())
print("Software revision short: " + getGitRevisionShortHash())
