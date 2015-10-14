#!/bin/sh

# Run this when changing the look of the v3d dmg window.
# usage: update_dmg_view.sh InstallV3d-2.539.dmg

# Remove old temp file to avoid error
rm -f temprw.dmg

# Create temporary writeable DMG so we can manipulate its DS_Store
hdiutil convert "$1" -format UDRW -o temprw.dmg

v3d_version=`echo $1 | cut -f2 -d '-' | cut -f1,2 -d '.'`
echo ${v3d_version}

# Mount new DMG and remember the device name.
device=$(hdiutil attach -readwrite -noverify -noautoopen temprw.dmg | egrep '^/dev/' | sed 1q | awk '{print $1}')
sleep 2

mkdir /Volumes/Vaa3d-${v3d_version}/.background
# background file must not be hidden when setting background
chflags nohidden /Volumes/Vaa3d-${v3d_version}/background.png

# Use applescript language to arrange view
echo "
   tell application \"Finder\"
     tell disk \"Vaa3d-${v3d_version}\"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {400, 100, 880, 340}
           set theViewOptions to the icon view options of container window
           set arrangement of theViewOptions to not arranged
           set icon size of theViewOptions to 96
           set text size of theViewOptions to 16
           set bgFile to file \":background.png\"
           set background picture of theViewOptions to bgFile
           set position of item \"Vaa3d.app\" of container window to {80, 110}
           set position of item \"Applications\" of container window to {350, 110}
           update without registering applications
           delay 2
     end tell
   end tell
" | osascript

# Rehide the background image
chflags hidden /Volumes/Vaa3d-${v3d_version}/background.png
sleep 2

cp "/Volumes/Vaa3d-${v3d_version}/.DS_Store" "./DS_Store_v3d_${v3d_version}"
hdiutil detach "/Volumes/Vaa3d-${v3d_version}"

