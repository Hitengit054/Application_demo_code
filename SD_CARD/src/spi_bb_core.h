/**
 * @file     : spi_bb_core.h
 * @brief    : header for spi bit banging
 * @author   : Shreel Chhatbar (shreel.chhatbar@vvdntech.in)
 * @copyright: TODO
 */

#ifndef __SPI_BB_CORE_H__
#define __SPI_BB_CORE_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include <qurt_signal.h>
#include <qapi_tlmm.h>
#include <qapi_gpioint.h>
#include <gpio_core.h>
#include <spi_core.h>

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/

#define SPI_PERIPHERAL                 (0)
#define SPI_BITBANG                     (1)

#define SPI_TIME_IN_US(x)               ((1000/(x*4))+1) /**< adding one to remove the demical point */
#define SPI_FREQ                        (3000000)

#define CS_ASSERT(x)                    spi_pin_clear(x.cs)
#define CS_DEASSERT(x)                  spi_pin_set(x.cs)

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

extern unsigned char spi_bus;
extern  QCLI_Group_Handle_t qcli_spi_group;

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

/**
 * @struct           : SPI_BB Context Structure.
 * @brief             : SPI_BB Context Structure.
 */
typedef struct spi_bb_config
{
    gpio_config_t *cs;           /**< SPI_BB CS Configuration */
    gpio_config_t *mosi;         /**< SPI_BB CS Configuration */
    gpio_config_t *miso;         /**< SPI_BB CS Configuration */
    gpio_config_t *clk;          /**< SPI_BB CS Configuration */
    spi_config_t *spi;          /**< Present State */
} spi_bb_config_t;

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

/**
 * @function  : Function to toggle spi pin.
 * @brief     : Function to toggle spi pin.
 * @param[in] : Pin to be toggled.
 * @return    : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval    : QAPI_OK on success.
 * @retval    : @see GPIO Error Codes
 */
qapi_Status_t spi_pin_toggle(gpio_config_t *spi_pin);

/**
 * @function  : Function to set spi pin.
 * @brief     : Function to set spi pin.
 * @param[in] : Pin to be set.
 * @return    : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval    : QAPI_OK on success.
 * @retval    : @see GPIO + SPI Error Codes
 */
qapi_Status_t spi_pin_set(gpio_config_t *spi_pin);

/**
 * @function  : Function to set spi pin.
 * @brief     : Function to set spi pin.
 * @param[in] : Pin to be set.
 * @return    : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval    : QAPI_OK on success.
 * @retval    : @see GPIO + SPI Error Codes
 */
qapi_Status_t spi_pin_clear(gpio_config_t *spi_pin);

/**
 * @function  : Function to init spi bit banging.
 * @brief     : Function to init spi bit banging.
 * @param[in] : Pointer to spi_bb_config_t.
 * @return    : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval    : QAPI_OK on success.
 * @retval    : @see GPIO + SPI Error Codes
 */
qapi_Status_t spi_bb_init(spi_bb_config_t *spi_bb_conf);

/**
 * @function  : Function to send data over spi.
 * @brief     : Function to send data over spi.
 * @param[in] : Pointer to spi_bb_config_t.
 * @param[in] : Data to be sent (8 bit).
 * @return    : int which is incoming data.
 * @retval    : Incoming data.
 */
int spi_bb_send(spi_bb_config_t *spi_conf, unsigned char data);

/**
 * @functions           : Function to receive data from spi.
 * @brief               : Function to receive data from spi.
 * @param[in]           : Pointer to spi_bb_config_t.
 * @return              : int which is received data.
 * @retval              : Incoming data.
 */
int spi_bb_recieve(spi_bb_config_t *spi_conf);

/**
 * @function   : Function used to configure SDcard SPI slave.
 * @brief      : Function used to configure SDcard SPI slave.
 * @param[out] : Pointer to qapi_SPIM_Config_t.
 * @return     : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval     : QAPI_OK on success.
 * @retval     : QAPI_ERR_INVALID_PARAM on Invalid Argument.
 */
qapi_Status_t sd_spi_config(qapi_SPIM_Config_t *config);

/**
 * @brief
 * Function to send multiple data over spi
 *
 * @details
 * Function to send bunch of data over spi for sdcard
 *
 * @param[in]   Spi_conf   Pointer to spi_bb_config_t.
 * @param[in]   Data       Pointer to data to be send.
 * @param[in]   len        Length of data to be send.
 *
 *
 * @return  0 on success, negative value on failure.
 *
 * @return  An error code if negative; one of the following values:
 *          @par
 *          QAPI_OK on success. \n
 *          QAPI_ERROR on failure.
 */
int spi_multi_send(spi_bb_config_t *spi_conf, unsigned char *data, uint16_t len);

/**
 * @brief
 * Function to receive multiple data over spi
 *
 * @details
 * Function to receive bunch of data over spi for sdcard
 *
 * @param[in]   Spi_conf   Pointer to spi_bb_config_t.
 * @param[out]  Data       Pointer to data to be Receive.
 * @param[in]   len        Length of data to be Receive.
 *
 *
 * @return  0 on success, negative value on failure.
 *
 * @return  An error code if negative; one of the following values:
 *          @par
 *          QAPI_OK on success. \n
 *          QAPI_ERROR on failure.
 */
int spi_multi_receive(spi_bb_config_t *spi_conf, uint8_t *data, uint16_t len);
#endif
