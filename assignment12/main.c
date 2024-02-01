/******************************************************************
 *
 * This the outline for the "device driver"
 *
 *****************************************************************/

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "device.h"

size_t DESCRIPTOR_RING_SIZE = 3;
size_t DEFAULT_BUFFER_SIZE = 1024;

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

uint32_t CTRL_FLAG_ENABLE_INTERRUPT = 0x1;
uint32_t CTRL_FLAG_ENABLE_OVERRUN_INTERRUPT = 0x2;
uint32_t CTRL_FLAG_DEVICE_RUNNING = 0x4;

uint32_t STATUS_FLAG_RUNNING = 0x1;
uint32_t STATUS_FLAG_NO_DESCRIPTORS = 0x2;
uint32_t STATUS_FLAG_BUFFER_NOT_LARGE_ENOUGH = 0x4;
uint32_t STATUS_FLAG_RECEIVE_INTERRUPT_PENDING = 0x8;
uint32_t STATUS_FLAG_OVERRUN_INTERRUPT_PENDING = 0x16;

static uint32_t CURRENT_OS_INDEX = 0;

void out(mpfc_desc *desc) {
    printf(ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET, desc->buffer);
}

#define logf(message, ...) printf(ANSI_COLOR_RED message "\n" ANSI_COLOR_RESET, __VA_ARGS__)
#define logcf(message, ...) printf(ANSI_COLOR_RED message ANSI_COLOR_RESET, __VA_ARGS__)
#define logc(message) printf(ANSI_COLOR_RED message ANSI_COLOR_RESET)
#define log(message) printf(ANSI_COLOR_RED message "\n" ANSI_COLOR_RESET)
#define next_index() ((CURRENT_OS_INDEX + 1) % mpfc_read_size())

void log_ownership() {
    mpfc_desc *descriptors = (mpfc_desc *)mpfc_read_base();
    size_t size = mpfc_read_size();
    logc("[");
    for (uint32_t i = 0; i < size; i++) {
        logcf("%c", descriptors[i].owned ? 'D' : 'O');
    }

    logc("]\n");
}

void log_descriptors() {
    mpfc_desc *descriptors = (mpfc_desc *)mpfc_read_base();
    size_t size = mpfc_read_size();
    for (uint32_t i = 0; i < size; i++) {
        logf(
            "BUFFER %i (owned: %x, overflow: %x, size: %i): %s",
            i,
            descriptors[i].owned,
            descriptors[i].overflow,
            descriptors[i].size,
            descriptors[i].buffer);
    }
}

//
// My little interrupt handler
// (Should be written as part of STEP-1)
//
void irq() {
    // Handle interrupts here
    // This function should be small, and should not block indefinitely.
    // ...
    //
    // You should inform the device that you have seen the interrupt.
    // You can use the status register to communicate with the device.
    log_descriptors();
    uint32_t status = mpfc_read_status();
    logf(
        "interrupt status (running: %x, no descriptors: %x, not large enough: %x, receive pending: %x, overrun pending: %x)",
        !!(status & STATUS_FLAG_RUNNING),
        !!(status & STATUS_FLAG_NO_DESCRIPTORS),
        !!(status & STATUS_FLAG_BUFFER_NOT_LARGE_ENOUGH),
        !!(status & STATUS_FLAG_RECEIVE_INTERRUPT_PENDING),
        !!(status & STATUS_FLAG_OVERRUN_INTERRUPT_PENDING));

    if (status & STATUS_FLAG_NO_DESCRIPTORS) {
        os_wakeup();
    }

    if (status & STATUS_FLAG_BUFFER_NOT_LARGE_ENOUGH) {
        os_wakeup();
    }

    if (status & STATUS_FLAG_RECEIVE_INTERRUPT_PENDING) {
        os_wakeup();
    }

    if (status & STATUS_FLAG_OVERRUN_INTERRUPT_PENDING) {
        os_wakeup();
    }
}

void init_descriptor_ring() {
    mpfc_desc *descriptors = calloc(sizeof(mpfc_desc), DESCRIPTOR_RING_SIZE);
    for (uint32_t i = 0; i < DESCRIPTOR_RING_SIZE; i++) {
        descriptors[i].owned = true;  // device owns it
        descriptors[i].buffer = malloc(DEFAULT_BUFFER_SIZE);
        descriptors[i].size = DEFAULT_BUFFER_SIZE;
        descriptors[i].overflow = false;
    }

    mpfc_write_size(DESCRIPTOR_RING_SIZE);
    mpfc_write_base(descriptors);
}

//
// Main function
//
int main(int argc, char *argv[]) {
    start_device_emulation();

    // Initialize the device:
    //  - STEP-1: Register interrupt handlers
    //  - STEP-2: Create the descriptors and buffers, and register them
    //  - STEP-3: Start the device running

    log("NYI");

    init_descriptor_ring();
    os_register_irq(irq);

    while (true) {
        mpfc_write_ctrl(CTRL_FLAG_DEVICE_RUNNING | CTRL_FLAG_ENABLE_INTERRUPT | CTRL_FLAG_ENABLE_OVERRUN_INTERRUPT);
        os_sleep();
        mpfc_write_ctrl(CTRL_FLAG_DEVICE_RUNNING);

        mpfc_desc *descriptors = (mpfc_desc *)mpfc_read_base();
        size_t size = mpfc_read_size();

        // STEP-4: Consume the data.

        // Try to receive from the device

        // If there's a valid descriptor, see what's in it, and return
        // ownership of it back to the device.

        // If there's no valid descriptor, then request an interrupt from
        // the device when new data is ready and go to sleep.

        logf("Current ownership: (index = %i)", CURRENT_OS_INDEX);
        log_descriptors();
        if (!descriptors[CURRENT_OS_INDEX].owned) {
            if (descriptors[CURRENT_OS_INDEX].overflow) {
                logf("%s", descriptors[CURRENT_OS_INDEX].buffer);
                size_t new_size = descriptors[CURRENT_OS_INDEX].size * 2;
                descriptors[CURRENT_OS_INDEX].owned = true;
                descriptors[CURRENT_OS_INDEX].size = new_size;
                descriptors[CURRENT_OS_INDEX].buffer = realloc(
                    descriptors[CURRENT_OS_INDEX].buffer,
                    new_size);
                mpfc_write_status(1);
            } else {
                out(&descriptors[CURRENT_OS_INDEX]);
                descriptors[CURRENT_OS_INDEX].owned = true;
                descriptors[CURRENT_OS_INDEX].overflow = false;
                mpfc_write_status(1);
            }
            
            CURRENT_OS_INDEX = next_index();
        }
    }

    return 0;
}
