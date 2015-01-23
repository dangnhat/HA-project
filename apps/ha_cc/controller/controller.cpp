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
#include "zone.h"
#include "MB1_System.h"
#include "ff.h"
#include "shell_cmds_fatfs.h"

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
static const uint16_t controller_max_num_of_devs = 64;
static ha_device controller_devs_buffer[controller_max_num_of_devs];
static const char controller_dev_list_filename[] = "dev_lst";
static ha_device_mng controller_dev_mng(controller_devs_buffer,
        controller_max_num_of_devs, controller_dev_list_filename);

/* Time to live for every ALIVE messages */
static const int16_t alive_ttl = 300; /* in second */

/* Time period to save device data */
static const uint8_t dev_list_save_period = 30; /* in seconds */

/* Scene management */
static scene_mng controller_scene_mng(&controller_dev_mng, &MB1_rtc,
        &ha_ns::sixlowpan_sender_pid, &ha_ns::sixlowpan_sender_gff_queue);

/* State and timeout counter when getting new scene from ble */
static bool new_scene_state = false;
static char new_scene_name[scene_ns::scene_max_name_chars_wout_folders];

static uint8_t new_scene_timeout_counter = 0;
static const uint8_t new_scene_timeout_max_counter = 25; /* in seconds */

static uint16_t new_scene_set_rule_timeout_count = 0;
static uint16_t new_scene_num_rule = 0;
static const uint16_t new_scene_set_rule_timeout_max_count_evrule = 1000; /* in ms */
static uint8_t new_scene_set_rule_resend_count = 0;
static uint8_t new_scene_set_rule_resend_max_count = 1;

/* Zone management */
static zone controller_zone_mng;

/*----------------------------- Controller namespace -------------------------*/

namespace controller_ns {

kernel_pid_t controller_pid;

/* Data Cir_queues */
cir_queue slp_to_controller_queue(slp_to_controller_queue_buffer,
        slp_to_controller_queue_size);
cir_queue ble_to_controller_queue(ble_to_controller_queue_buffer,
        ble_to_controller_queue_size);

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
            controller_prio, CREATE_STACKTEST, controller_func, NULL,
            "CC_controller");
    if (controller_pid > 0) {
        HA_NOTIFY("CC Controller thread created.\n");
    } else {
        HA_NOTIFY("Can't create CC Controller thread.\n");
    }
}

/*----------------------------- Static functions -----------------------------*/
/* Prototypes */
static void slp_gff_handler(uint8_t *gff_frame, ha_device_mng *dev_mng,
        scene_mng *scene_mng_p, kernel_pid_t to_ble_pid,
        cir_queue *from_ble_queue, cir_queue *to_ble_queue,
        kernel_pid_t to_slp_pid, cir_queue *from_slp_queue,
        cir_queue *to_slp_queue);

static void ble_gff_handler(uint8_t *gff_frame, ha_device_mng *dev_mng,
        scene_mng *scene_mng_p, kernel_pid_t to_ble_pid,
        cir_queue *from_ble_queue, cir_queue *to_ble_queue,
        kernel_pid_t to_slp_pid, cir_queue *from_slp_queue,
        cir_queue *to_slp_queue);

static void save_dev_list_with_1sec(uint8_t save_period,
        ha_device_mng *dev_mng);

static void process_scene_with_1sec(rtc_ns::time_t &cur_time,
        scene_mng *scene_mng_p);

static void set_dev_with_index_to_ble(uint32_t index, ha_device_mng *dev_mng,
        kernel_pid_t ble_pid, cir_queue *to_ble_queue);

static void set_inact_scene_name_with_index_to_ble(uint8_t index,
        scene_mng *scene_mng_p, kernel_pid_t ble_pid, cir_queue *to_ble_queue);

static void set_rule_with_index_to_ble(uint16_t index, char *scene_name,
        scene_mng *scene_mng_p, kernel_pid_t ble_pid, cir_queue *to_ble_queue);

static void new_scene_check_timeout_with_1sec(const uint8_t timeout_period, uint8_t &timeout_counter,
        bool &new_scene_state, scene_mng *scene_mng_p);

static void new_scene_set_rule_timeout_handler(uint8_t &resend_count, bool &new_scene_state,
        scene_mng *scene_mng_p, kernel_pid_t ble_pid, cir_queue *to_ble_queue);

static void new_scene_set_rule_1msTIM_ISR(void);

static void set_zone_name_to_ble(uint8_t index,
        zone *zone_p, kernel_pid_t ble_pid, cir_queue *to_ble_queue);

/* Functions */
static void *controller_func(void *)
{
    msg_t mesg;
    uint8_t gff_frame[ha_ns::GFF_MAX_FRAME_SIZE];

    /* Init message queue */
    msg_init_queue(controller_message_queue, controller_message_queue_size);

    /* Assign new_scene_set_rule_1msTIM_ISR to TIM6 */
    if (MB1_ISRs.subISR_assign(ISRMgr_ns::ISRMgr_TIM6, new_scene_set_rule_1msTIM_ISR) !=
            ISRMgr_ns::successful) {
        HA_DEBUG("controller_func: failed to add new_scene_set_rule_1msTIM_ISR to TIM6 int\n");
    }
    else {
        HA_DEBUG("controller_func: added new_scene_set_rule_1msTIM_ISR to TIM6 int\n");
    }

    /* restore old data */
    controller_dev_mng.restore();
    controller_scene_mng.restore();

    /* Wait for message */
    while (1) {
        msg_receive(&mesg);

        switch (mesg.type) {
        case ha_cc_ns::SLP_GFF_PENDING:
            HA_DEBUG("controller: SLP_GFF_PENDING\n");
            slp_gff_handler(gff_frame, &controller_dev_mng,
                    &controller_scene_mng, ble_thread_ns::ble_thread_pid,
                    NULL, &ble_thread_ns::controller_to_ble_msg_queue,
                    ha_ns::sixlowpan_sender_pid, (cir_queue *) mesg.content.ptr,
                    &ha_ns::sixlowpan_sender_gff_queue);
            break;

        case ha_cc_ns::BLE_GFF_PENDING:
            HA_DEBUG("controller: BLE_GFF_PENDING\n");
            ble_gff_handler(gff_frame, &controller_dev_mng,
                    &controller_scene_mng, ble_thread_ns::ble_thread_pid,
                    (cir_queue *) mesg.content.ptr,
                    &ble_thread_ns::controller_to_ble_msg_queue,
                    ha_ns::sixlowpan_sender_pid,
                    NULL, &ha_ns::sixlowpan_sender_gff_queue);
            break;

        case ha_cc_ns::ONE_SEC_INTERRUPT:
            rtc_ns::time_t cur_time;
            MB1_rtc.get_time(cur_time);
            controller_dev_mng.dec_all_devs_ttl();
            save_dev_list_with_1sec(dev_list_save_period, &controller_dev_mng);
            process_scene_with_1sec(cur_time, &controller_scene_mng);
            new_scene_check_timeout_with_1sec(new_scene_timeout_max_counter,
                    new_scene_timeout_counter,
                    new_scene_state, &controller_scene_mng);
            break;

        case ha_cc_ns::NEW_SCENE_SET_RULE_TIMEOUT:
            HA_DEBUG("controller: NEW_SCENE_SET_RULE_TIMEOUT\n");
            new_scene_set_rule_timeout_handler(new_scene_set_rule_resend_count, new_scene_state,
                    &controller_scene_mng,
                    ble_thread_ns::ble_thread_pid,
                    &ble_thread_ns::controller_to_ble_msg_queue);
            break;

        default:
            HA_DEBUG("controller: Unknown message %d\n", mesg.type);
            break;
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/
static void slp_gff_handler(uint8_t *gff_frame, ha_device_mng *dev_mng,
        scene_mng *scene_mng_p, kernel_pid_t to_ble_pid,
        cir_queue *from_ble_queue, cir_queue *to_ble_queue,
        kernel_pid_t to_slp_pid, cir_queue *from_slp_queue,
        cir_queue *to_slp_queue)
{
    uint8_t data_len;
    uint16_t cmd_id;
    uint32_t device_id;
    int16_t value, old_value;
    msg_t mesg;
    ha_device device_rpt;

    /* Check data */
    data_len = from_slp_queue->preview_data(false);
    if ((data_len + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE)
            > from_slp_queue->get_size()) {
        HA_DEBUG(
                "slp_gff_handler: Err, data len %hu + cmd_size + len_size != queue_size %ld\n",
                data_len, from_slp_queue->get_size());
        return;
    }

    /* Get data from queue */
    from_slp_queue->get_data(gff_frame,
            data_len + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);

    /* parse GFF frame */
    cmd_id = buf2uint16(&gff_frame[ha_ns::GFF_CMD_POS]);

    switch (cmd_id) {
    case ha_ns::SET_DEV_VAL:
        device_id = buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS]);
        value = (int16_t) buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 4]);
        HA_DEBUG("slp_gff_handler: SET_DEV_VAL (%lx, %d)\n", device_id, value);

        /* Processing scene by report */
        device_rpt.set_device_id(device_id);
        device_rpt.set_value(value);
        dev_mng->get_dev_val(device_id, old_value);
        if ((device_rpt.get_io_type() == ha_device_ns::input_device)
                && value != old_value) {
            HA_DEBUG(
                    "slp_gff_handler: report from input device, processing scene...\n");
            scene_mng_p->process(true, &device_rpt);
        }

        /* Save data to device manager */
        dev_mng->set_dev_val(device_id, value);
        dev_mng->set_dev_ttl(device_id, alive_ttl);

        /* forward to BLE */
        to_ble_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char *) to_ble_queue;
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
static void ble_gff_handler(uint8_t *gff_frame, ha_device_mng *dev_mng,
        scene_mng *scene_mng_p, kernel_pid_t to_ble_pid,
        cir_queue *from_ble_queue, cir_queue *to_ble_queue,
        kernel_pid_t to_slp_pid, cir_queue *from_slp_queue,
        cir_queue *to_slp_queue)
{
    uint8_t data_len;
    uint16_t cmd_id;
    msg_t mesg;
    uint16_t count;

    uint8_t index, num_scene;
    char scene_name[scene_ns::scene_max_name_chars_wout_folders];
    char scene_name2[scene_ns::scene_max_name_chars_wout_folders];

    uint16_t num_rule, rule_index;
    scene_ns::rule_t a_rule;

    uint8_t zone_id;

    /* Check data */
    data_len = from_ble_queue->preview_data(false);
    if ((data_len + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE)
            > from_ble_queue->get_size()) {
        HA_DEBUG(
                "ble_gff_handler: Err, data len %hu + cmd_size + len_size != queue_size %ld\n",
                data_len, from_ble_queue->get_size());
        return;
    }

    /* Get data from queue */
    from_ble_queue->get_data(gff_frame,
            data_len + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);

    /* parse GFF frame */
    cmd_id = buf2uint16(&gff_frame[ha_ns::GFF_CMD_POS]);

    switch (cmd_id) {
    case ha_ns::GET_NUM_OF_DEVS:
        HA_DEBUG("ble_gff_handler: GET_NUM_OF_DEVS\n");

        /* Send SET_NUM_OF_DEVS back */
        gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::SET_NUM_OF_DEVS_DATA_LEN;
        uint162buf(ha_ns::SET_NUM_OF_DEVS, &gff_frame[ha_ns::GFF_CMD_POS]);
        uint322buf((uint32_t) dev_mng->get_current_numofdev(),
                &gff_frame[ha_ns::GFF_DATA_POS]);

        to_ble_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char*) to_ble_queue;
        msg_send(&mesg, to_ble_pid, false);

        HA_DEBUG("ble_gff_handler: sent SET_NUM_OF_DEVS (%hu) to ble\n",
                dev_mng->get_current_numofdev());
        break;

    case ha_ns::GET_DEV_WITH_INDEXS:
        HA_DEBUG("ble_gff_handler: GET_DEV_WITH_INDEXS\n");

        for (count = 0; count < gff_frame[ha_ns::GFF_LEN_POS]; count += 4) {
            set_dev_with_index_to_ble(
                    buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS + count * 4]),
                    dev_mng, to_ble_pid, to_ble_queue);
        }
        break;

    case ha_ns::SET_DEV_VAL:
        HA_DEBUG("ble_gff_handler: SET_DEV_VAL (%lx, %d)\n",
                buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS]),
                (int16_t )buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 4]));

        /* forward to slp */
        to_slp_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char *) to_slp_queue;
        msg_send(&mesg, to_slp_pid, false);

        HA_DEBUG("ble_gff_handler: forwarded GFF SET_DEV_VAL to slp\n");
        break;

    case ha_ns::GET_ZONE_NAME:
        HA_DEBUG("ble_gff_handler: GET_ZONE_NAME (id %hu)\n",
                gff_frame[ha_ns::GFF_DATA_POS]);

        set_zone_name_to_ble(gff_frame[ha_ns::GFF_DATA_POS],
                &controller_zone_mng,
                ble_thread_ns::ble_thread_pid, &ble_thread_ns::controller_to_ble_msg_queue);

        break;

    case ha_ns::SET_ZONE_NAME:
        HA_DEBUG("ble_gff_handler: SET_ZONE_NAME (id %hu)\n",
                gff_frame[ha_ns::GFF_DATA_POS]);

        /* set zone name */
        zone_id = gff_frame[ha_ns::GFF_DATA_POS];
        controller_zone_mng.set_zone_name(zone_id,
                (char *) &gff_frame[ha_ns::GFF_DATA_POS + 1]);
        break;

    case ha_ns::GET_NUM_OF_SCENES:
        HA_DEBUG("ble_gff_handler: GET_NUM_OF_SCENES\n");

        /* Set back SET_NUM_OF_SCENES */
        gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::SET_NUM_OF_SCENES_DATA_LEN;
        uint162buf(ha_ns::SET_NUM_OF_SCENES, &gff_frame[ha_ns::GFF_CMD_POS]);
        gff_frame[ha_ns::GFF_DATA_POS] =
                scene_mng_p->get_num_of_active_scenes();
        gff_frame[ha_ns::GFF_DATA_POS + 1] =
                scene_mng_p->get_num_of_inactive_scenes();

        to_ble_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char*) to_ble_queue;
        msg_send(&mesg, to_ble_pid, false);
        break;

    case ha_ns::GET_ACT_SCENE_NAME_WITH_INDEXS:
        HA_DEBUG("ble_gff_handler: GET_ACT_SCENE_NAME_WITH_INDEXS (%hu)\n",
                gff_frame[ha_ns::GFF_DATA_POS]);

        /* Get index */
        index = gff_frame[ha_ns::GFF_DATA_POS];
        if (index != 0xFF && index != 0x00) {
            HA_DEBUG("ble_gff_handler: wrong index for active scene (%hu)\n",
                    index);
            break;
        }

        /* Return active scene name */
        scene_name[0] = '\0';
        scene_mng_p->get_active_scene(scene_name);

        /* Send back SET_ACT_SCENE_NAME_WITH_INDEXS */
        gff_frame[ha_ns::GFF_LEN_POS] =
                ha_ns::SET_ACT_SCENE_NAME_WITH_INDEXS_DATA_LEN;
        uint162buf(ha_ns::SET_ACT_SCENE_NAME_WITH_INDEXS,
                &gff_frame[ha_ns::GFF_CMD_POS]);
        gff_frame[ha_ns::GFF_DATA_POS] = 0;
        memcpy(&gff_frame[ha_ns::GFF_DATA_POS + 1], scene_name, 8);

        to_ble_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char*) to_ble_queue;
        msg_send(&mesg, to_ble_pid, false);

        HA_DEBUG("ble_gff_handler: sent active scene name back to ble (%s)\n",
                scene_name);
        break;

    case ha_ns::GET_INACT_SCENE_NAME_WITH_INDEXS:
        HA_DEBUG("ble_gff_handler: GET_INACT_SCENE_NAME_WITH_INDEXS (%hu)\n",
                gff_frame[ha_ns::GFF_DATA_POS]);

        /* check first index */
        if (gff_frame[ha_ns::GFF_DATA_POS] == 0xFF) {
            /* send all inactive scene name to ble thread */
            num_scene = scene_mng_p->get_num_of_inactive_scenes();
            for (count = 0; count < num_scene; count++) {
                set_inact_scene_name_with_index_to_ble(count, scene_mng_p,
                        ble_thread_ns::ble_thread_pid, &ble_thread_ns::controller_to_ble_msg_queue);
            }
        }
        else {
            /* loop through every indexes */
            for (count = 0; count < gff_frame[ha_ns::GFF_LEN_POS]; count++) {
                set_inact_scene_name_with_index_to_ble(gff_frame[ha_ns::GFF_DATA_POS + count],
                        scene_mng_p,
                        ble_thread_ns::ble_thread_pid, &ble_thread_ns::controller_to_ble_msg_queue);
            }
        }
        break;

    case ha_ns::GET_NUM_OF_RULES:
        HA_DEBUG("ble_gff_handler: GET_NUM_OF_RULES\n");

        /* Check scene's name */
        memcpy(scene_name, &gff_frame[ha_ns::GFF_DATA_POS], 8);
        scene_name[9] = '\0';
        scene_mng_p->get_user_scene(scene_name2);

        if (strcmp(scene_name, scene_name2) != 0) {
            HA_DEBUG("ble_gff_handler: Received scene name (%s) is not current running scene name (%s)\n",
                    scene_name, scene_name2);

            /* load scene_name to current running scene */
            scene_mng_p->set_user_scene(scene_name);
            scene_mng_p->restore_user_scene();
            HA_DEBUG("ble_gff_handler: Current running scene changed to %s\n",
                    scene_name);
        }

        /* Get num of rules of current running scene name and send back */
        gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::SET_NUM_OF_RULES_DATA_LEN;
        uint162buf(ha_ns::SET_NUM_OF_RULES, &gff_frame[ha_ns::GFF_CMD_POS]);
        memcpy(&gff_frame[ha_ns::GFF_DATA_POS], scene_name, 8);
        uint162buf(scene_mng_p->get_user_scene_ptr()->get_cur_num_rules(),
                &gff_frame[ha_ns::GFF_DATA_POS + 8]);

        to_ble_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char*) to_ble_queue;
        msg_send(&mesg, to_ble_pid, false);

        HA_DEBUG("ble_gff_handler: sent num of rules back to ble (%s, %hu)\n",
                scene_name, scene_mng_p->get_user_scene_ptr()->get_cur_num_rules());

        /* Restore old user scene */
        if (strcmp(scene_name, scene_name2) != 0) {
            scene_mng_p->set_user_scene(scene_name2);
            scene_mng_p->restore_user_scene();

            HA_DEBUG("ble_gff_handler: Old user scene %s restored\n", scene_name2);
        }
        break;

    case ha_ns::GET_RULE_WITH_INDEXS:
        HA_DEBUG("ble_gff_handler: GET_RULE_WITH_INDEXS\n");

        /* Check scene's name */
        memcpy(scene_name, &gff_frame[ha_ns::GFF_DATA_POS], 8);
        scene_name[9] = '\0';
        scene_mng_p->get_user_scene(scene_name2);

        if (strcmp(scene_name, scene_name2) != 0) {
            HA_DEBUG("ble_gff_handler: Received scene name (%s) is not current running scene name (%s)\n",
                scene_name, scene_name2);

            /* load scene_name to current running scene */
            scene_mng_p->set_user_scene(scene_name);
            scene_mng_p->restore_user_scene();
            HA_DEBUG("ble_gff_handler: Current running scene changed to %s\n",
                    scene_name);
        }

        /* check first index */
        if (buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 8]) == 0xFFFF) {
            /* send all rules to ble thread */
            num_rule = scene_mng_p->get_user_scene_ptr()->get_cur_num_rules();
            for (count = 0; count < num_rule; count++) {
                set_rule_with_index_to_ble(count, scene_name, scene_mng_p,
                        ble_thread_ns::ble_thread_pid, &ble_thread_ns::controller_to_ble_msg_queue);
            }
        }
        else {
            /* loop through every indexes */
            for (count = 0; count < ((gff_frame[ha_ns::GFF_LEN_POS] - 8) / 2); count++) {
                set_rule_with_index_to_ble(buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 8 + count*2]),
                        scene_name,
                        scene_mng_p,
                        ble_thread_ns::ble_thread_pid, &ble_thread_ns::controller_to_ble_msg_queue);
            }
        }

        /* Restore old user scene */
        if (strcmp(scene_name, scene_name2) != 0) {
            scene_mng_p->set_user_scene(scene_name2);
            scene_mng_p->restore_user_scene();

            HA_DEBUG("ble_gff_handler: Old user scene %s restored\n", scene_name2);
        }
        break;

    case ha_ns::SET_ACT_SCENE_NAME_WITH_INDEXS:
        HA_DEBUG("ble_gff_handler: SET_ACT_SCENE_NAME_WITH_INDEXS\n");

        /* don't care index */
        memcpy(scene_name, &gff_frame[ha_ns::GFF_DATA_POS + 1], 8);
        scene_name[9] = '\0';

        /* set active scene and restore */
        scene_mng_p->set_active_scene(scene_name);
        scene_mng_p->set_user_scene(scene_name);
        scene_mng_p->restore_user_scene();

        /* feedback to ble */
        scene_mng_p->get_user_scene(scene_name);
        memcpy(&gff_frame[ha_ns::GFF_DATA_POS], scene_name, 8);
        to_ble_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char*) to_ble_queue;
        msg_send(&mesg, to_ble_pid, false);

        HA_DEBUG("ble_gff_handler: sent SET_ACT_SCENE_NAME_WITH_INDEXS (%s) back to ble\n",
                scene_name);

        break;

    case ha_ns::SET_REMOVE_SCENE:
        HA_DEBUG("ble_gff_handler: SET_REMOVE_SCENE\n");

        /* get scene name */
        memcpy(scene_name, &gff_frame[ha_ns::GFF_DATA_POS], 8);
        scene_name[9] = '\0';

        /* compare with current running scene */
        scene_mng_p->get_user_scene(scene_name2);
        if (strcmp(scene_name, scene_name2) == 0) {
            HA_DEBUG("ble_gff_handler: Will not rename current running scene\n");
            scene_name[0] = '\0';
        }
        else {
            if (scene_mng_p->remove_inactive_scene(scene_name) == -1) {
                HA_DEBUG("ble_gff_hanlder: Failed to remove scene (%s)\n", scene_name);
                scene_name[0] = '\0';
            }
            else {
                HA_DEBUG("ble_gff_handler: %s removed\n", scene_name);
            }
        }

        /* Send SET_REMOVE_SCENE back */
        memcpy(&gff_frame[ha_ns::GFF_DATA_POS], scene_name, 8);
        to_ble_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char*) to_ble_queue;
        msg_send(&mesg, to_ble_pid, false);

        HA_DEBUG("ble_gff_handler: sent SET_REMOVE_SCENE (%s) back to ble\n",
                scene_name);
        break;

    case ha_ns::SET_RENAME_INACT_SCENE:
        HA_DEBUG("ble_gff_handler: SET_RENAME_INACT_SCENE\n");

        /* get old scene name */
        memcpy(scene_name, &gff_frame[ha_ns::GFF_DATA_POS], 8);
        scene_name[9] = '\0';
        /* compare with current running scene */
        scene_mng_p->get_user_scene(scene_name2);
        if (strcmp(scene_name, scene_name2) == 0) {
            HA_DEBUG("ble_gff_handler: Will not rename current running scene\n");
            scene_name[0] = '\0';
            scene_name2[0] = '\0';
        }
        else {
            /* get new name */
            memcpy(scene_name2, &gff_frame[ha_ns::GFF_DATA_POS + 8], 8);
            scene_name2[9] = '\0';
            scene_mng_p->rename_inactive_scene(scene_name, scene_name2);
        }

        /* send SET_RENAME_INACT_SCENE back to ble */
        memcpy(&gff_frame[ha_ns::GFF_DATA_POS], scene_name, 8);
        memcpy(&gff_frame[ha_ns::GFF_DATA_POS+8], scene_name2, 8);

        to_ble_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char*) to_ble_queue;
        msg_send(&mesg, to_ble_pid, false);

        HA_DEBUG("ble_gff_handler: sent SET_RENAME_INACT_SCENE (%s -> %s) back to ble\n",
                scene_name, scene_name2);
        break;

    case ha_ns::SET_NEW_SCENE:
        HA_DEBUG("ble_gff_handler: SET_NEW_SCENE\n");

        /* get scene name */
        memcpy(scene_name, &gff_frame[ha_ns::GFF_DATA_POS], 8);
        scene_name[9] = '\0';

        /* compare with current running scene */
        scene_mng_p->get_user_scene(scene_name2);

        if (strcmp(scene_name, scene_name2) != 0) {
            HA_DEBUG("ble_gff_handler: Received scene name (%s) is not current running scene name (%s)\n",
                scene_name, scene_name2);

            /* load scene_name to current running scene */
            scene_mng_p->set_user_scene(scene_name);
            scene_mng_p->get_user_scene_ptr()->new_scene();
            scene_mng_p->set_user_scene_valid_status(false);
            HA_DEBUG("ble_gff_handler: Current running scene changed to new scene %s\n",
                    scene_name);
        }

        /* send GET_NUM_OF_RULES */
        gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::GET_NUM_OF_RULES_DATA_LEN;
        uint162buf(ha_ns::GET_NUM_OF_RULES, &gff_frame[ha_ns::GFF_CMD_POS]);
        memcpy(&gff_frame[ha_ns::GFF_DATA_POS], scene_name, 8);

        to_ble_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char*) to_ble_queue;
        msg_send(&mesg, to_ble_pid, false);

        HA_DEBUG("ble_gff_handler: sent GET_NUM_OF_RULES (%s) to ble\n",
                scene_name);

        /* change to new scene state */
        new_scene_state = true;
        strcpy(new_scene_name, scene_name);
        new_scene_timeout_counter = new_scene_timeout_max_counter;

        HA_DEBUG("ble_gff_handler: Changed to new scene state (%s, %hu)\n",
                scene_name, new_scene_timeout_counter);
        break;

    case ha_ns::SET_NUM_OF_RULES:
        HA_DEBUG("ble_gff_handler: SET_NUM_OF_RULES\n");

        if(!new_scene_state) {
            HA_DEBUG("ble_gff_handler: not in new scene state, dropped\n");
            break;
        }

        /* get scene name */
        memcpy(scene_name, &gff_frame[ha_ns::GFF_DATA_POS], 8);
        scene_name[9] = '\0';

        /* compare with current running scene */
        scene_mng_p->get_user_scene(scene_name2);

        if (strcmp(scene_name, scene_name2) != 0) {
            HA_DEBUG("ble_gff_handler: Received scene name (%s) is not "
                    "current running scene name (%s), break\n",
                scene_name, scene_name2);
            break;
        }

        /* Set num rules */
        scene_mng_p->get_user_scene_ptr()->set_cur_num_rules(
                buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 8]));

        HA_DEBUG("ble_gff_handler: Set num of rules (%hu) to user scene (%s)\n",
                buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 8]), scene_name);

        /* Save num rules */
        new_scene_num_rule = buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 8]);

        /* Send GET_RULE_WITH_INDEXS */
        gff_frame[ha_ns::GFF_LEN_POS] = 10;
        uint162buf(ha_ns::GET_RULE_WITH_INDEXS, &gff_frame[ha_ns::GFF_CMD_POS]);
        memcpy(&gff_frame[ha_ns::GFF_DATA_POS], scene_name, 8);
        uint162buf(0xFFFF, &gff_frame[ha_ns::GFF_DATA_POS + 8]);

        to_ble_queue->add_data(gff_frame,
                gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE);
        mesg.type = ha_ns::GFF_PENDING;
        mesg.content.ptr = (char*) to_ble_queue;
        msg_send(&mesg, to_ble_pid, false);

        HA_DEBUG("ble_gff_handler: sent GET_RULE_WITH_INDEXS (%s, %hx) to ble\n",
                scene_name, 0xFFFF);

        /* Turn on timeout */
        new_scene_set_rule_timeout_count = new_scene_num_rule *
                new_scene_set_rule_timeout_max_count_evrule;
        new_scene_set_rule_resend_count = new_scene_set_rule_resend_max_count;
        HA_DEBUG("ble_gff_handler: turn on timeout counter for SET_RULE_WITH_INDEXS "
                "(%hu ms, resend %hu)\n",
                new_scene_set_rule_timeout_count, new_scene_set_rule_resend_count);
        break;

    case ha_ns::SET_RULE_WITH_INDEXS:
        HA_DEBUG("ble_gff_handler: SET_RULE_WITH_INDEXS\n");

        if(!new_scene_state) {
            HA_DEBUG("ble_gff_handler: not in new scene state, dropped\n");
            break;
        }

        /* get scene name */
        memcpy(scene_name, &gff_frame[ha_ns::GFF_DATA_POS], 8);
        scene_name[9] = '\0';

        /* compare with current running scene */
        scene_mng_p->get_user_scene(scene_name2);

        if (strcmp(scene_name, scene_name2) != 0) {
            HA_DEBUG("ble_gff_handler: Received scene name (%s) is not "
                    "current running scene name (%s), break\n",
                scene_name, scene_name2);
            break;
        }

        /* Set rule with index */
        rule_index = buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 8]);
        a_rule.is_valid = true;
        a_rule.num_in = 1;
        a_rule.num_out = 1;
        a_rule.is_active = gff_frame[ha_ns::GFF_DATA_POS + 10];
        a_rule.inputs[0].cond = gff_frame[ha_ns::GFF_DATA_POS + 11];
        switch(a_rule.inputs[0].cond) {
        case scene_ns::COND_IN_RANGE:
        case scene_ns::COND_IN_RANGE_EVDAY:
            a_rule.inputs[0].time_range.start = buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS + 12]);
            a_rule.inputs[0].time_range.end = buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS + 16]);
            break;

        default:
            a_rule.inputs[0].dev_val.device_id = buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS + 12]);
            a_rule.inputs[0].dev_val.value = buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 16]);
            break;
        }

        a_rule.outputs[0].action = gff_frame[ha_ns::GFF_DATA_POS + 20];
        switch(a_rule.outputs[0].action) {
        default:
            a_rule.outputs[0].dev_val.device_id = buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS + 21]);
            a_rule.outputs[0].dev_val.value = buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 25]);
            break;
        }

        if (scene_mng_p->get_user_scene_ptr()->add_rule_with_index(a_rule, rule_index) == 0) {
            HA_DEBUG("ble_gff_handler: added rule with index (%hu) to scene (%s)\n",
                    rule_index, scene_name);
        }
        else {
            HA_DEBUG("ble_gff_handler: failed to add rule with index (%hu) to scene (%s)\n",
                    rule_index, scene_name);
        }
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
static void save_dev_list_with_1sec(uint8_t save_period, ha_device_mng *dev_mng)
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
            set_dev_windex_gff_frame[ha_ns::GFF_LEN_POS] =
                    ha_ns::SET_DEVICE_WITH_INDEX_DATA_LEN;
            uint162buf(ha_ns::SET_DEV_WITH_INDEXS,
                    &set_dev_windex_gff_frame[ha_ns::GFF_CMD_POS]);
            uint322buf(count, &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS]);
            uint322buf(device_id,
                    &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS + 4]);
            uint162buf((uint16_t) value,
                    &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS + 8]);

            /* push data to ble_queue */
            to_ble_queue->add_data(set_dev_windex_gff_frame,
                    ha_ns::SET_DEVICE_WITH_INDEX_DATA_LEN + ha_ns::GFF_CMD_SIZE
                            + ha_ns::GFF_LEN_SIZE);

            /* pack and send message to ble */
            mesg.type = ha_ns::GFF_PENDING;
            mesg.content.ptr = (char *) to_ble_queue;

            msg_send(&mesg, ble_pid, false);

            HA_DEBUG(
                    "set_dev_windex_2_ble: GFF Sent, index %hu, device_id %lu, value %hd\n",
                    count, device_id, value);
        }

        return;
    }

    /* not all device */
    /* Get device id and value from index */
    dev_mng->get_dev_val_with_index(index, device_id, value);

    /* pack GFF */
    set_dev_windex_gff_frame[ha_ns::GFF_LEN_POS] =
            ha_ns::SET_DEVICE_WITH_INDEX_DATA_LEN;
    uint162buf(ha_ns::SET_DEV_WITH_INDEXS,
            &set_dev_windex_gff_frame[ha_ns::GFF_CMD_POS]);
    uint322buf(index, &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS]);
    uint322buf(device_id, &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS + 4]);
    uint162buf((uint16_t) value,
            &set_dev_windex_gff_frame[ha_ns::GFF_DATA_POS + 8]);

    /* push data to ble_queue */
    to_ble_queue->add_data(set_dev_windex_gff_frame,
            ha_ns::SET_DEVICE_WITH_INDEX_DATA_LEN + ha_ns::GFF_CMD_SIZE
                    + ha_ns::GFF_LEN_SIZE);

    /* pack and send message to ble */
    mesg.type = ha_ns::GFF_PENDING;
    mesg.content.ptr = (char *) to_ble_queue;

    msg_send(&mesg, ble_pid, false);

    HA_DEBUG(
            "set_dev_windex_2_ble: GFF Sent, index %lu, device_id %lu, value %hd\n",
            index, device_id, value);
}

/*----------------------------------------------------------------------------*/
static void set_inact_scene_name_with_index_to_ble(uint8_t index,
        scene_mng *scene_mng_p, kernel_pid_t ble_pid, cir_queue *to_ble_queue)
{
    char scene_name[scene_ns::scene_max_name_chars_wout_folders];
    msg_t mesg;
    uint8_t set_inact_scene_name_windex_gff_frame[ha_ns::SET_INACT_SCENE_NAME_WITH_INDEXS_DATA_LEN
            + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE];

    if (index == 0xFF) {
        return;
    }

    /* normal index */
    scene_name[0] = '\0';
    scene_mng_p->get_inactive_scene_with_index(index, scene_name);

    /* pack gff frame and send to ble */
    set_inact_scene_name_windex_gff_frame[ha_ns::GFF_LEN_POS] =
            ha_ns::SET_INACT_SCENE_NAME_WITH_INDEXS_DATA_LEN;
    uint162buf(ha_ns::SET_INACT_SCENE_NAME_WITH_INDEXS,
            &set_inact_scene_name_windex_gff_frame[ha_ns::GFF_CMD_POS]);
    set_inact_scene_name_windex_gff_frame[ha_ns::GFF_DATA_POS] = index;
    memcpy(&set_inact_scene_name_windex_gff_frame[ha_ns::GFF_DATA_POS + 1],
            scene_name, 8);

    to_ble_queue->add_data(set_inact_scene_name_windex_gff_frame,
            set_inact_scene_name_windex_gff_frame[ha_ns::GFF_LEN_POS]
                    + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);
    mesg.type = ha_ns::GFF_PENDING;
    mesg.content.ptr = (char*) to_ble_queue;
    msg_send(&mesg, ble_pid, false);

    HA_DEBUG("ble_gff_handler: sent inactive scene name back to ble (%hu, %s)\n",
            index, scene_name);
}

/*----------------------------------------------------------------------------*/
static void set_rule_with_index_to_ble(uint16_t index, char *scene_name,
        scene_mng *scene_mng_p, kernel_pid_t ble_pid, cir_queue *to_ble_queue)
{
    scene_ns::rule_t a_rule;
    uint8_t set_rule_windex_gff_frame[ha_ns::SET_RULE_WITH_INDEXS_DATA_LEN
                + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE];
    msg_t mesg;

    if (index == 0xFFFF) {
        return;
    }

    /* normal index, get rule */
    scene_mng_p->get_user_scene_ptr()->get_rule_with_index(a_rule, index);

    /* check valid bit */
    if (!a_rule.is_valid) {
        HA_DEBUG("set_rule_with_index_to_ble: rule (%hu) is not valid, dropped\n", index);
        return;
    }

    /* pack gff frame and send back to ble */
    set_rule_windex_gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::SET_RULE_WITH_INDEXS_DATA_LEN;
    uint162buf(ha_ns::SET_RULE_WITH_INDEXS, &set_rule_windex_gff_frame[ha_ns::GFF_CMD_POS]);
    memcpy(&set_rule_windex_gff_frame[ha_ns::GFF_DATA_POS], scene_name, 8);
    uint162buf(index, &set_rule_windex_gff_frame[ha_ns::GFF_DATA_POS + 8]);
    set_rule_windex_gff_frame[ha_ns::GFF_DATA_POS + 10] = a_rule.is_active ? 1 : 0;

    set_rule_windex_gff_frame[ha_ns::GFF_DATA_POS + 11] = a_rule.inputs[0].cond;
    switch (a_rule.inputs[0].cond) {
    case scene_ns::COND_IN_RANGE:
    case scene_ns::COND_IN_RANGE_EVDAY:
        uint322buf(a_rule.inputs[0].time_range.start,
                &set_rule_windex_gff_frame[ha_ns::GFF_DATA_POS + 12]);
        uint322buf(a_rule.inputs[0].time_range.end,
                &set_rule_windex_gff_frame[ha_ns::GFF_DATA_POS + 16]);
        break;

    default:
        uint322buf(a_rule.inputs[0].dev_val.device_id,
                &set_rule_windex_gff_frame[ha_ns::GFF_DATA_POS + 12]);
        uint162buf(a_rule.inputs[0].dev_val.value,
                &set_rule_windex_gff_frame[ha_ns::GFF_DATA_POS + 16]);
        break;
    };

    set_rule_windex_gff_frame[ha_ns::GFF_DATA_POS + 20] = a_rule.outputs[0].action;
    uint322buf(a_rule.outputs[0].dev_val.device_id,
            &set_rule_windex_gff_frame[ha_ns::GFF_DATA_POS + 21]);
    uint162buf(a_rule.outputs[0].dev_val.value,
            &set_rule_windex_gff_frame[ha_ns::GFF_DATA_POS + 25]);

    to_ble_queue->add_data(set_rule_windex_gff_frame,
            set_rule_windex_gff_frame[ha_ns::GFF_LEN_POS]
                    + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);
    mesg.type = ha_ns::GFF_PENDING;
    mesg.content.ptr = (char*) to_ble_queue;
    msg_send(&mesg, ble_pid, false);

    HA_DEBUG("ble_gff_handler: sent rule with index (%hu) back to ble\n",
            index);
}

/*----------------------------------------------------------------------------*/
static void process_scene_with_1sec(rtc_ns::time_t &cur_time,
        scene_mng *scene_mng_p)
{
    if (cur_time.sec == 0) {
        HA_DEBUG("process_scene_with_1sec: processing scene triggered by time, %hu:%hu:%hu\n",
                cur_time.hour, cur_time.min, cur_time.sec);
        scene_mng_p->process(false, NULL);
    }
}

/*----------------------------------------------------------------------------*/
static void new_scene_check_timeout_with_1sec(const uint8_t timeout_period, uint8_t &timeout_counter,
        bool &new_scene_state, scene_mng *scene_mng_p)
{
    char scene_name[scene_ns::scene_max_name_chars_wout_folders];

    if ((new_scene_state) && (timeout_counter > 0)) {
        timeout_counter--;
        if (timeout_counter == 0) {
            scene_mng_p->get_active_scene(scene_name);
            scene_mng_p->set_user_scene(scene_name);
            scene_mng_p->restore_user_scene();

            new_scene_state = false;

            HA_DEBUG("new_scene_check_timeout_with_1sec: restored active scene (%s)\n",
                    scene_name);
        }
        else {
            HA_DEBUG("new_scene_check_timeout_with_1sec: in new scene state, timeout %hu\n",
                    timeout_counter);
        }
    }
}

/*----------------------------------------------------------------------------*/
static void new_scene_set_rule_1msTIM_ISR(void)
{
    msg_t mesg;

    if (new_scene_set_rule_timeout_count > 0) {
        new_scene_set_rule_timeout_count--;
        if (new_scene_set_rule_timeout_count == 0) {
            mesg.type = ha_cc_ns::NEW_SCENE_SET_RULE_TIMEOUT;
            msg_send(&mesg, controller_pid, false);
        }
    }
}

/*----------------------------------------------------------------------------*/
static void new_scene_set_rule_timeout_handler(uint8_t &resend_count, bool &new_scene_state,
        scene_mng *scene_mng_p, kernel_pid_t ble_pid, cir_queue *to_ble_queue)
{
    uint16_t invalid_index;
    uint8_t a_gff_frame[10 + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE];
    msg_t mesg;
    char scene_name[scene_ns::scene_max_name_chars_wout_folders];

    if (!new_scene_state) {
        HA_DEBUG("new_scene_set_rule_timeout_handler: not in new scene state\n");
        return;
    }

    if (resend_count > 0) {
        /* Check invalid rules and resend get_rule_with_index */
        if (scene_mng_p->get_user_scene_ptr()->find_invalid_rule(invalid_index, false)) {
            do {
                a_gff_frame[ha_ns::GFF_LEN_POS] = 10;
                uint162buf(ha_ns::GET_RULE_WITH_INDEXS,
                        &a_gff_frame[ha_ns::GFF_CMD_POS]);
                memcpy(&a_gff_frame[ha_ns::GFF_DATA_POS], new_scene_name, 8);
                uint162buf(invalid_index, &a_gff_frame[ha_ns::GFF_DATA_POS + 8]);

                to_ble_queue->add_data(a_gff_frame,
                        a_gff_frame[ha_ns::GFF_LEN_POS]
                                + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);
                mesg.type = ha_ns::GFF_PENDING;
                mesg.content.ptr = (char*) to_ble_queue;
                msg_send(&mesg, ble_pid, false);

                HA_DEBUG("new_scene_set_rule_timeout_handler: Resend GET_RULE_WITH_INDEXS"
                        "(%s, %hu) to ble\n", new_scene_name, invalid_index);
            }
            while (scene_mng_p->get_user_scene_ptr()->find_invalid_rule(invalid_index, true));

            resend_count--;
            new_scene_set_rule_timeout_count = new_scene_num_rule *
                    new_scene_set_rule_timeout_max_count_evrule;
        }
        else {
            /* No invalid rule, save new scene and send new scene back to ble */
            new_scene_state = false;
            scene_mng_p->get_user_scene_ptr()->save();
            HA_DEBUG("new_scene_set_rule_timeout_handler: no invalid rule, "
                    "clear new scene state (%hu),"
                    "(%s) saved\n", new_scene_state, new_scene_name);

            a_gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::SET_NEW_SCENE_DATA_LEN;
            uint162buf(ha_ns::SET_NEW_SCENE, &a_gff_frame[ha_ns::GFF_CMD_POS]);
            memcpy(&a_gff_frame[ha_ns::GFF_DATA_POS], new_scene_name, 8);

            to_ble_queue->add_data(a_gff_frame,
                    a_gff_frame[ha_ns::GFF_LEN_POS]
                            + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);
            mesg.type = ha_ns::GFF_PENDING;
            mesg.content.ptr = (char*) to_ble_queue;
            msg_send(&mesg, ble_pid, false);

            HA_DEBUG("new_scene_set_rule_timeout_handler: Resend SET_NEW_SCENE (%s)"
                    "to ble\n", new_scene_name);

            /* restore active scene */
            scene_mng_p->get_active_scene(scene_name);
            scene_mng_p->set_user_scene(scene_name);
            scene_mng_p->restore_user_scene();

            HA_DEBUG("new_scene_set_rule_timeout_handler: active scene (%s) restored\n",
                    scene_name);
        }
    }
    else {
        /* check invalid rules */
        if (!scene_mng_p->get_user_scene_ptr()->find_invalid_rule(invalid_index, false)) {
            /* No invalid rule, save new scene and send new scene back to ble */
            new_scene_state = false;
            scene_mng_p->get_user_scene_ptr()->save();
            HA_DEBUG("new_scene_set_rule_timeout_handler: no invalid rule, "
                    "clear new scene state (%hu),"
                    "(%s) saved\n", new_scene_state, new_scene_name);

            a_gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::SET_NEW_SCENE_DATA_LEN;
            uint162buf(ha_ns::SET_NEW_SCENE, &a_gff_frame[ha_ns::GFF_CMD_POS]);
            memcpy(&a_gff_frame[ha_ns::GFF_DATA_POS], new_scene_name, 8);

            to_ble_queue->add_data(a_gff_frame,
                    a_gff_frame[ha_ns::GFF_LEN_POS]
                            + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);
            mesg.type = ha_ns::GFF_PENDING;
            mesg.content.ptr = (char*) to_ble_queue;
            msg_send(&mesg, ble_pid, false);

            HA_DEBUG("new_scene_set_rule_timeout_handler: Resend SET_NEW_SCENE (%s)"
                    "to ble\n", new_scene_name);
        }
        else {
            HA_DEBUG("new_scene_set_rule_timeout_handler: Still invalid rule\n");
        }

        /* restore active scene */
        scene_mng_p->get_active_scene(scene_name);
        scene_mng_p->set_user_scene(scene_name);
        scene_mng_p->restore_user_scene();

        HA_DEBUG("new_scene_set_rule_timeout_handler: active scene (%s) restored\n",
                scene_name);
    }

}

/*----------------------------------------------------------------------------*/
static void set_zone_name_to_ble(uint8_t index,
        zone *zone_p, kernel_pid_t ble_pid, cir_queue *to_ble_queue)
{
    char zone_name[zone_ns::zone_name_max_size];
    uint8_t zone_id;
    FRESULT fres;
    DIR dir;
    FILINFO finfo;
    msg_t mesg;
    uint8_t set_zone_name_gff_frame[ha_ns::SET_ZONE_NAME_DATA_LEN
            + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE];

    if (index == 0xFF) {
        /* Open zones folder to get all zone name */
        fres = f_opendir(&dir, ZONES_FOLDER);
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

            if (finfo.fname[0] == '.') {
                continue;
            }

            /* pack gff frame */
            zone_id = (uint8_t)atoi(finfo.fname);

            set_zone_name_gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::SET_ZONE_NAME_DATA_LEN;
            uint162buf(ha_ns::SET_ZONE_NAME, &set_zone_name_gff_frame[ha_ns::GFF_CMD_POS]);
            set_zone_name_gff_frame[ha_ns::GFF_DATA_POS] = zone_id;
            zone_p->get_zone_name(zone_id, zone_ns::zone_name_max_size,
                    zone_name);
            memcpy(&set_zone_name_gff_frame[ha_ns::GFF_DATA_POS + 1], zone_name,
                       zone_ns::zone_name_max_size);

            to_ble_queue->add_data(set_zone_name_gff_frame,
                  set_zone_name_gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                         + ha_ns::GFF_LEN_SIZE);
            mesg.type = ha_ns::GFF_PENDING;
            mesg.content.ptr = (char*) to_ble_queue;
            msg_send(&mesg, ble_pid, false);

            HA_DEBUG("ble_gff_handler: sent SET_ZONE_NAME (%hu, %s) to ble\n",
                   zone_id, zone_name);
        }

        /* close dir */
        f_closedir(&dir);
    }

    /* normal index */
    zone_id = index;
    zone_p->get_zone_name(zone_id, zone_ns::zone_name_max_size,
           zone_name);

    /* pack SET_ZONE_NAME gff frame to send to ble */
    set_zone_name_gff_frame[ha_ns::GFF_LEN_POS] = ha_ns::SET_ZONE_NAME_DATA_LEN;
    uint162buf(ha_ns::SET_ZONE_NAME, &set_zone_name_gff_frame[ha_ns::GFF_CMD_POS]);
    set_zone_name_gff_frame[ha_ns::GFF_DATA_POS] = zone_id;
    memcpy(&set_zone_name_gff_frame[ha_ns::GFF_DATA_POS + 1], zone_name,
           zone_ns::zone_name_max_size);

    to_ble_queue->add_data(set_zone_name_gff_frame,
            set_zone_name_gff_frame[ha_ns::GFF_LEN_POS] + ha_ns::GFF_CMD_SIZE
                   + ha_ns::GFF_LEN_SIZE);
    mesg.type = ha_ns::GFF_PENDING;
    mesg.content.ptr = (char*) to_ble_queue;
    msg_send(&mesg, ble_pid, false);

    HA_DEBUG("ble_gff_handler: sent SET_ZONE_NAME (%hu, %s) to ble\n",
           zone_id, zone_name);

}


/*----------------------- Scenes shell command -------------------------------*/
void controller_scene_cmd(int argc, char** argv)
{
    scene_mng_cmd(controller_scene_mng, MB1_rtc, argc, argv);
}

/*----------------------- Zone shell command ---------------------------------*/
void controller_zone_cmd(int argc, char** argv)
{
    zone_cmd(controller_zone_mng, argc, argv);
}
