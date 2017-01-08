import os
import pydux
from frozendict import frozendict
import time

import logging
log = logging.getLogger(__name__)

from functools import partial
from initial_state import mainmenu_items, playback_items

def display_vol(volume):
    return "%d" % volume
def display_playback_opts(menu_id):
    return playback_items[menu_id]
def display_add_opts(state):
    return state['add_menu_items'][state['add_menu_id']]

def get_bar(num, maxn=100, fill=True, num_leds=7):
    div = float(maxn / num_leds)
    bar = 0
    tot = 0
    for i in range(num_leds):
        if num >= div * i:
            if fill:
                bar += 1 << i
            else:
                bar = 1 << i
    return bar

def reduce_knob(knob, options, reducer=5, wrap=False):
    if knob < 0:
        if wrap:
            knob = (reducer * len(options) - 1)
        else:
            knob = 0
    elif knob > (reducer * len(options) - 1):
        if wrap:
            knob = 0
        else:
            knob = (reducer * len(options) - 1)
    
    reduced = knob // reducer 
    return knob, reduced

class Reducers():
    def init(self, _, state):
        return state

    # only thing that calls this is fip, so only do something if fip is playing
    def update_now_playing(self, state, value):
        if state['playlist_selected']  == state['add_menu_items'].index('fip'):
            log.info("updating now playing")
            return state.copy(now_playing = value, now_playing_knob = 0);
        return state

    def update_battery(self, state, value):
        log.debug("battery = %s" % value)
        return state.copy(battery = value)
        
    def but1_press(self, state, value):
        return state

    def but2_press(self, state, value):
        """
        if state['mainmenu_id'] == mainmenu_items.index('playback'):
            but2_led = False
        if state['mainmenu_id'] == mainmenu_items.index('add'):
            but2_led = False
        """
        return state.copy(but2_led = False)

    def but1_release(self, state, value):
        return state

    def but2_release(self, state, value):
        if state['mainmenu_id'] == mainmenu_items.index('add'):
            log.info("adding %s" % display_add_opts(state))
            display = [state['display'][0],"adding..."]
            return state.copy(but2_led = True, display = display, change_playlist = True, playlist_selected = state['add_menu_id'])
        if state['mainmenu_id'] == mainmenu_items.index('playback'):
            display = [state['display'][0],"done"]
            log.info("changing play state to %s" % playback_items[state['playback_menu_id']])
            return state.copy(but2_led = True, display = display, change_playback = True)
        return state

    def change_knob1(self, state, value):
        mainmenu_knob, mainmenu_id = reduce_knob(state['mainmenu_knob'] + value, mainmenu_items)

        knob1_leds = get_bar(mainmenu_id * 10, 10 * len(mainmenu_items), fill=False, num_leds=len(mainmenu_items))
        knob2_leds = 0
        but2_led = False

        display = ["> %s" % (mainmenu_items[mainmenu_id]), '']

        if mainmenu_id == mainmenu_items.index('playback'):
            display[1] = display_playback_opts(state['playback_menu_id'])
            knob2_leds = get_bar(10 * state['playback_menu_id'], 10 * len(playback_items), fill=False, num_leds=len(playback_items))
            but2_led = True

        elif mainmenu_id == mainmenu_items.index('volume'):
            display[1] = display_vol(state['volume'])
            knob2_leds = get_bar(state['volume'], 100)

        elif mainmenu_id == mainmenu_items.index('add'):
            display[1] = display_add_opts(state)
            knob2_leds = get_bar(10 * state['add_menu_id'], 10 * len(state['add_menu_items']), fill=False, num_leds=len(state['add_menu_items']))
            but2_led = True

        elif mainmenu_id == mainmenu_items.index('now playing'):
            display[1] = state['now_playing']

        elif mainmenu_id == mainmenu_items.index('battery'):
            display[1] = "%s V" % state['battery']

        return state.copy(display = display, mainmenu_id = mainmenu_id, mainmenu_knob = mainmenu_knob, knob1_leds = knob1_leds, knob2_leds = knob2_leds, but2_led = but2_led, now_playing_knob = 0)
            
    def change_knob2(self, state, value):
        if state['mainmenu_id'] == mainmenu_items.index('volume'): 
            volume = state['volume'] + value * 2
            if volume > 100:
                volume = 100
            if volume < 0:
                volume = 0
            display = [state['display'][0], display_vol(volume)]
            knob2_leds = get_bar(volume, 100)
            return state.copy(change_volume = True, volume = volume, display = display, knob2_leds = knob2_leds)

        elif state['mainmenu_id'] == mainmenu_items.index('playback'): 
            playback_menu_knob, playback_menu_id = reduce_knob(state['playback_menu_knob'] + value, playback_items)
            display = [state['display'][0], display_playback_opts(playback_menu_id)]
            knob2_leds = get_bar(10 * playback_menu_id, 10 * len(playback_items), fill=False, num_leds=len(playback_items))
            return state.copy(display = display, playback_menu_knob = playback_menu_knob, playback_menu_id = playback_menu_id, knob2_leds = knob2_leds)

        elif state['mainmenu_id'] == mainmenu_items.index('add'): 
            add_menu_knob, add_menu_id = reduce_knob(state['add_menu_knob'] + value, state['add_menu_items'])
            display = [state['display'][0], display_add_opts(state)]
            knob2_leds = get_bar(10 * add_menu_id, 10 * len(state['add_menu_items']), fill=False, num_leds=len(state['add_menu_items']))
            return state.copy(display = display, add_menu_knob = add_menu_knob, add_menu_id = add_menu_id, knob2_leds = knob2_leds)

        elif state['mainmenu_id'] == mainmenu_items.index('now playing'):
            now_playing_knob, now_playing_char= reduce_knob(state['now_playing_knob'] + value, state['now_playing'], reducer=2)
            now_playing = state['now_playing'][now_playing_char:]
            display = [state['display'][0], now_playing]
            return state.copy(display = display, now_playing_knob = now_playing_knob, now_playing_char = now_playing_char)

        return state
    
    def volume_changed(self, state, value):
        return state.copy(change_volume = False)

    def playlist_changed(self, state, value):
        display = [state['display'][0],"%s" % value]
        return state.copy(change_playlist = False, display = display, now_playing = value)

    def playback_changed(self, state, value):
        log.info("playback changed")
        display = [state['display'][0],"state = %s" % value]
        return state.copy(change_playback = False, display = display)

def make_action_method(name):
    '''Returns a method that returns a dict to be passed to dispatch'''
    def action_method(value = None):
        return {'type': name, 'value': value}
    return action_method

def get_methods(cls):
    methods = [x for x in dir(cls) if callable(getattr(cls, x))]
    return filter(lambda x: not x.startswith('__'), methods)

action_types = get_methods(Reducers)
#just an empty object
def actions(): pass
#then we give it a method for each action
for action in action_types:
    setattr(actions, action, make_action_method(action))

