#include "stdio.h"
#include "stdint.h"

#include "cir_queue.h"

const uint16_t queue_size = 33;
uint8_t queue_buffer[queue_size];
const uint8_t data_size = 11;
uint8_t data[data_size] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
uint8_t data_ret[data_size];

cir_queue a_queue(queue_buffer, queue_size);

int main(void) {
    while (1) {
        /* add data */
        a_queue.add_data(data, data_size);

        /* get data */
        if (a_queue.get_size() != data_size) {
            printf("queue_size %lu != data_size %u\n", a_queue.get_size(), data_size);
            break;
        }

        a_queue.get_data(data_ret, data_size);
        for (int8_t count = 0; count < data_size; count++) {
            printf("%d ", data_ret[count]);
        }
        printf("\n");
    }
}
