/**
 * @file     : sd_card_fs_demo.h
 * @brief    : SD Card FS API's
 * @author   :
 * @copyright: TODO
 */

#ifndef __SDFS_DEMO_H__
#define __SDFS_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include <qcli_api.h>

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
 * @function        : Initialize_SDFS_Demo
 * @brief           : Initialize_SDFS_Demo to register SDFS demo
 * @param[in]       : void
 * @return          : void
 */
void Initialize_SDFS_Demo(void);

/**
 * @function        : Test all FS System API
 * @brief           : Write data to file, read and delete
 * @param[in]       : void
 * @return          : Status
 * @retval          : QAPI_OK on success
 * @retval          : QAPI_ERROR on failure
 */
qapi_Status_t sdcard_fs_test(void);

#endif
