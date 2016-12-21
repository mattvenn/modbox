from frozendict import frozendict
from functools import partial
import pickle
import logging
log = logging.getLogger(__name__)

mainmenu_items = [ 'volume', 'playback', 'add']
addmenu_items = ['fip', 'random album', 'cadena ser']
playback_items = ['play', 'stop', 'clear', 'skip']

initial_state = frozendict({
    'volume' : 0,
    'mainmenu_id' : 0,
    'mainmenu_knob' : 0,

    'playback_menu_id' : 0,
    'playback_menu_knob' : 0,

    'add_menu_id' : 0,
    'add_menu_knob' : 0,

    'change_playback' : False,
    'display' : ['modbox', ''],
    'knob1_leds' : 0,
    'knob2_leds' : 0,
    'but1_led' : 0,
    'but2_led' : 0,
})

state_file = 'state.pkl'

def read():
    log.debug('reading initial state')
    try:
        with open(state_file) as fh:
            state = pickle.load(fh)
            return state
    except:
        log.debug('error reading state file, using hard-coded initial state')
        return initial_state


def write(state):
    log.debug('writing state file')
    with open(state_file, 'w') as fh:
        pickle.dump(state, fh)
