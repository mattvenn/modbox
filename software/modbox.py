#!/usr/bin/python
import paho.mqtt.client as mqtt
import struct
import json
import time
import sys

with open("config.json") as fh:
    config = json.load(fh)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

def register_modules():
    # register modules
    for module in config:
        print("registering module %s with id %d" % (module["type"], module["id"]))
        reg_msg = struct.pack("<BB", module["id"], module["modchange_msg_len"])
        stat, mid = client.publish("/modbox/register", reg_msg, qos=2)

def on_publish(client, userdata, mid):
    print("sent msg id %s to client" % (mid))

def action_button(modchange_msg):
    if len(modchange_msg) != 2:
        print("wrong length message")
        return
    but_id, change = struct.unpack("<BB", modchange_msg)
    print("button id %d changed to %d" % (but_id, change))
    if change:
        msg = struct.pack("<BB", but_id, 0);
        setmod(msg)
    else:
        msg = struct.pack("<BB", but_id, 1);
        setmod(msg)

def setmod(msg):
	stat, mid = client.publish("/modbox/setmod", msg, qos=2)
	if stat != mqtt.MQTT_ERR_SUCCESS:
	    print("problem sending")
	else:
	    print("message id [%d] sent" % (mid))

def on_message(client, userdata, message):
    print("Received message on topic '" + message.topic)
    if message.topic == '/modbox/start':
        register_modules()
    if message.topic == '/modbox/modchange':
        print("mod change message")
        # get the id
        mod_id, = struct.unpack("<B", message.payload[0])
        for mod in config:
            if mod['id'] == mod_id:
                mod_config = mod
                if mod_config['type'] == 'button':
                    action_button(message.payload)
                break
        else:
            print("no module with that id")


client = mqtt.Client()
client.on_connect = on_connect
client.on_publish = on_publish
client.on_message = on_message

client.connect("127.0.0.1", 1883, 60)
time.sleep(0.1)
client.subscribe("/modbox/start") # subscribe after connect!
client.subscribe("/modbox/modchange") # subscribe after connect!
client.loop_start()

while True:
	time.sleep(0.5)
