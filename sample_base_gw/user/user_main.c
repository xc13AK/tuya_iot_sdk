/***********************************************************
*  File: linux_base_nw.c
*  Author: nzy
*  Date: 20171213
***********************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "tuya_cloud_types.h"
#include "tuya_iot_base_api.h"
#include "uni_log.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
// 涂鸦云上的产品KEY，不同产品KEY不相同，需登陆tuya.com创建产品分配唯一key
#define PRODUCT_KEY "GXxoKf27eVjA7x1c" // 此key可用于测试

/***********************************************************
*************************variable define********************
***********************************************************/

/***********************************************************
*************************function define********************
***********************************************************/
OPERATE_RET get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset,
                                 IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data);
VOID upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data);
OPERATE_RET dev_get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset,
                                           IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data);
VOID dev_upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data);


// 网关OTA升级通知
// fw->tp <= DEV_NM_NOT_ATH_SNGL:指示网关本身的应用程序更新
// fw->tp > DEV_NM_NOT_ATH_SNGL:指示网关上特定的适配器模块的固件升级
VOID gw_ug_inform_cb(IN CONST FW_UG_S *fw)
{
    PR_DEBUG("Rev GW Upgrade Info\r\n");
    PR_DEBUG("fw->fw_url:%s\r\n", fw->fw_url);
    PR_DEBUG("fw->fw_md5:%s\r\n", fw->fw_md5);
    PR_DEBUG("fw->sw_ver:%s\r\n", fw->sw_ver);
    PR_DEBUG("fw->file_size:%d\r\n", fw->file_size);

    // 启动网关升级流程
    OPERATE_RET op_ret = OPRT_OK;
    op_ret = tuya_iot_upgrade_gw(fw, get_file_data_cb, upgrade_notify_cb, NULL);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_upgrade_gw err:%d",op_ret);
        return;
    }
}

// 网关升级流程固件获取回调
OPERATE_RET get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset,
                                     IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    PR_DEBUG("Rev File Data\r\n");
    PR_DEBUG("Total_len:%d ", total_len);
    PR_DEBUG("Offset:%d Len:%d\r\n", offset, len);

    return OPRT_OK;
}

// 网关升级流程结果通知
VOID upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
   PR_DEBUG("gw upgrade end..\n"); 
}

// 子设备OTA升级通知
// dev_id->终端设备id
// fw->tp > DEV_NM_NOT_ATH_SNGL:指示特定类型终端设备设备
VOID dev_ug_inform_cb(IN CONST CHAR_T *dev_id,IN CONST FW_UG_S *fw)
{
    OPERATE_RET op_ret = OPRT_OK;
    op_ret = tuya_iot_upgrade_dev(dev_id,fw,dev_get_file_data_cb,dev_upgrade_notify_cb,NULL);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_upgrade_dev err:%d",op_ret);
        return;
    }
}

// 终端设备升级流程固件获取回调
OPERATE_RET dev_get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset,
                                           IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    PR_DEBUG("Rev File Data\r\n");
    PR_DEBUG("Total_len:%d ", total_len);
    PR_DEBUG("Offset:%d Len:%d\r\n", offset, len);

    return OPRT_OK;
}

// 终端设备网关升级流程结果通知
VOID dev_upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
   PR_DEBUG("upgrade fw end\n"); 
}


// 网关状态通知回调
VOID status_changed_cb(IN CONST GW_STATUS_E status)
{
    PR_DEBUG("gw status changed to %d \r\n", status);
}

// 设备特定数据查询入口
VOID dev_dp_query_cb(IN CONST TY_DP_QUERY_S *dp_qry)
{
    PR_DEBUG("Rev DP Query Cmd\r\n");
}

// 设备格式化指令数据下发入口
VOID dev_obj_dp_cb(IN CONST TY_RECV_OBJ_DP_S *dp)
{
    OPERATE_RET op_ret = OPRT_OK;
    int i;

    PR_DEBUG("dev_obj_dp_cb:cid=%s, cmd_tp=%d, cnt=%d\r\n", dp->cid, dp->cmd_tp, dp->dps_cnt);
    for (i=0; i<dp->dps_cnt; i++)
    {
        PR_DEBUG("[%i]dpid=%d, type=%d, value=%d, time_stamp=%lu\r\n", i, dp->dps[i].dpid, dp->dps[i].type, dp->dps[i].value, dp->dps[i].time_stamp);
    }  
    
    // 格式化类数据上报demo，收什么回什么，需根据实际应用情况处理
    op_ret = dev_report_dp_json_async(dp->cid,dp->dps,dp->dps_cnt);
    if(OPRT_OK != op_ret) {
        PR_ERR("dev_report_dp_json_async op_ret:%d",op_ret);
        return;
    }
}

// 设备透传类数据下发入口
VOID dev_raw_dp_cb(IN CONST TY_RECV_RAW_DP_S *dp)
{
    OPERATE_RET op_ret = OPRT_OK;

    PR_DEBUG("dev_raw_dp_cb:dpid=%d, len=%d\r\n", dp->dpid, dp->len);
    
    // 透传类数据上报demo，收什么回什么，需根据实际应用情况处理
    op_ret = dev_report_dp_raw_sync(dp->cid,dp->dpid,dp->data,dp->len,0);
    if(OPRT_OK != op_ret) {
        PR_ERR("dev_report_dp_json_async op_ret:%d",op_ret);
        return;
    }
}

// 网关网络状态回调
STATIC VOID __get_nw_status(IN CONST GW_BASE_NW_STAT_T stat)
{
    PR_DEBUG("network status is :%d \r\n", stat);

}

// 网关复位回调通知，设备应用程序需要重启
VOID gw_rst_inform_cb(VOID)
{
    PR_DEBUG("gw reset!!!! \r\n");

    // to do add application reset code
    CHAR_T buf[256];
    sprintf(buf,"kill %d",getpid());
    PR_DEBUG("buf:%s",buf);
    system(buf);
}

// 使能设备入网通知回调
BOOL_T gw_add_dev_cb(IN CONST GW_PERMIT_DEV_TP_T tp,IN CONST BOOL_T permit,IN CONST UINT_T timeout)
{
    PR_DEBUG("tp:%d timeout:%d",tp,timeout);

    return TRUE;
}

// 网关设备删除回调通知
VOID gw_del_cb(IN CONST CHAR_T *dev_id)
{
    PR_DEBUG("gw_del_cb id:%s",dev_id);
}

// 子设备涂鸦云绑定通知
// op_ret==OPRT_OK 成功 其他:失败
VOID gw_bind_ifm_cb(IN CONST CHAR_T *dev_id,IN CONST OPERATE_RET op_ret)
{
    PR_DEBUG("gw bind device op_ret:%d",op_ret);

}

/***********************************************************
*  Function: main
*  Input: argc argv
*  Output: none
*  Return: int
***********************************************************/
int main(int argc,char **argv)
{
    // tuya iot system init
    OPERATE_RET op_ret = OPRT_OK;
    
    system("mkdir /etc/jffs2/iot/");
    op_ret = tuya_iot_init("/etc/jffs2/iot/"); // 路径必须对应一个可读写文件系统分区
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_init err:%d \r\n",op_ret);
        return -1;
    }

    // 设备的uuid ahth_key,用于涂鸦云设备的安全认证，此处为默认调试用，正式产品需向涂鸦申请，每个设备所用key均为唯一
    GW_PROD_INFO_S prod_info = {"003tuyatestf7f149185","NeA8Wc7srpAZHEMuru867oblOLN2QCC1"};
    op_ret = tuya_iot_set_gw_prod_info(&prod_info);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_set_gw_prod_info err:%d \r\n",op_ret);
        return -2;
    }

    // 涂鸦网关提供的设备状态/指令下发回调接口
    TY_IOT_CBS_S iot_cbs = {
        status_changed_cb,\
        gw_ug_inform_cb,\
        gw_rst_inform_cb,\
        dev_obj_dp_cb,\
        dev_raw_dp_cb,\
        dev_dp_query_cb,\
        NULL,
    };

    // 涂鸦网关提供的设备添加使能、删除回调接口
    TY_IOT_GW_CBS_S iot_gw_cbs =  {
        gw_add_dev_cb,
        gw_del_cb,
        NULL,
        NULL,
        gw_bind_ifm_cb,
    };

    // 涂鸦网关上对应的无线管理适配器配置，可支持多种不同的无线适配器，比如zigbee、433共存的情况
    // 版本信息对应着网关上适配器的固件版本，用以维护适配器固件升级用
    GW_ATTACH_ATTR_T attr[] = {
        {GP_DEV_ZB,"1.0.0"},
    };

    // 涂鸦iot网关初始化
    op_ret = tuya_iot_gw_init(&iot_cbs,&iot_gw_cbs,PRODUCT_KEY,USER_SW_VER,attr,CNTSOF(attr));
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_gw_init err:%d \r\n",op_ret);
        return -3;
    }

    // 涂鸦iot网络状态管理回调注册
    op_ret = tuya_iot_reg_get_nw_stat_cb(__get_nw_status);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_reg_get_nw_stat_cb err:%d \r\n",op_ret);
        return -4;
    }

    // 注册子设备
    op_ret = tuya_iot_gw_bind_dev(GP_DEV_ZB, 0, "123456", "3M29mEb0i3bqMnPi", "1.0.0");
    //IN CONST USER_DEV_DTL_DEF_T uddd, IN CONST CHAR_T *id,IN CONST CHAR_T *pk,IN CONST CHAR_T *ver);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_gw_bind_dev err1:%d \r\n",op_ret);
        return -4;
    }    
    op_ret = tuya_iot_gw_bind_dev(GP_DEV_ZB, 0, "123457", "z6kRdPw4tsQonlEV", "1.0.0");
    //IN CONST USER_DEV_DTL_DEF_T uddd, IN CONST CHAR_T *id,IN CONST CHAR_T *pk,IN CONST CHAR_T *ver);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_gw_bind_dev err1:%d \r\n",op_ret);
        return -4;
    }       
   
   	//wait here	
    while (1) {
        sleep(5);
    }

    return 0;
}
