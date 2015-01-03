/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        zone.cpp
 * @brief       zone class.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#include <string.h>
#include <stdlib.h>

#include "zone.h"
#include "ff.h"
#include "shell_cmds_fatfs.h"

static const char zone_cmd_usage[] = "Usage:\n"
        "zone -s id(hex) name, set zone name\n"
        "zone -g id(hex), get zone name\n"
        "zone -h, get this help\n";

using namespace zone_ns;

/*----------------------------- Configurations -------------------------------*/
#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

/*----------------------------- Public methods -------------------------------*/
zone::zone(void)
{
    return;
}

/*----------------------------------------------------------------------------*/
int8_t zone::set_zone_name(uint8_t zone_id, const char *zone_name)
{
    FIL file;
    FRESULT fres, fres2;
    FILINFO fno;
    char path[zone_file_max_path_size];

    /* check directory existed or not */
    fres = f_stat(ZONES_FOLDER, &fno);
    if (fres == FR_NO_FILE) {
        HA_DEBUG("zone::set_zone_name:" ZONES_FOLDER "is not exists, creating\n");
        fres2 = f_mkdir(ZONES_FOLDER);
        if (fres2 == FR_OK) {
            HA_DEBUG("zone::set_zone_name:" ZONES_FOLDER "is created\n");
        }
        else {
            print_ferr(fres);
            return -1;
        }
    }
    else if (fres != FR_OK) {
        print_ferr(fres);
        return -1;
    }

    /* build path */
    snprintf(path, zone_file_max_path_size, ZONES_FOLDER "/%x", zone_id);

    /* open zone file */
    fres = f_open(&file, path, FA_WRITE | FA_CREATE_ALWAYS);
    if (fres != FR_OK) {
        HA_DEBUG("zone::set_zone_name: Error when open file %s\n", path);
        print_ferr(fres);
        return -1;
    }
    f_sync(&file);

    /* write name to file */
    f_printf(&file, "%s", zone_name);

    /* close file */
    f_close(&file);

    return 0;
}

/*----------------------------------------------------------------------------*/
int8_t zone::get_zone_name(uint8_t zone_id, uint8_t buf_size, char *zone_name)
{
    FIL file;
    FRESULT fres;
    char path[zone_file_max_path_size];

    /* build path */
    snprintf(path, zone_file_max_path_size, ZONES_FOLDER "/%x", zone_id);

    /* open file */
    fres = f_open(&file, path, FA_READ | FA_OPEN_ALWAYS);
    if (fres != FR_OK) {
        HA_DEBUG("zone::get_zone_name: Error when open file %s\n", path);
        print_ferr(fres);
        return -1;
    }

    /* get name */
    memset(zone_name, '\0', buf_size);
    f_gets(zone_name, buf_size, &file);

    /* close file */
    f_close(&file);

    return 0;
}

/*----------------------------------------------------------------------------*/
void zone::get_zone_folder_name(uint8_t buf_size, char *zone_folder_name)
{
    strncpy(zone_folder_name, ZONES_FOLDER, buf_size);
    zone_folder_name[buf_size] = '\0';
}

/*----------------------------- Shell command --------------------------------*/
void zone_cmd(zone &zone_obj, int argc, char **argv)
{
    uint8_t zone_id;
    char *zone_name_p;
    char zone_name[zone_name_max_size];

    if (argc == 1) {
        printf("Err: too few argument, zone -h to get help.\n");
        return;
    }

    for (uint8_t count = 1; count < argc; count++) {
        if (argv[count][0] == '-') { /* option */
            switch(argv[count][1]) {
            case 's':
                if (count + 2 >= argc) {
                    printf("Err: too few arguments for %s\n", argv[count]);
                    return;
                }

                zone_id = strtol(argv[++count], NULL, 16);
                zone_name_p = argv[++count];
                zone_obj.set_zone_name(zone_id, zone_name_p);

                /* read back */
                zone_obj.get_zone_name(zone_id, zone_name_max_size, zone_name);
                printf("Zone 0x%x: %s\n", zone_id, zone_name);

                break;

            case 'g':
                if (count + 1 >= argc) {
                    printf("Err: too few arguments for %s\n", argv[count]);
                    return;
                }

                zone_id = strtol(argv[++count], NULL, 16);
                zone_obj.get_zone_name(zone_id, zone_name_max_size, zone_name);
                printf("Zone 0x%x: %s\n", zone_id, zone_name);
                break;

            case 'h':
                printf("%s", zone_cmd_usage);
                break;

            default:
                printf("Unknown option %s\n", argv[count]);
                return;
            }
        }
    }/* end for */
}
