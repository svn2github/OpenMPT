branching release braches
=========================

 1. adjust buildbot configuration by copying current trunk configuration to a
    new branch configuration and replace `trunk` with the branch version (i.e.
    `127`)
 2. add release build configuration to the buildbot branch configuration file
 3. branch the current trunk HEAD (`$VER` is the branch version):
    `svn copy -m "branch OpenMPT-$VER" https://source.openmpt.org/svn/openmpt/trunk/OpenMPT https://source.openmpt.org/svn/openmpt/branches/OpenMPT-$VER`
 4. update versions in trunk
    `https://source.openmpt.org/svn/openmpt/trunk/OpenMPT`:
     1. set OpenMPT version in `common/versionNumber.h` to
        `1.$(($VER + 1)).00.01`
     2. run `build/update_libopenmpt_version.sh bumpminor`
 5. update versions in branch
    `https://source.openmpt.org/svn/openmpt/branches/OpenMPT-$VER`:
     1. set OpenMPT version in `common/versionNumber.h` to
        `1.$VER.00.$MINORMINOR+1`
     2. run `build/update_libopenmpt_version.sh bumpprerel`

