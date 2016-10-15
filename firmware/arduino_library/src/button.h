#ifndef button_h
#define button_h

#include <PubSubClient.h>
#define UPDATE_LEN 1

class Button {
    private:
        PubSubClient* client;
        char id;
        boolean button;
        boolean led;
    public:
        Button(char id, PubSubClient& client);
        void update();
        boolean changed();
        boolean publish();
        char get_id();
};

#endif
