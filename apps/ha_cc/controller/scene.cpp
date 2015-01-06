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

#include <stdlib.h>

#include "scene.h"
#include "ff.h"
#include "shell_cmds_fatfs.h"
#include "gff_mesg_id.h"
#include "common_msg_id.h"
#include "ha_gff_misc.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace scene_ns;

static const char save_line_rule[] = "R: %u %u %u %u\n"; /* is_valid, is_active, num_in, num_out */
static const char save_line_i0[] = "I: %u\n";          /* cond */
static const char save_line_i1_devval[] = "%lx %d\n"; /* device id, value */
static const char save_line_i1_time[] = "%lx %lx\n";  /* time start, time end */
static const char save_line_o0[] = "O: %u\n";          /* action */
static const char save_line_o1_devval[] = "%lx %d\n"; /* device id, value */

/*----------------------------------------------------------------------------*/
scene::scene(void)
{
    cur_num_rules = 0;
    name[0] = '\0';
    clear_all_rules();

    last_invalid_index = 0;
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
void scene::set_cur_num_rules(uint16_t num_rules)
{
    cur_num_rules = num_rules;
}

/*----------------------------------------------------------------------------*/
void scene::new_scene(void)
{
    cur_num_rules = 0;
    clear_all_rules();
}

/*----------------------------------------------------------------------------*/
int8_t scene::add_rule_with_index(rule_t &rule, uint16_t index)
{
    if (index >= scene_max_rules) {
        return -1;
    }

    memcpy(&rules_list[index], &rule, sizeof(rule_t));
    if (index >= cur_num_rules) {
        cur_num_rules = index + 1;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
int8_t scene::get_rule_with_index(rule_t &rule, uint16_t index)
{
    if (index >= scene_max_rules) {
        return -1;
    }

    memcpy(&rule, &rules_list[index], sizeof(rule_t));

    return 0;
}

/*----------------------------------------------------------------------------*/
void scene::remove_rule_with_index(uint16_t index)
{
    if (index >= scene_max_rules) {
        return;
    }

    rules_list[index].is_valid = false;
}

/*----------------------------------------------------------------------------*/
bool scene::find_invalid_rule(uint16_t &index, bool cont)
{
    if (!cont) {
        for(uint8_t count = 0; count < cur_num_rules; count++) {
            if (!rules_list[count].is_valid) {
                index = count;
                last_invalid_index = index;
                return true;
            }
        }
    }
    else {
        for(uint8_t count = last_invalid_index + 1; count < cur_num_rules; count++) {
            if (!rules_list[count].is_valid) {
                index = count;
                last_invalid_index = index;
                return true;
            }
        }
    }

    return false;
}

/*----------------------------------------------------------------------------*/
int8_t scene::save(void)
{
    FIL file;
    FRESULT fres;
    uint8_t count_io, count_rule;

    if(name == NULL) {
        return -1;
    }

    /* open file */
    fres = f_open(&file, name, FA_WRITE | FA_CREATE_ALWAYS);
    if (fres != FR_OK) {
        HA_DEBUG("scene::save: Error when open file %s\n", name);
        print_ferr(fres);
        return -1;
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

            if (rules_list[count_rule].inputs[count_io].cond == COND_IN_RANGE ||
                    rules_list[count_rule].inputs[count_io].cond == COND_IN_RANGE_EVDAY) {
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

    return 0;
}

/*----------------------------------------------------------------------------*/
int8_t scene::restore(void)
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
        return -1;
    }

    /* new scene */
    new_scene();

    /* open file */
    fres = f_open(&file, name, FA_READ | FA_OPEN_ALWAYS);
    if (fres != FR_OK) {
        HA_DEBUG("scene::restore: Error when open file %s\n", name);
        print_ferr(fres);
        return -1;
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

            if ((read_rule.inputs[count_io].cond == COND_IN_RANGE) ||
                    (read_rule.inputs[count_io].cond == COND_IN_RANGE_EVDAY)) {
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

    return 0;
}

/*----------------------------------------------------------------------------*/
void scene::process(bool trigger_by_report,
        ha_device *device_rpt, ha_device_mng *cur_device_mng,
        rtc *rtc_obj,
        cir_queue *out_queue, kernel_pid_t out_pid)
{
    bool all_cond_satisfied, has_trigger_src;
    uint16_t c_rule, c_in, c_out;
    uint32_t cur_time;
    int16_t value;
    uint8_t act_gff[ha_ns::SET_DEV_VAL_DATA_LEN + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE];
    msg_t mesg;

    for (c_rule = 0; c_rule < cur_num_rules; c_rule++) {

        /* Check valid and active */
        if (!rules_list[c_rule].is_valid || !rules_list[c_rule].is_active) {
            HA_DEBUG("scene::process: Rule %hu is not valid or active\n", c_rule);
            continue;
        }

        /* process inputs */
        all_cond_satisfied = true;
        has_trigger_src = false;
        for (c_in = 0; c_in < rules_list[c_rule].num_in; c_in++) {
            input_t *input_p = &rules_list[c_rule].inputs[c_in];

            switch (input_p->cond) {

            case COND_IN_RANGE:
                HA_DEBUG("scene::process: COND_IN_RANGE\n");

                if (!trigger_by_report) {
                    has_trigger_src = true;
                }

                cur_time = rtc_obj->get_time_packed();
                if (cur_time < input_p->time_range.start ||
                        cur_time > input_p->time_range.end) {
                    all_cond_satisfied = false;
                }

                HA_DEBUG("scene::process: acs %hd, hts %hd, cur_time %lx, start %lx, end %lx\n",
                        all_cond_satisfied, has_trigger_src,
                        cur_time, input_p->time_range.start, input_p->time_range.end);
                break;

            case COND_IN_RANGE_EVDAY:
                if (!trigger_by_report) {
                    has_trigger_src = true;
                }

                HA_DEBUG("scene::process: COND_IN_RANGE_EVDAY\n");

                cur_time = rtc_obj->get_time_packed();
                cur_time = cur_time & 0xFFFF;

                /* only hour, min, sec will be cared */
                if ( cur_time < (input_p->time_range.start & 0xFFFF) ||
                        cur_time > (input_p->time_range.end & 0xFFFF)) {
                    all_cond_satisfied = false;
                }

                HA_DEBUG("scene::process: acs %hd, hts %hd, cur_time %lx, start %lx, end %lx\n",
                        all_cond_satisfied, has_trigger_src,
                        cur_time,
                        input_p->time_range.start & 0xFFFF, input_p->time_range.end & 0xFFFF);
                break;

            case COND_EQUAL_THR:
            case COND_LESS_THAN_THR:
            case COND_LESS_OR_EQUAL_THR:
            case COND_GREATER_THAN_THR:
            case COND_GREATER_OR_EQUAL_THR:

                switch (input_p->cond) {
                case COND_EQUAL_THR:
                    HA_DEBUG("scene::process: COND_EQUAL_THR\n");
                    break;
                case COND_LESS_THAN_THR:
                    HA_DEBUG("scene::process: COND_LESS_THAN_THR\n");
                    break;
                case COND_LESS_OR_EQUAL_THR:
                    HA_DEBUG("scene::process: COND_LESS_OR_EQUAL_THR\n");
                    break;
                case COND_GREATER_THAN_THR:
                    HA_DEBUG("scene::process: COND_GREATER_THAN_THR\n");
                    break;
                case COND_GREATER_OR_EQUAL_THR:
                    HA_DEBUG("scene::process: COND_GREATER_OR_EQUAL_THR\n");
                    break;
                default:
                    break;
                }

                if (trigger_by_report &&
                    (device_rpt->get_device_id() == input_p->dev_val.device_id)) {
                    has_trigger_src = true;

                    /* check new status of this device */
                    value = device_rpt->get_value();
                }
                else {
                    /* check old status in cur_device_mng */
                    if (cur_device_mng->get_dev_val(input_p->dev_val.device_id, value) == -1) {
                        HA_DEBUG("scene::process: can't find dev %lx -> false\n",
                                input_p->dev_val.device_id);
                        /* TODO: check again */
                        all_cond_satisfied = false;
                        break;
                    }
                }

                /* compare value */
                switch (input_p->cond) {

                case COND_EQUAL_THR:
                    if (value != input_p->dev_val.value) {
                        all_cond_satisfied = false;
                    }
                    break;

                case COND_LESS_THAN_THR:
                    if (value >= input_p->dev_val.value) {
                        all_cond_satisfied = false;
                    }
                    break;

                case COND_LESS_OR_EQUAL_THR:
                    if (value > input_p->dev_val.value) {
                        all_cond_satisfied = false;
                    }
                    break;

                case COND_GREATER_THAN_THR:
                    if (value <= input_p->dev_val.value) {
                        all_cond_satisfied = false;
                    }
                    break;

                case COND_GREATER_OR_EQUAL_THR:
                    if (value < input_p->dev_val.value) {
                        all_cond_satisfied = false;
                    }
                    break;

                default:
                    break;
                }

                HA_DEBUG("scene::process: acs %hd, hts %hd, dev_rpt %lx, val %hd,"
                        "dev_i %lx, thres %hd\n",
                        all_cond_satisfied, has_trigger_src,
                        device_rpt->get_device_id(), value,
                        input_p->dev_val.device_id, input_p->dev_val.value);
                break;

            case COND_CHANGE_VAL:
            case COND_CHANGE_VAL_OVER_THR:
                switch (input_p->cond) {
                case COND_CHANGE_VAL:
                    HA_DEBUG("scene::process: COND_CHANGE_VAL\n");
                    break;
                case COND_CHANGE_VAL_OVER_THR:
                    HA_DEBUG("scene::process: COND_CHANGE_VAL_OVER_THR\n");
                    break;
                }

                if (trigger_by_report &&
                    (device_rpt->get_device_id() == input_p->dev_val.device_id)) {
                    has_trigger_src = true;

                    /* compare new value of this device with old value */
                    if (cur_device_mng->get_dev_val(device_rpt->get_device_id(), value) == -1) {
                        /* Can't find device */
                        HA_DEBUG("scene::process: can't find dev %lx -> false\n",
                                device_rpt->get_device_id());
                        all_cond_satisfied = false;
                    }
                    else {
                        /* Found device, evaluate new value with old value */
                        switch (input_p->cond) {

                        case COND_CHANGE_VAL:
                            if (device_rpt->get_value() == value) {
                                all_cond_satisfied = false;
                            }
                            break;

                        case COND_CHANGE_VAL_OVER_THR:
                            if (abs(device_rpt->get_value() - value) <= input_p->dev_val.value){
                                /* change was not over threshold */
                                all_cond_satisfied = false;
                            }
                            break;
                        }
                    }/* end evaluating new and old value */
                }
                else { /* the device was not changed */
                    all_cond_satisfied = false;
                }

                HA_DEBUG("scene::process: acs %hd, hts %hd, dev_rpt %lx, val %hd,"
                        "dev_i %lx, thres %hd\n",
                        all_cond_satisfied, has_trigger_src,
                        device_rpt->get_device_id(), value,
                        input_p->dev_val.device_id, input_p->dev_val.value);

                break;

            default:
                break;
            }/* end switch input's conditions*/

            if (!all_cond_satisfied) {
                /* No need to check anymore */
                break;
            }
        }

        /* process outputs */
        if (all_cond_satisfied && has_trigger_src) {
            HA_DEBUG("scene::process: Processing output...\n");

            for (c_out = 0; c_out < rules_list[c_rule].num_out; c_out++) {
                output_t *output_p = &rules_list[c_rule].outputs[c_out];

                switch (output_p->action) {
                case ACT_SET_DEV_VAL:
                    HA_DEBUG("scene::process: ACT_SET_DEV_VAL, dev %lx, val %hd\n",
                            output_p->dev_val.device_id, output_p->dev_val.value);

                    /* pack gff frame */
                    act_gff[ha_ns::GFF_LEN_POS] = ha_ns::SET_DEV_VAL_DATA_LEN;
                    uint162buf(ha_ns::SET_DEV_VAL, &act_gff[ha_ns::GFF_CMD_POS]);
                    uint322buf(output_p->dev_val.device_id, &act_gff[ha_ns::GFF_DATA_POS]);
                    uint162buf((uint16_t)output_p->dev_val.value, &act_gff[ha_ns::GFF_DATA_POS + 4]);

                    /* push to out_queue */
                    out_queue->add_data(act_gff, ha_ns::SET_DEV_VAL_DATA_LEN +
                            ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);

                    /* send GFF pending message */
                    mesg.type = ha_ns::GFF_PENDING;
                    mesg.content.ptr = (char *)out_queue;

                    msg_send(&mesg, out_pid, false);

                    HA_DEBUG("scene::process: Sent SET_DEV_VAL gff message\n");
                    break;

                default:
                    HA_DEBUG("scene::process: unknown action %hu\n", output_p->action);
                    break;
                }
            }/* end for, all outputs processed */
        }/* end if for outputs */

    }/* end for, all rules processed */
}

/*----------------------------------------------------------------------------*/
void scene::print(rtc *rtc_obj)
{
    uint16_t c_rule, c_in, c_out;

    HA_NOTIFY("Scene: %s\n"
            "---\n", name);
    for (c_rule = 0; c_rule < cur_num_rules; c_rule++) {
        HA_NOTIFY("Rule %hu, valid %hu, active %hu, num_in %hu, num_out %hu\n",
                c_rule,
                rules_list[c_rule].is_valid, rules_list[c_rule].is_active,
                rules_list[c_rule].num_in, rules_list[c_rule].num_out);
        if (rules_list[c_rule].is_valid) {
            for (c_in = 0; c_in < rules_list[c_rule].num_in; c_in++) {
                print_input(rules_list[c_rule].inputs[c_in], rtc_obj);
            }

            for (c_out = 0; c_out < rules_list[c_rule].num_out; c_out++) {
                print_output(rules_list[c_rule].outputs[c_out]);
            }
        }

        HA_NOTIFY("\n");
    }
}

/*----------------------------------------------------------------------------*/
void scene::print_input(input_t &input, rtc *rtc_obj)
{
    rtc_ns::time_t time;

    /* print condition */
    switch (input.cond) {
    case COND_EQUAL_THR:
        HA_NOTIFY("COND_EQUAL_THR\n");
        break;
    case COND_LESS_THAN_THR:
        HA_NOTIFY("COND_LESS_THAN_THR\n");
        break;
    case COND_LESS_OR_EQUAL_THR:
        HA_NOTIFY("COND_LESS_OR_EQUAL_THR\n");
        break;
    case COND_GREATER_THAN_THR:
        HA_NOTIFY("COND_GREATER_THAN_THR\n");
        break;
    case COND_GREATER_OR_EQUAL_THR:
        HA_NOTIFY("COND_GREATER_OR_EQUAL_THR\n");
        break;
    case COND_CHANGE_VAL:
        HA_NOTIFY("COND_CHANGE_VAL\n");
        break;
    case COND_CHANGE_VAL_OVER_THR:
        HA_NOTIFY("COND_CHANGE_VAL_OVER_THR\n");
        break;
    case COND_IN_RANGE:
        HA_NOTIFY("COND_IN_RANGE\n");
        break;
    case COND_IN_RANGE_EVDAY:
        HA_NOTIFY("COND_IN_RANGE_EVDAY\n");
        break;
    default:
        HA_NOTIFY("cond: %hu\n", input.cond);
        break;
    }

    /* print parameters */
    switch (input.cond) {
    case COND_EQUAL_THR:
    case COND_LESS_THAN_THR:
    case COND_LESS_OR_EQUAL_THR:
    case COND_GREATER_THAN_THR:
    case COND_GREATER_OR_EQUAL_THR:
    case COND_CHANGE_VAL:
    case COND_CHANGE_VAL_OVER_THR:
        HA_NOTIFY("Device id: %lx, threshold: %hd\n",
                input.dev_val.device_id, input.dev_val.value);
        break;

    case COND_IN_RANGE:
        rtc_obj->packed_to_time(input.time_range.start, time);
        HA_NOTIFY("Start: %hu:%hu:%hu, %hu %hu %hu\n", time.hour, time.min, time.sec,
                time.day, time.month, time.year);
        rtc_obj->packed_to_time(input.time_range.end, time);
        HA_NOTIFY("End: %hu:%hu:%hu, %hu %hu %hu\n", time.hour, time.min, time.sec,
                time.day, time.month, time.year);
        break;

    case COND_IN_RANGE_EVDAY:
        rtc_obj->packed_to_time(input.time_range.start, time);
        HA_NOTIFY("Start: %hu:%hu:%hu\n", time.hour, time.min, time.sec);
        rtc_obj->packed_to_time(input.time_range.end, time);
        HA_NOTIFY("End: %hu:%hu:%hu\n", time.hour, time.min, time.sec);
        break;

    default:
        break;
    }
}

/*----------------------------------------------------------------------------*/
void scene::print_output(output_t &output)
{
    /* print action */
    switch (output.action) {
    case ACT_SET_DEV_VAL:
        HA_NOTIFY("ACT_SET_DEV_VAL\n");
        break;
    default:
        HA_NOTIFY("action: %hu\n", output.action);
        break;
    }

    /* print parameters */
    switch (output.action) {
    case ACT_SET_DEV_VAL:
        HA_NOTIFY("Device id: %lx, value: %hd\n",
                output.dev_val.device_id, output.dev_val.value);
        break;

    default:
        break;
    }
}

/*----------------------------------------------------------------------------*/
void scene::clear_all_rules(void)
{
    uint16_t count;
    for (count = 0; count < scene_max_rules; count++) {
        rules_list[count].is_valid = false;
    }
}
