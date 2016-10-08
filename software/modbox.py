#!/usr/bin/python
import paho.mqtt.client as mqtt
import time
import sys

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

def on_publish(client, userdata, mid):
    print("sent to client")
    print client
    print userdata
    print mid

def send_led(led):
	stat, mid = client.publish("/modbox/led", led, qos=2)
	if stat != mqtt.MQTT_ERR_SUCCESS:
	    print("problem sending")
	else:
	    print("message %d sent id %d" % (out,mid))

def on_message(client, userdata, message):
    print("Received message '" + str(message.payload) + "' on topic '"
        + message.topic + "' with QoS " + str(message.qos))
    if message.payload == '1':
	send_led(0)
    if message.payload == '0':
	send_led(1)

client = mqtt.Client()
client.on_connect = on_connect
client.on_publish = on_publish
client.on_message = on_message


client.connect("127.0.0.1", 1883, 60)
time.sleep(0.1)
client.subscribe("/modbox/button") # subscribe after connect!
client.loop_start()

out = 1
while True:
	time.sleep(0.5)


