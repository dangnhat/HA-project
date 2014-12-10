/**
 * @file shell_cmds_sixlowpan.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 10-Nov-2014
 * @brief Source files for 6lowpan related shell commands.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern "C" {
#include "msg.h"
}

#include "shell_cmds_sixlowpan.h"
#include "ha_sixlowpan.h"
#include "gff_mesg_id.h"
#include "ff.h"
#include "shell_cmds_fatfs.h"
#include "ha_gff_misc.h"

const char slp_usage[] = "Usage:\n"
                    "6lowpan, show current 6lowpan configurations\n"
                    "6lowpan -p prefix3:prefix2:prefix1:prefix0, set 64bit prefixes\n"
                    "6lowpan -n node_id (hex), set node id (should always be > 0)\n"
                    "6lowpan -t netdev_type, set network device type,"
                        "r (root router), n (node router), or h (host)\n"
                    "6lowpan -c channel, set channel\n"
                    "6lowpan -s, start 6lowpan stack\n"
                    "6lowpan -x device_id(hex) value(hex), send SET-DEV_VAL message to a device\n"
                    "6lowpan -h, get help\n"
                    "note: multiple options can be combined together\n";
const char slp_prefix_pattern[] = "%lx:%lx:%lx:%lx";

void sixlowpan_config(int argc, char** argv)
{
    char line_buffer[32];
    FRESULT fres;
    FIL file;
    uint16_t count;

    uint32_t prefixes[4];   /* only 16bit per prefix is needed, */
                            /* we use uint32_t because %hx is not supported */
    uint32_t node_id;
    char netdev_type;
    uint32_t channel;
    UINT byte_read;
    int byte_written;

    msg_t mesg;
    uint32_t sto_device_id;
    uint16_t sto_value;
    uint8_t set_dev_val_buffer[1 + 2 + ha_ns::SET_DEV_VAL_DATA_LEN];

    char config_str[ha_ns::sixlowpan_pattern_maxsize];

    if (argc == 1) {
        /* print current configurations */
        fres = f_open(&file, ha_ns::sixlowpan_config_file, FA_READ);
        if (fres != FR_OK) {
            printf("Error when reading config file %s\n", ha_ns::sixlowpan_config_file);
            print_ferr(fres);
            return;
        }

        while( f_gets(line_buffer, sizeof(line_buffer), &file) ){
            printf("%s", line_buffer);
        }

        f_close(&file);
        return;
    }

    /* open file and read all configurations */
    memset(prefixes, 0, sizeof(prefixes));
    node_id = 0;
    netdev_type = '0';
    channel = 0;

    fres = f_open(&file, ha_ns::sixlowpan_config_file, FA_READ | FA_OPEN_ALWAYS);
    if (fres != FR_OK) {
        print_ferr(fres);
        return;
    }

    fres = f_read(&file, config_str, ha_ns::sixlowpan_pattern_maxsize, &byte_read);
    if (fres != FR_OK) {
        printf("Err when read config file, byte_read %u\n", byte_read);
        print_ferr(fres);
        return;
    }

    sscanf(config_str, ha_ns::sixlowpan_config_pattern,
            &prefixes[3], &prefixes[2], &prefixes[1], &prefixes[0],
            &node_id,
            &netdev_type,
            &channel);

    f_close(&file);

    /* process option */
    for (count = 1; count < argc; count++) {
        if (argv[count][0] == '-') { /* an option */
            switch (argv[count][1]) {
            case 'p':
                if (count + 1 >= argc) {
                    printf("Err: missing argument for option %s\n", argv[count]);
                    return;
                }
                count++;
                sscanf(argv[count], slp_prefix_pattern, &prefixes[3], &prefixes[2],
                        &prefixes[1], &prefixes[0]);
                break;

            case 'n':
                if (count + 1 >= argc) {
                    printf("Err: missing argument for option %s\n", argv[count]);
                    return;
                }
                count++;
                node_id = strtol(argv[count], NULL, 16);
                if (node_id < 1) {
                    printf("Err: argument for -n should > 0\n");
                    return;
                }
                break;

            case 't':
                if (count + 1 >= argc) {
                    printf("Err: missing argument for option %s\n", argv[count]);
                    return;
                }
                count++;
                netdev_type = argv[count][0];
                if (netdev_type != 'r' && netdev_type != 'n' && netdev_type != 'h') {
                    printf("Err: argument for -t should be r, n, or h\n");
                    return;
                }
                break;

            case 'c':
                if (count + 1 >= argc) {
                    printf("Err: missing argument for option %s\n", argv[count]);
                    return;
                }
                count++;
                channel = atoi(argv[count]);
                break;

            case 's':
                mesg.type = ha_ns::SIXLOWPAN_RESTART;
                msg_send(&mesg, ha_ns::sixlowpan_sender_pid, false);
                return;

            case 'x':
                if (count + 2 >= argc) {
                    printf("Err: missing argument for option %s\n", argv[count]);
                    return;
                }

                sto_device_id = strtol(argv[count + 1], NULL, 16);
                sto_value = strtol(argv[count + 2], NULL, 10);

                set_dev_val_buffer[0] = ha_ns::SET_DEV_VAL_DATA_LEN;
                uint162buf(ha_ns::SET_DEV_VAL, &set_dev_val_buffer[1]);
                uint322buf(sto_device_id, &set_dev_val_buffer[3]);
                uint162buf(sto_value, &set_dev_val_buffer[7]);

                ha_ns::sixlowpan_sender_gff_queue.add_data(set_dev_val_buffer,
                        3 + set_dev_val_buffer[0]);

                mesg.type = ha_ns::GFF_PENDING;
                mesg.content.ptr = (char *) &ha_ns::sixlowpan_sender_gff_queue;
                msg_send(&mesg, ha_ns::sixlowpan_sender_pid, false);
                return;

            case 'h':
                /* print help */
                printf("%s", slp_usage);
                return;
            default:
                break;
            }/* end switch */
        }/* end option */
    }

    /* Write configurations to file */
    fres = f_open(&file, ha_ns::sixlowpan_config_file, FA_WRITE | FA_CREATE_ALWAYS);
    if (fres != FR_OK) {
        printf("Err when open config file to write data\n");
        print_ferr(fres);
        return;
    }
    f_sync(&file);

    byte_written = f_printf(&file, ha_ns::sixlowpan_config_pattern,
            prefixes[3], prefixes[2], prefixes[1], prefixes[0],
            node_id,
            netdev_type,
            channel);
    f_sync(&file);

    if (byte_written < 0) {
        printf("Err when write data to config file\n");
        print_ferr(fres);
        f_close(&file);
        return;
    }
    else {
        printf("%d bytes written to config file\n", byte_written);
        f_close(&file);
    }

    /* read back */
    fres = f_open(&file, ha_ns::sixlowpan_config_file, FA_READ);
    if (fres != FR_OK) {
        printf("Error when reading config file %s\n", ha_ns::sixlowpan_config_file);
        print_ferr(fres);
        return;
    }

    puts("---");
    while( f_gets(line_buffer, sizeof(line_buffer), &file) ){
        printf("%s", line_buffer);
    }

    f_close(&file);

    return;
}
