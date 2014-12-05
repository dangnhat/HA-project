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

#include "scene_mng.h"
#include "ff.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

static const uint8_t default_scene_index = 0;
static const uint8_t user_scene_index = 1;

static const char default_scene[] = "default";
static const char scenes_folder[] = "scenes/";
static const char active_user_scene_file[] = "actscene";

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
void set_user_active_scene(const char *name)
{
    /* TODO: */
}

/*----------------------------------------------------------------------------*/
void get_user_active_scene(char *name)
{
    /* TODO: */
}

/*----------------------------------------------------------------------------*/
void scene_mng::restore(void)
{
    restore_default_scene();
//    restore_user_active_scene(); /* TODO: change this one */
}

/*----------------------------------------------------------------------------*/
void scene_mng::restore_default_scene(void)
{
    char default_name[scene_max_name_chars];
    strcpy(default_name, scenes_folder);
    strcat(default_name, default_scene);

    /* set name for default scene */
    scenes_list[default_scene_index].scene_obj.set_name(default_name);
    if (scenes_list[default_scene_index].scene_obj.restore() != 0) {
        HA_DEBUG("scene_mng::restore_default_scene: failed to restore %s\n", default_name);
        scenes_list[default_scene_index].valid = false;
    }
    else {
        HA_DEBUG("scene_mng::restore_default_scene: %s restored\n", default_name);
        scenes_list[default_scene_index].valid = true;
    }
}

/*----------------------------------------------------------------------------*/
void scene_mng::restore_user_active_scene(void)
{
    char user_active_scene_name[scene_max_name_chars];

    /* get name of user active scene from file */
    get_user_active_scene(user_active_scene_name);

    /* set name and restore user scene */
    scenes_list[user_scene_index].scene_obj.set_name(user_active_scene_name);
    if (scenes_list[user_scene_index].scene_obj.restore() != 0) {
        HA_DEBUG("scene_mng::restore_user_active_scene: failed to restore %s\n",
                user_active_scene_name);
        scenes_list[user_scene_index].valid = false;
    }
    else {
        HA_DEBUG("scene_mng::restore_default_scene: %s restored\n", user_active_scene_name);
        scenes_list[user_scene_index].valid = true;
    }
}

/*----------------------------------------------------------------------------*/
void scene_mng::save(void)
{
    save_default_scene();
    save_user_active_scene();
}

/*----------------------------------------------------------------------------*/
void scene_mng::save_default_scene(void)
{
    scenes_list[default_scene_index].scene_obj.save();
}

/*----------------------------------------------------------------------------*/
void scene_mng::save_user_active_scene(void)
{
    char user_active_scene_name[scene_max_name_chars];

    /* save name of user active scene to active scene file */
    scenes_list[user_scene_index].scene_obj.get_name(user_active_scene_name);
    set_user_active_scene(user_active_scene_name);

    /* save user active scene */
    scenes_list[user_scene_index].scene_obj.save();
}

/*----------------------------------------------------------------------------*/
scene *scene_mng::get_user_active_scene_ptr(void)
{
    return (get_scene_ptr_with_index(user_scene_index));
}

/*----------------------------------------------------------------------------*/
void scene_mng::set_user_active_scene_valid_status(bool status)
{
    scenes_list[user_scene_index].valid = status;
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
void scene_mng::print_user_active_scene(void)
{
    if (scenes_list[user_scene_index].valid) {
        HA_NOTIFY("User active scene:\n"
                    "---\n");
        scenes_list[user_scene_index].scene_obj.print(rtc_p);
    }
    else {
        HA_NOTIFY("User scene: Invalid\n");
    }
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

/*----------------------------------------------------------------------------*/
scene *scene_mng::get_scene_ptr_with_index(uint8_t index)
{
    if (index >= max_num_scenes || !scenes_list[index].valid) {
        return NULL;
    }
    return &scenes_list[index].scene_obj;
}

