/**
 * @file     : sdcard_fs_core.c
 * @brief    : SD Card FS PI's
 * @author   : Shreel Chhatbar (shreel.chhatbar@vvdntech.in)
 * @copyright: TODO
 */

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include <sdcard_fs_core.h>
#include <stdio.h>
#include <stdarg.h>

#include <qapi_types.h>
#include <qapi_delay.h>

#include <qcli.h>
#include <qcli_api.h>

#ifdef CONFIG_PROFILE
#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>
#endif

#include <qurt_error.h>
#include <qurt_thread.h>
#include <qurt_mutex.h>
#include <qurt_signal.h>

#include <qapi_heap_status.h>
#include <qapi/qapi.h>
#include <qapi/qapi_status.h>
#include <qapi/qapi_uart.h>
#include <qapi/qapi_reset.h>
#include <qapi/qapi_tlmm.h>

#include "ff_headers.h"
#include "ff_stdio.h"
#include "ff_sdhcdisk.h"
#include "sd_card_core.h"
#include <skylo_log.h>

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/

//#define Skylo_Printf(LOG_LEVEL,...) QCLI_Printf(LOG_LEVEL,(NULL), __VA_ARGS__)

/* The number of bytes read/written to the example files at a time. */
#define fsRAM_BUFFER_SIZE 				(200)

/* The number of bytes written to the file that uses f_putc() and f_getc(). */
#define fsPUTC_FILE_SIZE				(100)

#define mainRAM_DISK_SECTOR_SIZE        (512UL)
#define mainRAM_DISK_SECTORS            ( ( 1UL * 1024UL ) / mainRAM_DISK_SECTOR_SIZE )
#define mainIO_MANAGER_CACHE_SIZE       ( 15UL * mainRAM_DISK_SECTOR_SIZE )

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

extern sdcard_conf_t sdcard_conf;
FF_Disk_t *g_pxDisk_ptr = NULL;


/* CCITT32 */
uint32_t crctab[] = { 0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc,
    0x17c56b6b, 0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f,
    0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a,
    0x384fbdbd, 0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
    0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75, 0x6a1936c8,
    0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
    0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e,
    0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
    0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84,
    0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d, 0xd4326d90, 0xd0f37027,
    0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022,
    0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077,
    0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c,
    0x2e003dc5, 0x2ac12072, 0x128e9dcf, 0x164f8078, 0x1b0ca6a1,
    0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
    0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb,
    0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
    0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d,
    0x40d816ba, 0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
    0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692, 0x8aad2b2f,
    0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044,
    0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050, 0xe9362689,
    0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683,
    0xd1799b34, 0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59,
    0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c,
    0x774bb0eb, 0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
    0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53, 0x251d3b9e,
    0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
    0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48,
    0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
    0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2,
    0xe6ea3d65, 0xeba91bbc, 0xef68060b, 0xd727bbb6, 0xd3e6a601,
    0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604,
    0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6,
    0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad,
    0x81b02d74, 0x857130c3, 0x5d8a9099, 0x594b8d2e, 0x5408abf7,
    0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
    0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd,
    0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
    0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b,
    0x0fdc1bec, 0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
    0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 0xc5a92679,
    0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12,
    0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676, 0xea23f0af,
    0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5,
    0x9e7d9662, 0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06,
    0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03,
    0xb1f740b4 };

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/

/**
 * @function  : To initialize sdcard from fat file system.
 * @brief     : To initialize the sd card.
 * @param[in] : void
 * @return    : 0 on succces and negative values on error.
 * @retval    : QAPI OK, for success
 * @retval    : QAPI ERROR, for failure
 */
qapi_Status_t sdcard_fs_init(void)
{

    /* Create the RAM disk. */
    /* g_pxDisk_ptr = FF_RAMDiskInit( mainRAM_DISK_NAME, ucRAMDisk, mainRAM_DISK_SECTORS, mainIO_MANAGER_CACHE_SIZE
     * ); */
    if ( g_pxDisk_ptr == NULL ) {
        g_pxDisk_ptr = FF_SDDiskInit(mainRAM_DISK_NAME);

        if ( g_pxDisk_ptr != NULL ) {
            Skylo_Printf(SKYLO_DEBUG,"[SDFS] SDCARD FS Init Success\n");
            sdcard_conf.total_sectors = g_pxDisk_ptr->pxIOManager->xPartition.ulTotalSectors;
            return QAPI_OK;
        } else {
            Skylo_Printf(SKYLO_DEBUG,"[SDFS] SDCARD FS Init Fails\n");
            return QAPI_ERROR;
        }
    } else {
        Skylo_Printf(SKYLO_DEBUG, "[SDFS] SDCARD FS INIT ALREADY INITIALIZED\n");
        return QAPI_OK;
    }
}

/**
 * @function  : To show sd card partition states.
 * @brief     : To show sd card partition states.
 * @param[in] : void
 * @return    : 0 on succces and negative values on error.
 * @retval    : QAPI OK, for success
 * @retval    : QAPI ERROR, for failure
 */
qapi_Status_t sdcard_fs_stat(void)
{
    return(FF_SDDiskShowPartition(g_pxDisk_ptr));
}

/**
 * @function   Function to get the size
 * @brief      Function to get the size
 * @param[out] Total size in MB
 * @param[out] Used size in MB
 * @return     Status
 * @retval     QAPI_OK on success
 * @retval     QAPI_ERROR on failure
 */
qapi_Status_t sdcard_fs_get_size(uint32_t *sTotal,uint32_t *sUsed)
{
    if( pdFAIL == FF_SDDiskShowPartition(g_pxDisk_ptr ))
        return QAPI_ERROR;

    *sTotal = g_pxDisk_ptr->ulTotalSizeMB;
    *sUsed = g_pxDisk_ptr->ulUsedSizeMB;

    return QAPI_OK;
}

/**
 * @function  : Function to position write to a binary file.
 * @brief     : Function to position write to a binary file.
 * @param[in] : file index number
 * @param[in] : buffer to write into the file
 * @param[in] : length of the data that needs to be written into the file
 * @param[in] : position position at which data is to be written
 * @return    : 0 on succces and negative values on error.
 * @retval    : QAPI OK, for success
 * @retval    : QAPI ERROR, for failure
 */
qapi_Status_t sd_fs_pwrite_bin(char * fs_name,void *buffer,uint32_t length,uint32_t position)
{
    //Skylo_Printf(SKYLO_DEBUG,"File name : %s\tLength : %d\tPosition : %d\n",fs_name,length,position);

    qapi_Status_t status = QAPI_ERROR;
    FF_FILE *l_fd_ptr;
    uint32_t write_length = 0;

    if ( length == 0 ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] zero length.\n");
        return status;
    }

    /* Open a file using a predefined path for the fs provided */
    l_fd_ptr = ff_fopen(fs_name,"w");
    if ( l_fd_ptr == NULL ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] Open Fails %d\n", status);
        return status;
    }

    /* Go To The Position Specified In Argument */
    status = ff_fseek(l_fd_ptr,position,FF_SEEK_SET);
    if ( status != QAPI_OK ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] seek Fails %d.\n",status);
        return status;
    }

    write_length = ff_fwrite((uint8_t *)buffer,1,length,l_fd_ptr);
    if ( write_length != length ) {
        /* status = ff_errno(); */
        Skylo_Printf(SKYLO_ERR,"[SDFS] write Fails %d\n", status);
        return status;
    }

    status = ff_fclose(l_fd_ptr);
    if ( status != QAPI_OK ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] File Close Fails %d\n", status);
        return status;
    }

    return QAPI_OK;
}

/**
 * @function        : Function to position read the binary file.
 * @brief           : Function to position read the binary file.
 * @param[in]       : file index number
 * @param[out]      : buffer to store the data that has been read
 * @param[in]       : length of the data that needs to be read into the file
 * @param[in]       : position from which the data needs to read
 * @return          : 0 on succces and negative values on error.
 * @retval          : QAPI OK, for success
 * @retval          : QAPI ERROR, for failure
 */
qapi_Status_t sd_fs_pread_bin(char * fs_name,void *buffer,uint32_t length,uint32_t position)
{
    //Skylo_Printf(SKYLO_DEBUG,"File name : %s\tLength : %d\tPosition : %d\n",fs_name,length,position);

    FF_FILE *l_fd_ptr;
    qapi_Status_t status = QAPI_ERROR;
    uint32_t read_length = 0;

    if ( length == 0 ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] zero length.\n");
        return status;
    }

    /* Open a file using a predefined path for the fs provided */
    l_fd_ptr = ff_fopen(fs_name,"r");
    if ( l_fd_ptr == NULL ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] Open Fails %d\n", status);
        return status;
    }

    /* Go To The Position Specified In Argument */
    status = ff_fseek(l_fd_ptr, position, FF_SEEK_SET);
    if ( status != QAPI_OK ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] seek Fails %d.\n",status);
        return status;
    }

    read_length = ff_fread((uint8_t *)buffer,1,length,l_fd_ptr);

    status = ff_fclose(l_fd_ptr);
    if ( status != QAPI_OK ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] File Close Fails %d\n", status);
        return status;
    }

    if ( read_length != length ) {
        /* status = ff_errno(); */
        Skylo_Printf(SKYLO_ERR,"[SDFS] Read Fails %d\n", status);
        return status;
    }

    return QAPI_OK;
}

/**
 * @function  : Function to format the SD CARD
 * @brief     : Function to format the SD CARD
 * @param[in] : void
 * @return    : Status
 * @retval    : QAPI_OK on success
 * @retval    : QAPI_ERROR on failure
 */
qapi_Status_t sdcard_fs_format(void)
{
    if ( pdPASS != FF_SDDiskFormat( g_pxDisk_ptr, g_pxDisk_ptr->xStatus.bPartitionNumber ) ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] Unable to format the SD Card\n");
        g_pxDisk_ptr = NULL;
        return QAPI_ERROR;
    } else {
        FF_FS_Add( mainRAM_DISK_NAME,g_pxDisk_ptr  );
        // FF_PRINTF( "FF_SDDiskInit: Mounted SD-card as root \"%s\"\n", pcName );
        FF_SDDiskShowPartition( g_pxDisk_ptr );
    }

    return QAPI_OK;
}

/**
 * @function        : Function to Remove file from SD card.
 * @brief           : Remove (delete, or unlink) a file from file sytstem.
 * @param[in]       : file index number
 * @return          : 0 on succces and NULL on error.
 * @retval          : QAPI OK, for success
 * @retval          : QAPI ERROR, for failure
 */
qapi_Status_t sd_fs_delete(char * fs_name)
{
    /* Delete a file. */
    if ( QAPI_OK == ff_remove(fs_name) ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] File delete success\n");
        return QAPI_OK;
    } else {
        Skylo_Printf(SKYLO_ERR,"[SDFS] File delete Fails\n");
        return QAPI_ERROR;
    }
}

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
qapi_Status_t sd_fs_file_len(char * fs_name,uint32_t *file_length)
{
    //Skylo_Printf(SKYLO_DEBUG,"File name : %s\tLength : %d\tPosition : %d\n",fs_name,length,position);

    FF_FILE *l_fd_ptr;
    qapi_Status_t status = QAPI_ERROR;
    // uint32_t read_length = 0;

    /* Open a file using a predefined path for the fs provided */
    l_fd_ptr = ff_fopen(fs_name,"r");
    if ( l_fd_ptr == NULL ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] Open Fails %d\n", status);
        return status;
    }

    *file_length = ff_filelength(l_fd_ptr);

    status = ff_fclose(l_fd_ptr);
    if ( status != QAPI_OK ) {
        Skylo_Printf(SKYLO_ERR,"[SDFS] File Close Fails %d\n", status);
        return status;
    }

    return QAPI_OK;
}

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
qapi_Status_t check_crc(uint8_t *crc_buffer, uint32_t file_len, uint32_t *crc)
{
    uint8_t tabindex = 0;
    uint8_t count = 0;

    *crc = 0;

    for ( int index=0; index < file_len; index++ ) {
        count = crc_buffer[index];
        tabindex = (*crc>>24)^count;
        *crc = ((*crc << 8)&0xffffffff) ^ crctab[tabindex];
    }

    while ( file_len ) {
        count = ((file_len) & 0xFF);
        file_len = file_len >> 8;
        *crc = ((*crc << 8)&0xffffffff) ^ crctab[(*crc >> 24) ^ count];
    }

    *crc = ((~(*crc))&0xffffffff);

    return QAPI_OK;
}


