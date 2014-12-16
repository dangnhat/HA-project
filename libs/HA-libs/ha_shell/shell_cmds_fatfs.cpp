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
const char default_drive_path[] = "0:/";

const char ls_usage[] = "Usage:\n"
        "ls, list files and folders in current directory\n"
        "ls [path], list files and folders in path\n"
        "ls -h, get this help.\n";
const char ls_line_template[] = "%c%c%c%c\t%lu\t%04u %02u %02u %02u:%02u:%02u\t%s\n";
/* drws size year month day hh:mm:sec name */

const char cat_usage[] = "Usage:\n"
        "cat [file_name], concatenate files and print on the standard output.\n"
        "cat -h, get this help.\n";

const char touch_usage[] = "Usage:\n"
        "touch [file_name], change file timestamp.\n"
        "touch -h, get this help.\n";

const char rm_usage[] = "Usage:\n"
        "rm [file/sub-folder], remove a file or a empty sub-folder.\n"
        "rm -r [sub-folder], remove sub-folder with all of its contents.\n"
        "rm -h, get this help.\n"
        "Note: total path name shouldn't be > ";
const uint16_t RM_MAX_PATH_LEN = 64;

const char rn_usage[] = "Usage:\n"
        "rn [old path] [new path], rename old path name to new path name.\n"
        "rn -h, get this help.\n";
const uint16_t RN_MAX_PATH_LEN = 64;

const char mkdir_usage[] = "Usage:\n"
        "mkdir [sub-folder], make directories.\n"
        "mkdir -h, get this help.\n";

const char cd_usage[] = "Usage:\n"
        "cd [path], change working directory.\n"
        "cd -h, get this help.\n";

const char pwd_usage[] = "Usage:\n"
        "pwd, print name of current/working directory.\n"
        "pwd -h, get this help.\n"
        "Note: total path name is limited to ";
const uint16_t PWD_MAX_PATH_LEN = 64;

/*------------------- Global var for FAT FS ----------------------------------*/
static FATFS fatfs;

/*------------------- Static functions ---------------------------------------*/
static void parse_fdate(WORD fdate, uint16_t &year, uint8_t &month, uint8_t &day);
static void parse_ftime(WORD ftime, uint8_t &hour, uint8_t &min, uint8_t &sec);

static void del_folder_r(TCHAR* path);

/*----------------------------------------------------------------------------*/
void mount(int argc, char** argv)
{
    FRESULT fres;

    if (argc > 1) {
        printf("Err: wrong number of arguments\n");
        return;
    }

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

    if (argc > 1) {
        printf("Err: wrong number of arguments\n");
        return;
    }

    fres = f_mount(NULL, default_drive_path, 1);
    if (fres != FR_OK) {
        print_ferr(fres);
    }

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
    int count;

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
    } else {
        for (count = 1; count < argc; count++) {
            if (argv[count][0] == '-') { /* options */
                switch (argv[count][1]) {
                case 'h':
                    printf("%s", ls_usage);
                    return;
                default:
                    printf("Err: unknow option.\n");
                    return;
                }
            }
            else { /* path */
                path_p = argv[count];
                break;
            }
        } /* end for */
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
                (finfo.fattrib & AM_SYS) ? '-' : 's',
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
    char line[64];
    char* path_p = NULL;
    int count;

    if (argc == 1) {
        printf("Err: missing arguments. Try -h for help.\n");
        return;
    }

    for (count = 1; count < argc; count++) {
        if (argv[count][0] == '-') { /* options */
            switch (argv[count][1]) {
            case 'h':
                printf("%s", cat_usage);
                return;
            default:
                printf("Err: unknow option.\n");
                return;
            }
        }
        else { /* path */
            path_p = argv[count];
            break;
        }
    } /* end for */

    /* open file */
    fres = f_open(&file, path_p, FA_READ);
    if (fres != FR_OK) {
        print_ferr(fres);
        return;
    }

    /* read file */
    while( f_gets(line, sizeof(line), &file) ){
        printf("%s", line);
        fflush(stdout);
    }

    f_close(&file);
}

/*----------------------------------------------------------------------------*/
void touch(int argc, char** argv)
{
    int count;
    FIL file;
    FRESULT fres;
    DWORD timestamp;
    FILINFO finfo;

    if (argc == 1) {
        printf("Err: missing arguments. Try -h for help.\n");
        return;
    }

    for (count = 1; count < argc; count++) {
        if (argv[count][0] == '-') { /* options */
            switch (argv[count][1]) {
            case 'h':
                printf("%s", touch_usage);
                return;
            default:
                printf("Err: unknow option.\n");
                return;
            }
        }
        else { /* file name */
            /* open file */
            fres = f_open(&file, argv[count], FA_OPEN_ALWAYS);
            if (fres != FR_OK) {
                print_ferr(fres);
                return;
            }

            /* close file */
            fres = f_close(&file);
            if (fres != FR_OK) {
                print_ferr(fres);
                return;
            }

            /* change timestamp */
            timestamp = get_fattime();
            finfo.fdate = (WORD) (timestamp >> 16);
            finfo.ftime = (WORD) timestamp;
            fres = f_utime(argv[count], &finfo);
            if (fres != FR_OK) {
                print_ferr(fres);
                return;
            }

            return;
        }
    } /* end for */
}

/*----------------------------------------------------------------------------*/
void rm(int argc, char** argv)
{
    int count;
    bool r_flag = false;
    bool cont_flag = true;
    FRESULT fres;
    FILINFO finfo;

    TCHAR path[RM_MAX_PATH_LEN];

    if (argc == 1) {
        printf("Err: missing arguments. Try -h to get help.\n");
        return;
    }

    for (count = 1; count < argc; count++) {
        if (argv[count][0] == '-') { /* option */
            switch (argv[count][1]) {
            case 'h':
                printf("%s%d\n", rm_usage, RM_MAX_PATH_LEN);
                cont_flag = false;
                break;
            case 'r':
                r_flag = true;
                cont_flag = true;
                break;
            default:
                printf("Err: unknow option.\n");
                cont_flag = false;
                break;
            }
        }/* end option */
        else { /* path */
            /* check if it is folder or file */
            f_stat(argv[count], &finfo);

            if (!r_flag || !(finfo.fattrib & AM_DIR)) { /* just unlink */
                fres = f_unlink(argv[count]);
                if (fres != FR_OK) {
                    print_ferr(fres);
                }
            }
            else { /* folder with -r */
                memcpy(path, argv[count], strlen(argv[count]));
                path[strlen(argv[count])] = '\0';
                del_folder_r(path);
            }

            cont_flag = false;

        }/* end path */

        if (!cont_flag) {
            break;
        }
    }/* end for */
}

/*----------------------------------------------------------------------------*/
void mv(int argc, char** argv)
{
    FRESULT fres;

    if (argc != 3) {
        printf("Err: missing arguments. Try -h to get help.\n");
        return;
    }

    /* rename */
    fres = f_rename(&argv[1][0], &argv[2][0]);
    if (fres != FR_OK) {
        print_ferr(fres);
    }
}

/*----------------------------------------------------------------------------*/
void mkdir(int argc, char** argv)
{
    bool cont_flag;
    int count;
    FRESULT fres;

    if (argc == 1) {
        printf("Err: missing argument. Try -h to get help.\n");
        return;
    }

    for (count = 1; count < argc; count++) {
        if (argv[count][0] == '-') { /* option */
            switch (argv[count][1]) {
            case 'h':
                printf("%s", mkdir_usage);
                cont_flag = false;
                break;
            default:
                printf("Err: unknow option.\n");
                cont_flag = false;
                break;
            }
        }/* end option */
        else { /* sub-folder path */
            fres = f_mkdir(argv[count]);
            if (fres != FR_OK) {
                print_ferr(fres);
                cont_flag = false;
            }
        }

        if (!cont_flag) {
            break;
        }
    }/* end for */
}

/*----------------------------------------------------------------------------*/
void cd(int argc, char** argv) {
    bool cont_flag;
    int count;
    FRESULT fres;

    if (argc == 1) {
        return;
    }

    for (count = 1; count < argc; count++) {
        if (argv[count][0] == '-') { /* option */
            switch (argv[count][1]) {
            case 'h':
                printf("%s", cd_usage);
                cont_flag = false;
                break;
            default:
                printf("Err: unknow option.\n");
                cont_flag = false;
                break;
            }
        }/* end option */
        else { /* sub-folder path */
            fres = f_chdir(argv[count]);
            if (fres != FR_OK) {
                print_ferr(fres);
                cont_flag = false;
            }
        }

        if (!cont_flag) {
            break;
        }
    }/* end for */
}

/*----------------------------------------------------------------------------*/
void pwd(int argc, char** argv) {
    int count;
    FRESULT fres;
    char path[PWD_MAX_PATH_LEN];

    for (count = 1; count < argc; count++) {
        if (argv[count][0] == '-') { /* option */
            switch (argv[count][1]) {
            case 'h':
                printf("%s%d\n", pwd_usage, PWD_MAX_PATH_LEN);
                return;
            default:
                printf("Err: unknow option.\n");
                return;
            }
        }/* end option */
    }/* end for */

    /* no error with option */
    fres = f_getcwd(path, PWD_MAX_PATH_LEN);
    if (fres != FR_OK) {
        print_ferr(fres);
        return;
    }

    printf("%s\n", path);
}

/*----------------------------------------------------------------------------*/
void print_ferr(FRESULT res)
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

/*------------------- Static functions implementation ------------------------*/
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

/*----------------------------------------------------------------------------*/
static void del_folder_r(TCHAR* path) /* path is also used for workspace */
{
    FRESULT fres;
    DIR dir;
    FILINFO finfo;
    int pathlen;

    /* opendir */
    fres = f_opendir(&dir, path);
    if (fres != FR_OK) {
        printf("%s, opening: ", path);
        print_ferr(fres);
        return;
    }

    /* readdir */
    pathlen = strlen(path);
    while (1) {
        fres = f_readdir(&dir, &finfo);
        if (fres != FR_OK) {
            printf("%s, reading: ", path);
            print_ferr(fres);
        }

        if (finfo.fname[0] == 0) { /* end of dir */
            break;
        }

        if (finfo.fname[0] == '.') { /* ignore . or .. */
            continue;
        }

        /* file or sub-folder */
        sprintf(&path[pathlen], "/%s", finfo.fname);

        if (finfo.fattrib & AM_DIR) { /* sub dir */
            del_folder_r(path);
        }
        else { /* file */
            fres = f_unlink(path);
            if (fres != FR_OK) {
                printf("%s, unlinking: ", path);
                print_ferr(fres);
            }
        }

        /* restore old path */
        path[pathlen] = '\0';
    }/* end while, folder is empty */

    /* close dir */
    fres = f_closedir(&dir);
    if (fres != FR_OK) {
        printf("%s, closing: ", path);
        print_ferr(fres);
    }

    /* unlink dir */
    fres = f_unlink(path);
    if (fres != FR_OK) {
        printf("%s, unlinking: ", path);
        print_ferr(fres);
    }
}
