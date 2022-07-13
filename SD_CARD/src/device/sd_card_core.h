/**
 * @file     : sd_card_core.h
 * @brief    : header for SDCARD API's
 * @author   : Shreel Chhatbar (shreel.chhatbar@vvdntech.in)
 * @copyright: TODO
 */

#ifndef __SD_CORE_H__
#define __SD_CORE_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include <qurt_signal.h>
#include <qapi_tlmm.h>
#include <qurt_mutex.h>
#include <qurt_error.h>
#include <spi_bb_core.h>

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/

#define TAKE_SDCARD_LOCK(__lock__)         ((qurt_mutex_lock_timed(&(__lock__), QURT_TIME_WAIT_FOREVER)) == QURT_EOK)
#define RELEASE_SDCARD_LOCK(__lock__)      do { qurt_mutex_unlock(&(__lock__)); } while(0)

/************************************
 *      RESPONSE LENGTH             *
 ***********************************/

#define R1                          (1)
#define R2                          (2)
#define R3                          (5)
#define R7                          (5)

#define START_BIT                   (7)
#define TRANSMISSION_BIT            (6)
#define END_BIT                     (0)


#define BLOCK_SIZE                  (512)
#define SD_SPI_CHUNK_SIZE           (10)
#define REGISTER_SIZE               (16)
#define CRC_SIZE                    (2)
#define BLOCK_READ                  (17)
#define MULTI_BLOCK_READ            (18)
#define STOP_TRANS                  (12)
#define BLOCK_WRITE                 (24)
#define MULTIPLE_BLOCK_WRITE        (25)
#define ERASE_START                 (32)
#define ERASE_END                   (33)
#define ERASE                       (38)
#define START_TOKEN                 (0XFE)
#define READ_CID                    (10)
#define CID                         READ_CID
#define READ_CSD                    (9)
#define CSD                         READ_CSD

#define SD_DETECT                   (1)
#define SD_REMOVED                  (0)

#define SD_COMMAND_TIMEOUT          (5000)    /* Number of times to query card for correct result */

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

extern  QCLI_Group_Handle_t qcli_spi_group;

enum sdcard_status{
    SD_UNINITIALIZED_E,             /**< 0, enumerator for sdcard uninitialised */
    SD_INITIALIZING_E,              /**< enumerator for sdcard is initializing */
    SD_ENABLED_E,                   /**< enumerator for sdcard is enabled */
    SD_FAILED_E                     /**< enumerator for sdcard initalization failed */
};

/**
 * @struct           : SD Card data packet structure.
 * @brief             : SD Card data packet structure.
 */
typedef struct __attribute__((packed, aligned(1))) sdcard_conf_s {
    spi_bb_config_t spi_conf;       /**< to store spi config */
    int cs_pin;                     /**< chip select pin number */
    int mosi_pin;                   /**< mosi pin no */
    int miso_pin;                   /**< miso pin */
    int clk_pin;                    /**< clock pin */
    int tx_bytes;                   /**< transmitted bytes */
    int rx_bytes;                   /**< received bytes */
    int tx_speed;                   /**< transmit speed */
    int rx_speed;                   /**< receive speed */
    int status;                     /**< status of sdcard */
    uint32_t total_sectors;         /**< totals sectores in sd card */
    uint8_t sd_detect_flag;         /**< flag, whether sdcard detected or not */
    qurt_mutex_t mutex;             /**< sdcard mutex */
} sdcard_conf_t;

extern sdcard_conf_t sdcard_conf;

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

/**
 * @function    : This function takes sdcard config structure and initialize sdcard.
 * @brief       : This function takes sdcard config structure and initialize sdcard.
 * @param[in]   : sdcard config structure
 * @return      : On Success QAPI_OK is returned and SD Card Context Is Initialized, On Failure System Is Reseted.
 * @retval      : QAPI_OK if successfully intialized sd card.
 */
qapi_Status_t sdcard_init(/* sdcard_conf_t *sdcard */);

/**
 * @function    : This function send different sdcard command.
 * @brief       : This function send different sdcard command.
 * @param[in]   : sdcard config structure
 * @param[in]   : sdcard command
 * @param[in]   : Argument of sdcard command
 * @param[in]   : Response of that sdcard command
 * @return      : status of send command to sdcard
 * @retval      : QAPI_ERROR if error occured in sdcard_command
 * @retval      : QAPI_SUCCESS if successfully command send to sd card
 */
int32_t sdcard_command(sdcard_conf_t *sdcard,uint8_t command,uint32_t arguement,uint8_t response);

/**
 * @function    : This function finds total sectors of sdcard.
 * @brief       : This function finds total sectors of sdcard.
 * @param[in]   : sdcard config structure
 * @return      : Total sector in sdcard at sdcard structure
 * @retval      : QAPI_ERROR if error occured in calculate total sectors
 * @retval      : QAPI_SUCCESS if successfully found total sectors
 */
int sdcard_find_total_sector(/* sdcard_conf_t *sdcard */);

/**
 * @function    : Function to read the reg data from sd card.
 * @brief       : Function to read the reg data from sd card.
 * @param[in]   : sdcard config structure
 * @param[in]   : register name
 * @param[out]  : buffer to store the contents of the reg
 * @return      : fill the buffer with content of perticulat register
 * @retval      : QAPI_ERROR if error occured in getting reg info
 * @retval      : QAPI_SUCCESS if successfully found reg info in buffer
 */
qapi_Status_t sdcard_register_info(/* sdcard_conf_t *sdcard, */int reg,uint8_t *buffer);

/**
 * @function    : Function to write data to sdcard.
 * @brief       : Function to write data to sdcard.
 * @param[in]   : sdcard config structure
 * @param[in]   : Address of data to be write to sdcard
 * @param[in]   : Length of data array
 * @param[in]   : On which sector to write
 * @return      : status of sdcard_wirte success failure
 * @retval      : QAPI_ERROR if error occured in writing to sdcard
 * @retval      : QAPI_SUCCESS if successfully write data to sdcard
 */
int sdcard_write(/* sdcard_conf_t *sdcard, */uint8_t *data, int len, uint32_t sector);

/**
 * @brief     : Function To Clean SD Card.
 * @param[in] : sdcard to format
 * @return    : status of sdcard_erase function
 * @retval    : QAPI_OK if successfully erase data from sdcard
 * @retval    : QAPI_ERROR if error occured to erase sdcard
 */
int sdcard_clean(sdcard_conf_t *sdcard);

/**
 * @function    : Function to read data from sdcard.
 * @brief       : Function to read data from sdcard.
 * @param[in]   : sdcard config structure
 * @param[out]  : Address of data to be read from sdcard
 * @param[in]   : Length of data array
 * @param[in]   : from which sector to read
 * @return      : read data in buffer from sdcard
 * @retval      : QAPI_OK if successfully read data from sdcard
 * @retval      : QAPI_ERROR if error occured in reading from sdcard
 */
int sdcard_read(/* sdcard_conf_t *sdcard, */uint8_t *data, int len, uint32_t sector);

/**
 * @function  : Function to erase data from sdcard.
 * @brief     : Function to erase data from sdcard.
 * @param[in] : sdcard config structure
 * @param[in] : Starting address of sector
 * @param[in] : Ending address of sector
 * @return    : status of sdcard_erase function
 * @retval    : QAPI_OK if successfully erase data from sdcard
 * @retval    : QAPI_ERROR if error occured to erase sdcard
 */
qapi_Status_t sdcard_erase(sdcard_conf_t *sdcard,uint32_t start_addr, uint32_t end_addr);

/**
 * @function    : Function to print the string for the response recieved if it has any errors.
 * @brief       : Functin to print string for response.
 * @param[in]   : response recieved.
 * @return      : None.
 * @retval      : None.
 */
void sdcard_response_string(uint8_t response);

/**
 * @function    : Generate CRC table for CRC7 in global array
 * @brief       : Generate CRC table for CRC7 in global array
 */
void GenerateCRCTable(void);

/**
 * @function    : CRCAdd
 * @brief       : Adds a message byte to the current CRC-7 to get a the new CRC-7
 * @param[in]   : CRC current CRC-7
 * @param[in]   : message_byte add to create new CRC - 7
 * @return      : New generated CRC-7
 * @retval      : New generated CRC-7
 */
uint8_t CRCAdd(uint8_t CRC, uint8_t message_byte);

/**
 * @function    : getCRC
 * @brief       : Returns the CRC-7 for command and the arguement given.
 * @param[in]   : command to get CRC-7
 * @param[in]   : arguement of that command to create CRC - 7
 * @return      : New generated CRC-7
 * @retval      : New generated CRC-7
 */
uint8_t getCRC(uint8_t command,uint32_t arguement);

/**
 * @function    : crc16_ccitt
 * @brief       : Function to generate CRC16-CCITT checksum
 * @param[in]   : block of data to be calculated for checksum
 * @param[in]   : blockLength length of data
 * @param[in]   : starting value of crc table (0)
 * @return      : New generated CRC16-CCITT
 * @retval      : New generated CRC16-CCITT
 */
unsigned short crc16_ccitt(const unsigned char block[], unsigned int blockLength, unsigned short crc);

/**
 * @function  : Function to set intrrupt for sdcard detection.
 * @brief     : Function to set intrrupt for sdcard detection.
 * @param[in] : NONE
 * @return    : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval    : QAPI_OK on success.
 * @retval    : @see GPIO Error Codes
 */
qapi_Status_t sd_card_detect_init(void);

/**
 * @function    : SD_CARD detect call back function.
 * @brief       : SD_CARD detect call back function.
 * @return      : None.
 * @retval      : None.
 */
void sd_detect_callback(qapi_GPIOINT_Callback_Data_t event);

/**
 * @function    : Function to read sd card detection flag.
 * @brief       : Function to read sd card detection flag.
 * @return      : Success value of sd card initialize flag.
 * @retval      : 1 if sdcard detect
 * @retval      : 0 if sdcard removed
 */
int is_sd_card_available(void);

#endif
