set -e
mkdir server_gcc
cd server_gcc
cmake -DCMAKE_BUILD_TYPE=Release -DWITH_STATIC=ON -DWITH_NEL_TESTS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_LUA51=OFF -DWITH_LUA52=ON -DWITH_RYZOM=ON -DWITH_RYZOM_SERVER=ON -DWITH_RYZOM_CLIENT=OFF -DWITH_RYZOM_TOOLS=OFF -DWITH_NEL_TOOLS=OFF -DWITH_NELNS=OFF -DWITH_QT5=OFF -DWITH_LIBGSF=OFF -DFINAL_VERSION=OFF -DWITH_DRIVER_OPENGL=OFF -DWITH_DRIVER_OPENAL=OFF -DWITH_PCH=OFF ../../code
cd ..
