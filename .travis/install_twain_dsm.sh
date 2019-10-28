#!/usr/bin/env bash

cd $TRAVIS_BUILD_DIR

sudo apt install build-essential cmake debhelper devscripts fakeroot qt4-qmake libqt4-dev libfreeimage-dev -y

git clone https://github.com/twain/twain-dsm.git

# Patch the mkdsm.sh file
cd twain-dsm
mv mkdsm.sh mkdsm.sh~
head -n96 mkdsm.sh~ > mkdsm.sh
cat ../.travis/ubuntu18.04-mkdsm-patch >> mkdsm.sh
tail -n +97 mkdsm.sh~ >> mkdsm.sh
sed -i '203d' mkdsm.sh
sed -i '202d' mkdsm.sh
sed -i '184d' mkdsm.sh
chmod +x ./mkdsm.sh
rm ./mkdsm.sh~

# Set deb compatibility to 9
echo "9" > TWAIN_DSM/debian/compats

# Disable -Wall and -Werror
sed -i 's/-Wall//g' TWAIN_DSM/src/CMakeLists.txt
sed -i 's/-Werror//g' TWAIN_DSM/src/CMakeLists.txt

ANSWER=y ./mkdsm.sh

dpkg -i twaindsm_2.4.2-1_amd64.deb

# Patch DS Common.h
cd ..
git clone https://github.com/twain/twain-samples.git
cd twain-samples/TWAIN-Samples/
mv common/Common.h common/Common.h~
head -n208 common/Common.h~ > common/Common.h
echo "#include <unistd.h>" >> common/Common.h
tail -n +209 common/Common.h~ >> common/Common.h
rm 

cd Twain_DS_sample01/
qmake -makefile TWAINDS_sample01.pro
make

mkdir -p /usr/local/lib/twain/sample2
cp images/TWAIN_logo.png /usr/local/lib/twain/sample2
cp src/TWAINDS_FreeImage.ds /usr/local/lib/twain/sample2
