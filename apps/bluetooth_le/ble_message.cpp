/*
 * ble_message.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: AnhTrinh
 */

#include "MB1_System.h"
#include <stdio.h>
#include "ble_message.h"
#include "ble_transaction.h"
#include "device_info.h"

using namespace ble_cmd_id_ns;
using namespace ble_message_ns;

void parse_ble_msg(uint8_t* dataBuf, uint8_t len)
{
    const uint8_t max_buf_size = 5;
    bleMsg bt_msg;
    devInfo devList[max_buf_size];
    int16_t devVal;

    switch (dataBuf[2]) {
    case NUM_OF_DEVS:
        // send number of devices to mobile
        bt_msg.len = 4;
        bt_msg.cmdIDh = BLE_MESSAGE_SET;
        bt_msg.cmdIDl = NUM_OF_DEVS;
        intToUint8(max_buf_size, bt_msg.data);
        send_ble_msg(bt_msg);
        break;

    case DEV_WITH_INDEX:
        // send device info to mobile
        init_ble_database(devList, max_buf_size);
        bt_msg.len = 10 * (sizeof(devList) / sizeof(*devList));
        bt_msg.cmdIDh = BLE_MESSAGE_SET;
        bt_msg.cmdIDl = DEV_WITH_INDEX;
//        intToUint8(0x12345678, bt_msg.data);
        getMsgData(devList, bt_msg.data, bt_msg.len);
        send_ble_msg(bt_msg);
        break;
    case DEV_VAL:
//        devID = buildDevID(dataBuf[3], dataBuf[4],dataBuf[5],dataBuf[6]);
//        for(uint8_t i = 0; i < sizeof(devList) / sizeof(*devList); i++){
//            if(devList[i].devID == devID){
//
//            }
//        }
        devVal = convertArr8bitTo16bit(dataBuf[7], dataBuf[8]);
        switch (dataBuf[6]) {
            case dev_id_ns::ON_OFF_BULB:
                if(devVal != 0){
                    MB1_Led_green.on();
                 }else{
                     MB1_Led_green.off();
                 }
            break;
            case dev_id_ns::EVENT_SENSOR:
            break;

            default:
            break;
        }
        break;

    case NUM_OF_SCENES:

        break;
    case ACT_SCENE_WITH_INDEX:

        break;
    case INACT_SCENE_WITH_INDEX:

        break;
    case NUM_OF_RULES:

        break;
    case RULE_WITH_INDEX:

        break;
    case ZONE_NAME:

        break;
    default:
        break;
    }
}

void init_ble_database(devInfo* devList, uint8_t len)
{

    for (uint8_t i = 0; i < len; i++) {
//		printf("devIdx = %d\n", i);
        devList[i].devIdx = i;
    }

    devList[0].devID = buildDevID(0, 0, 0, dev_id_ns::BUTTON);
    devList[0].devVal = 0;

    devList[1].devID = buildDevID(1, 0, 0, dev_id_ns::ON_OFF_BULB);
    devList[1].devVal = 1;

    devList[2].devID = buildDevID(0, 0, 0, dev_id_ns::EVENT_SENSOR);
    devList[2].devVal = 1;

    devList[3].devID = buildDevID(1, 0, 0, dev_id_ns::DIMMER);
    devList[3].devVal = 45;

    devList[4].devID = buildDevID(2, 0, 0, dev_id_ns::RGB_LED);
    devList[4].devVal = 0xfa12;
}

void send_ble_msg(bleMsg msg)
{
    uint8_t sendDataBuf[msg.len + 3];
    bleMsgToArray(msg, sendDataBuf, sizeof(sendDataBuf));
    for (uint8_t i = 0; i < sizeof(sendDataBuf); i++) {
        printf("%x\n", sendDataBuf[i]);
    }
//    printf("%d\n", sizeof(sendDataBuf));
    ble_cmd_attributes_write(ATT_WRITE_ADDR, 0x00, sizeof(sendDataBuf),
            sendDataBuf);
}


void init_ble_msg(bleMsg msg)
{
    uint8_t sendDataBuf[msg.len + 3];
    bleMsgToArray(msg, sendDataBuf, sizeof(sendDataBuf));
    for (uint8_t i = 0; i < sizeof(sendDataBuf); i++) {
        printf("%x\n", sendDataBuf[i]);
    };
}

bleMsg arrToBTMsg(uint8_t dataBuf[])
{
    bleMsg msg;
    msg.len = dataBuf[0];
    msg.cmdIDh = dataBuf[1];
    msg.cmdIDl = dataBuf[2];
    for (uint8_t i = 0; i < msg.len; i++) {
        msg.data[i] = dataBuf[i + 3];
    }
    return msg;
}

void bleMsgToArray(bleMsg BTMsg, uint8_t* buffer, uint8_t len)
{
    buffer[0] = BTMsg.len;
    buffer[1] = BTMsg.cmdIDh;
    buffer[2] = BTMsg.cmdIDl;
    for (uint8_t i = 3; i < len; i++) {
        buffer[i] = BTMsg.data[i - 3];
    }
}

void getMsgData(devInfo devList[], uint8_t* buffer, uint8_t bufLen)
{
    uint8_t devIdx[4];
    uint8_t devID[4];
    uint8_t devVal[2];

    for (uint8_t i = 0; i < bufLen; i++) {
        switch (i % 10) {
        // Get devIdx;
        case 0:
        case 1:
        case 2:
        case 3:
            intToUint8(devList[i / 10].devIdx, devIdx);
            buffer[i] = devIdx[i % 10];
            break;
            // Get devId
        case 4:
        case 5:
        case 6:
        case 7:
            intToUint8(devList[i / 10].devID, devID);
            buffer[i] = devID[i % 10 - 4];
            break;
            // Get val
        case 8:
        case 9:
            convert16bitTo8bit(devList[i / 10].devVal, devVal);
            buffer[i] = devVal[i % 10 - 8];
            break;

        default:
            printf("invalid \n");
            break;
        }
    }
}

void intToUint8(int inNum, uint8_t* buffer)
{
    buffer[0] = (uint8_t) (inNum >> 24);
    buffer[1] = (uint8_t) (inNum >> 16);
    buffer[2] = (uint8_t) (inNum >> 8);
    buffer[3] = (uint8_t) (inNum);
}

void convert16bitTo8bit(int16_t inNum, uint8_t* buffer)
{
    buffer[0] = (uint8_t) (inNum >> 8);
    buffer[1] = (uint8_t) inNum;
}

int16_t convertArr8bitTo16bit(uint8_t high, uint8_t low)
{
    int16_t retVal;
    retVal = ((((int16_t) high) << 8)) | ((int16_t) low);
    return retVal;
}

uint32_t buildDevID(uint8_t zoneID, uint8_t nodeID, uint8_t enpID,
        uint8_t devID)
{
    uint32_t devId = (((uint32_t) zoneID) << 24) | (((uint32_t) nodeID) << 16)
            | (((uint32_t) enpID) << 8) | (((uint32_t) devID));
    return devId;
}