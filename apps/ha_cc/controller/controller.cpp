/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        controller.cpp
 * @brief       Home automation CC's controller thread.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#include <cstdint>

extern "C" {
#include "thread.h"
#include "msg.h"
}

#include "gff_mesg_id.h"
#include "ha_gff_misc.h"

#include "controller.h"
#include "ha_device_mng.h"
#include "scene_mng.h"
#include "cc_msg_id.h"
#include "ble_transaction.h"
#include "ha_sixlowpan.h"

/*----------------------------- Configurations -------------------------------*/
#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

/* Data Cir_queues */
static const uint16_t slp_to_controller_queue_size = 256;
static uint8_t slp_to_controller_queue_buffer[slp_to_controller_queue_size];

static const uint16_t ble_to_controller_queue_size = 1024;
static uint8_t ble_to_controller_queue_buffer[ble_to_controller_queue_size];

/* Message queue */
static const uint16_t controller_message_queue_size = 64;
static msg_t controller_message_queue[controller_message_queue_size];

/* Controller thread stack */
static const uint16_t controller_stack_size = 2048;
static char controller_stack[controller_stack_size];
static const char controller_prio = PRIORITY_MAIN;
static void *controller_func(void *arg);

/* Device management */
static const uint16_t controller_max_num_of_devs = 128;
static ha_device controller_devs_buffer[controller_max_num_of_devs];
static const char controller_dev_list_filename[] = "dev_lst";
static ha_device_mng controller_dev_mng(controller_devs_buffer, controller_max_num_of_devs,
        controller_dev_list_filename);

/* Time to live for every ALIVE messages */
static const int8_t alive_ttl = 30; /* in second */

/* Time period to save device data */
static const uint8_t dev_list_save_period = 5; /* in seconds */

/* Scene management */
static scene_mng controller_scene_mng(&controller_dev_mng, &MB1_rtc,
        &ha_ns::sixlowpan_sender_pid, &ha_ns::sixlowpan_sender_gff_queue);

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
        "scene -h, get help.\n";

enum scene_cmd_type_e: uint8_t {
    NO_SCENE,
    DEFAULT_SCENE,
    USER_SCENE,
};

/*----------------------------- Controller namespace -------------------------*/

namespace controller_ns {

kernel_pid_t controller_pid;

/* Data Cir_queues */
cir_queue slp_to_controller_queue(slp_to_controller_queue_buffer, slp_to_controller_queue_size);
cir_queue ble_to_controller_queue(ble_to_controller_queue_buffer, ble_to_controller_queue_size);

}

/*----------------------------- Public functions -----------------------------*/
using namespace controller_ns;

/**
 * @brief   Create and start controller thread,
 */
void controller_start(void)
{
    /* Create controller thread */
    controller_pid = thread_create(controller_stack, controller_stack_size,
            controller_prio, CREATE_STACKTEST, controller_func, NULL, "CC Controller");
    if (controller_pid > 0) {
        HA_NOTIFY("CC Controller thread created.\n");
    }
    else {
        HA_NOTIFY("Can't create CC Controller thread.\n");
    }
}

/*----------------------------- Static functions -----------------------------*/
/* Prototypes */
static void slp_gff_handler(uint8_t *gff_frame, ha_device_mng *dev_mng, scene_mng *scene_mng_p,
        kernel_pid_t to_ble_pid, cir_queue *from_ble_queue, cir_queue *to_ble_queue,
        kernel_pid_t to_slp_pid, cir_queue *from_slp_queue, cir_queue *to_slp_queue);

static void ble_gff_handler(uint8_t *gff_frame, ha_device_mng *dev_mng, scene_mng *scene_mng_p,
        kernel_pid_t to_ble_pid, cir_queue *from_ble_queue, cir_queue *to_ble_queue,
        kernel_pid_t to_slp_pid, cir_queue *from_slp_queue, cir_queue *to_slp_queue);

static void save_dev_list_with_1sec(uint8_t save_period, ha_device_mng *dev_mng);

static void process_scene_with_1sec(rtc_ns::time_t &cur_time, scene_mng *scene_mng_p);

static void set_dev_with_index_to_ble(uint32_t index, ha_device_mng *dev_mng,
        kernel_pid_t ble_pid, cir_queue *to_ble_queue);

/* Functions */
static void *controller_func(void *) {
    msg_t mesg;
    uint8_t gff_frame[ha_ns::GFF_MAX_FRAME_SIZE];

    /* Init message queue */
    msg_init_queue(controller_message_queue, controller_message_queue_size);

    /* restore old data */
    controller_dev_mng.restore();
    controller_scene_mng.restore();

    /* Wait for message */
    while (1) {
        msg_receive(&mesg);

        switch (mesg.type) {
        case ha_cc_ns::SLP_GFF_PENDING:
            HA_DEBUG("controller: SLP_GFF_PENDING\n");
            slp_gff_handler(gff_frame, &controller_dev_mng, &controller_scene_mng,
                    ble_thread_ns::ble_thread_pid,
                    NULL, &ble_thread_ns::controller_to_ble_msg_queue,
                    ha_ns::sixlowpan_sender_pid,
                    (cir_queue *)mesg.content.ptr, &ha_ns::sixlowpan_sender_gff_queue);
            break;
        case ha_cc_ns::BLE_GFF_PENDING:
            HA_DEBUG("controller: BLE_GFF_PENDING\n");
            ble_gff_handler(gff_frame, &controller_dev_mng, &controller_scene_mng,
                    ble_thread_ns::ble_thread_pid,
                    (cir_queue *)mesg.content.ptr, &ble_thread_ns::controller_to_ble_msg_queue,
                    ha_ns::sixlowpan_sender_pid,
                    NULL, &ha_ns::sixlowpan_sender_gff_queue);
            break;
        case ha_cc_ns::ONE_SEC_INTERRUPT:
            rtc_ns::time_t cur_time;
            MB1_rtc.get_time(cur_time);
            controller_dev_mng.dec_all_devs_ttl();
            save_dev_list_with_1sec(dev_list_save_period, &controller_dev_mng);
            process_scene_with_1sec(cur_time, &controller_scene_mng);
            break;
        default:
            HA_DEBUG("controller: Unknown message %d\n", mesg.type);
            break;
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/
static void slp_gff_handler(uint8_t *gff_frame, ha_device_mng *dev_mng, scene_mng *scene_mng_p,
        kernel_pid_t to_ble_pid, cir_queue *from_ble_queue, cir_queue *to_ble_queue,
        kernel_pid_t to_slp_pid, cir_queue *from_slp_queue, cir_queue *to_slp_queue)
{
    uint8_t data_len;
    uint16_t cmd_id;
    uint32_t device_id;
    int16_t value, old_value;
    msg_t mesg;
    ha_device device_rpt;

    /* Check data */
    data_len = from_slp_queue->preview_data(false);
    if ((data_len + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE) > from_slp_queue->get_size()) {
        HA_DEBUG("slp_gff_handler: Err, data len %hu + cmd_size + len_size != queue_size %ld\n",
                data_len, from_slp_queue->get_size());
        return;
    }

    /* Get data from queue */
    from_slp_queue->get_data(gff_frame, data_len + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);

    /* parse GFF frame */
    cmd_id = buf2uint16(&gff_frame[ha_ns::GFF_CMD_POS]);

    switch (cmd_id) {
    case ha_ns::SET_DEV_VAL:
        device_id = buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS]);
        value = (int16_t)buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 4]);
        HA_DEBUG("slp_gff_handler: SET_DEV_VAL (%lx, %d)\n", device_id, value);

        /* Processing scene by report */
        device_rpt.set_device_id(device_id);
        device_rpt.set_value(value);
        dev_mng->get_dev_val(device_id, old_value);
        if ((device_rpt.get_io_type() == ha_device_ns::input_device) &&
                value != old_value){
            HA_DEBUG("slp_gff_handler: report from input device, processing scene...\n");
            scene_mng_p->process(true, &device_rpt);
        }

        /* Save data to device manager */
        dev_mng->set_dev_val(device_id, value);
        dev_mng->set_dev_ttl(device_id, alive_ttl);

        /* forward to BLE */
        to_ble_queue->add_data(gff_frame,gff_frame[ha_ns::GFF_LEN_POS]
                                + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char *)to_ble_queue;
        msg_send(&mesg, to_ble_pid, false);
        HA_DEBUG("slp_gff_handler: SET_DEV_VAL forwarded to ble\n");

        break;

    case ha_ns::ALIVE:
        device_id = buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS]);
        HA_DEBUG("slp_gff_handler: ALIVE (%lx)\n", device_id);

        dev_mng->set_dev_ttl(device_id, alive_ttl);
        break;

    default:
        HA_DEBUG("slp_gff_handler: unknown cmd id %x\n", cmd_id);
        break;
    }
}

/*----------------------------------------------------------------------------*/
static void ble_gff_handler(uint8_t *gff_frame, ha_device_mng *dev_mng, scene_mng *scene_mng_p,
        kernel_pid_t to_ble_pid, cir_queue *from_ble_queue, cir_queue *to_ble_queue,
        kernel_pid_t to_slp_pid, cir_queue *from_slp_queue, cir_queue *to_slp_queue)
{
    uint8_t data_len;
    uint16_t cmd_id;
    msg_t mesg;
    uint8_t count;

    /* Check data */
    data_len = from_ble_queue->preview_data(false);
    if ((data_len + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE) > from_ble_queue->get_size()) {
        HA_DEBUG("ble_gff_handler: Err, data len %hu + cmd_size + len_size != queue_size %ld\n",
                data_len, from_ble_queue->get_size());
        return;
    }

    /* Get data from queue */
    from_ble_queue->get_data(gff_frame, data_len + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);

    /* parse GFF frame */
    cmd_id = buf2uint16(&gff_frame[ha_ns::GFF_CMD_POS]);

    switch(cmd_id) {
    case ha_ns::GET_NUM_OF_DEVS:
        HA_DEBUG("ble_gff_handler: GET_NUM_OF_DEVS\n");

        /* Send SET_NUM_OF_DEVS back */
        gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::SET_NUM_OF_DEVS_DATA_LEN;
        uint162buf(ha_ns::SET_NUM_OF_DEVS, &gff_frame[ha_ns::GFF_CMD_POS]);
        uint322buf((uint32_t)dev_mng->get_current_numofdev(), &gff_frame[ha_ns::GFF_DATA_POS]);

        to_ble_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char*)to_ble_queue;
        msg_send(&mesg, to_ble_pid, false);

        HA_DEBUG("ble_gff_handler: sent SET_NUM_OF_DEVS (%hu) to ble\n",
                dev_mng->get_current_numofdev());
        break;

    case ha_ns::GET_DEV_WITH_INDEXS:
        HA_DEBUG("ble_gff_handler: GET_DEV_WITH_INDEXS\n");

        for (count = 0; count < gff_frame[ha_ns::GFF_LEN_POS]; count += 4) {
            set_dev_with_index_to_ble(buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS + count*4]),
                    dev_mng,
                    to_ble_pid, to_ble_queue);
        }
        break;

    case ha_ns::SET_DEV_VAL:
        HA_DEBUG("ble_gff_handler: SET_DEV_VAL (%lx, %d)\n",
                buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS]),
                (int16_t)buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 4]));

        /* forward to slp */
        to_slp_queue->add_data(gff_frame, gff_frame[ha_ns::GFF_LEN_POS]
                             + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char *)to_slp_queue;
        msg_send(&mesg, to_slp_pid, false);

        HA_DEBUG("ble_gff_handler: forwarded GFF SET_DEV_VAL to slp\n");
        break;

    default:
        HA_DEBUG("ble_gff_handler: Unknow command id %hu\n", cmd_id);
        break;
    }
}

/*----------------------------------------------------------------------------*/
void second_int_callback(void)
{
    msg_t mesg;

    mesg.type = ha_cc_ns::ONE_SEC_INTERRUPT;
    msg_send(&mesg, controller_pid, false);
}

/*----------------------------------------------------------------------------*/
static void save_dev_list_with_1sec (uint8_t save_period, ha_device_mng *dev_mng)
{
    static uint8_t time_count = 0;

    time_count++;
    if (time_count > save_period) {
        time_count = 0;
        HA_DEBUG("save_dev_list: Saving devices list\n");
        dev_mng->save();
        HA_DEBUG("save_dev_list: Saved\n");
    }

}

/*----------------------------------------------------------------------------*/
void controller_list_devices(int argc, char** argv)
{
    controller_dev_mng.print_all_devices();
}

/*----------------------------------------------------------------------------*/
static void set_dev_with_index_to_ble(uint32_t index, ha_device_mng *dev_mng,
        kernel_pid_t ble_pid, cir_queue *to_ble_queue)
{
    uint8_t set_dev_windex_gff_frame[ha_ns::SET_DEVICE_WITH_INDEX_DATA_LEN
                                     + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE];
    uint32_t device_id;
    int16_t value;
    uint16_t count;
    msg_t mesg;

    if (index == ha_ns::SET_DEV_WITH_INDEX_ALL_DEVS) {
        /* All device */
        /* reorder */
        dev_mng->reorder();

        for (count = 0; count < dev_mng->get_current_numofdev(); count++) {
            /* Get device id and value from index */
            dev_mng->get_dev_val_with_index(count, device_id, value);

            /* pack GFF */
            set_dev_windex_gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::SET_DEVICE_WITH_INDEX_DATA_LEN;
            uint162buf(ha_ns::SET_DEV_WITH_INDEXS, &set_dev_windex_gff_frame[ha_ns::GFF_CMD_POS]);
            uint322buf(count, &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS]);
            uint322buf(device_id, &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS + 4]);
            uint162buf((uint16_t) value, &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS + 8]);

            /* push data to ble_queue */
            to_ble_queue->add_data(set_dev_windex_gff_frame, ha_ns::SET_DEVICE_WITH_INDEX_DATA_LEN
                                             + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);

            /* pack and send message to ble */
            mesg.type = ha_ns::GFF_PENDING;
            mesg.content.ptr = (char *)to_ble_queue;

            msg_send(&mesg, ble_pid, false);

            HA_DEBUG("set_dev_windex_2_ble: GFF Sent, index %hu, device_id %lu, value %hd\n",
                    count, device_id, value);
        }

        return;
    }

    /* not all device */
    /* Get device id and value from index */
    dev_mng->get_dev_val_with_index(index, device_id, value);

    /* pack GFF */
    set_dev_windex_gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::SET_DEVICE_WITH_INDEX_DATA_LEN;
    uint162buf(ha_ns::SET_DEV_WITH_INDEXS, &set_dev_windex_gff_frame[ha_ns::GFF_CMD_POS]);
    uint322buf(index, &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS]);
    uint322buf(device_id, &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS + 4]);
    uint162buf((uint16_t) value, &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS + 8]);

    /* push data to ble_queue */
    to_ble_queue->add_data(set_dev_windex_gff_frame, ha_ns::SET_DEVICE_WITH_INDEX_DATA_LEN
                                     + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);

    /* pack and send message to ble */
    mesg.type = ha_ns::GFF_PENDING;
    mesg.content.ptr = (char *)to_ble_queue;

    msg_send(&mesg, ble_pid, false);

    HA_DEBUG("set_dev_windex_2_ble: GFF Sent, index %lu, device_id %lu, value %hd\n",
                        index, device_id, value);
}

/*----------------------------------------------------------------------------*/
static void process_scene_with_1sec(rtc_ns::time_t &cur_time, scene_mng *scene_mng_p)
{
    if (cur_time.sec == 0) {
        HA_DEBUG("process_scene_with_1sec: processing scene triggered by time, %hu:%hu:%hu\n",
                cur_time.hour, cur_time.min, cur_time.sec);
        scene_mng_p->process(false, NULL);
    }
}

/*----------------------- Scenes shell command -------------------------------*/
void controller_scene_cmd(int argc, char** argv)
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
                    scene_p = controller_scene_mng.get_default_scene_ptr();
                }
                else if (argv[count][0] == 'u') {
                    scene_type = USER_SCENE;
                    scene_p = controller_scene_mng.get_user_active_scene_ptr();
                }
                else {
                    printf("Err: should be d or u for option -s\n");
                    return;
                }
                break;

            case 'l':
                if (scene_type == USER_SCENE) { /* can be considered as all scenes with -l */
                    controller_scene_mng.print_user_active_scene();
                }
                else if (scene_type == DEFAULT_SCENE) {
                    controller_scene_mng.print_default_scene();
                }
                else {
                    controller_scene_mng.print_default_scene();
                    controller_scene_mng.print_user_active_scene();
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

                    input.time_range.start = MB1_rtc.time_to_packed(in_time);

                    /* end time */
                    in_time.month = atoi(argv[++count]);
                    in_time.day = atoi(argv[++count]);
                    in_time.year = atoi(argv[++count]);
                    in_time.hour = atoi(argv[++count]);
                    in_time.min = atoi(argv[++count]);
                    in_time.sec = atoi(argv[++count]);

                    input.time_range.end = MB1_rtc.time_to_packed(in_time);

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

                    input.time_range.start = MB1_rtc.time_to_packed(in_time) & 0x00FF;

                    /* end time */
                    in_time.hour = atoi(argv[++count]);
                    in_time.min = atoi(argv[++count]);
                    in_time.sec = atoi(argv[++count]);

                    input.time_range.end = MB1_rtc.time_to_packed(in_time) & 0x00FF;
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
                    controller_scene_mng.set_default_scene_valid_status(false);
                }
                else if (scene_type == USER_SCENE) {
                    printf("Halt processing user scene\n");
                    controller_scene_mng.set_user_active_scene_valid_status(false);
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
                    controller_scene_mng.set_default_scene_valid_status(true);
                }
                else if (scene_type == USER_SCENE) {
                    printf("Restart processing user scene\n");
                    controller_scene_mng.set_user_active_scene_valid_status(true);
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
