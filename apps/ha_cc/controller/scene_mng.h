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
     * @brief   Save all scenes.
     */
    void save(void);

    /**
     * @brief   Restore all scenes.
     */
    void restore(void);

    /*------------------------ Current running user's scene ------------------*/
    /**
     * @brief   Set current running user scene name.
     *
     * @param[in]   name, scene name.
     */
    void set_user_scene(const char *name);

    /**
     * @brief   Get current running user scene name.
     *
     * @param[out]  name, scene name, size of the buffer for name MUST be >=
     *              scene_ns::scene_max_name_chars_wout_folders.
     */
    void get_user_scene(char *name);

    /**
     * @brief   Set valid status of user scene.
     */
    void set_user_scene_valid_status(bool status);

    /**
     * @brief   Restore default scene.
     *          Set valid bit to false when user's scene can't be restored.
     */
    void restore_user_scene(void);

    /**
     * @brief   Get user active scene pointer.
     *
     * @return  pointer to user active scene object, NULL if error.
     */
    scene *get_user_scene_ptr(void);

    /**
     * @brief   Print user active scene.
     */
    void print_user_scene(void);

    /*------------------------ Active scene ----------------------------------*/
    /**
     * @brief   Save active scene name in active scene file.
     *
     * @param[in]   name, scene name.
     */
    void set_active_scene(const char *name);

    /**
     * @brief   Get active scene name from file.
     *
     * @param[out]  name, scene name, size of the buffer for name MUST be >=
     *              scene_ns::scene_max_name_chars_wout_folders.
     */
    void get_active_scene(char *name);

    /**
     * @brief   Get number of active scene.
     *
     * @return  always return 1.
     */
    uint8_t get_num_of_active_scenes(void) { return 1; }

    /*------------------------ Inactive scenes -------------------------------*/
    /**
     * @brief   Get number of inactive scene.
     *
     * @return  num of inactive scenes.
     */
    uint8_t get_num_of_inactive_scenes(void);

    /**
     * @brief   Get inactive scene name with index.
     *
     * @param[in]   index.
     * @paran[out]  name, size of the buffer for name MUST be >=
     *              scene_ns::scene_max_name_chars_wout_folders.
     */
    void get_inactive_scene_with_index(uint8_t index, char *name);

    /**
     * @brief   Remove an inactive scene.
     *
     * @param[in]   name,
     *
     * @return  -1 if error.
     */
    int8_t remove_inactive_scene(const char *name);

    /**
     * @brief   Rename an inactive scene.
     *
     * @param[in]   old name,
     * @param[in]   new_name,
     *
     * @return  -1 if error.
     */
    int8_t rename_inactive_scene(const char *old_name, const char *new_name);

    /*------------------------ Default scene ---------------------------------*/
    /**
     * @brief   Save default scene.
     */
    void save_default_scene(void);

    /**
     * @brief   Restore default scene.
     */
    void restore_default_scene(void);

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

    /**
     * @brief   Filter out directory names in path. (Unix style for path name: a/b/c)
     *
     * @param[in/out]   path_name, should be end with '\0'
     */
    void not_dir(char* path_name);

    ha_device_mng *device_mng_p;
    rtc *rtc_p;
    kernel_pid_t *out_pid_p;
    cir_queue *out_queue_p;
    scenes_list_obj_t scenes_list[max_num_scenes];
};

/*----------------------------- Shell command --------------------------------*/
/**
 * @brief   Shell command of scene management.
 *
 * @param[in]   &scene_mng_obj, a zone object.
 * @param[in]   &rtc_obj, a rtc object.
 * @param[in]   argc,
 * @param[in]   argv,
 */
void scene_mng_cmd(scene_mng &scene_mng_obj, rtc &rtc_obj, int argc, char **argv);

#endif // SCENE_MNG_H_
