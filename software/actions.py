import os
import pydux
from frozendict import frozendict

import logging
log = logging.getLogger(__name__)

from functools import partial

class Reducers():
    def init(self, _, state):
        return state
    def trigger(self, state, value):
        '''bit ugly but gives the abiliy to trigger any state subscribers'''
        return state
    
    def change_knob_2(self, state, value):
        if value > 0:
            log.debug("vol mode")
            state['display'][0] = 'vol mode'
            state['display'][1] = "vol = %d" % state['volume']
            return state.copy(display = state['display'], location = 'volume')
        elif value < 0:
            log.debug("disp mode")
            state['display'][0] = 'display mode'
            state['display'][1] = "disp = %d" % state['disp']
            return state.copy(display = state['display'], location = 'display')
        return state 
            
    def change_knob_1(self, state, value):
        if state['location'] == 'volume': 
            volume = state['volume'] + value 
            state['display'][1] = "vol = %d" % volume
            return state.copy(volume = volume, display = state['display'])
        if state['location'] == 'display': 
            disp = state['disp'] + value 
            state['display'][1] = "disp = %d" % disp
            return state.copy(disp = disp, display = state['display'])
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

