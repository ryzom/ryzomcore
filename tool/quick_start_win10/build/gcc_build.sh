set -e
cd server_gcc
cmake -DNL_VERSION_PATCH=$CLIENT_PATCH_VERSION .
make -j`nproc`
cd ..
