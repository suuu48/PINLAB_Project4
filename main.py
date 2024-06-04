#!/usr/bin/env python3

## record
import argparse
import tempfile
import queue
import sys
import sounddevice as sd
import soundfile as sf
import numpy
assert numpy  # avoid "imported but unused" message (W0611)

## metadata
import re
import json
import wave

## STT
from google.cloud import speech
import io
import os

## DB
from io import BytesIO
import numpy as np 
import requests
from pymongo import MongoClient
from scipy.io.wavfile import read, write
from scipy.io import wavfile
import sounddevice as sd
import wavio
import pyglet

### sentence ###
# def sentence():
#     sentence = ['Hello My name is Trump', 'Nice to meet you', 'See you again']

#     text_sentence = open('Text.txt', 'w')
#     text_sentence.write(str(sentence))
#     text_sentence.close()

#     print("Read the sentence ({}/5)".format(i+1))
#     print("( press Ctrl+C to stop the recording )")
#     print()
#     print("<< {} >>".format(sentence[i]))
#     print("")

### record ###
def record(filename):
    sd.default.device = 'default'
    sd.default.samplerate = 44100
    sd.default.channels = 1
    subtype = 'PCM_16'

    q = queue.Queue()

    def callback(indata, frames, time, status):
        if status:
            print(status, file=sys.stderr)
        q.put(indata.copy())
                                
    try:
        device_info = sd.query_devices(sd.default.device,'input')
        sd.default.samplerate = int(device_info['default_samplerate'])

        with sf.SoundFile(filename, mode='x', samplerate=sd.default.samplerate, channels=1, subtype=subtype) as file:
            with sd.InputStream(samplerate=sd.default.samplerate, device=sd.default.device, 
                                channels=1, 
                                callback=callback):
                while True:
                    file.write(q.get())

    except KeyboardInterrupt:
        print('\nRecording finished: ' + repr(filename))
        print("")

### metadata ###
def metadata(filename, count):
    def add_meta(filename, meta):
        with open(filename, 'a+b') as f:
            f.write(json.dumps(meta).encode('utf-8'))

    def read_meta(filename):
        with open("{}".format(filename), 'rb') as f:
            data = str(f.read())
        meta = re.findall(r'xff.*({.*})\'\Z', data)[-1]
        return meta

    def get_duration(filename):
        audio = wave.open(filename)
        frames = audio.getnframes()
        rate = audio.getframerate()
        duration = frames / float(rate)
        duration = round(duration,2)
        return duration

    def to_text(filename):
        text_meta = open('Meta_{}.txt'.format(filename), 'w')
        text_meta.write(read_meta(filename))
        text_meta.close()

    add_meta(filename, {'File Name':filename, 'Sample Rate':sd.default.samplerate, 
                        'Channels':'1', 'Length':get_duration(filename),
                        'Sentence':sentence[count]})
    to_text(filename)

### STT ###
os.environ["GOOGLE_APPLICATION_CREDENTIALS"]='project5-stt-0526b9367905.json'

def stt(filename):
    client = speech.SpeechClient()

    with io.open(filename, 'rb') as audio_file:
        content = audio_file.read()

    audio = speech.RecognitionAudio(content=content)
    config = speech.RecognitionConfig(encoding=speech.RecognitionConfig.AudioEncoding.LINEAR16,
                                    sample_rate_hertz=44100, language_code="en-US",)
    ####
    response = client.recognize(config=config, audio=audio)

    for result in response.results:
        text_stt = open('STT_{}.txt'.format(filename), 'w')
        text_stt.write("{}".format(result.alternatives[0].transcript))
        text_stt.close()

        print("Transcript: {}".format(result.alternatives[0].transcript))
        #print("uTranscript: {}".format(result.alternatives[0].transcript))            

### DB ###
def db_in(filename):
    client = MongoClient("mongodb+srv://0101:0101@wav-with-py.jnnis.mongodb.net/myFirstDatabase?retryWrites=true&w=majority")   
    db = client.test
    collection = db["buf1"]

    f1 = open('Meta_{}.txt'.format(filename))
    meta_file = f1.read()

    wav_file = wavio.read(filename)
    wavio.write(filename, wav_file.data, wav_file.rate)

    f2 = open('STT_{}.txt'.format(filename))
    STT_file = f2.read()

    collection.insert_one(
        {   "filename": filename,
            "metadata": meta_file,
            "rate": wav_file.rate,
            "data": wav_file.data.tolist(),
            "STT": STT_file
        })
    
    print("Successfully saving {}'s files to database".format(filename))  
    print("")   

### main ###
sentence = ['Hello My name is Trump', 'Nice to meet you', 'See you again']
count = 0

os.system("clear")

for i in range(3):

    print("Read the sentence ({}/3)".format(i+1))
    print("( press Ctrl+C to stop the recording )")
    print()
    print("<< {} >>".format(sentence[i]))
    print("")

    filename = 'stt_test{}.wav'.format(i+1)

    record(filename)

    metadata(filename,count)

    stt(filename)

    db_in(filename)

    count += 1

    os.system("clear")

print("Thanks")
exit()