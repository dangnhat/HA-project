/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        zone.h
 * @brief       zone class.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#ifndef ZONE_H_
#define ZONE_H_

#include <stdint.h>

namespace zone_ns {

#define ZONES_FOLDER    "zones"

const uint8_t zone_file_max_path_size = 16;
const uint8_t zone_name_max_size = 16;
};

class zone {
public:
    /**
     * @brief   Constructor
     */
    zone(void);

    /**
     * @brief   Set zone name to file.
     *
     * @param[in]   zone_id.
     * @param[in]   zone_name.
     *
     * @return      0 on success, -1 if fail.
     */
    int8_t set_zone_name(uint8_t zone_id, const char *zone_name);

    /**
     * @brief   Get zone name to file.
     *
     * @param[in]   zone_id.
     * @param[in]   buf_size, size of the buffer for zone_name.
     * @param[out]  zone_name.
     *
     * @return      0 on success, -1 if fail.
     */
    int8_t get_zone_name(uint8_t zone_id, uint8_t buf_size, char *zone_name);
private:
};

/*----------------------------- Shell command --------------------------------*/
/**
 * @brief   Shell command to zone.
 *
 * @param[in]   &zone_obj, a zone object.
 * @param[in]   argc,
 * @param[in]   argv,
 */
void zone_cmd(zone &zone_obj, int argc, char **argv);

#endif // ZONE_H_
