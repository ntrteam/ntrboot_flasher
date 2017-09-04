// platform specific stuff

#include "common.h"
#include "device.h"
#include "protocol.h"
#include "protocol_ntr.h"
#include "ui.h"

void Flashcart::sendCommand(const uint8_t *cmdbuf, uint16_t response_len, uint8_t *resp, uint32_t flags) {
    NTR_SendCommand(cmdbuf, response_len, flags, resp);
}

void Flashcart::showProgress(uint32_t current, uint32_t total) {
    ShowProgress(TOP_SCREEN, current, total);
}