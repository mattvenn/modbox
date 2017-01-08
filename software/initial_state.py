from frozendict import frozendict
from functools import partial
import pickle
import logging
log = logging.getLogger(__name__)

mainmenu_items = ['add', 'volume', 'playback', 'now playing', 'battery']
playback_items = ['play', 'stop', 'clear', 'skip']


def get_initial_state(playlists):
    playlists.append('random album')

    initial_state = frozendict({
        'volume' : 0,
        'mainmenu_id' : 0,
        'mainmenu_knob' : 0,

        'playback_menu_id' : 0,
        'playback_menu_knob' : 0,

        'add_menu_id' : 0,
        'add_menu_knob' : 0,
        'add_menu_items' : playlists,

        'change_playback' : False,
        'change_volume' : False,
        'change_playlist' : False,

        'playlist_selected' : 0,
        'now_playing' : '',
        'now_playing_knob' : 0,
        'now_playing_char' : 0,

        'display' : ['modbox', ''],
        'knob1_leds' : 0,
        'knob2_leds' : 0,
        'but1_led' : 0,
        'but2_led' : 0,
        'battery' : 0,
    })

    return initial_state

state_file = 'state.pkl'

def read(playlists):
    log.debug('reading initial state')
    try:
        with open(state_file) as fh:
            state = pickle.load(fh)
            state['add_menu_items'] = playlists
            return state
    except:
        log.debug('error reading state file, using hard-coded initial state')
        return get_initial_state(playlists)


def write(state):
    log.debug('writing state file')
    with open(state_file, 'w') as fh:
        pickle.dump(state, fh)
