import abc
import struct
import paho.mqtt.client as mqtt


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
        print("registering module %s with id %d" % (self.__class__.__name__, self.id))
        reg_msg = struct.pack("<BB", self.id, self.get_modchange_msglen())
        print("reg msg = [%s]" % reg_msg.encode('hex'))
        stat, mid = self.mqtt_client.publish("/modbox/register", reg_msg, qos=2)

    def send_setmod(self, msg):
        stat, mid = self.mqtt_client.publish("/modbox/setmod", msg, qos=2)
        if stat != mqtt.MQTT_ERR_SUCCESS:
            print("problem sending")
        else:
            print("message id [%d] sent" % (mid))


class Button(Module):
    def get_modchange_msglen(self):
        return 2

    def action(self, modchange_msg):
        if len(modchange_msg) != 2:
            print("wrong length message")
            return
        but_id, change = struct.unpack("<BB", modchange_msg)
        print("button id %d changed to %d" % (but_id, change))
        if change:
            self.send_setmod(struct.pack("<BB", but_id, 0));
        else:
            self.send_setmod(struct.pack("<BB", but_id, 1));

class Knobs(Module):
    def get_modchange_msglen(self):
        return 4

    def action(self, modchange_msg):
        # this should be in the base class
        if len(modchange_msg) != self.get_modchange_msglen():
            print("wrong length message, got %d, expected %d" % (len(modchange_msg), self.get_modchange_msglen()))
            return
        knob_id, knob1, knob2, buttons = struct.unpack("<BBBB", modchange_msg)
        but1, but2 = False, False
        if buttons & 1:
            but1 = True
        if buttons & 2:
            but2 = True
        print("button id %d updated %d %d %d %d" % (knob_id, knob1, knob2, but1, but2))
        setmod_msg = struct.pack("<BBBB", knob_id, knob1, knob2, buttons);
        # bytearray forces mqtt to treat as data not strings. this should be in base class
        self.send_setmod(bytearray(setmod_msg))
