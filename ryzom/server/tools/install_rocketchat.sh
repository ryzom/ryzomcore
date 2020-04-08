#!/bin/bash
VERSION="1.0.0-rc.0"

BUILD_PATH=~/builds

echo "INSTALLATION OF VERSION $VERSION !!!!"

echo "If you don't have install all requirements check at:"
echo "https://rocket.chat/docs/installation/manual-installation/ubuntu/"
echo "To install meteor : curl https://install.meteor.com/ | sh"

echo "Erasing old..."
rm -rf $BUILD_PATH/RocketChat/Rocket.Chat.old

mv Rocket.Chat Rocket.Chat.old

sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 2930ADAE8CAF5059EE73BB4B58712A2291FA4AD5

cd ~/builds/RocketChat/
if [[ ! -d "megacorp" ]]
	echo "Missing megacorp. Cloning it.."
	hg clone ssh://hg@bitbucket.org/ryzom/megacorp
fi

cd megacorp
hg pull
hg update -v
cd ..

git clone https://github.com/RocketChat/Rocket.Chat.git

cd Rocket.Chat
git checkout $VERSION

cp $BUILD_PATH/RocketChat/megacorp/ryzom-rocket-bridge/ packages/
echo -e "\nryzom-rocket-bridge" >> .meteor/packages

##cd packages/rocketchat-iframe-login
##sed -i -e $'s/console.log/check(result.token, String);\\\n\\\tconsole.log/g' iframe_server.js
##cd ../..


npm install --production
#npm audit fix // ???

rm -rf ../rc-bundle
meteor build ../rc-bundle --architecture os.linux.x86_64
meteor build ../rc-bundle --architecture os.linux.x86_64


cd ../rc-bundle
tar xvfz Rocket.Chat.tar.gz

cd ~
mv Rocket.Chat/ Rocket.Chat.old
cp -r src/rc-bundle/bundle/ Rocket.Chat/

cd ~/Rocket.Chat/programs/server
npm install
tools@chat:~/src$
