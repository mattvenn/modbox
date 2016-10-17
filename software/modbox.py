#!/usr/bin/python
import paho.mqtt.client as mqtt
from modules import Button
import struct
import json
import time
import sys


def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

def on_publish(client, userdata, mid):
    print("sent msg id %s to client" % (mid))

def on_message(client, userdata, message):
    print("Received message on topic '" + message.topic)
    if message.topic == '/modbox/start':
        for module in modules:
            module.register()

    elif message.topic == '/modbox/modchange':
        print("mod change message")
        # get the id
        mod_id, = struct.unpack("<B", message.payload[0])
        # find the right module
        for module in modules:
            if module.id == mod_id:
                module.action(message.payload)
                break
        else:
            print("no module with that id")
    else:
        print("no handler for that topic")

# open config
with open("config.json") as fh:
    config = json.load(fh)

client = mqtt.Client()
client.on_connect = on_connect
client.on_publish = on_publish
client.on_message = on_message

# build a list of module objects
modules = []
for module in config:
    if module['type'] == 'button':
        modules.append(Button(module['id'], client))

client.connect("127.0.0.1", 1883, 60)
time.sleep(0.1)
client.subscribe("/modbox/start")  # when modbox starts, this will cause all modules to be registered
client.subscribe("/modbox/modchange") # whenever a control changes
client.loop_start()

while True:
	time.sleep(0.5)
