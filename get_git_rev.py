import subprocess

Import("env")

def get_git_revision_hash():
    return subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode('ascii').strip()

def get_git_revision_short_hash():
    return subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).decode('ascii').strip()

git_rev = "-DGIT_REV=" + get_git_revision_hash()
git_rev_short = "-DGIT_REV_SHORT=" + get_git_revision_short_hash()

env.Append(BUILD_FLAGS=[ git_rev, git_rev_short ])

print("git revision long: " + get_git_revision_hash())
print("git revision short: " + get_git_revision_short_hash())
