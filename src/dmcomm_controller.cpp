// This file is part of the DMComm project by BladeSabre. License: MIT.

#include "DMComm.h"

namespace DMComm {

Controller::Controller(uint16_t buffer[], uint16_t buffer_size) :
    buffer_(buffer), buffer_size_(buffer_size) {}

void Controller::connect(ClassicCommunicator& classic_comm) {
    classic_comm_ = &classic_comm;
}

void Controller::execute(BaseDigiROM& digirom, uint16_t listen_timeout_ms) {
    current_comm_ = nullptr;
    switch(digirom.signal_type()) {
    case kSignalTypeV:
    case kSignalTypeX:
    case kSignalTypeY:
        current_comm_ = classic_comm_;
        break;
    }
    if (current_comm_ == nullptr) {
        return; //TODO should we have some return values?
    }
    current_digirom_ = &digirom;
    digirom.prepare();
    current_comm_->prepare(digirom.signal_type(), listen_timeout_ms);
    if (digirom.turn() == 0) {
        while (receive());
    } else if (digirom.turn() == 1) {
        while (true) {
            if (!send()) {
                return;
            }
            if (!receive()) {
                return;
            }
        }
    } else if (digirom.turn() == 2) {
        while (true) {
            if (!receive()) {
                return;
            }
            if (!send()) {
                return;
            }
        }
    }
}

bool Controller::send() {
    uint16_t length = current_digirom_->send(buffer_, buffer_size_);
    if (length == 0) {
        return false;
    }
    current_comm_->send(buffer_, length);
    return true;
}

bool Controller::receive() {
    ReceiveOutcome outcome = current_comm_->receive(buffer_, buffer_size_);
    current_digirom_->receive(buffer_, outcome);
    return outcome.status == kStatusReceived;
}

}  // namespace DMComm
