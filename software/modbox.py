#!/usr/bin/python
import paho.mqtt.client as mqtt
from modules import Button, Knobs, LCD
import struct
import json
import time
import sys
import initial_state #import read, write
from store import store
from actions import actions
from functools import partial

import logging

log_format = logging.Formatter('%(asctime)s - %(name)-16s - %(levelname)-8s - %(message)s')
# configure the client logging
log = logging.getLogger('')
# has to be set to debug as is the root logger
log.setLevel(logging.INFO)

# create console handler and set level to info
ch = logging.StreamHandler()
# create formatter for console
ch.setFormatter(log_format)
log.addHandler(ch)

def on_connect(client, userdata, flags, rc):
    log.info("Connected with result code "+str(rc))

def on_publish(client, userdata, mid):
    log.debug("sent msg id %s to client" % (mid))

def on_message(client, userdata, message):
    log.debug("Received message on topic " + message.topic)
    if message.topic == '/modbox/start':
        for module in modules:
            module.register()
        for module in modules:
            module.post_register()
        handle_changes()

    elif message.topic == '/modbox/modchange':
        log.debug("mod change message")
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
    elif module['type'] == 'knobs':
        modules.append(Knobs(module['id'], client))
    elif module['type'] == 'lcd':
        modules.append(LCD(module['id'], client))

client.connect("127.0.0.1", 1883, 60)
time.sleep(0.1)
client.subscribe("/modbox/start")  # when modbox starts, this will cause all modules to be registered
client.subscribe("/modbox/modchange") # whenever a control changes
client.loop_start()

# force a reset
client.publish("/modbox/reset", "", qos=2)

display = ['','']
def handle_changes():
    state = store.get_state()
    log.debug(state)

    if state['display'][0] != display[0]:
        lcd_updated = True
        display[0] = state['display'][0]
        modules[1].update(state['display'][0],0)

    # hack because 2 consecutive lcd messages can break screen
    modules[0].update(state['knob1_leds'], state['knob2_leds'], state['but1_led'], state['but2_led'])

    if state['display'][1] != display[1]:
        display[1] = state['display'][1]
        modules[1].update(state['display'][1],1)
    initial_state.write(state)

    if state['change_playback'] is not False:
        store.dispatch(actions.change_playback(state))
        
        

store.dispatch(actions.init(initial_state.read()))
store.subscribe(partial(handle_changes))

while True:
	time.sleep(0.5)
