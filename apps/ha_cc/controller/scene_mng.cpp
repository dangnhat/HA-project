/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        scene.cpp
 * @brief       Scene manager class.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#include <stdlib.h>

#include "scene_mng.h"
#include "ff.h"
#include "shell_cmds_fatfs.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

static const uint8_t default_scene_index = 0;
static const uint8_t user_scene_index = 1;

#define SCENES_FOLDER       "SCENES"
#define DEFAULT_SCENE_FILE  "DEFAULT"
#define ACTIVE_SCENE_FILE   "ACTSCENE"

static const char scene_cmd_usage[] = "Usage:\n"
        "scene -l, show current default scene and user active scene.\n"
        "scene -l -s d|u, list default scene (d) or user active scene (u).\n"
        "scene -s d|u -a index active(0|1) -i cond (dev(hex) val | start end) -o act dev val, "
        "add a new rule to a scene.\n"
        "scene -s d|u -d index, remove a rule from scene.\n"
        "scene -s d|u -p, halt processing scene. Should be done before adding or removing rules.\n"
        "scene -s d|u -r, restart scene.\n"
        "scene -s d|u -v, save scene to file.\n"
        "scene -s d|u -e, restore scene from file.\n"
        "scene -n old_name new_name, rename scene.\n"
        "scene -h, get help.\n";

enum scene_cmd_type_e: uint8_t {
    NO_SCENE,
    DEFAULT_SCENE,
    USER_SCENE,
};

using namespace scene_ns;
using namespace scene_mng_ns;

/*----------------------------------------------------------------------------*/
scene_mng::scene_mng(ha_device_mng *cur_device_mng_p, rtc *rtc_obj_p,
            kernel_pid_t *out_pid_p, cir_queue *out_cir_queue_p)
{
    device_mng_p = cur_device_mng_p;
    rtc_p = rtc_obj_p;
    this->out_pid_p = out_pid_p;
    out_queue_p = out_cir_queue_p;

    /* set all scenes to invalid */
    for (uint8_t count; count < max_num_scenes; count++) {
        scenes_list[count].valid = false;
    }
}

/*----------------------------------------------------------------------------*/
void scene_mng::process(bool trigger_by_rpt, ha_device *a_device_rpt)
{
    for (uint8_t count = 0; count < max_num_scenes; count++) {
        if (scenes_list[count].valid) {
            HA_DEBUG("scene_mng::process: scene %hu is valid\n", count);
            scenes_list[count].scene_obj.process(trigger_by_rpt, a_device_rpt,
                    device_mng_p, rtc_p, out_queue_p, *out_pid_p);
        }
        else {
            HA_DEBUG("scene_mng::process: scene %hu is NOT valid\n", count);
        }
    }
}

/*----------------------------------------------------------------------------*/
void scene_mng::save(void)
{
    /* TODO: this seems to be really useless, implement when needed */
}

/*----------------------------------------------------------------------------*/
void scene_mng::restore(void)
{
    char user_active_name[scene_max_name_chars_wout_folders];

    restore_default_scene();

    /* restore user active scene */
    get_active_scene(user_active_name);
    set_user_scene(user_active_name);
    restore_user_scene();
}

/*------------------------ Current running user's scene ----------------------*/
void scene_mng::set_user_scene(const char *name)
{
    char name_with_folder[scene_max_name_chars];

    /* Build path name */
    strcpy(name_with_folder, SCENES_FOLDER "/");
    strcat(name_with_folder, name);

    scenes_list[user_scene_index].scene_obj.set_name(name_with_folder);
}

/*----------------------------------------------------------------------------*/
void scene_mng::get_user_scene(char *name)
{
    char name_with_folder[scene_max_name_chars];

    strcpy(name_with_folder, scenes_list[user_scene_index].scene_obj.get_name());
    not_dir(name_with_folder);

    memcpy(name, name_with_folder, scene_max_name_chars_wout_folders);
    name[scene_max_name_chars_wout_folders] = '\0';
}

/*----------------------------------------------------------------------------*/
scene *scene_mng::get_user_scene_ptr(void)
{
    return (get_scene_ptr_with_index(user_scene_index));
}

/*----------------------------------------------------------------------------*/
void scene_mng::restore_user_scene(void)
{
    /* set name for default scene */
    if (scenes_list[user_scene_index].scene_obj.restore() != 0) {
        HA_NOTIFY("Failed to restore user scene (%s)\n",
                scenes_list[user_scene_index].scene_obj.get_name());
        scenes_list[user_scene_index].valid = false;
    }
    else {
        HA_NOTIFY("User scene (%s) restored\n",
                scenes_list[user_scene_index].scene_obj.get_name());
        scenes_list[user_scene_index].valid = true;
    }
}

/*----------------------------------------------------------------------------*/
void scene_mng::set_user_scene_valid_status(bool status)
{
    scenes_list[user_scene_index].valid = status;
}

/*----------------------------------------------------------------------------*/
void scene_mng::print_user_scene(void)
{
    if (scenes_list[user_scene_index].valid) {
        HA_NOTIFY("Current running user scene (%s):\n"
                    "---\n", scenes_list[user_scene_index].scene_obj.get_name());
        scenes_list[user_scene_index].scene_obj.print(rtc_p);
    }
    else {
        HA_NOTIFY("Current running user scene (%s): Invalid\n",
                scenes_list[user_scene_index].scene_obj.get_name());
    }
}

/*------------------------ Active scene --------------------------------------*/
void scene_mng::set_active_scene(const char *name)
{
    FIL file;
    FRESULT fres;

    fres = f_open(&file, ACTIVE_SCENE_FILE, FA_WRITE | FA_CREATE_ALWAYS);
    if (fres != FR_OK) {
        HA_DEBUG("scene_mng::set_active_scene: Error when open file %s\n", ACTIVE_SCENE_FILE);
        print_ferr(fres);
        return;
    }
    f_sync(&file);

    /* write data to file */
    f_printf(&file, "%s\n", name);

    /* close file */
    f_close(&file);
}

/*----------------------------------------------------------------------------*/
void scene_mng::get_active_scene(char *name)
{
    FIL file;
    FRESULT fres;
    char line[16];
    uint16_t count;

    fres = f_open(&file, ACTIVE_SCENE_FILE, FA_READ | FA_OPEN_ALWAYS);
    if (fres != FR_OK) {
        HA_DEBUG("scene_mng::get_active_scene: Error when open file %s\n", ACTIVE_SCENE_FILE);
        print_ferr(fres);
        return;
    }

    /* read data from file */
    f_gets(line, 16, &file);

    /* close file */
    f_close(&file);

    /* copy to name */
    memcpy(name, line, scene_max_name_chars_wout_folders - 1);
    name[scene_max_name_chars_wout_folders] = '\0';

    /* remove ending '\n' */
    for (count = 0; count < scene_max_name_chars_wout_folders; count++) {
        if (name[count] == '\n') {
            name[count] = '\0';
            break;
        }
    }
}

/*------------------------ Inactive scene --------------------------------------*/
uint8_t scene_mng::get_num_of_inactive_scenes(void)
{
    DIR dir;
    FRESULT fres;
    FILINFO finfo;
    uint8_t retval = 0;
    char current_running_scene[scene_max_name_chars_wout_folders];

    /* get current running scene name */
    get_user_scene(current_running_scene);

    /* open dir */
    fres = f_opendir(&dir, SCENES_FOLDER);
    if (fres != FR_OK) {
        print_ferr(fres);
        return 0;
    }

    /* read dir */
    while (1) {
        fres = f_readdir(&dir, &finfo);
        if (fres != FR_OK) { /* error when read dir */
            print_ferr(fres);
            return 0;
        }

        if (finfo.fname[0] == 0) { /* end of dir */
            break;
        }

        /* compare with default scene name and .., ., and active scene name */
        if (strcmp(finfo.fname, DEFAULT_SCENE_FILE) == 0 ||
                strcmp(finfo.fname, current_running_scene) == 0 ||
                finfo.fname[0] == '.') {
            continue;
        }

        retval++;
    }

    /* close dir */
    f_closedir(&dir);

    return retval;
}

/*----------------------------------------------------------------------------*/
void scene_mng::get_inactive_scene_with_index(uint8_t index, char *name)
{
    DIR dir;
    FRESULT fres;
    FILINFO finfo;
    uint8_t count;
    char current_running_scene[scene_max_name_chars_wout_folders];

    /* get current running scene name */
    get_user_scene(current_running_scene);

    /* open dir */
    fres = f_opendir(&dir, SCENES_FOLDER);
    if (fres != FR_OK) {
        print_ferr(fres);
        return;
    }

    /* read dir */
    count = 0;
    while (1) {
        fres = f_readdir(&dir, &finfo);
        if (fres != FR_OK) { /* error when read dir */
            print_ferr(fres);
            return;
        }

        if (finfo.fname[0] == 0) { /* end of dir */
            break;
        }

        /* compare with default scene name and .., . */
        if (strcmp(finfo.fname, DEFAULT_SCENE_FILE) == 0 ||
                strcmp(finfo.fname, current_running_scene) == 0 ||
                finfo.fname[0] == '.') {
            continue;
        }

        /* an scene is here */
        if (count == index) {
            /* it's here */
            memcpy(name, finfo.fname, scene_max_name_chars_wout_folders);
            name[scene_max_name_chars_wout_folders-1] = '\0';
            break;
        }

        count++;
    }

    /* close dir */
    f_closedir(&dir);
}

/*----------------------------------------------------------------------------*/
int8_t scene_mng::remove_inactive_scene(const char *name)
{
    FRESULT fres;
    char name_with_folder[scene_max_name_chars];

    /* Build path name */
    strcpy(name_with_folder, SCENES_FOLDER "/");
    strcat(name_with_folder, name);

    fres = f_unlink(name_with_folder);
    if (fres != FR_OK) {
        HA_NOTIFY("scene_mng::remove_inactive_scene failed\n");
        print_ferr(fres);
        return -1;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
int8_t scene_mng::rename_inactive_scene(const char *old_name, const char *new_name)
{
    FRESULT fres;
    char name_with_folder[scene_max_name_chars];
    char name_with_folder_new[scene_max_name_chars];

    /* Build paths */
    strcpy(name_with_folder, SCENES_FOLDER "/");
    strcat(name_with_folder, old_name);

    strcpy(name_with_folder_new, SCENES_FOLDER "/");
    strcat(name_with_folder_new, new_name);

    /* rename */
    fres = f_rename(name_with_folder, name_with_folder_new);
    if (fres != FR_OK) {
        HA_NOTIFY("scene_mng::rename_inactive_scene failed\n");
        print_ferr(fres);
        return -1;
    }

    return 0;
}

/*------------------------ Default scene -------------------------------------*/
void scene_mng::restore_default_scene(void)
{
    /* set name for default scene */
    scenes_list[default_scene_index].scene_obj.set_name(SCENES_FOLDER "/" DEFAULT_SCENE_FILE);
    if (scenes_list[default_scene_index].scene_obj.restore() != 0) {
        HA_NOTIFY("Failed to restore default scene (%s)\n",
                SCENES_FOLDER "/" DEFAULT_SCENE_FILE);
        scenes_list[default_scene_index].valid = false;
    }
    else {
        HA_NOTIFY("Default scene (%s) restored\n",
                SCENES_FOLDER "/" DEFAULT_SCENE_FILE);
        scenes_list[default_scene_index].valid = true;
    }
}

/*----------------------------------------------------------------------------*/
void scene_mng::save_default_scene(void)
{
    scenes_list[default_scene_index].scene_obj.save();
}

/*----------------------------------------------------------------------------*/
scene *scene_mng::get_default_scene_ptr(void)
{
    return (get_scene_ptr_with_index(default_scene_index));
}


/*----------------------------------------------------------------------------*/
void scene_mng::set_default_scene_valid_status(bool status)
{
    scenes_list[default_scene_index].valid = status;
}

/*----------------------------------------------------------------------------*/
void scene_mng::print_default_scene(void)
{
    if (scenes_list[default_scene_index].valid) {
        HA_NOTIFY("Default scene:\n"
                "---\n");
        scenes_list[default_scene_index].scene_obj.print(rtc_p);
    }
    else {
        HA_NOTIFY("Default scene: Invalid\n");
    }
}

/*------------------------ Private methods -----------------------------------*/
scene *scene_mng::get_scene_ptr_with_index(uint8_t index)
{
    if (index >= max_num_scenes) {
        return NULL;
    }
    return &scenes_list[index].scene_obj;
}

/*----------------------------------------------------------------------------*/
void scene_mng::not_dir(char* path_name)
{
    int16_t last_splash_pos = -1;
    uint16_t count = 0;

    while (1) {
        if (path_name[count] == '/') {
            last_splash_pos = count;
        }

        if (path_name[count] == '\0') {
            break;
        }

        count++;
    }

    if (last_splash_pos >= 0) {
        memmove(&path_name[0], &path_name[last_splash_pos + 1], count - last_splash_pos);
    }
}

/*----------------------------- Shell command --------------------------------*/
void scene_mng_cmd(scene_mng &scene_mng_obj, rtc &rtc_obj, int argc, char **argv)
{
    uint8_t count, scene_type = NO_SCENE;
    scene *scene_p = NULL;
    scene_ns::rule_t rule;
    scene_ns::input_t input;
    scene_ns::output_t output;
    rtc_ns::time_t in_time;

    uint16_t index;
    bool active, adding_rule = false;
    uint8_t num_in = 0, num_out = 0;

    if (argc == 1) {
        printf("Err: too few argument, scene -h to get help.\n");
        return;
    }

    for (count = 0; count < argc; count++) {
        if (argv[count][0] == '-') { /* option */
            switch (argv[count][1]){
            case 's':
                if (count + 1 > argc) {
                    if (count + 1 >= argc) {
                        printf("Err: missing argument for option %s\n", argv[count]);
                        return;
                    }
                }

                count++;
                if (argv[count][0] == 'd') {
                    scene_type = DEFAULT_SCENE;
                    scene_p = scene_mng_obj.get_default_scene_ptr();
                }
                else if (argv[count][0] == 'u') {
                    scene_type = USER_SCENE;
                    scene_p = scene_mng_obj.get_user_scene_ptr();
                }
                else {
                    printf("Err: should be d or u for option -s\n");
                    return;
                }
                break;

            case 'l':
                if (scene_type == USER_SCENE) { /* can be considered as all scenes with -l */
                    scene_mng_obj.print_user_scene();
                }
                else if (scene_type == DEFAULT_SCENE) {
                    scene_mng_obj.print_default_scene();
                }
                else {
                    scene_mng_obj.print_default_scene();
                    scene_mng_obj.print_user_scene();
                }
                return;

            case 'a': /* add new rule to scene */
                if (scene_type == NO_SCENE) {
                    printf("Err: scene type is not defined with -s\n");
                    return;
                }

                if (count + 2 >= argc) {
                    printf("Err: to few argument for -a\n");
                    return;
                }

                /* get active and valid */
                index = atoi(argv[++count]);
                active = atoi(argv[++count]);

                /* get current rule in index position */
                if (scene_p->get_rule_with_index(rule, index) == -1) {
                    printf("Err: can't get rule from index %hu\n", index);
                    return;
                }

                /* Set active, valid */
                rule.is_active = active;
                rule.is_valid = true;

                /* Set to adding rule state */
                adding_rule = true;
                break;

            case 'i':
                if (!adding_rule) {
                    printf("Err: -i without -a option\n");
                    return;
                }

                if (count + 1 >= argc) {
                    printf("Err: too few argument for option %s\n", argv[count]);
                    return;
                }

                /* increase num in */
                num_in++;
                if (num_in > scene_ns::rule_max_input) {
                    printf("Err: too much inputs (max %hu)\n", scene_ns::rule_max_input);
                    return;
                }

                /* get one input */
                input.cond = atoi(argv[++count]);
                switch (input.cond) {
                case scene_ns::COND_EQUAL_THR:
                case scene_ns::COND_GREATER_OR_EQUAL_THR:
                case scene_ns::COND_GREATER_THAN_THR:
                case scene_ns::COND_LESS_THAN_THR:
                case scene_ns::COND_LESS_OR_EQUAL_THR:
                case scene_ns::COND_CHANGE_VAL_OVER_THR:
                    /* follow by device id (hex) and threshold */
                    if (count + 2 >= argc) {
                        printf("Err: too few argument for this input, cond (%hu)\n",
                                input.cond);
                        return;
                    }

                    input.dev_val.device_id = strtol(argv[++count], NULL, 16);
                    input.dev_val.value = atoi(argv[++count]);

                    break;

                case scene_ns::COND_CHANGE_VAL:
                    /* follow by device id only */
                    if (count + 1 >= argc) {
                        printf("Err: too few argument for this input, cond (%hu)\n",
                                input.cond);
                        return;
                    }

                    input.dev_val.device_id = strtol(argv[++count], NULL, 16);
                    input.dev_val.value = 0;

                    break;

                case scene_ns::COND_IN_RANGE:
                    /* follow by start time (month, day, year, hour, min, sec)
                     * and end time */
                    if (count + 12 >= argc) {
                        printf("Err: too few argument for this input, cond (%hu)\n",
                                input.cond);
                        return;
                    }

                    /* start time */
                    in_time.month = atoi(argv[++count]);
                    in_time.day = atoi(argv[++count]);
                    in_time.year = atoi(argv[++count]);
                    in_time.hour = atoi(argv[++count]);
                    in_time.min = atoi(argv[++count]);
                    in_time.sec = atoi(argv[++count]);

                    input.time_range.start = rtc_obj.time_to_packed(in_time);

                    /* end time */
                    in_time.month = atoi(argv[++count]);
                    in_time.day = atoi(argv[++count]);
                    in_time.year = atoi(argv[++count]);
                    in_time.hour = atoi(argv[++count]);
                    in_time.min = atoi(argv[++count]);
                    in_time.sec = atoi(argv[++count]);

                    input.time_range.end = rtc_obj.time_to_packed(in_time);

                    break;

                case scene_ns::COND_IN_RANGE_EVDAY:
                    /* follow by start time in day (hour, min, sec) and end time */
                    if (count + 6 >= argc) {
                        printf("Err: too few argument for this input, cond (%hu)\n",
                                input.cond);
                        return;
                    }

                    /* start time */
                    in_time.hour = atoi(argv[++count]);
                    in_time.min = atoi(argv[++count]);
                    in_time.sec = atoi(argv[++count]);

                    input.time_range.start = rtc_obj.time_to_packed(in_time) & 0x0000FFFF;

                    /* end time */
                    in_time.hour = atoi(argv[++count]);
                    in_time.min = atoi(argv[++count]);
                    in_time.sec = atoi(argv[++count]);

                    input.time_range.end = rtc_obj.time_to_packed(in_time) & 0x0000FFFF;
                    break;

                default:
                    printf("Err: unknow condtion(%hu)\n", input.cond);
                    return;
                }

                /* add input to rule */
                rule.num_in = num_in;
                memcpy(&rule.inputs[num_in - 1], &input, sizeof(scene_ns::input_t));

                break;

            case 'o':
                if (!adding_rule) {
                    printf("Err: -o without -a option\n");
                    return;
                }

                if (count + 1 >= argc) {
                    printf("Err: too few argument for option %s\n", argv[count]);
                    return;
                }

                /* increase num out */
                num_out++;
                if (num_out > scene_ns::rule_max_output) {
                    printf("Err: too much outputs (max %hu)\n", scene_ns::rule_max_output);
                    return;
                }

                /* get one output */
                output.action = atoi(argv[++count]);
                switch(output.action) {
                case scene_ns::ACT_SET_DEV_VAL:
                    /* follow by device id (hex) and value */
                    if (count + 2 >= argc) {
                        printf("Err: too few argument for this output, act (%hu)\n",
                                output.action);
                        return;
                    }

                    output.dev_val.device_id = strtol(argv[++count], NULL, 16);
                    output.dev_val.value = atoi(argv[++count]);

                    break;

                default:
                    printf("Err: unknow action %hu\n", output.action);
                    break;
                }

                /* add output to rule */
                rule.num_out = num_out;
                memcpy(&rule.outputs[num_out - 1], &output, sizeof(scene_ns::output_t));
                break;

            case 'd':
                if (scene_type == NO_SCENE) {
                    printf("Err: scene type is not defined with -s\n");
                    return;
                }

                if (count + 1 >= argc) {
                    printf("Err: to few argument for option %s\n", argv[count]);
                    return;
                }

                scene_p->remove_rule_with_index(atoi(argv[++count]));
                printf("Rule %hu removed\n", atoi(argv[++count]));
                break;

            case 'p':
                /* halt processing scene */
                if (scene_type == NO_SCENE) {
                    printf("Err: scene type is not defined with -s\n");
                    return;
                }

                if (scene_type == DEFAULT_SCENE) {
                    printf("Halt processing default scene\n");
                    scene_mng_obj.set_default_scene_valid_status(false);
                }
                else if (scene_type == USER_SCENE) {
                    printf("Halt processing user scene\n");
                    scene_mng_obj.set_user_scene_valid_status(false);
                }
                break;

            case 'r':
                /* restart processing scene */
                if (scene_type == NO_SCENE) {
                    printf("Err: scene type is not defined with -s\n");
                    return;
                }

                if (scene_type == DEFAULT_SCENE) {
                    printf("Restart processing default scene\n");
                    scene_mng_obj.set_default_scene_valid_status(true);
                }
                else if (scene_type == USER_SCENE) {
                    printf("Restart processing user scene\n");
                    scene_mng_obj.set_user_scene_valid_status(true);
                }
                break;

            case 'v':
                if (scene_type == NO_SCENE) {
                    printf("Err: scene type is not defined with -s\n");
                    return;
                }

                scene_p->save();
                break;

            case 'e':
                if (scene_type == NO_SCENE) {
                    printf("Err: scene type is not defined with -s\n");
                    return;
                }

                scene_p->restore();
                break;

            case 'n':
                if (count + 2 >= argc) {
                    printf("Err: to few argument for option %s\n", argv[count]);
                    return;
                }

                scene_mng_obj.rename_inactive_scene(argv[count+1], argv[count+2]);
                return;

            case 'h':
                printf("%s", scene_cmd_usage);
                break;

            default:
                break;
            }/* end switch options */
        } /* end if options */
    } /* end for */

    if (adding_rule) {
        /* add rule to index */
        if (scene_p->add_rule_with_index(rule, index) == -1) {
            printf("Err: when adding rule to index %hu\n", index);
        }
    }
}

