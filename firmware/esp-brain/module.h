#ifndef module_h
#define module_h

#include <PubSubClient.h>
#include <Wire.h>

class Module
{
    protected:
        PubSubClient* client;
        char id;
    public:
        void update(byte *data, unsigned int length);
        virtual boolean changed();
        virtual boolean publish();
        char get_id();
};

#endif module_h
