import tweepy
import logging
import re, json
from secrets import *
from store import store
from actions import actions

import logging
log = logging.getLogger(__name__)

class FIPListener(tweepy.StreamListener):

    def on_status(self, status):
        log.info(status.text)

    def on_data(self, data):
        data = json.loads(data)
        m = re.search(r'#nowplaying (.*) https:', data['text'])
        if m is not None:
            now_playing = m.group(1).encode('utf-8')
            log.info("update: %s" % now_playing)
            store.dispatch(actions.update_now_playing(now_playing))

        else:
            log.warning("failed: %s" % data['text'])

    def on_error(self, status_code):
        log.error(status_code)
        return False

def start_listener():
    auth = tweepy.OAuthHandler(consumer_key, consumer_secret)
    auth.set_access_token(access_token, access_secret)
    api = tweepy.API(auth)

    fipListener = FIPListener()
    fipStream = tweepy.Stream(auth = api.auth, listener=fipListener)
    log.info("starting fip twitter stream")
    fipStream.filter(follow=[2211149702])

if __name__ == '__main__':
    start_listener()
