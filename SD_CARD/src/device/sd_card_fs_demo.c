/**
 * @file     : sd_card_fs_demo.c
 * @brief    : SD Card FS demo for test SDCARD
 * @author   :
 * @copyright: TODO
 */

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include <qcli.h>
#include <qcli_api.h>

#include <qapi/qapi_status.h>
#include <sd_card_fs_demo.h>
#include <sdcard_fs_core.h>

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

/**
 * @function        : Initialize file system for sdcard
 * @brief           : Initialize file system for sdcard
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_init(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

/**
 * @function        : Write data to SDCARD
 * @brief           : Write data to sd card
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_write(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

/**
 * @function        : Read data from sd card
 * @brief           : Read fixed length data from sd card
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_read(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);


/**
 * @function        : Show the File system status
 * @brief           : Show the File system status
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_stat(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

/**
 * @function        : Test all FS system API
 * @brief           : Write data to file, read and delete
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

/**
 * @brief
 * Format the SD card using file system.
 *
 * @details
 * FS_format() This function Format sd card with FS.
 *
 * @param[in]   Parameter_Count     No. of parameter given by user.
 * @param[in]   Parameter_List      List of actual parameter valuegiven by user.
 *
 * @return      0 on success, negative value on failure.
 * @return      An error code if negative; one of the following values:
 *                  @par
 *                  QCLI_STATUS_SUCCESS_E on success. \n
 *                  QCLI_STATUS_ERROR_E on failure. \n
 */
QCLI_Command_Status_t FS_format(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

/**
 * @function        : Remove file from sdcard
 * @brief           : Remove file from sdcard
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_remove(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

/* Handle for our QCLI Command Group. */
extern QCLI_Group_Handle_t qcli_spi_group;

/* Handle for our QCLI Command Group. */
QCLI_Group_Handle_t qcli_sdfs_group;

const QCLI_Command_t sdfs_cmd_list[] =
{
    {
        .Command_Function = FS_init,
        .Start_Thread = false,
        .Command_String = "FS_init",
        .Usage_String = "Start\n\t<1: Initialize FS>",
        .Description = "Initialize file system on SD card"
    },
    {
        .Command_Function = FS_write,
        .Start_Thread = false,
        .Command_String = "FS_write",
        .Usage_String = "File name\n\t<File name>\n\t<data to write>",
        .Description = "Write file to sd card"
    },
    {
        .Command_Function = FS_read,
        .Start_Thread = false,
        .Command_String = "FS_read",
        .Usage_String = "File name\n\t<File name>\n\t<No. of bytes to read>",
        .Description = "Read available file from sd card"
    },
    {
        .Command_Function = FS_test,
        .Start_Thread = false,
        .Command_String = "FS_test",
        .Usage_String = "Start\n\t<1: Initialize FS test>",
        .Description = "write data to file and read same data"
    },
    {
        .Command_Function = FS_stat,
        .Start_Thread = false,
        .Command_String = "FS_stat",
        .Usage_String = "Start\n\t<1: Read sdcard stat>",
        .Description = "Read SDCARD status"
    },
    {
        .Command_Function = FS_remove,
        .Start_Thread = false,
        .Command_String = "FS_remove",
        .Usage_String = "Filename\n\t<File name to delete>",
        .Description = "Delete File from FS"
    },
    {
        .Command_Function = FS_format,
        .Start_Thread = false,
        .Command_String = "FS_format",
        .Usage_String = "Format SD card>",
        .Description = "Formate SD card"
    },
};

const QCLI_Command_Group_t sdfs_cmd_group =
{
    "SD_FS",
    (sizeof(sdfs_cmd_list) / sizeof(sdfs_cmd_list[0])),
    sdfs_cmd_list
};

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/

/**
 * @function        : Initialize_SDFS_Demo
 * @brief           : Initialize_SDFS_Demo to register SDFS demo
 * @param[in]       : void
 * @return          : void
 */
void Initialize_SDFS_Demo(void)
{
    /* Attempt to rgister the Command Groups with the qcli framework.*/
    qcli_sdfs_group = QCLI_Register_Command_Group(qcli_spi_group, &sdfs_cmd_group);
    if ( qcli_sdfs_group )
        QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: SD CARD File system Registered\n");

}

/**
 * @function        : Initialize file system for sdcard
 * @brief           : Initialize file system for sdcard
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_init(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if ( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid ||
            (Parameter_List[0].Integer_Value > 1) )
        return QCLI_STATUS_USAGE_E;

    if ( QAPI_OK == sdcard_fs_init() ) {
        QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: SD CARD FS_Init success\n");
        return QCLI_STATUS_SUCCESS_E;
    } else {
        QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: SD CARD FS_Init failed\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**
 * @function        : Show the File system status
 * @brief           : Show the File system status
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_stat(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if ( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid ||
            (Parameter_List[0].Integer_Value > 1) )
        return QCLI_STATUS_USAGE_E;

    if ( QAPI_OK == sdcard_fs_stat() ) {
        QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: SD CARD FS_stat read success\n");
        return QCLI_STATUS_SUCCESS_E;
    } else {
        QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: SD CARD FS_stat read failed\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**
 * @function        : Write data to SDCARD
 * @brief           : Write data to sd card
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_write(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *buffer = NULL;
    char *fs_name = NULL;
    char file_name[50] = "/sdcard/";
    char *p_fsname = file_name;
    uint32_t length;

    if ( Parameter_Count != 2 || !Parameter_List )
        return QCLI_STATUS_USAGE_E;

    fs_name = (char *) Parameter_List[0].String_Value;
    buffer = (char *) Parameter_List[1].String_Value;
    length = strlen(buffer);

    strcat(p_fsname, fs_name);
    if ( QAPI_OK == sd_fs_pwrite_bin(p_fsname, buffer, length, 0) ) {
        QCLI_Printf(SKYLO_DEBUG,qcli_sdfs_group,"SDFS: File FS_write success\n");
        return QCLI_STATUS_SUCCESS_E;
    } else {
        QCLI_Printf(SKYLO_ERR,qcli_sdfs_group,"SDFS: File FS_write failed\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**
 * @function        : Read data from sd card
 * @brief           : Read fixed length data from sd card
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_read(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *fs_name = NULL;
    char *buffer = NULL;
    char file_name[50] = "/sdcard/";
    char *p_fsname = file_name;
    uint32_t length;

    if ( Parameter_Count != 2 || !Parameter_List || !Parameter_List[1].Integer_Is_Valid )
        return QCLI_STATUS_USAGE_E;

    fs_name = (char *) Parameter_List[0].String_Value;
    length = Parameter_List[1].Integer_Value;

    if ( length == 0 || length > 1024 ) {
        QCLI_Printf(SKYLO_ERR,qcli_sdfs_group,"SDFS: zero length failed.\n");
        return QCLI_STATUS_ERROR_E;
    }

    if ( NULL == (buffer = (char *)malloc(length)) )
    {
        QCLI_Printf(SKYLO_ERR, "SDFS: Buffer Allocation Null in %s at %d\n ", __func__, __LINE__);
        return QAPI_ERROR;
    }

    strcat(p_fsname, fs_name);
    if ( QAPI_OK == sd_fs_pread_bin(p_fsname, buffer, length, 0) ) {
        QCLI_Printf(SKYLO_DEBUG,qcli_sdfs_group,"SDFS: File FS_read success\n");
        QCLI_Printf(SKYLO_DEBUG,qcli_sdfs_group,"SDFS: File FS_read data - %s\n", buffer);
        free(buffer);
        return QCLI_STATUS_SUCCESS_E;
    } else {
        QCLI_Printf(SKYLO_ERR,qcli_sdfs_group,"SDFS: File SD card read failed\n");
        free(buffer);
        return QCLI_STATUS_ERROR_E;
    }

    free(buffer);
    return QCLI_STATUS_SUCCESS_E;
}

/**
 * @function        : Remove file from sdcard
 * @brief           : Remove file from sdcard
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_remove(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *fs_name = NULL;
    char file_name[50] = "/sdcard/";
    char *p_fsname = file_name;

    if ( Parameter_Count != 1 || !Parameter_List )
        return QCLI_STATUS_USAGE_E;

    fs_name = (char *) Parameter_List[0].String_Value;

    strcat(p_fsname, fs_name);
    if ( QAPI_OK == sd_fs_delete(p_fsname) ) {
        QCLI_Printf(SKYLO_DEBUG,qcli_sdfs_group,"SDFS: File FS_remove success\n");
        return QCLI_STATUS_SUCCESS_E;
    } else {
        QCLI_Printf(SKYLO_ERR,qcli_sdfs_group,"SDFS: File FS_remove failed\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**
 * @function        : Test all FS system API
 * @brief           : Write data to file, read and delete
 * @param[in]       : Parameter Count
 * @param[in]       : Parameter List
 * @return          : Status
 * @retval          : QCLI_STATUS_SUCCESS_E on success
 * @retval          : QCLI_STATUS_ERROR_E on failure
 */
QCLI_Command_Status_t FS_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if ( Parameter_Count != 1 || !Parameter_List || !Parameter_List[0].Integer_Is_Valid ||
            (Parameter_List[0].Integer_Value > 1) )
        return QCLI_STATUS_USAGE_E;

    if ( QAPI_OK == sdcard_fs_test() ) {
        QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: SD CARD FS_Test success\n");
        return QCLI_STATUS_SUCCESS_E;
    } else {
        QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: SD CARD FS_Test failed\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**
 * @brief
 * Format the SD card using file system.
 *
 * @details
 * FS_format() This function Format sd card with FS.
 *
 * @param[in]   Parameter_Count     No. of parameter given by user.
 * @param[in]   Parameter_List      List of actual parameter valuegiven by user.
 *
 * @return      0 on success, negative value on failure.
 * @return      An error code if negative; one of the following values:
 *                  @par
 *                  QCLI_STATUS_SUCCESS_E on success. \n
 *                  QCLI_STATUS_ERROR_E on failure. \n
 */
QCLI_Command_Status_t FS_format(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if ( QAPI_OK == sdcard_fs_format() ) {
        QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: SD CARD Format success\n");
        return QCLI_STATUS_SUCCESS_E;
    } else {
        QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: SD CARD Format failed\n");
        return QCLI_STATUS_ERROR_E;
    }
    return QCLI_STATUS_SUCCESS_E;
}

/**
 * @function        : Test all FS System API
 * @brief           : Write data to file, read and delete
 * @param[in]       : void
 * @return          : Status
 * @retval          : QAPI_OK on success
 * @retval          : QAPI_ERROR on failure
 */
qapi_Status_t sdcard_fs_test(void)
{
    char write_buffer[30] = "SKYLO_SDCARD_TEST";
    char read_b[30] = {};
    char *read_buffer = read_b;
    char fs_name[30] = "/sdcard/SKYLT";
    uint32_t w_length;

    w_length = strlen(write_buffer);

    if ( QAPI_OK == sdcard_fs_init() ) {
        QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: CARD FS_Init success\n");
    } else {
        QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: CARD FS_Init failed\n");
        return QAPI_ERROR;
    }

    if ( QAPI_OK == sd_fs_pwrite_bin(fs_name, write_buffer, w_length, 0) ) {
        QCLI_Printf(SKYLO_DEBUG,qcli_sdfs_group,"SDFS: File FS_write success\n");
    } else {
        QCLI_Printf(SKYLO_ERR,qcli_sdfs_group,"SDFS: File FS_write failed\n");
        return QAPI_ERROR;
    }

    if ( QAPI_OK == sd_fs_pread_bin(fs_name, read_buffer, w_length, 0) ) {
        QCLI_Printf(SKYLO_DEBUG,qcli_sdfs_group,"SDFS: File FS_read success\n");
    } else {
        QCLI_Printf(SKYLO_ERR,qcli_sdfs_group,"SDFS: File FS_read failed\n");
        return QAPI_ERROR;
    }

    if ( QAPI_OK == sd_fs_delete(fs_name) ) {
        QCLI_Printf(SKYLO_DEBUG,qcli_sdfs_group,"SDFS: File FS_remove success\n");
    } else {
        QCLI_Printf(SKYLO_ERR,qcli_sdfs_group,"SDFS: File FS_remove failed\n");
        return QAPI_ERROR;
    }

    // if ( QAPI_OK == sdcard_fs_format() ) {
    //     QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: SD CARD FS_format success\n");
    // } else {
    //     QCLI_Printf(SKYLO_INFO,qcli_sdfs_group, "SDFS: SD CARD FS_format failed\n");
    //     return QAPI_ERROR;
    // }

    if ( strncmp(write_buffer,read_buffer,w_length) == 0 ) {
        QCLI_Printf(SKYLO_DEBUG,qcli_sdfs_group,"SDFS: File Both write read data are same\n");
    } else {
        QCLI_Printf(SKYLO_DEBUG,qcli_sdfs_group,"SDFS: File Both write read data are not same\n");
        return QAPI_ERROR;
    }

    return QAPI_OK;
}
