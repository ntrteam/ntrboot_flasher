#include "hid.h"

bool CheckButton(uint32_t button) {
    uint32_t t_pressed = 0;
    for(; (t_pressed < 0x13000) && ((HID_STATE & button) == button); t_pressed++);
    return (t_pressed >= 0x13000);
}

uint32_t WaitButton(uint32_t button) {
    uint32_t state = (HID_STATE & button);
    while(state == (HID_STATE & button)); // wait for button press
    state = (HID_STATE & button);
    while((HID_STATE & button) != 0); // wait for let go

    return state;
}