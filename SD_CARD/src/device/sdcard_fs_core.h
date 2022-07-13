/**
 * @file     : sdcard_fs_core.h
 * @brief    : header for SDCARD FS API's
 * @author   : Shreel Chhatbar (shreel.chhatbar@vvdntech.in)
 * @copyright: TODO
 */

#ifndef __SDFS_CORE_H__
#define __SDFS_CORE_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include "ff_headers.h"
#include "ff_stdio.h"
#include "ff_sdhcdisk.h"
#include <qapi/qapi.h>
#include <qapi/qapi_status.h>

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/

#define mainRAM_DISK_NAME                       "/sdcard"

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

/**
 * @function  : To initialize the sd card
 * @brief     : To initialize the sd card
 * @param[in] : void
 * @return    : status
 * @retval    : QAPI OK, for success
 * @retval    : QAPI ERROR, for failure
 */
qapi_Status_t sdcard_fs_init(void);

/**
 * @function  : Function to position write to a binary file
 * @brief     : Function to position write to a binary file
 * @param[in] : file index number
 * @param[in] : buffer to write into the file
 * @param[in] : length of the data that needs to be written into the file
 * @param[in] : position position at which data is to be written
 * @return    : 0 on succces and negative values on error.
 * @retval    : QAPI OK, for success
 * @retval    : QAPI ERROR, for failure
 */
qapi_Status_t sd_fs_pwrite_bin(char * fs_name,void *buffer,uint32_t length,uint32_t position);

/**
 * @function   : Function to position read the binary file
 * @brief      : Function to position read the binary file
 * @param[in]  : file index number
 * @param[out] : buffer to store the data that has been read
 * @param[in]  : length of the data that needs to be read into the file
 * @param[in]  : position from which the data needs to read
 * @return     : 0 on succces and negative values on error.
 * @retval     : QAPI OK, for success
 * @retval     : QAPI ERROR, for failure
 */
qapi_Status_t sd_fs_pread_bin(char * fs_name,void *buffer,uint32_t length,uint32_t position);

/**
 * @function  : to get the status of the sd card
 * @brief     : to get the status of the sd card
 * @param[in] : void
 * @return    : 0 on succces and negative values on error.
 * @retval    : QAPI OK, for success
 * @retval    : QAPI ERROR, for failure
 */
qapi_Status_t sdcard_fs_stat(void);

/**
 * @function   Function to get the size
 * @brief      Function to get the size
 * @param[out] Total size in MB
 * @param[out] Used size in MB
 * @return     Status
 * @retval     QAPI_OK on success
 * @retval     QAPI_ERROR on failure
 */
qapi_Status_t sdcard_fs_get_size(uint32_t *sTotal,uint32_t *sUsed);

/**
 * @function  : Function to format the SD CARD
 * @brief     : Function to format the SD CARD
 * @param[in] : void
 * @return    : Status
 * @retval    : QAPI_OK on success
 * @retval    : QAPI_ERROR on failure
 */
qapi_Status_t sdcard_fs_format(void);


/**
 * @function        : Function to Remove file from SD card.
 * @brief           : Remove (delete, or unlink) a file from the embedded FAT file system.
 *                    A file cannot be removed if it is open
 * @param[in]       : file index number
 * @return          : 0 on succces and NULL on error.
 * @retval          : QAPI OK, for success
 * @retval          : QAPI ERROR, for failure
 */
qapi_Status_t sd_fs_delete(char * fs_name);

/**
 * @brief
 * Function to find length of file
 *
 * @details
 * sd_fs_file_len() This function used to find length of the file stored in
 * SD card.
 *
 * @param[in]    Fs_name          Name of the file
 * @param[out]   File_length      Output length of file
 *
 * @return  0 on success, negative value on failure.
 *
 * @return  An error code if negative; one of the following values:
 *          @par
 *          QAPI_OK on success. \n
 *          QAPI_ERROR on failure.
 */
qapi_Status_t sd_fs_file_len(char * fs_name,uint32_t *file_length);

/**
 * @brief
 * Function to calculate crc of data given.
 *
 * @details
 * check_crc() This function used to calculate crc-32 of the data given
 *
 * @param[in]    Crc_buffer          Data buffer
 * @param[in]    File_length         Length of the data
 * @param[out]   Crc                 Output calculated CRC
 *
 * @return  0 on success
 *
 * @return  An error code if negative; one of the following values:
 *          @par
 *          QAPI_OK on success. \n
 */
qapi_Status_t check_crc(uint8_t *crc_buffer, uint32_t file_len, uint32_t *crc);

#endif
