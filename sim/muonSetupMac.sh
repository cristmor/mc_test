#!/bin/bash

#  setup ROOT environment

#  drop_from_path taken from /root/build/bin/thisroot.sh
drop_from_path()
{
   # Assert that we got enough arguments
   if test $# -ne 2 ; then
      echo "drop_from_path: needs 2 arguments"
      return 1
   fi

   local p=$1
   local drop=$2

   newpath=`echo $p | sed -e "s;:${drop}:;:;g" \
                          -e "s;:${drop}\$;;g"   \
                          -e "s;^${drop}:;;g"   \
                          -e "s;^${drop}\$;;g"`
}

export CRYHOME=/home/cristmor/work/cry_v1.7
export CRYDATAPATH=/home/cristmor/work/cry_v1.7/data

export G4BASE=/home/cristmor/work/g4/geant4.10.06.p02-install/source
export G4INSTALL=/home/cristmor/work/g4/geant4.10.06.p02-install/share/Geant4-10.6.2/geant4make

source /home/cristmor/work/g4/geant4.10.06.p02-install/share/Geant4-10.6.2/geant4make/geant4make.sh
export G4BIN="$PWD"

#export LD_LIBRARY_PATH=/Users/sshanto/hep/g4/geant4.10.06.p01-install/lib
#export DYLD_LIBRARY_PATH=/Users/sshanto/hep/g4/geant4.10.06.p01-install/lib:/Applications/root_v6.18.00/lib
#export SHLIB_PATH=/Users/sshanto/hep/g4/geant4.10.06.p01-install/lib
#export LIBPATH=/Users/sshanto/hep/g4/geant4.10.06.p01-install/lib

