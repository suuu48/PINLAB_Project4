sudo apt update
sudo apt upgrade

sudo apt install python3-pip

sudo pip3 install pymongo

cd portaudio
sudo apt install libasound-dev
sudo apt install libportaudio2
sudo chmod 777 portaudio
./configure && make
sudo make install
cp /usr/local/lib/libportaudio.a /home/mydir
cp /usr/local/lib/libportaudio.so.2 /home/mydir

pip3 install sounddevice
pip3 install soundfile
pip3 install --upgrade google-api-python-client
pip3 install google-cloud-speech
pip3 install argparse
pip3 install wave
pip3 install wavio
pip3 install pyglet
pip3 install scipy

