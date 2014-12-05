/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        scene.h
 * @brief       Scene manager class.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#ifndef SCENE_MNG_H_
#define SCENE_MNG_H_

#include <stdint.h>
#include <string.h>

extern "C" {
#include "thread.h"
}

#include "scene.h"
#include "cir_queue.h"
#include "ha_device_mng.h"

namespace scene_mng_ns {

const uint8_t max_num_scenes = 2;

typedef struct scenes_list_obj_s {
    bool valid;
    scene scene_obj;
} scenes_list_obj_t;

}

using namespace scene_ns;
using namespace scene_mng_ns;

class scene_mng {

public:

    /**
     * @brief   constructor.
     *
     * @param[in]   cur_device_mng_p, pointer to a device manager object.
     * @param[in]   rtc_obj_p, pointer to a rtc object.
     * @param[in]   out_pid_p, pointer to pid of thread will be sent messages to.
     * @param[in]   out_cir_queue_p, pointer to cir_queue will be pushed actions into.
     */
    scene_mng(ha_device_mng *cur_device_mng_p, rtc *rtc_obj_p,
            kernel_pid_t *out_pid_p, cir_queue *out_cir_queue_p);

    /**
     * @brief   Process default scenes and user's scene.
     *
     * @param[in]   trigger_by_rpt, true if this has been triggered by report.
     *              false if this has been triggered by time.
     * @param[in]   &out_cir_queue, cir_queue will be pushed actions into.
     */
    void process(bool trigger_by_rpt, ha_device *a_device_rpt);

    /**
     * @brief   Set user's active scene.
     *
     * @param[in]   name, scene name.
     */
    void set_user_active_scene(const char *name);

    /**
     * @brief   Get user's active scene.
     *
     * @param[in]   name, scene name.
     */
    void get_user_active_scene(char *name);

    /**
     * @brief   Restore all scenes.
     */
    void restore(void);

    /**
     * @brief   Read active scene name from file and restore user's scene.
     */
    void restore_user_active_scene(void);

    /**
     * @brief   Restore default scene.
     */
    void restore_default_scene(void);

    /**
     * @brief   Save all scenes.
     */
    void save(void);

    /**
     * @brief   Save default scene.
     */
    void save_default_scene(void);

    /**
     * @brief   Save user's scene and active scene name in active scene file.
     */
    void save_user_active_scene(void);

    /**
     * @brief   Save user's scene to a file, will not change active scene name.
     *
     * @param[in]   name, file name to be saved.
     */
    void save_user_scene_as(const char *name);

    /**
     * @brief   Set valid status of user scene.
     */
    void set_user_active_scene_valid_status(bool status);

    /**
     * @brief   Get user active scene pointer.
     *
     * @return      pointer to user active scene object, NULL if error.
     */
    scene *get_user_active_scene_ptr(void);

    /**
     * @brief   Set valid status of user scene.
     */
    void set_default_scene_valid_status(bool status);

    /**
     * @brief   Get default scene pointer.
     *
     * @return      pointer to default scene object, NULL if error.
     */
    scene *get_default_scene_ptr(void);

    /**
     * @brief   Print user active scene.
     */
    void print_user_active_scene(void);

    /**
     * @brief   Print default scene.
     */
    void print_default_scene(void);

private:

    /**
     * @brief   Get scene pointer by index.
     *
     * @param[in]   index.
     *
     * @return      pointer to scene object, NULL if error.
     */
    scene *get_scene_ptr_with_index(uint8_t index);

    /**
     * @brief   Restore scene with index.
     *
     * @param[in]   index.
     */
    void restore_scene_with_index(uint8_t index);

    /**
     * @brief   Save scene with index.
     *
     * @param[in]   index.
     */
    void save_scene_with_index(uint8_t index);

    ha_device_mng *device_mng_p;
    rtc *rtc_p;
    kernel_pid_t *out_pid_p;
    cir_queue *out_queue_p;
    scenes_list_obj_t scenes_list[max_num_scenes];
};

#endif // SCENE_MNG_H_
