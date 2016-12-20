import os
import pydux
from frozendict import frozendict

import logging
log = logging.getLogger(__name__)

from functools import partial
from initial_state import mainmenu_items, playback_items

def display_vol(volume):
    return "vol = %d" % volume
def display_playback_opts(playback_menu_id):
    return playback_items[playback_menu_id]

class Reducers():
    def init(self, _, state):
        return state
    def trigger(self, state, value):
        '''bit ugly but gives the abiliy to trigger any state subscribers'''
        return state

    def but1_release(self, state, value):
        return state

    def but2_release(self, state, value):
        if state['mainmenu_id'] == mainmenu_items.index('playback'):
            return state.copy(change_playback = True)
        if state['mainmenu_id'] == mainmenu_items.index('fip'):
            log.info("starting fip")
            display = [state['display'][0],"starting fip"]
            return state.copy(display = display)
        if state['mainmenu_id'] == mainmenu_items.index('random album'):
            log.info("starting random album")
            display = [state['display'][0],"adding album"]
            return state.copy(display = display)

    def change_knob1(self, state, value):
        mainmenu_id = state['mainmenu_id'] + value

        if mainmenu_id < 0:
            mainmenu_id = len(mainmenu_items) - 1
        elif mainmenu_id > len(mainmenu_items) - 1:
            mainmenu_id = 0

        display = ["%d %s" % (mainmenu_id, mainmenu_items[mainmenu_id]), '']

        if mainmenu_id == mainmenu_items.index('playback'):
            display[1] = display_playback_opts(state['playback_menu_id'])

        elif mainmenu_id == mainmenu_items.index('volume'):
            display[1] = display_vol(state['volume'])

        return state.copy(display = display, mainmenu_id = mainmenu_id)
    """
        if value > 0:
            log.debug("vol mode")
            state['display'][0] = 'vol mode'
            state['display'][1] = "vol = %d" % state['volume']
            return state.copy(display = state['display'], location = 'volume')
        elif value < 0:
            log.debug("playback mode")
            state['display'][0] = 'display mode'
            state['display'][1] = "disp = %d" % state['disp']
            return state.copy(display = state['display'], location = 'display')
        return state 
    """
            
    def change_knob2(self, state, value):
        if state['mainmenu_id'] == mainmenu_items.index('volume'): 
            volume = state['volume'] + value 
            if volume > 100:
                volume = 100
            if volume < 0:
                volume = 0
            display = [state['display'][0], display_vol(volume)]
            return state.copy(volume = volume, display = display)
        if state['mainmenu_id'] == mainmenu_items.index('playback'): 
            playback_menu_id = state['playback_menu_id'] + value 
            if playback_menu_id > len(playback_items) - 1:
                playback_menu_id = 0
            elif playback_menu_id < 0:
                playback_menu_id = len(playback_items) - 1

            display = [state['display'][0], display_playback_opts(playback_menu_id)]
            return state.copy(display = display, playback_menu_id = playback_menu_id)
        return state

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

