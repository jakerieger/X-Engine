cwd=$(pwd)
contentDir=$cwd/Game/Content
xpak=$cwd/build/Debug/bin/XPak/xpakc.exe

echo Generating asset descriptor files...
echo
find $contentDir -type f -exec $xpak generate {} \;

echo
echo Done.