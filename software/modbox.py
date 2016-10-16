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
        reg_msg = struct.pack("<BB", module["id"], module["msg_len"])
        stat, mid = client.publish("/modbox/register", reg_msg, qos=2)

def on_publish(client, userdata, mid):
    print("sent to client %s %s" % (userdata, mid))

def setmod(msg):
	stat, mid = client.publish("/modbox/setmod", msg, qos=2)
	if stat != mqtt.MQTT_ERR_SUCCESS:
	    print("problem sending")
	else:
	    print("message %d sent id %d" % (out,mid))

def on_message(client, userdata, message):
    print("Received message '" + str(message.payload) + "' on topic '"
        + message.topic + "' with QoS " + str(message.qos))
    if message.topic == '/modbox/start':
        register_modules()
    if message.topic == '/modbox/modchange':
        print("mod change message")

client = mqtt.Client()
client.on_connect = on_connect
client.on_publish = on_publish
client.on_message = on_message

client.connect("127.0.0.1", 1883, 60)
time.sleep(0.1)
client.subscribe("/modbox/start") # subscribe after connect!
client.subscribe("/modbox/modchange") # subscribe after connect!
client.loop_start()

out = 1
while True:
	time.sleep(0.5)


