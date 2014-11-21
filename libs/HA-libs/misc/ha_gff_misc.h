/**
 * @file ha_gff_misc.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 11-Nov-2014
 * @brief This contains headers for some functions to manipulate data in GFF format.
 */

#ifndef HA_GFF_MISC_H_
#define HA_GFF_MISC_H_

/**
 * @brief   Convert 2 bytes from buffer to uint16_t
 *
 * @param[in]   buffer,
 *
 * @return      uint16_t value.
 */
uint16_t buf2uint16(uint8_t* buffer);

/**
 * @brief   Convert 4 bytes from buffer to uint32_t
 *
 * @param[in]   buffer,
 *
 * @return      uint32_t value.
 */
uint32_t buf2uint32(uint8_t* buffer);

/**
 * @brief   Convert 2 bytes from uint16_t to buffer
 *
 * @param[in]   value, an unit16_t value.
 * @param[in]   buffer,
 */
void uint162buf(uint16_t value, uint8_t* buffer);

/**
 * @brief   Convert 4 bytes from uint32_t to buffer
 *
 * @param[in]   value, an unit32_t value.
 * @param[in]   buffer,
 */
void uint322buf(uint32_t value, uint8_t* buffer);

/**
 * @brief   Get zone id from device id.
 *
 * @param[in]   device_id
 *
 * @return      zone_id
 */
uint8_t parse_zone_deviceid(uint32_t device_id);

/**
 * @brief   Get node id from device id.
 *
 * @param[in]   device_id
 *
 * @return      none_id
 */
uint16_t parse_node_deviceid(uint32_t device_id);

/**
 * @brief   Get end point id from device id.
 *
 * @param[in]   device_id
 *
 * @return      endpoint_id
 */
uint8_t parse_ep_deviceid(uint32_t device_id);

/**
 * @brief   Get device type from device id.
 *
 * @param[in]   device_id
 *
 * @return      device type
 */
uint8_t parse_devtype_deviceid(uint32_t device_id);

/**
 * @brief   Get device type from common device type and sub type.
 *
 * @param[in]   dev_type_common
 * @param[in]   sub_type
 *
 * @return      device type
 */
uint8_t combine_dev_type(uint8_t dev_type_common, uint8_t sub_type);

/**
 * @brief   Convert device type to name.
 *
 * @param[in]   device_type
 *
 * @return      type name.
 */
const char* device_type_to_name(uint8_t device_type);

#endif /* HA_GFF_MISC_H_ */
