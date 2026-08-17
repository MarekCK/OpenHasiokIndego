#pragma once

typedef enum
{
    CMD_STOP = 0,
    CMD_FORWARD,
    CMD_BACKWARD,
    CMD_LEFT,
    CMD_RIGHT
} ohi_cmd_t;

extern volatile ohi_cmd_t ohi_cmd;
