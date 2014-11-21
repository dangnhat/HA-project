/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        scene.cpp
 * @brief       Scene class.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#include "scene.h"
#include "ff.h"
#include "shell_cmds_fatfs.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace scene_ns;

static const char save_line_rule[] = "R: %u %u %u %u\n"; /* is_valid, is_active, num_in, num_out */
static const char save_line_i0[] = "I: %u\n";          /* cond */
static const char save_line_i1_devval[] = "%lu %d\n"; /* device id, value */
static const char save_line_i1_time[] = "%lu %lu\n";  /* time start, time end */
static const char save_line_o0[] = "O: %u\n";          /* action */
static const char save_line_o1_devval[] = "%lu %d\n"; /* device id, value */

/*----------------------------------------------------------------------------*/
scene::scene(void)
{
    cur_num_rules = 0;
    name[0] = '\0';
    clear_all_rules();
}

/*----------------------------------------------------------------------------*/
char *scene::get_name(void)
{
    return name;
}

/*----------------------------------------------------------------------------*/
void scene::get_name(char *name_buffer)
{
    memcpy(name_buffer, name, sizeof(name_buffer));
}

/*----------------------------------------------------------------------------*/
void scene::set_name(const char *new_name)
{
    memcpy(name, new_name, scene_max_name_chars - 1);
    name[scene_max_name_chars - 1] = '\0';
}

/*----------------------------------------------------------------------------*/
uint16_t scene::get_cur_num_rules(void)
{
    return cur_num_rules;
}

/*----------------------------------------------------------------------------*/
void scene::new_scene(void)
{
    cur_num_rules = 0;
    clear_all_rules();
}

/*----------------------------------------------------------------------------*/
void scene::add_rule_with_index(rule_t &rule, uint16_t index)
{
    if (index >= scene_max_rules) {
        return;
    }

    memcpy(&rules_list[index], &rule, sizeof(rule_t));
    if (index > cur_num_rules) {
        cur_num_rules = index;
    }
}

/*----------------------------------------------------------------------------*/
bool scene::find_invalid_rule(uint16_t &index)
{
    for(uint8_t count = 0; count < cur_num_rules; count++) {
        if (!rules_list[count].is_valid) {
            index = count;
            return true;
        }
    }

    return false;
}

/*----------------------------------------------------------------------------*/
void scene::save(void)
{
    FIL file;
    FRESULT fres;
    uint8_t count_io, count_rule;

    if(name == NULL) {
        return;
    }

    /* open file */
    fres = f_open(&file, name, FA_WRITE | FA_CREATE_ALWAYS);
    if (fres != FR_OK) {
        HA_DEBUG("scene::save: Error when open file %s\n", name);
        print_ferr(fres);
        return;
    }
    f_sync(&file);

    /* write data */
    for (count_rule = 0; count_rule < cur_num_rules; count_rule++) {
        /* Save rule */
        f_printf(&file, save_line_rule,
                rules_list[count_rule].is_valid, rules_list[count_rule].is_active,
                rules_list[count_rule].num_in, rules_list[count_rule].num_out);

        /* Save input */
        for (count_io = 0; count_io < rules_list[count_rule].num_in; count_io++){
            f_printf(&file, save_line_i0, rules_list[count_rule].inputs[count_io].cond);

            if (rules_list[count_rule].inputs[count_io].cond == COND_IN_RANGE) {
                f_printf(&file, save_line_i1_time,
                        rules_list[count_rule].inputs[count_io].time_range.start,
                        rules_list[count_rule].inputs[count_io].time_range.end);
            }
            else {
                f_printf(&file, save_line_i1_devval,
                        rules_list[count_rule].inputs[count_io].dev_val.device_id,
                        rules_list[count_rule].inputs[count_io].dev_val.value);
            }
        }

        /* Save output */
        for (count_io = 0; count_io < rules_list[count_rule].num_out; count_io++) {
            f_printf(&file, save_line_o0, rules_list[count_rule].outputs[count_io].action);
            f_printf(&file, save_line_o1_devval,
                    rules_list[count_rule].outputs[count_io].dev_val.device_id,
                    rules_list[count_rule].outputs[count_io].dev_val.value);
        }

        f_sync(&file);
    }

    f_close(&file);
}

/*----------------------------------------------------------------------------*/
void scene::restore(void)
{
    FIL file;
    FRESULT fres;
    uint8_t count_io, count_rule;
    TCHAR* ret;
    char line[32];
    rule_t read_rule;

    unsigned int is_valid_ui, is_active_ui, num_in_ui, num_out_ui, condact_ui;
    int value_i;

    if (name == NULL) {
        return;
    }

    /* open file */
    fres = f_open(&file, name, FA_READ | FA_OPEN_ALWAYS);
    if (fres != FR_OK) {
        HA_DEBUG("scene::restore: Error when open file %s\n", name);
        print_ferr(fres);
        return;
    }

    /* Read file */
    count_rule = 0;
    while (1) {

        /* Read rule */
        ret = f_gets(line, sizeof(line), &file);
        if (ret == 0) {
            break;
        }
        sscanf(line, save_line_rule, &is_valid_ui, &is_active_ui, &num_in_ui, &num_out_ui);
        read_rule.is_valid = (bool) is_valid_ui;
        read_rule.is_active = (bool) is_active_ui;
        read_rule.num_in = (uint8_t)num_in_ui;
        read_rule.num_out = (uint8_t)num_out_ui;

        /* Read input */
        for (count_io = 0; count_io < read_rule.num_in; count_io++) {

            /* get conditon */
            ret = f_gets(line, sizeof(line), &file);
            if (ret == 0) {
                break;
            }
            sscanf(line, save_line_i0, &condact_ui);
            read_rule.inputs[count_io].cond = (uint8_t)condact_ui;

            /* get input for condition */
            ret = f_gets(line, sizeof(line), &file);
            if (ret == 0) {
                break;
            }

            if (read_rule.inputs[count_io].cond == COND_IN_RANGE) {
                sscanf(line, save_line_i1_time,
                        &read_rule.inputs[count_io].time_range.start,
                        &read_rule.inputs[count_io].time_range.end);
            }
            else {
                sscanf(line, save_line_i1_devval,
                        &read_rule.inputs[count_io].dev_val.device_id,
                        &value_i);
                read_rule.inputs[count_io].dev_val.value = (int16_t) value_i;
            }

        }/* end read input */

        /* Read output */
        for (count_io = 0; count_io < read_rule.num_out; count_io++) {

            /* get action */
            ret = f_gets(line, sizeof(line), &file);
            if (ret == 0) {
                break;
            }
            sscanf(line, save_line_o0, &condact_ui);
            read_rule.outputs[count_io].action = (uint8_t)condact_ui;

            /* get output for action */
            ret = f_gets(line, sizeof(line), &file);
            if (ret == 0) {
                break;
            }
            sscanf(line, save_line_o1_devval,
                    &read_rule.outputs[count_io].dev_val.device_id,
                    &value_i);
            read_rule.outputs[count_io].dev_val.value = (int16_t) value_i;

        }/* end read output */

        /* add rule to rules_list */
        add_rule_with_index(read_rule, count_rule);
        count_rule++;
    }/* end while */

    /* close file */
    f_close(&file);
}
