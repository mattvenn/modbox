#include "module.h"


char Module::get_id()
{
    return this->id;
}

boolean Module::changed()
{
    return false;
}

boolean Module::publish()
{
    return false;
}
void Module::update(byte *data, unsigned int length)
{
    Wire.beginTransmission(this->id);
    // TODO skip a byte
    Wire.write(data, length);
    Wire.endTransmission();
}

