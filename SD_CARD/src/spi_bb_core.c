/**
 * @file     : spi_bb_core.c
 * @brief    : APIs for spi bit banging.
 * @author   : Shreel Chhatbar (shreel.chhatbar@vvdntech.in)
 * @copyright: TODO
 */

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <pal_core.h>

#include <qapi/qapi_status.h>
#include <qapi_gpioint.h>
#include <qapi_delay.h>
#include <qurt_signal.h>
#include <qurt_thread.h>

#include <gpio_core.h>
#include <spi_core.h>
#include <spi_bb_core.h>
#include <sd_card_core.h>
#include <malloc.h>
#include <skylo_log.h>

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/

#define SPI_FREQ_KHZ                (100)
//#define Skylo_Printf(LOG_LEVEL,...) QCLI_Printf(LOG_LEVEL,(qcli_gpio_group), __VA_ARGS__)

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

extern spi_config_t spi0;
unsigned char spi_bus = SPI_PERIPHERAL;

qapi_SPIM_Config_t sd_slave = {0};
/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/

/**
 * @function  : Function to toggle spi pin.
 * @brief     : Function to toggle spi pin.
 * @param[in] : Pin to be toggled.
 * @return    : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval    : QAPI_OK on success.
 * @retval    : @see GPIO Error Codes
 */
qapi_Status_t spi_pin_toggle(gpio_config_t *spi_pin)
{
    if ( spi_pin->state == QAPI_GPIO_HIGH_VALUE_E )
        return spi_pin_clear(spi_pin);
    else
        return spi_pin_set(spi_pin);

    return QAPI_ERROR;
}

/**
 * @function  : Function to set spi pin.
 * @brief     : Function to set spi pin.
 * @param[in] : Pin to be set.
 * @return    : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval    : QAPI_OK on success.
 * @retval    : @see GPIO + SPI Error Codes
 */
qapi_Status_t spi_pin_set(gpio_config_t *spi_pin)
{
    return qapi_TLMM_Drive_Gpio(spi_pin->gpio_id, spi_pin->config.pin, QAPI_GPIO_HIGH_VALUE_E);
}

/**
 * @function  : Function to set spi pin.
 * @brief     : Function to set spi pin.
 * @param[in] : Pin to be set.
 * @return    : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval    : QAPI_OK on success.
 * @retval    : @see GPIO + SPI Error Codes
 */
qapi_Status_t spi_pin_clear(gpio_config_t *spi_pin)
{
    return qapi_TLMM_Drive_Gpio(spi_pin->gpio_id, spi_pin->config.pin, QAPI_GPIO_LOW_VALUE_E);
}

/**
 * @function  : Function to init spi bit banging.
 * @brief     : Function to init spi bit banging.
 * @param[in] : Pointer to spi_bb_config_t.
 * @return    : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval    : QAPI_OK on success.
 * @retval    : @see GPIO + SPI Error Codes
 */
qapi_Status_t spi_bb_init(spi_bb_config_t *spi_bb_conf)
{
    //all the pins are high in their idle state.
    //all pins are output except miso.
    Skylo_Printf(SKYLO_DEBUG,"[SDBB] SPI BB INIT\n");
    qapi_Status_t status = QAPI_ERROR;

    if ( spi_bus == SPI_BITBANG ) {

        if ( QAPI_OK == (status = gpio_config(spi_bb_conf->mosi)) ) {
            if ( QAPI_OK != (status = gpio_drive(spi_bb_conf->mosi, QAPI_GPIO_HIGH_VALUE_E)) ) {
                Skylo_Printf(SKYLO_ERR,"[SDBB] MOSI GPIO Drive Fails (E%d).\n", status);
                return status;
            }
        } else {
            Skylo_Printf(SKYLO_ERR,"[SDBB] MOSI GPIO Config Fails (E%d).\n", status);
            return status;
        }

        if ( QAPI_OK == (status = gpio_config(spi_bb_conf->clk)) ) {
            if ( QAPI_OK != (status = gpio_drive(spi_bb_conf->clk, QAPI_GPIO_LOW_VALUE_E)) ) {
                Skylo_Printf(SKYLO_ERR,"[SDBB] CLK GPIO Drive Fails (E%d).\n", status);
                return status;
            }
        } else {
            Skylo_Printf(SKYLO_ERR,"[SDBB] CLK GPIO Config Fails (E%d).\n", status);
            return status;
        }

        if ( QAPI_OK != (status = gpio_config(spi_bb_conf->miso)) ) {
            Skylo_Printf(SKYLO_ERR,"[SDBB] MISO Config Fails (E%d).\n", status);
            return status;
        }

        if ( QAPI_OK == (status = gpio_config(spi_bb_conf->cs)) ) {
            if ( QAPI_OK != (status = gpio_drive(spi_bb_conf->cs,QAPI_GPIO_HIGH_VALUE_E)) ) {
                Skylo_Printf(SKYLO_ERR,"[SDBB] CS GPIO Drive Fails (E%d).\n", status);
                return status;
            }
        } else {
            Skylo_Printf(SKYLO_ERR,"[SDBB] CS GPIO Config Fails (E%d).\n", status);
            return status;
        }
        Skylo_Printf(SKYLO_DEBUG,"[SDBB] SPI BB INIT DONE\n");

        return status;
    } else {
        spi_bb_conf->spi = &spi0;

        if ( QAPI_OK != (status = spi0_init()) ) {
            Skylo_Printf(SKYLO_ERR,"[SDBB]: Unable To Enable SPI0(E%d).\n", status);
            return QAPI_ERROR;
        }

        if ( QAPI_OK != sd_spi_config(&sd_slave) )
            Skylo_Printf(SKYLO_DEBUG,"[SDBB]: SPI Slave Config Failed.\n");

        //spi_config(spi_bb_conf->spi, sd_slave);
        return QAPI_OK;
    }
}

/**
 * @function  : Function to send data over spi.
 * @brief     : Function to send data over spi.
 * @param[in] : Pointer to spi_bb_config_t.
 * @param[in] : Data to be sent (8 bit).
 * @return    : int which is incoming data.
 * @retval    : Incoming data.
 */
int spi_bb_send(spi_bb_config_t *spi_conf, unsigned char data)
{
    int i;
    uint8_t data_in = 0;

    /* SPI Mode SHIFT_MSB_FIRST + CPOL = 0 + CPHA = 1 */
    if ( spi_bus == SPI_BITBANG ) {
        /* send bits 7..0 */
        for ( i = 0; i < 8; i++ ) {
            qapi_TLMM_Drive_Gpio(spi_conf->clk->gpio_id, spi_conf->clk->config.pin, QAPI_GPIO_LOW_VALUE_E);
            //spi_pin_clear(&(spi_conf->clk));

            // consider leftmost bit
            // set line high if bit is 1, low if bit is 0
            if (data & (1<<7))
                qapi_TLMM_Drive_Gpio(spi_conf->mosi->gpio_id, spi_conf->mosi->config.pin, QAPI_GPIO_HIGH_VALUE_E);
            //spi_pin_set(&(spi_conf->mosi));
            else
                qapi_TLMM_Drive_Gpio(spi_conf->mosi->gpio_id, spi_conf->mosi->config.pin, QAPI_GPIO_LOW_VALUE_E);
            //spi_pin_clear(&(spi_conf->mosi));

            data_in <<= 1;
            data_in |= gpio_read(spi_conf->miso);

            // pulse clock to indicate that bit value should be read
            qapi_TLMM_Drive_Gpio(spi_conf->clk->gpio_id, spi_conf->clk->config.pin, QAPI_GPIO_HIGH_VALUE_E);
            //spi_pin_set(&(spi_conf->clk));

            // shift byte left so next bit will be leftmost
            data <<= 1;
        }

        qapi_TLMM_Drive_Gpio(spi_conf->clk->gpio_id, spi_conf->clk->config.pin, QAPI_GPIO_LOW_VALUE_E);
    } else {

        if ( QAPI_OK != spi_data_transfer(spi_conf->spi,&data,1,&data_in,1,sd_slave) ) {

            Skylo_Printf(SKYLO_DEBUG,"[SDBB]: SPI Data transfer failed\n");

            return QAPI_ERROR;
            //TODO : Check where ever it gets calls for return error
        } else {
            return data_in;
        }
    }

    //spi_pin_clear(&(spi_conf->clk));
    return data_in;
}

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
int spi_multi_send(spi_bb_config_t *spi_conf, unsigned char *data_out, uint16_t len)
{
    uint8_t *data_in = NULL;

    if ( NULL == (data_in = buff_alloc(BLOCK_SIZE)) ) {
        Skylo_Printf(SKYLO_ERR, "[SDBB]: Buffer Allocation Null In %s at Line %d\n",__func__, __LINE__);
        return QAPI_ERROR;
    }

    if ( (data_out == NULL) || (len < 0) || (spi_conf == NULL) ) {
        Skylo_Printf(SKYLO_ERR, "[SDBB]: Buffer Allocation Null In %s at Line %d\n",__func__, __LINE__);
        free(data_in);
        return QAPI_ERROR;
    }

    if ( QAPI_OK != spi_data_transfer(spi_conf->spi,data_out,len,data_in,len,sd_slave) ) {

        Skylo_Printf(SKYLO_DEBUG,"[SDBB]: SPI Data transfer failed\n");

        free(data_in);
        return QAPI_ERROR;
    }

    free(data_in);
    return QAPI_OK;

}

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
int spi_multi_receive(spi_bb_config_t *spi_conf, uint8_t *data_in, uint16_t len)
{
    uint8_t *data_out= NULL;

    if ( NULL == (data_out = buff_alloc(SD_SPI_CHUNK_SIZE)) ) {
        Skylo_Printf(SKYLO_ERR, "[SDBB]: Buffer Allocation Null In %s at %d\n", __func__, __LINE__);
        return QAPI_ERROR;
    }

    if ( (data_in == NULL) || (len < 0) || (spi_conf == NULL) ) {
        Skylo_Printf(SKYLO_ERR, "[SDBB]: Buffer Allocation Null In %s at Line %d\n",__func__, __LINE__);
        free(data_out);
        return QAPI_ERROR;
    }

    memset (data_in, 0xff, len);
    if ( QAPI_OK != spi_data_transfer(spi_conf->spi,data_out,len,data_in,len,sd_slave) ) {

        Skylo_Printf(SKYLO_DEBUG,"[SDBB]: SPI Data transfer failed\n");

        free(data_out);
        return QAPI_ERROR;
        //TODO : Check where ever it gets calls for return error
    }

    free(data_out);
    return QAPI_OK;

}

/**
 * @functions           : Function to receive data from spi.
 * @brief               : Function to receive data from spi.
 * @param[in]           : Pointer to spi_bb_config_t.
 * @return              : int which is received data.
 * @retval              : Incoming data.
 */
int spi_bb_recieve(spi_bb_config_t *spi_conf)
{
    int response = 0;
    response = spi_bb_send(spi_conf,0xFF);
    //Skylo_Printf(SKYLO_DEBUG,"Resp : 0x%x\n",response);
    return response;
}

/**
 * @function   : Function used to configure SDcard SPI slave.
 * @brief      : Function used to configure SDcard SPI slave.
 * @param[out] : Pointer to qapi_SPIM_Config_t.
 * @return     : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval     : QAPI_OK on success.
 * @retval     : QAPI_ERR_INVALID_PARAM on Invalid Argument.
 */
qapi_Status_t sd_spi_config(qapi_SPIM_Config_t *config)
{
    if ( config == NULL ) {
        Skylo_Printf(SKYLO_ERR,"[SDBB]: Instance Context Is Null\n");
        return QAPI_ERR_INVALID_PARAM;
    }

    /*
     * QAPI_SPIM_CLK_IDLE_LOW_E Clock signal is low when in Idle.
     * QAPI_SPIM_CLK_IDLE_HIGH_E Clock signal is high when in Idle.
     */
    config->SPIM_Clk_Polarity = QAPI_SPIM_CLK_IDLE_LOW_E;

    /*
     * QAPI_SPIM_INPUT_FIRST_MODE_E In both master and slave, the input bit is shifted in first.
     * QAPI_SPIM_OUTPUT_FIRST_MODE_E In both master and slave, the output bit is shifted in first.
     */
    config->SPIM_Shift_Mode = QAPI_SPIM_OUTPUT_FIRST_MODE_E;

    /*
     * QAPI_SPIM_CS_ACTIVE_LOW_E During the Idle state, the CS line is held low.
     * QAPI_SPIM_CS_ACTIVE_HIGH_E During the Idle state, the CS line is held high.
     */
    config->SPIM_CS_Polarity = QAPI_SPIM_CS_ACTIVE_LOW_E;

    /*
     * QAPI_SPIM_CS_DEASSERT_E CS is deasserted while transferring data for N clock cycles.
     * QAPI_SPIM_CS_KEEP_ASSERTED_E CS is asserted as long as the core is in Run state.
     */
    config->SPIM_CS_Mode = QAPI_SPIM_CS_DEASSERT_E;

    /*
     * QAPI_SPIM_CLK_NORMAL_E Turns off the SPI clock during the Idle state.
     * QAPI_SPIM_CLK_ALWAYS_ON_E Runs the SPI clock during the Idle state
     */
    config->SPIM_Clk_Always_On = QAPI_SPIM_CLK_NORMAL_E;

    /* 8Bits Per Word */
    config->SPIM_Bits_Per_Word = 8;

    /* Chip Select 0 */
    config->SPIM_Slave_Index = 0;

    /* Min Max SPI Clock Frequency */
    config->min_Slave_Freq_Hz = SPI_FREQ;

    config->max_Slave_Freq_Hz = SPI_FREQ;

    /* Deassertion Time Between Two Sucessive SPI Transfer */
    config->deassertion_Time_Ns = 0;

    /*
     * 0 - Software Loop Back Disabled
     * 1 - Software Loop Back Enabled
     */
    config->loopback_Mode = 0;

    config->hs_Mode = 0;

    return QAPI_OK;
}
