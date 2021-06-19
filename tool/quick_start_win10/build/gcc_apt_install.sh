set -e
sudo apt update
sudo apt install -y software-properties-common
# sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install cmake build-essential -y
sudo apt install gcc-8 g++-8 -y
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 60
sudo apt install bison autoconf automake -y
sudo apt install libpng-dev -y
sudo apt install libjpeg-dev -y
sudo apt install libgif-dev libfreetype6-dev -y
sudo apt install freeglut3-dev -y
sudo apt install liblua5.2-dev libluabind-dev libcpptest-dev -y
sudo apt install libogg-dev libvorbis-dev libopenal-dev -y
sudo apt install libavcodec-dev libavformat-dev libavdevice-dev libswscale-dev libpostproc-dev -y
sudo apt install libmysqlclient-dev -y
sudo apt install libxml2-dev -y
sudo apt install libcurl4-openssl-dev libssl-dev -y
sudo apt install libsquish-dev -y
sudo apt install liblzma-dev -y
sudo apt install libgsf-1-dev -y
sudo apt install qtbase5-dev qttools5-dev qttools5-dev-tools -y
gcc -v
g++ -v
