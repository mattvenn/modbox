import abc
import struct
from store import store
import paho.mqtt.client as mqtt
from actions import actions
from functools import partial
import logging
log = logging.getLogger(__name__)

class Module(object):
    __metaclass__ = abc.ABCMeta
    def __init__(self, id, mqtt_client):
        self.id = id
        self.mqtt_client = mqtt_client

    @abc.abstractmethod
    def get_modchange_msglen(self):
        return

    @abc.abstractmethod
    def action(self, payload):
        return

    def register(self):
        log.info("registering module %s with id %d" % (self.__class__.__name__, self.id))
        reg_msg = struct.pack("<BB", self.id, self.get_modchange_msglen())
        log.debug("reg msg = [%s]" % reg_msg.encode('hex'))
        stat, mid = self.mqtt_client.publish("/modbox/register", reg_msg, qos=2)

    def post_register(self):
        pass

    def send_setmod(self, msg):
        stat, mid = self.mqtt_client.publish("/modbox/setmod", msg, qos=0)
        if stat != mqtt.MQTT_ERR_SUCCESS:
            log.warning("problem sending")
        else:
            log.debug("message id [%d] sent" % (mid))


class Button(Module):
    def get_modchange_msglen(self):
        return 2

    def action(self, modchange_msg):
        if len(modchange_msg) != 2:
            log("wrong length message")
            return
        but_id, change = struct.unpack("<BB", modchange_msg)
        log.info("button id %d changed to %d" % (but_id, change))
        if change:
            self.send_setmod(struct.pack("<BB", but_id, 0));
        else:
            self.send_setmod(struct.pack("<BB", but_id, 1));

class Knobs(Module):
    def get_modchange_msglen(self):
        return 4

    def post_register(self):
        self.last_knob1 = 0
        self.last_knob2 = 0
        self.last_but1 = False
        self.last_but2 = False


    def action(self, modchange_msg):
        # this should be in the base class
        if len(modchange_msg) != self.get_modchange_msglen():
            log.warning("wrong length message, got %d, expected %d" % (len(modchange_msg), self.get_modchange_msglen()))
            return
        
        knob_id, knob1, knob2, buttons = struct.unpack("<BBBB", modchange_msg)

        but1, but2 = False, False
        if buttons & 1:
            but1 = True
        if buttons & 2:
            but2 = True

        if but1 == True and but1 != self.last_but1:
            store.dispatch(actions.but1_press())
        if but2 == True and but2 != self.last_but2:
            store.dispatch(actions.but2_press())

        if but1 == False and but1 != self.last_but1:
            store.dispatch(actions.but1_release())
        if but2 == False and but2 != self.last_but2:
            store.dispatch(actions.but2_release())
            

        if knob1 > self.last_knob1:
            store.dispatch(partial(actions.change_knob1,1)())
        elif knob1 < self.last_knob1:
            store.dispatch(partial(actions.change_knob1,-1)())

        if knob2 > self.last_knob2:
            store.dispatch(partial(actions.change_knob2,1)())
        elif knob2 < self.last_knob2:
            store.dispatch(partial(actions.change_knob2,-1)())

        self.last_knob1 = knob1
        self.last_knob2 = knob2

        self.last_but1 = but1
        self.last_but2 = but2
        """
            '>' : actions.next_page,
            '<' : actions.previous_page,
            '2' : partial(actions.go_to_book, 0),
        """
        """
        print("button id %d updated %d %d %d %d" % (knob_id, knob1, knob2, but1, but2))
        setmod_msg = struct.pack("<BBBB", knob_id, num_to_bar(knob1), num_to_bar(knob2), buttons);
        # bytearray forces mqtt to treat as data not strings. this should be in base class
        self.send_setmod(bytearray(setmod_msg))

        if knob1 % 4 or knob2 % 4 == 0:
            lcd_id = 2
            lcd_msg = "%04d        %04d" % (knob1, knob2)
            setmod_msg = struct.pack("<BB16s", lcd_id, 1, lcd_msg.ljust(16))
            self.send_setmod(bytearray(setmod_msg))
        """

    def update(self, knob1, knob2, but1, but2):
        buttons = 0
        if but1:
            buttons += 1
        if but2:
            buttons += 2
        setmod_msg = struct.pack("<BBBB", self.id, knob1, knob2, buttons);
        # bytearray forces mqtt to treat as data not strings. this should be in base class
        self.send_setmod(bytearray(setmod_msg))

class LCD(Module):

    def get_modchange_msglen(self):
        return 0

    def update(self, lcd_msg, row=0):
        log.debug("updating lcd row %d with %s" % (row, lcd_msg))
        setmod_msg = struct.pack("<BB16s", self.id, row, lcd_msg.ljust(16))
        self.send_setmod(bytearray(setmod_msg))

    def action(self, modchange_msg):
        pass

