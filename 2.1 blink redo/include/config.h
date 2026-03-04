#include "types.h"

class Config
{
private:
    BLINK_SPEED speed;

public:
    Config();
    Config(BLINK_SPEED spd);
    ~Config();
    uint16_t getSpeed() const;
};

Config::Config() : speed(BLINK_SPEED::SLUGGISH)
{
}

inline Config::Config(BLINK_SPEED spd)
{
    speed = spd;
}

Config::~Config()
{
}

inline uint16_t Config::getSpeed() const
{
    switch (speed)
    {
    case BLINK_SPEED::FAST:
        return 200;
    case BLINK_SPEED::MEDIUM:
        return 400;
    case BLINK_SPEED::SLOW:
        return 800;
    default:
        return 1200;
    }
}
 