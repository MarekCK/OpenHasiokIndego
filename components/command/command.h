#pragma once

typedef enum
{
    CMD_STOP = 0,
    CMD_FORWARD,
    CMD_BACKWARD,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_BLADE_ON,
    CMD_BLADE_OFF
} ohi_cmd_t;

extern volatile ohi_cmd_t ohi_cmd;
