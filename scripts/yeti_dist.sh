#!/bin/bash

set -xe

rm -rf "dist"
mkdir -p "dist/Release64"

# Copy files from release build in, without copying obj/
pushd "x64/Release"
find * -not -path 'obj*' -exec cp -r --parents '{}' ../../dist/Release64/ \;
popd

# Copy in d3dcompiler from windows kit 8.1
cp /cygdrive/c/Program\ Files\ \(x86\)/Windows\ Kits/8.1/Redist/D3D/x64/d3dcompiler_47.dll dist/Release64/

# Copy associated files that should be included with the distribution
cp LICENSE.md dist/Release64/

# Delete new VS2015 incremental pdb files, these are just build artifacts
# and aren't needed for later symbol resolution etc
find dist/Release64/ -iname '*.ipdb' -exec rm '{}' \;
find dist/Release64/ -iname '*.iobj' -exec rm '{}' \;

# Make a copy of the main distribution folder that has PDBs
cp -R dist/Release64 dist/ReleasePDBs64

# Remove all pdbs
find dist/Release64/ -iname '*.pdb' -exec rm '{}' \;

# Remove any build associated files that might have gotten dumped in the folders
rm -f dist/Release64/*.{exp,lib,metagen,xml} dist/Release64/*.vshost.*

# Delete all but xml files from PDB folder as well (large files, and not useful)
rm -f dist/ReleasePDBs64/*.{exp,lib,metagen} dist/ReleasePDBs64/*.vshost.*

pushd "dist/ReleasePDBs64"
zip -r - * > ../YetiSDK-windows-renderdoc.zip
popd
