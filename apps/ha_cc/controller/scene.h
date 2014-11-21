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

namespace scene_ns {

const uint8_t rule_max_input = 1;
const uint8_t rule_max_output = 1;

const uint8_t scene_max_name_chars = 8 + 1;
const uint16_t scene_max_rules = 64;

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
    COND_TOGGLE_ON_OFF = 0x05,      /* Condition: toggle on/off,
                                    parameter: device id */
    COND_IN_RANGE = 0x06,           /* Condition: in a time range,
                                    parameter: time range (start time and end time) */
};

/*-------------------------- ACTION DEFINITIONS ------------------------------*/
enum act_e: uint8_t {
    ACT_ON_ONE = 0x01,      /* Action: turn on one device,
                            parameter: device id (set device value = 100, on) */
    ACT_OFF_ONE = 0x02,     /* Action: turn off one device,
                            parameter: device id (set device value = 0, off) */
    ACT_TOGGLE_ONE = 0x03,  /* Action: toggle one device,
                            parameter: device id */
    ACT_LEVEL_ONE = 0x04,   /* Action: set level of one device to a value,
                            parameter: device id and a value (1 - 10 * 10%) */
    ACT_DIM_ONE = 0x05,     /* Action: dim one device,
                            parameter: device id and a value (should be in percent) */
    ACT_BLINK_ONE = 0x06,   /* Action: blink one device,
                            parameter: device id (set device value to 101, blink) */
    ACT_ON_ALL = 0x10,      /* Action: turn on all device,
                            parameter: none (set value for all device to 100) */
    ACT_OFF_ALL = 0x11,     /* Action: turn off all device,
                            parameter: none (set value for all device to 0) */
    ACT_BLINK_ALL = 0x12    /* Action: blink all device,
                            parameter: none (set value for all device to 101) */
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
     * @brief   Clear cur_num_rules and set all rules to invalid.
     */
    void new_scene(void);

    /**
     * @brief   Add a new rule to index position in rules_list.
     *
     * @param[in]   &rule, new rule to be added.
     * @param[in]   index.
     */
    void add_rule_with_index(rule_t &rule, uint16_t index);

    /**
     * @brief   Find the first invalid rule from index 0  to cur_num_rules.
     *
     * @param[out]  index, index of a invalid rule.
     *
     * @return  true if invalid rule in [0-cur_num_rules] has been found, otherwise, false.
     */
    bool find_invalid_rule(uint16_t &index);

    /**
     * @brief   Process rules and output action to out_queue (in SET_DEV_VAL GFF format).
     *
     * @param[in]   trigger_by_report, true if this process action was triggered by report,
     *              otherwise, it was triggered by time.
     * @param[in]   &out_queue, output action (SET_DEV_VAL) will be pushed to this queue.
     * @param[in]   *mesg_queue, GFF_PENDING message will be pushed to this queue for
     *              every output action.
     */
    void process(bool trigger_by_report, cir_queue &out_queue, msg_t *mesg_queue);

    /**
     * @brief   Save data to file.
     */
    void save(void);

    /**
     * @brief   Read data from file.
     */
    void restore(void);

private:

    /**
     * @brief   Clear all rules to invalid.
     */
    void clear_all_rules(void);

    char name[scene_max_name_chars];

    uint16_t cur_num_rules;
    rule_t rules_list[scene_max_rules];
};

#endif // SCENE_H_
