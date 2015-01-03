/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        scene.h
 * @brief       Scene class.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#ifndef SCENE_H_
#define SCENE_H_

#include <stdint.h>
#include <string.h>

extern "C" {
#include "msg.h"
}

#include "cir_queue.h"
#include "ha_device_mng.h"
#include "MB1_rtc.h"

namespace scene_ns {

const uint8_t rule_max_input = 2;
const uint8_t rule_max_output = 2;

const uint8_t scene_max_name_chars = 20;
const uint8_t scene_max_name_chars_wout_folders = 8 + 1;
const uint16_t scene_max_rules = 25;

/*-------------------------- CONDITION DEFINITIONS ---------------------------*/
enum cond_e: uint8_t {
    COND_EQUAL_THR = 0x00,          /* Condition: equal to threshold,
                                    parameter: device id, threshold value */
    COND_LESS_THAN_THR = 0x01,      /* Condition: less than threshold,
                                    parameter: device id, threshold value */
    COND_LESS_OR_EQUAL_THR = 0x02,  /* Condition: less than or equal to threshold,
                                    device id, parameter: threshold value  */
    COND_GREATER_THAN_THR = 0x03,   /* Condition: greater than threshold,
                                    parameter: device id, threshold value */
    COND_GREATER_OR_EQUAL_THR = 0x04, /* Condition: greater than or equal to threshold,
                                    parameter: device id, threshold value */
    COND_CHANGE_VAL = 0x05,         /* Condition: change value,
                                    parameter: device id */
    COND_CHANGE_VAL_OVER_THR = 0x08,    /* Condition: change value over a threshold,
                                    parameter: device id, threshold value */
    COND_IN_RANGE = 0x06,           /* Condition: in a time range,
                                    parameter: time range (start time and end time)
                                    Time is packed in 32bit following this format:
                                    bit 0-4: second / 2.
                                    bit 5-10: minute.
                                    bit 11-15: hour.
                                    bit 16-20: day in month.
                                    bit 21-24: month.
                                    bit 25-31: number of years from timebase.year. */
    COND_IN_RANGE_EVDAY = 0x07,     /* Condition: in a time range of every day,
                                    parameter: time range (start time and end time)
                                    Time in packed format, only hour, min, sec will be
                                    cared */
};

/*-------------------------- ACTION DEFINITIONS ------------------------------*/
enum act_e: uint8_t {
    ACT_SET_DEV_VAL = 0x00,         /* Set value for a device,
                                    param: device_id, value */
    ACT_SET_DEV_MULT_VALS = 0x01,   /* Set multiple value for a device, followed by
                                    ACT_SET_DEV_MULT_VALS, and ended with ACT_SET_DEV_MULT_VALS_END.
                                    param: device_id, value */ /* TODO: later */
    ACT_SET_DEV_MULT_VALS_END = 0x02,   /* End value for ACT_SET_DEV_MULT_VALS,
                                    param: device_id, value */ /* TODO: later */
};

typedef struct dev_val_s {
    uint32_t device_id;
    int16_t value;
} dev_val_t;

typedef struct time_range_s {
    uint32_t start;
    uint32_t end;
} time_range_t;

typedef struct input_s {
    uint8_t cond;
    union {
        dev_val_t dev_val;
        time_range_t time_range;
    };
} input_t;

typedef struct output_s {
    uint8_t action;
    union {
        dev_val_t dev_val;
    };
} output_t;

typedef struct rule_s {
    bool is_valid = false;

    bool is_active = false;
    uint8_t num_in = 0;
    uint8_t num_out = 0;
    input_t inputs[rule_max_input];
    output_t outputs[rule_max_output];
} rule_t;

}

using namespace scene_ns;

class scene {

public:
    /**
     * @brief   constructor.
     */
    scene(void);

    /**
     * @brief   Get name of scene.
     *
     * @return  pointer to internal buffer holding scene's name.
     */
    char *get_name(void);

    /**
     * @brief   Get name of scene, user provided buffer for scene's name.
     *
     * @param[out]  name_buffer, buffer holding scene's name.
     */
    void get_name(char *name_buffer);

    /**
     * @brief   Set name of scene.
     *
     * @param[in]  new_name.
     */
    void set_name(const char *new_name);

    /**
     * @brief   Get current number of rules.
     *
     * @return  Current number of rules.
     */
    uint16_t get_cur_num_rules(void);

    /**
     * @brief   Set current number of rules.
     *
     * @param[in]   Current number of rules.
     */
    void set_cur_num_rules(uint16_t num_rules);

    /**
     * @brief   Clear cur_num_rules and set all rules to invalid.
     */
    void new_scene(void);

    /**
     * @brief   Add a new rule to index position in rules_list.
     *
     * @param[in]   &rule, new rule to be added.
     * @param[in]   index.
     *
     * @return      -1 if error.
     */
    int8_t add_rule_with_index(rule_t &rule, uint16_t index);

    /**
     * @brief   Get a rule in index position.
     *
     * @param[out]  &rule, a rule has been retrieved.
     * @param[in]   index.
     *
     * @return      -1 if error.
     */
    int8_t get_rule_with_index(rule_t &rule, uint16_t index);

    /**
     * @brief   Remove rule from index position in rules_list. (Set to invalid)
     *
     * @param[in]   index.
     */
    void remove_rule_with_index(uint16_t index);

    /**
     * @brief   Find the first invalid rule from index 0  to cur_num_rules.
     *          find_invalid_rule with cont = false should be called before any cont = true
     *          to initialize the last invalid index value.
     *
     * @param[out]  index, index of a invalid rule.
     * @param[in]   cont, false then the search will be done from the beginning,
     *              otherwise, it will begin from the last invalid position.
     *
     * @return  true if invalid rule in [0-cur_num_rules] has been found, otherwise, false.
     */
    bool find_invalid_rule(uint16_t &index, bool cont);

    /**
     * @brief   Process rules and output action to out_queue (in SET_DEV_VAL GFF format).
     *
     * @param[in]   trigger_by_report, true if this process action was triggered by report,
     *              otherwise, it was triggered by time.
     * @param[in]   *a_device_rpt, report from an input device (SET_DEV_VAL message). Can be
     *              Will not be used when this process has been triggered by time.
     * @param[in]   *cur_device_mng, current devices' status. (hasn't not been applied new report).
     * @param[in]   *rtc_obj, rtc object.
     * @param[out]  *out_queue, output action (SET_DEV_VAL) will be pushed to this queue.
     * @param[in]   out_pid, GFF_PENDING message will be sent to this thread for
     *              every output action.
     */
    void process(bool trigger_by_report,
            ha_device *a_device_rpt, ha_device_mng *cur_device_mng,
            rtc *rtc_obj,
            cir_queue *out_queue, kernel_pid_t out_pid);

    /**
     * @brief   Save data to file.
     *
     * @return  0 if success, -1 on error.
     */
    int8_t save(void);

    /**
     * @brief   Read data from file.
     *
     * @return  0 if success, -1 on error.
     */
    int8_t restore(void);

    /**
     * @brief   Print scene via HA_NOTIFY.
     *
     * @param[in]   *rtc_obj, rtc object.
     */
    void print(rtc *rtc_obj);

private:

    /**
     * @brief   Clear all rules to invalid.
     */
    void clear_all_rules(void);

    /* @brief   Print input.
     *
     * @param[in]   input, an input to be printed.
     * @param[in]   *rtc_obj, rtc object.
     */
    void print_input(input_t &input, rtc *rtc_obj);

    /* @brief   Print output.
     *
     * @param[in]   output, an output to be printed.
     */
    void print_output(output_t &output);

    char name[scene_max_name_chars];

    uint16_t cur_num_rules;
    rule_t rules_list[scene_max_rules];

    uint16_t last_invalid_index;
};

#endif // SCENE_H_
