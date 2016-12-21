from frozendict import frozendict
from functools import partial
import pickle
import logging
log = logging.getLogger(__name__)

mainmenu_items = [ 'volume', 'playback', 'random album', 'fip']
playback_items = ['start', 'stop']

initial_state = frozendict({
    'volume' : 0,
    'mainmenu_id' : 0,
    'mainmenu_knob' : 0,
    'playback_menu_id' : 0,
    'playback_menu_knob' : 0,
    'change_playback' : False,
    'display' : ['modbox', ''],
})

"""
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
    write_state                      = dict(state)
    write_state['library']           = state['library'].copy(page = 0)
    location = state['location']
    if location == 'menu':
        location = 'library'
    write_state['location']          = location
    write_state['backing_up_log']    = False
    write_state['replacing_library'] = False
    write_state['shutting_down']     = False
    with open(state_file, 'w') as fh:
        pickle.dump(frozendict(write_state), fh)
"""
