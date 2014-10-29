/**
 * @file shell_cmds_fatfs.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 27-Oct-2014
 * @brief Implementation for FAT-FS related shell commands. This implement is
 * based on elm-chan FAT file system library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "shell_cmds_fatfs.h"
#include "ff.h"
#include "diskio.h"
#include "MB1_System.h"

/*------------------- Const and Definitions ----------------------------------*/
/* TODO: move this fat fs initialization code to HA system init */
/* 10ms or 1ms timer is required for FAT FS */
const ISRMgr_ns::ISR_t timer_1ms = ISRMgr_ns::ISRMgr_TIM6;

const char default_drive_path[] = "0:/";

const char ls_usage[] = "Usage:\n"
        "\tls, list files and folders in current directory\n"
        "\tls [path], list files and folders in path\n";
const char ls_line_template[] = "%c%c%c%c\t%lu\t%04u %02u %02u %02u:%02u:%02u\t%s\n";
/* drws size year month day hh:mm:sec name */

/*------------------- Global var for FAT FS ----------------------------------*/
FATFS fatfs;

/*------------------- Static functions ---------------------------------------*/
static void print_ferr(FRESULT res);
static void parse_fdate(WORD fdate, uint16_t &year, uint8_t &month, uint8_t &day);
static void parse_ftime(WORD ftime, uint8_t &hour, uint8_t &min, uint8_t &sec);

/*----------------------------------------------------------------------------*/
void mount(int argc, char** argv)
{
    FRESULT fres;

    /* assign timer for FAT FS */
    /* TODO: move this fat fs initialization code to HA system init */
    MB1_ISRs.subISR_assign(timer_1ms, disk_timerproc_1ms);

    fres = f_mount(&fatfs, default_drive_path, 1);
    if (fres != FR_OK) {
        print_ferr(fres);
        return;
    }
    printf("FAT FS is mounted to %s\n", default_drive_path);
}

/*----------------------------------------------------------------------------*/
void umount(int argc, char** argv)
{
    FRESULT fres;

    fres = f_mount(NULL, default_drive_path, 1);
    if (fres != FR_OK) {
        print_ferr(fres);
    }

    /* remove timer */
    /* TODO: move this fat fs initialization code to HA system init */
    MB1_ISRs.subISR_remove(timer_1ms, disk_timerproc_1ms);

    printf("FAT FS is umounted from %s\n", default_drive_path);
}

/*----------------------------------------------------------------------------*/
void ls(int argc, char** argv)
{
    char path_tmp[8];
    char *path_p = NULL;
    FRESULT fres;
    DIR dir;
    FILINFO finfo;
    uint16_t year;
    uint8_t month, day, hour, min, sec;

    if (argc == 1) {
        /* list files and folder in current directory */
        if (_FS_RPATH < 1) {
            printf("Note: relative path was disabled, list default drive path: %s\n",
                    default_drive_path);
            memcpy(path_tmp, default_drive_path, sizeof(default_drive_path));
        }
        else {
            strcpy(path_tmp, ".");
        }

        path_p = path_tmp;
    }
    else if (argc == 2) {
        path_p = argv[1];
    }
    else {
        printf("Err: Wrong number of arguments.\n");
        /* print ls usage */
        printf("%s", ls_usage);
        return;
    }

    /* List files and folders in path_p */
    fres = f_opendir(&dir, path_p);
    if (fres != FR_OK) {
        print_ferr(fres);
        return;
    }

    while (1) {
        fres = f_readdir(&dir, &finfo);
        if (fres != FR_OK) { /* error when read dir */
            print_ferr(fres);
            return;
        }

        if (finfo.fname[0] == 0) { /* end of dir */
            break;
        }

        /* print info */
        /* parse date, time */
        parse_fdate(finfo.fdate, year, month, day);
        parse_ftime(finfo.ftime, hour, min, sec);

        printf(ls_line_template,
                (finfo.fattrib & AM_DIR) ? 'd' : '-',
                'r',
                (finfo.fattrib & AM_RDO) ? '-' : 'w',
                (finfo.fattrib & AM_SYS) ? '-' : 'w',
                finfo.fsize,
                year, month, day,
                hour, min, sec,
                finfo.fname);

    }

    /* close dir */
    f_closedir(&dir);
}

/*----------------------------------------------------------------------------*/
void cat(int argc, char** argv)
{
    FRESULT fres;
    FIL file;
    char line[512];

    if (argc != 2) {
        printf("Wrong number of arguments.\n");
        /* TODO: print usage */
        return;
    }

    /* open file */
    fres = f_open(&file, argv[1], FA_READ);
    if (fres != FR_OK) {
        print_ferr(fres);
        return;
    }

    /* read file */
    while( f_gets(line, sizeof(line), &file) ){
        puts(line);
    }
}

/*------------------- Static functions implementation ------------------------*/
static void print_ferr(FRESULT res)
{
    switch (res) {
    case FR_OK:
        printf("FR_OK\n");
        break;
    case FR_DISK_ERR:
        printf("FR_DISK_ERR\n");
        break;
    case FR_INT_ERR:
        printf("FR_INT_ERR\n");
        break;
    case FR_NOT_READY:
        printf("FR_NOT_READY\n");
        break;
    case FR_NO_FILE:
        printf("FR_NO_FILE\n");
        break;
    case FR_NO_PATH:
        printf("FR_NO_PATH\n");
        break;
    case FR_INVALID_NAME:
        printf("FR_INVALID_NAME\n");
        break;
    case FR_DENIED:
        printf("FR_DENIED\n");
        break;
    case FR_EXIST:
        printf("FR_EXIST\n");
        break;
    case FR_INVALID_OBJECT:
        printf("FR_INVALID_OBJECT\n");
        break;
    case FR_WRITE_PROTECTED:
        printf("FR_WRITE_PROTECTED\n");
        break;
    case FR_INVALID_DRIVE:
        printf("FR_INVALID_DRIVE\n");
        break;
    case FR_NOT_ENABLED:
        printf("FR_NOT_ENABLED\n");
        break;
    case FR_NO_FILESYSTEM:
        printf("FR_NO_FILESYSTEM\n");
        break;
    case FR_MKFS_ABORTED:
        printf("FR_MKFS_ABORTED\n");
        break;
    case FR_TIMEOUT:
        printf("FR_TIMEOUT\n");
        break;
    case FR_LOCKED:
        printf("FR_LOCKED\n");
        break;
    case FR_NOT_ENOUGH_CORE:
        printf("FR_NOT_ENOUGH_CORE\n");
        break;
    case FR_TOO_MANY_OPEN_FILES:
        printf("FR_TOO_MANY_OPEN_FILES\n");
        break;
    case FR_INVALID_PARAMETER:
        printf("FR_INVALID_PARAMETER\n");
        break;
    default:
        printf("Unknown err code\n");
        break;
    }
}

/*----------------------------------------------------------------------------*/
static void parse_fdate(WORD fdate, uint16_t &year, uint8_t &month, uint8_t &day)
{
    year = ((uint16_t) fdate >> 9) + 1980;
    month = (uint8_t) ((fdate & 0x00FF) >> 5);
    day = (uint8_t) (fdate & 0x001F);
}

/*----------------------------------------------------------------------------*/
static void parse_ftime(WORD ftime, uint8_t &hour, uint8_t &min, uint8_t &sec)
{
    hour = (uint8_t) (ftime >> 11);
    min = (uint8_t) ((ftime & 0x07FF) >> 5);
    sec = (uint8_t) (ftime & 0x001F);
}
