/**
 * @file     : sd_card_core.c
 * @brief    : SD Card API's
 * @author   : Shreel Chhatbar (shreel.chhatbar@vvdntech.in)
 * @copyright: TODO
 */

/*
 *|--------+-----+--------+---------|
 *| Func   | Pin | Dir    | PULL    |
 *|--------+-----+--------+---------|
 *| CLK    | 18  | OUTPUT | PULL UP |
 *|--------+-----+--------+---------|
 *| MOSI   | 19  | OUTPUT | PULL UP |
 *|--------+-----+--------+---------|
 *| MISO   | 20  | INPUT  | PULL UP |
 *|--------+-----+--------+---------|
 *| CS     | 23  | OUTPUT | PULL UP |
 *|--------+-----+--------+---------|
 */

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <pal_core.h>
#include <qurt_thread.h>
#include <qapi_delay.h>
#include <qapi/qapi_status.h>
#include <spi_core.h>
#include <spi_bb_core.h>
#include <sd_card_core.h>
#include <utils.h>
#include <malloc.h>
#include <gpio_core.h>
#include <skylo_log.h>


/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/

//#define Skylo_Printf(LOG_LEVEL,...) QCLI_Printf(LOG_LEVEL,(qcli_gpio_group), __VA_ARGS__)

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

sdcard_conf_t sdcard_conf = {

    .spi_conf.cs = &gpio_global[GLBL_GPIO_SDCARD_BB_CS_PIN_E],
    .spi_conf.mosi = &gpio_global[GLBL_GPIO_SDCARD_BB_MOSI_PIN_E],
    .spi_conf.miso = &gpio_global[GLBL_GPIO_SDCARD_BB_MISO_PIN_E],
    .spi_conf.clk = &gpio_global[GLBL_GPIO_SDCARD_BB_CLK_PIN_E],
};

extern sdcard_conf_t sdcard_conf;
sdcard_conf_t *sdcard = &sdcard_conf;


uint8_t sd_detect_flag;                 /**< flag for sd card detection */
unsigned char CRCPoly = 0x89;           /**<  the value of our CRC-7 polynomial */
uint8_t internal_buffer[BLOCK_SIZE];

unsigned char CRCTable[256];            /**< to store crc data */

extern unsigned char spi_bus;

uint8_t type_;                          /* sd card type */

/** status for card in the idle state */
uint8_t const R1_IDLE_STATE = 0X01;
/** Standard capacity V1 SD card */
uint8_t const SD_CARD_TYPE_SD1 = 1;
/** Standard capacity V2 SD card */
uint8_t const SD_CARD_TYPE_SD2 = 2;
/** High Capacity SD card */
uint8_t const SD_CARD_TYPE_SDHC = 3;
/** status bit for illegal command */
uint8_t const R1_ILLEGAL_COMMAND = 0X04;

static const unsigned short crc16_ccitt_table[256] = {        /**< CRC16-CCITT x^16 +x^12 +x^5 +1 table */
    0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50A5U, 0x60C6U, 0x70E7U,
    0x8108U, 0x9129U, 0xA14AU, 0xB16BU, 0xC18CU, 0xD1ADU, 0xE1CEU, 0xF1EFU,
    0x1231U, 0x0210U, 0x3273U, 0x2252U, 0x52B5U, 0x4294U, 0x72F7U, 0x62D6U,
    0x9339U, 0x8318U, 0xB37BU, 0xA35AU, 0xD3BDU, 0xC39CU, 0xF3FFU, 0xE3DEU,
    0x2462U, 0x3443U, 0x0420U, 0x1401U, 0x64E6U, 0x74C7U, 0x44A4U, 0x5485U,
    0xA56AU, 0xB54BU, 0x8528U, 0x9509U, 0xE5EEU, 0xF5CFU, 0xC5ACU, 0xD58DU,
    0x3653U, 0x2672U, 0x1611U, 0x0630U, 0x76D7U, 0x66F6U, 0x5695U, 0x46B4U,
    0xB75BU, 0xA77AU, 0x9719U, 0x8738U, 0xF7DFU, 0xE7FEU, 0xD79DU, 0xC7BCU,
    0x48C4U, 0x58E5U, 0x6886U, 0x78A7U, 0x0840U, 0x1861U, 0x2802U, 0x3823U,
    0xC9CCU, 0xD9EDU, 0xE98EU, 0xF9AFU, 0x8948U, 0x9969U, 0xA90AU, 0xB92BU,
    0x5AF5U, 0x4AD4U, 0x7AB7U, 0x6A96U, 0x1A71U, 0x0A50U, 0x3A33U, 0x2A12U,
    0xDBFDU, 0xCBDCU, 0xFBBFU, 0xEB9EU, 0x9B79U, 0x8B58U, 0xBB3BU, 0xAB1AU,
    0x6CA6U, 0x7C87U, 0x4CE4U, 0x5CC5U, 0x2C22U, 0x3C03U, 0x0C60U, 0x1C41U,
    0xEDAEU, 0xFD8FU, 0xCDECU, 0xDDCDU, 0xAD2AU, 0xBD0BU, 0x8D68U, 0x9D49U,
    0x7E97U, 0x6EB6U, 0x5ED5U, 0x4EF4U, 0x3E13U, 0x2E32U, 0x1E51U, 0x0E70U,
    0xFF9FU, 0xEFBEU, 0xDFDDU, 0xCFFCU, 0xBF1BU, 0xAF3AU, 0x9F59U, 0x8F78U,
    0x9188U, 0x81A9U, 0xB1CAU, 0xA1EBU, 0xD10CU, 0xC12DU, 0xF14EU, 0xE16FU,
    0x1080U, 0x00A1U, 0x30C2U, 0x20E3U, 0x5004U, 0x4025U, 0x7046U, 0x6067U,
    0x83B9U, 0x9398U, 0xA3FBU, 0xB3DAU, 0xC33DU, 0xD31CU, 0xE37FU, 0xF35EU,
    0x02B1U, 0x1290U, 0x22F3U, 0x32D2U, 0x4235U, 0x5214U, 0x6277U, 0x7256U,
    0xB5EAU, 0xA5CBU, 0x95A8U, 0x8589U, 0xF56EU, 0xE54FU, 0xD52CU, 0xC50DU,
    0x34E2U, 0x24C3U, 0x14A0U, 0x0481U, 0x7466U, 0x6447U, 0x5424U, 0x4405U,
    0xA7DBU, 0xB7FAU, 0x8799U, 0x97B8U, 0xE75FU, 0xF77EU, 0xC71DU, 0xD73CU,
    0x26D3U, 0x36F2U, 0x0691U, 0x16B0U, 0x6657U, 0x7676U, 0x4615U, 0x5634U,
    0xD94CU, 0xC96DU, 0xF90EU, 0xE92FU, 0x99C8U, 0x89E9U, 0xB98AU, 0xA9ABU,
    0x5844U, 0x4865U, 0x7806U, 0x6827U, 0x18C0U, 0x08E1U, 0x3882U, 0x28A3U,
    0xCB7DU, 0xDB5CU, 0xEB3FU, 0xFB1EU, 0x8BF9U, 0x9BD8U, 0xABBBU, 0xBB9AU,
    0x4A75U, 0x5A54U, 0x6A37U, 0x7A16U, 0x0AF1U, 0x1AD0U, 0x2AB3U, 0x3A92U,
    0xFD2EU, 0xED0FU, 0xDD6CU, 0xCD4DU, 0xBDAAU, 0xAD8BU, 0x9DE8U, 0x8DC9U,
    0x7C26U, 0x6C07U, 0x5C64U, 0x4C45U, 0x3CA2U, 0x2C83U, 0x1CE0U, 0x0CC1U,
    0xEF1FU, 0xFF3EU, 0xCF5DU, 0xDF7CU, 0xAF9BU, 0xBFBAU, 0x8FD9U, 0x9FF8U,
    0x6E17U, 0x7E36U, 0x4E55U, 0x5E74U, 0x2E93U, 0x3EB2U, 0x0ED1U, 0x1EF0U
};

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/

void type(uint8_t value) {type_ = value;}
int get_type(void) { return type_; }

/**
 * @function    : This function takes sdcard config structure and initialize sdcard.
 * @brief       : This function takes sdcard config structure and initialize sdcard.
 * @param[in]   : sdcard config structure
 * @return      : On Success QAPI_OK is returned and SD Card Context Is Initialized, On Failure System Is Reseted.
 * @retval      : QAPI_OK if successfully intialized sd card.
 */
qapi_Status_t sdcard_init(/* sdcard_conf_t *sdcard */)
{
    int looper = 0;
    //uint8_t response = 0;
    qapi_Status_t status;
    int time_count = 0;
    uint32_t arg = 0;

    if ( QAPI_OK != (sd_card_detect_init()) ) {
        Skylo_Printf(SKYLO_ERR,"[SD] ***************Resetting board SD card Detect Pin Init Fails ***************\n");
        set_system_reset_reason(SK_RESET_REASON_SD_CARD_PIN);
        //return QAPI_ERROR;
    }

    if ( SD_DETECT != sdcard_conf.sd_detect_flag ) {
        Skylo_Printf(SKYLO_ERR,"[SD] ***************Resetting board SD card not detected***************\n");
        set_system_reset_reason(SK_RESET_REASON_NO_SD_CARD);
        //return QAPI_ERROR;
    }

    if ( QURT_EOK != (status = qurt_mutex_create(&(sdcard->mutex))) ) {
        Skylo_Printf(SKYLO_ERR,"[SD] ***************Resetting board SD card Mutex Fails(E%d) ***************\n",
                status);
        set_system_reset_reason(SK_RESET_REASON_SD_MUTEX);
        //return QAPI_ERROR;
    }

    sdcard->status = SD_INITIALIZING_E;
    Skylo_Printf(SKYLO_DEBUG,"[SD] INIT SD CARD\n");

    //TODO: if this fails , sd card cannot be accessed and probably try again or remove sd card calls.
    status = spi_bb_init(&(sdcard->spi_conf));
    if ( status != QAPI_OK ) {
        sdcard->status = SD_FAILED_E;
        Skylo_Printf(SKYLO_ERR,"[SD] ***************Resetting board SD card BitBang/Peripheral Init Fails(E%d) \
                                    ***************\n",
                status);
        set_system_reset_reason(SK_RESET_REASON_SD_BB_INIT);
        //return QAPI_ERROR;
    }

    GenerateCRCTable();
    CS_DEASSERT((sdcard->spi_conf));

    /* Clock Synchronisation Over GPIO BitBang */
    for ( looper = 0; looper < 16; looper++ ) {
        spi_bb_send(&(sdcard->spi_conf), 0xFF);
    }

#if 0
    /* TODO: Take care of all the commands according to the response of R1 */
    while( ((status = sdcard_command(sdcard,0,0x00,R1)) != 0x01) && time_count < 0xFF )
        time_count++;
#endif

    Skylo_Printf(SKYLO_DEBUG,"[SD] SENDING CMD0\n");
    while ( (status = sdcard_command(sdcard, 0, 0x00, R1)) != R1_IDLE_STATE );
    // sdcard_command(sdcard, 0, 0x00, R1);
    Skylo_Printf(SKYLO_DEBUG, "[SD] Response : %d\n", status);

#if 0
    if ( time_count >= 0xFF ) {
        time_count = 0;
        sdcard->status = SD_FAILED_E;
        Skylo_Printf(SKYLO_ERR,"CMD0 Failed\n");
        return QAPI_ERROR;
    }
#endif

    Skylo_Printf(SKYLO_DEBUG,"[SD] SENDING CMD8\n");
    while ( ((status = sdcard_command(sdcard,8,0x1AA,R7)) != R1_IDLE_STATE) && time_count < 0xFF )
        time_count++;

    if ( time_count >= 0xFF ) {
        time_count = 0;
        sdcard->status = SD_FAILED_E;
        Skylo_Printf(SKYLO_ERR,"[SD] CMD8 Failed %d\n",status);
        return QAPI_ERROR;
    }
    Skylo_Printf(SKYLO_DEBUG,"[SD] Response : %d\n",status);

    if ( status & R1_ILLEGAL_COMMAND ) {
        type(SD_CARD_TYPE_SD1);
    } else {

        // only need last byte of r7 response
        for ( uint8_t i = 0; i < 4; i++ )
            status = spi_bb_recieve(&(sdcard->spi_conf));

        if (status != 0XAA) {

            Skylo_Printf(SKYLO_DEBUG,"[SD] Error in command 8 : %d\n",status);
            return QAPI_ERROR;
        }

        type(SD_CARD_TYPE_SD2);
    }

    Skylo_Printf(SKYLO_DEBUG,"[SD] SD card type detected %s\n", get_type() == SD_CARD_TYPE_SD2 ?
                                "SD_CARD_TYPE_SD2":"SD_CARD_TYPE_SD1");

    // initialize card and send host supports SDHC if SD2
    arg = (get_type() == SD_CARD_TYPE_SD2 ? 0X40000000 : 0);

    do {
        Skylo_Printf(SKYLO_DEBUG,"[SD] SENDING CMD55\n");
        status = sdcard_command(sdcard,55,0x00,R1);
        Skylo_Printf(SKYLO_DEBUG,"[SD] Response : %d\n",status);

        Skylo_Printf(SKYLO_DEBUG,"[SD] SENDING ACMD41 with arg 0x%0X\n",arg);
        // status = sdcard_command(sdcard,41,0x40000000,R1);
        status = sdcard_command(sdcard,41,arg,R1);
    } while ( status != 0x00 );
    Skylo_Printf(SKYLO_DEBUG,"[SD] Response : %d\n",status);

    if ( get_type() == SD_CARD_TYPE_SD2 ) {
        // if SD2 read OCR register to check for SDHC card
        Skylo_Printf(SKYLO_DEBUG,"[SD] SENDING CMD58\n");
        sdcard_command(sdcard, 58, 0x00, R1);
        Skylo_Printf(SKYLO_DEBUG, "[SD] Response : %d\n", status);
        // return QAPI_ERROR;

        if ( ((status = spi_bb_recieve(&(sdcard->spi_conf))) & 0xC0) == 0xC0 )
            type(SD_CARD_TYPE_SDHC);

        // discard rest of ocr - it contains allowed voltage range
        for ( uint8_t i = 0; i < 3; i++ )
            status = spi_bb_recieve(&(sdcard->spi_conf));
    }

    // Skylo_Printf(SKYLO_DEBUG,"[SD] SENDING ACMD58\n");
    // while ( ((status = sdcard_command(sdcard,58,0x00,R3)) != 0x00) && time_count < 0xFF )
    //     time_count++;

    // if ( time_count >= 0xFF ) {
    //     time_count = 0;
    //     sdcard->status = SD_FAILED_E;
    //     Skylo_Printf(SKYLO_ERR,"[SD] CMD58 Failed\n");
    //     return QAPI_ERROR;
    // }
    // Skylo_Printf(SKYLO_DEBUG,"[SD] Response : %d\n",status);

    /* Skylo_Printf(SKYLO_DEBUG,"[SD] Getting total sectors\n"); */
    /* while ( ((sdcard_find_total_sector(sdcard)) != QAPI_OK) && time_count < 0x05 ) { */
    /* time_count++; */
    /* } */

    /* if ( time_count >= 0xFF ) { */
    /* time_count = 0; */
    /* sdcard->status = SD_FAILED_E; */
    /* Skylo_Printf(SKYLO_ERR,"[SD] Couldn't get the total number of sectors in the SD card\n"); */
    /* return QAPI_ERROR; */
    /* } */

    Skylo_Printf(SKYLO_DEBUG,"[SD] SD CARD is ready\n");

    sdcard->status = SD_ENABLED_E;
    return 1;
}

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
int32_t sdcard_command(sdcard_conf_t *sdcard,uint8_t command,uint32_t arguement,uint8_t response)
{
    if ( sdcard->status == SD_UNINITIALIZED_E || sdcard-> status == SD_FAILED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Initialization had failed or pending\n");
        return QAPI_ERROR;
    }

    uint8_t sd_frame[6];
    uint8_t sd_command = (1<<TRANSMISSION_BIT)|command;
    uint32_t sd_arguement = arguement;
    uint8_t sd_response[5] = {0,0,0,0,0};
    uint8_t sd_crc7 = ((getCRC(sd_command,sd_arguement))<<1) | (1<<END_BIT);
    int counter = 0;
    int time_count = 0;

    if ( command == 8 )
        sd_crc7 = 0x87;
    else
        sd_crc7 = 0x95;

    sd_frame[0] = sd_command | 0x40;
    sd_frame[1] = arguement >> (3*8) & 0xFF;
    sd_frame[2] = arguement >> (2*8) & 0xFF;
    sd_frame[3] = arguement >> (1*8) & 0xFF;
    sd_frame[4] = arguement >> (0*8) & 0xFF;
    sd_frame[5] = sd_crc7;

    //Skylo_Printf(SKYLO_DEBUG,"[SD] SD CARD COMMAND(%d)\n",command);

    /* Get SD Card Attention By Sending Clocks */
    spi_bb_recieve(&(sdcard->spi_conf));
    CS_ASSERT((sdcard->spi_conf));
    spi_bb_recieve(&(sdcard->spi_conf));

    for ( counter = 0 ; counter < 6 ; counter++ )
        spi_bb_send(&(sdcard->spi_conf),sd_frame[counter]);

    /* Wait till SD Card Drives MISO High */
    // while ( ((sd_response[0] = spi_bb_recieve(&(sdcard->spi_conf))) == 0xFF) && time_count < SD_COMMAND_TIMEOUT )
    //     time_count++;

    for ( int i = 0; i< SD_COMMAND_TIMEOUT; i++ ) {

        sd_response[0] = spi_bb_recieve(&(sdcard->spi_conf));

         if ( !(sd_response[0] & 0x80) )
             break;
    }

    if ( time_count >= SD_COMMAND_TIMEOUT ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Response not recieved in desired time\n");
        return QAPI_ERROR;
    }

    // if ( response == R2 || response == R3 || response == R7 ) {
    //     sd_response[1] = spi_bb_recieve(&(sdcard->spi_conf));
    //     if ( response == R3 || response == R7 ) {
    //         sd_response[2] = spi_bb_recieve(&(sdcard->spi_conf));
    //         sd_response[3] = spi_bb_recieve(&(sdcard->spi_conf));
    //         sd_response[4] = spi_bb_recieve(&(sdcard->spi_conf));
    //         Skylo_Printf(SKYLO_DEBUG,"[SD] R0 : %X\t,R1 : %X\t,R2 : %X\t,R3 : %X\t,R4 : %X\t\n",
    //                sd_response[0],sd_response[1],sd_response[2],sd_response[3],sd_response[4]);
    //     } else {
    //         //Skylo_Printf(SKYLO_DEBUG,"[SD] R0 : %X\t,R1 : %X\t\n",sd_response[0],sd_response[1]);
    //     }
    // } else {
    //     //Skylo_Printf(SKYLO_DEBUG,"[SD] R0 : %X\n",sd_response[0]);
    // }

    /* Make SD Card Listening Ready For Next Command */
    // spi_bb_recieve(&(sdcard->spi_conf));
    CS_DEASSERT((sdcard->spi_conf));
    // spi_bb_send(&(sdcard->spi_conf), 0xFF);
    if ( command != 8 )
        spi_bb_recieve(&(sdcard->spi_conf));

    //Skylo_Printf(SKYLO_DEBUG,"[SD] R0 : %X\n",sd_response[0]);

    sdcard_response_string(sd_response[0]);

    return(sd_response[0]);
}

/**
 * @function    : This function finds total sectors of sdcard.
 * @brief       : This function finds total sectors of sdcard.
 * @param[in]   : sdcard config structure
 * @return      : Total sector in sdcard at sdcard structure
 * @retval      : QAPI_ERROR if error occured in calculate total sectors
 * @retval      : QAPI_SUCCESS if successfully found total sectors
 */
int sdcard_find_total_sector(/* sdcard_conf_t *sdcard */)
{
    if ( sdcard->status == SD_UNINITIALIZED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Need to initialize the sd card first\n");
        return QAPI_ERROR;
    }

    if ( sdcard-> status == SD_FAILED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] SD card was not initialized and has failed\n");
        return QAPI_ERROR;
    }

    uint8_t read_buffer[17] = {0};
    uint32_t size = 0;
    uint32_t sector = 0;
    qapi_Status_t status;
    //int i;

    memset(read_buffer,0,sizeof(read_buffer));
    status = sdcard_register_info(/* sdcard, */CSD, read_buffer);
    if ( status == QAPI_ERROR ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Unable to read the csd register\n");
        return QAPI_ERROR;
    }

    //for ( i=0;i<REGISTER_SIZE;i++ )
    //    Skylo_Printf(SKYLO_DEBUG,"[SD] %x\t",((read_buffer[i])));

    size = (((int)read_buffer[9]) | (((int)read_buffer[8]) << 8) | (((int)read_buffer[7] & 0x3F) << 16));
    Skylo_Printf(SKYLO_DEBUG,"[SD] SIZE - 0x%x\t\n", size);
    sector = (size+1)*1024;
    sdcard->total_sectors = sector;
    Skylo_Printf(SKYLO_DEBUG,"[SD] total sector - %d\t", sdcard->total_sectors);
    return sdcard->total_sectors;
}

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
qapi_Status_t sdcard_register_info(/* sdcard_conf_t *sdcard, */int reg,uint8_t *buffer)
{
    if ( sdcard->status == SD_UNINITIALIZED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Need to initialize the sd card first\n");
        return QAPI_ERROR;
    }

    if ( sdcard-> status == SD_FAILED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] SD card was not initialized and has failed\n");
        return QAPI_ERROR;
    }

    uint8_t crc[2] = {0,0};
    uint8_t command = 0;
    int response = 0;
    //uint16_t Cal_CRC = 0;
    //uint16_t Data_CRC = 0;
    int byte_counter = 0;
    int block_counter;
    int data_counter;
    int total_block;
    uint8_t temp;
    int  error_index;
    int speed_time[2];

    if ( reg == CID ) {
        command = READ_CID;
        total_block = 1;
    } else {
        command = READ_CSD;
        total_block = 1;
    }

    speed_time[0] = sapp_get_time_in_msec();

    sapp_msec_delay(10);
    //Skylo_Printf(SKYLO_DEBUG,"[SD] CMD Reg Info\n");

    /* Tell SD Card Which Register Is Going To Be Read */
    if ( (response = sdcard_command(sdcard,command,0x00,R1)) == 0 ) {

        CS_ASSERT((sdcard->spi_conf));
        data_counter = 0;
        for ( block_counter = 0; block_counter < total_block ; block_counter++ ) {
            temp = 0;
            error_index = 0;
            memset(crc,0,sizeof(crc));
            memset(internal_buffer,0,sizeof(internal_buffer));

            /* Wait Till SD Card Is Not Ready To Give Data */
            while ( (response = spi_bb_recieve(&(sdcard->spi_conf))) != 0xFE ) {
                //Skylo_Printf(SKYLO_DEBUG,"%d : 0x%x\n",error_index,response);
                if ( (++error_index) == 255 ) {
                    Skylo_Printf(SKYLO_ERR,"[SD] Not getting start token for reading register - %d\n", reg);
                    spi_bb_recieve(&(sdcard->spi_conf));
                    CS_DEASSERT((sdcard->spi_conf));
                    spi_bb_recieve(&(sdcard->spi_conf));
                    return QAPI_ERROR;
                }
            }

            /* Read SD Card Register */
            for ( byte_counter = 0; byte_counter < REGISTER_SIZE; byte_counter++ ) {
                temp = spi_bb_recieve(&(sdcard->spi_conf));
                internal_buffer[byte_counter] = temp;
                if (data_counter < REGISTER_SIZE)
                    buffer[data_counter] = temp;
                data_counter++;
            }

            // /* Read CRC From SD Card */
            // crc[0] = spi_bb_recieve(&(sdcard->spi_conf));
            // crc[1] = spi_bb_recieve(&(sdcard->spi_conf));

            // /* Adjuct CRC Received From SD Card */
            // Data_CRC = ((uint16_t)(crc[0]*256) + crc[1]);

            // /* Calculate CRC From Data Received */
            // Cal_CRC = crc16_ccitt(internal_buffer, REGISTER_SIZE, 0);

            // if ( Cal_CRC != Data_CRC ) {
            //     Skylo_Printf(SKYLO_ERR,"[SD] CRC not match for %d",reg);
            //     return QAPI_ERROR;
            // }
        }

    } else {
        Skylo_Printf(SKYLO_ERR,"[SD] SPI CMD9 response error\n");
        spi_bb_recieve(&(sdcard->spi_conf));
        CS_DEASSERT((sdcard->spi_conf));
        spi_bb_recieve(&(sdcard->spi_conf));
        return QAPI_ERROR;
    }

    /* Make SD Card Listening Ready For Next Command */
    spi_bb_recieve(&(sdcard->spi_conf));
    CS_DEASSERT((sdcard->spi_conf));
    spi_bb_recieve(&(sdcard->spi_conf));

    /* Calculate Execution Time For Register Read */
    speed_time[1] = sapp_get_time_in_msec();
    sdcard->rx_speed =  (1000 * (total_block*BLOCK_SIZE))/(speed_time[1] - speed_time[0]);
    sdcard->rx_bytes += (total_block*BLOCK_SIZE);
    //Skylo_Printf(SKYLO_DEBUG,"[SD] Total Bytes read : %d \t", sdcard->rx_bytes);
    //Skylo_Printf(SKYLO_DEBUG,"[SD] Total Bytes read : %d in %d ms @ %d Bps\n"
    //        ,(total_block*BLOCK_SIZE),(speed_time[1] - speed_time[0]),sdcard->rx_speed);

    //Skylo_Printf(SKYLO_DEBUG,"[SD] Reading Register Done\n");

    return QAPI_OK;
}

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
int sdcard_write(/* sdcard_conf_t *sdcard, */uint8_t *data, int len, uint32_t sector)
{
    if ( sdcard->status == SD_UNINITIALIZED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Need to initialize the sd card first\n");
        return QAPI_ERROR;
    }

    if ( sdcard-> status == SD_FAILED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] SD card was not initialized and has failed\n");
        return QAPI_ERROR;
    }

    if ( data == NULL ) {
        Skylo_Printf(SKYLO_ERR, "Error buffer null\n");
        return QAPI_ERROR;
    }

    if ( TAKE_SDCARD_LOCK(sdcard->mutex) ) {
        /* Skylo_Printf(SKYLO_DEBUG,"[SD] Got lock in function : %s\n",__func__); */
        uint8_t crc[2] = {0,0};
        uint8_t command = 0;
        uint8_t response = 0;
        uint16_t Cal_CRC = 0;
        int byte_counter = 0;
        int block_counter;
        int total_block;
        int speed_time[2];
        uint8_t *temp = NULL;

        if ( spi_bus == SPI_PERIPHERAL ) {

            if ( NULL == (temp = buff_alloc(SD_SPI_CHUNK_SIZE)) ) {
                Skylo_Printf(SKYLO_ERR, "[SD] Buf Allocation Null\n");
                return QAPI_ERROR;
            }
        }

        if ( len <= BLOCK_SIZE ) {
            command = BLOCK_WRITE;
            total_block = 1;
        } else {
            command = MULTIPLE_BLOCK_WRITE;
            total_block = len/BLOCK_SIZE;
            if ( len%BLOCK_SIZE ) {
                total_block++;
            }
        }
        //Skylo_Printf(SKYLO_DEBUG,"[SD] Write Length : %d,Total Blocks : %d starting from sector : %d\n",
        //        len, total_block, sector);

        speed_time[0] = sapp_get_time_in_msec();

        /* Tell SDCard To Multiple/Single Block Wrte */
        if ( (response = sdcard_command(sdcard,command,sector,R1)) == 0 ) {
            CS_ASSERT((sdcard->spi_conf));
            /* Write Single/Multiple Block To SD Card */
            for ( block_counter = 0; block_counter < total_block ; block_counter++ ) {
                memset(crc,0,sizeof(crc));
                memset(internal_buffer,0,sizeof(internal_buffer));

                /* Fregment Buffer Into BlockSize Before Writting To SD Card. */
                if ( len != BLOCK_SIZE ) {
                    memcpy(internal_buffer,data+(block_counter*BLOCK_SIZE),
                            (block_counter == total_block - 1)?(len%BLOCK_SIZE):BLOCK_SIZE);
                } else {
                    memcpy(internal_buffer,data+(block_counter*BLOCK_SIZE),BLOCK_SIZE);
                }

                /* Calculate CRC From Buffer Chunk To Be Written */
                Cal_CRC = crc16_ccitt(internal_buffer, BLOCK_SIZE, 0);

                /* Send Token For Single/Multi Block Write To SDCard Before writting Block Data */
                if(command == MULTIPLE_BLOCK_WRITE)
                    spi_bb_send(&(sdcard->spi_conf),0xFC);
                else
                    spi_bb_send(&(sdcard->spi_conf), START_TOKEN);

                if ( spi_bus == SPI_BITBANG ) {
                    /* Write Block Data Into SD Card */
                    for ( byte_counter = 0; byte_counter < BLOCK_SIZE; byte_counter++ ) {
                        /* Skylo_Printf(SKYLO_DEBUG,"WRITE : %d : 0x%x\n",byte_counter,internal_buffer[byte_counter]); */
                        spi_bb_send(&(sdcard->spi_conf), internal_buffer[byte_counter]);
                    }
                } else {
                    for ( byte_counter = 0; byte_counter < (BLOCK_SIZE/SD_SPI_CHUNK_SIZE); byte_counter++ ) {
                        memcpy(temp, internal_buffer+(byte_counter*SD_SPI_CHUNK_SIZE), SD_SPI_CHUNK_SIZE);
                        spi_multi_send(&(sdcard->spi_conf),temp, SD_SPI_CHUNK_SIZE);
                        memset(temp, 0 , SD_SPI_CHUNK_SIZE);
                    }
                    if ( (BLOCK_SIZE % SD_SPI_CHUNK_SIZE) != 0 ) {
                        memcpy(temp, internal_buffer+(byte_counter*SD_SPI_CHUNK_SIZE), (BLOCK_SIZE % SD_SPI_CHUNK_SIZE));
                        spi_multi_send(&(sdcard->spi_conf),temp, (BLOCK_SIZE % SD_SPI_CHUNK_SIZE));
                    }
                    free(temp);
                }

                crc[0] = Cal_CRC & 0x00FF;
                crc[1] = (Cal_CRC >> 8) & 0X00FF;

                /* Send Calculated CRC To SD Card */
                spi_bb_send(&(sdcard->spi_conf), crc[0]);
                spi_bb_send(&(sdcard->spi_conf), crc[1]);

                /* Check Whether Data Is Written Successfully Or Not */
                if( ((response = spi_bb_recieve(&(sdcard->spi_conf))) & 0x1F) != 0x05 ) {
                    Skylo_Printf(SKYLO_ERR,"[SD] Data not accepted by SDCARD with status = %x\n", response);
                    RELEASE_SDCARD_LOCK(sdcard->mutex);
                    CS_DEASSERT((sdcard->spi_conf));
                    spi_bb_recieve(&(sdcard->spi_conf));
                    return QAPI_ERROR;
                }

                /* Wait Till SD Card Keeps MISO Line LOW */
                while((response = spi_bb_recieve(&(sdcard->spi_conf))) == 0x00);
            }

            /* Send End Token For MultiBlock Write To SD Card */
            if ( command == MULTIPLE_BLOCK_WRITE ) {
                spi_bb_send(&(sdcard->spi_conf),0xFD);

                /* Wait Till SD Card Keeps MISO Line LOW */
                while((response = spi_bb_recieve(&(sdcard->spi_conf))) == 0x00);
            }
        } else {
            Skylo_Printf(SKYLO_ERR,"[SD] SPI CMD24 response error 0x%X\n",response);
            RELEASE_SDCARD_LOCK(sdcard->mutex);
            CS_DEASSERT((sdcard->spi_conf));
            spi_bb_recieve(&(sdcard->spi_conf));
            free(temp);
            return QAPI_ERROR;
        }

        /* Make SD Card Listening Ready For Next Command */
        spi_bb_recieve(&(sdcard->spi_conf));
        CS_DEASSERT((sdcard->spi_conf));
        spi_bb_recieve(&(sdcard->spi_conf));

        /* Calculate Execution Time For Buffer Write */
        speed_time[1] = sapp_get_time_in_msec();
        sdcard->tx_speed =  (1000 * (total_block*BLOCK_SIZE))/(speed_time[1] - speed_time[0]);
        sdcard->tx_bytes += (total_block*BLOCK_SIZE);
        //Skylo_Printf(SKYLO_DEBUG,"[SD] Total Bytes written : %d \t", sdcard->tx_bytes);
        //Skylo_Printf(SKYLO_DEBUG,"[SD] Bytes written : %d in %d ms @ %d Bps\n"
        //        ,(total_block*BLOCK_SIZE),(speed_time[1] - speed_time[0]),sdcard->tx_speed);

        //Skylo_Printf(SKYLO_DEBUG,"[SD] Write Done\n");
        RELEASE_SDCARD_LOCK(sdcard->mutex);
    } else {
        Skylo_Printf(SKYLO_ERR,"[SD] Unable to take the SD lock\n");
        return QAPI_ERROR;
    }

    /* Verfy Bytes Written To SD Card */
    //sdcard_read(verify_buffer,BLOCK_SIZE,sector);
    return QAPI_OK;

}

/**
 * @brief     : Function To Clean SD Card.
 * @param[in] : sdcard to format
 * @return    : status of sdcard_erase function
 * @retval    : QAPI_OK if successfully erase data from sdcard
 * @retval    : QAPI_ERROR if error occured to erase sdcard
 */
int sdcard_clean(sdcard_conf_t *sdcard)
{
    if ( sdcard->status == SD_UNINITIALIZED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Need to initialize the sd card first\n");
        return QAPI_ERROR;
    }

    if ( sdcard-> status == SD_FAILED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] SD card was not initialized and has failed\n");
        return QAPI_ERROR;
    }

    if ( sdcard == NULL ) {
        Skylo_Printf(SKYLO_ERR, "[SD] Error buffer null\n");
        return QAPI_ERROR;
    }

    return (sdcard_erase(sdcard,0,sdcard->total_sectors-1));
}

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
int sdcard_read(/* sdcard_conf_t *sdcard, */uint8_t *data, int len, uint32_t sector)
{

    if ( sdcard->status == SD_UNINITIALIZED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Need to initialize the sd card first\n");
        return QAPI_ERROR;
    }

    if ( sdcard-> status == SD_FAILED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] SD card was not initialized and has failed\n");
        return QAPI_ERROR;
    }

    if ( data == NULL ) {
        Skylo_Printf(SKYLO_ERR, "[SD] Error buffer null\n");
        return QAPI_ERROR;
    }
    if ( TAKE_SDCARD_LOCK(sdcard->mutex) ) {
        /* Skylo_Printf(SKYLO_DEBUG,"[SD] Got lock in function : %s\n",__func__); */
        uint8_t crc[2] = {0,0};
        uint8_t command = 0;
        int response = 0;
        uint8_t command_stop = 0;
        uint16_t Cal_CRC = 0;
        uint16_t Data_CRC = 0;
        uint8_t sd_response_stop[5] = {0,0,0,0,0};
        int byte_counter = 0;
        int block_counter;
        int data_counter;
        int total_block;
        uint8_t temp;
        int error_index;
        int speed_time[2];
        uint8_t *temp_buf = NULL;
        uint8_t *read_buf = NULL;

        if ( spi_bus == SPI_PERIPHERAL ) {

            if ( NULL == (temp_buf = buff_alloc(BLOCK_SIZE)) ) {
                Skylo_Printf(SKYLO_ERR, "[SD] Buffer Allocation Null\n");
                return QAPI_ERR_NO_MEMORY;
            }

            if ( NULL == (read_buf = buff_alloc(SD_SPI_CHUNK_SIZE)) ) {
                Skylo_Printf(SKYLO_ERR, "[SD] Buffer Allocation Null\n");
                free(temp_buf);
                return QAPI_ERR_NO_MEMORY;
            }
        }

        if ( len <= BLOCK_SIZE ) {
            command = BLOCK_READ;
            total_block = 1;
        } else {
            command = MULTI_BLOCK_READ;
            command_stop = STOP_TRANS;
            total_block = len/BLOCK_SIZE;
            if ( len%BLOCK_SIZE ) {
                total_block++;
            }
        }

        //Skylo_Printf(SKYLO_DEBUG,"[SD] Read Length : %d,Total Blocks : %d, Sector : %d\n",len,total_block,sector);

        speed_time[0] = sapp_get_time_in_msec();

        /* Tell SD Card To Get Ready For Multiple/Single Block Read */
        if ( (response = sdcard_command(sdcard,command,sector,R1)) == 0 ) {

            CS_ASSERT((sdcard->spi_conf));
            data_counter = 0;

            /* Read Multiple/Single Block Of Data From SD Card */
            for ( block_counter = 0; block_counter < total_block ; block_counter++ ) {
                temp = 0;
                error_index = 0;
                memset(crc,0,sizeof(crc));
                memset(internal_buffer,0,sizeof(internal_buffer));

                /* Wait For Start Token From SD Card */
                while ( (response = spi_bb_recieve(&(sdcard->spi_conf))) != 0xFE ) {
                    if ( (++error_index) == 255 ) {
                        Skylo_Printf(SKYLO_ERR,"[SD] Not getting start token for sector - %d\n",
                                (sector + block_counter));
                        RELEASE_SDCARD_LOCK(sdcard->mutex);
                        CS_DEASSERT((sdcard->spi_conf));
                        spi_bb_recieve(&(sdcard->spi_conf));
                        free(read_buf);
                        free(temp_buf);
                        return QAPI_ERROR;
                    }
                }

                if ( spi_bus == SPI_BITBANG ) {
                    /* Read Data Block From SD Card */
                    for ( byte_counter = 0; byte_counter < BLOCK_SIZE; byte_counter++ ) {
                        temp = spi_bb_recieve(&(sdcard->spi_conf));
                        internal_buffer[byte_counter] = temp;
                        /* Skylo_Printf(SKYLO_DEBUG,"READ : %d : 0x%x\n",byte_counter,internal_buffer[byte_counter]); */
                        if ( data_counter < len )
                            data[data_counter] = temp;
                        data_counter++;
                    }
                } else {
#if 1
                    for ( byte_counter = 0; byte_counter < (BLOCK_SIZE/SD_SPI_CHUNK_SIZE); byte_counter++ ) {

                        spi_multi_receive(&(sdcard->spi_conf),read_buf, SD_SPI_CHUNK_SIZE);
                        memcpy(temp_buf+(byte_counter*SD_SPI_CHUNK_SIZE), read_buf, SD_SPI_CHUNK_SIZE);
                        memset(read_buf, 0 , SD_SPI_CHUNK_SIZE);
                    }
                    if ( (BLOCK_SIZE % SD_SPI_CHUNK_SIZE) != 0 ) {
                        spi_multi_receive(&(sdcard->spi_conf),read_buf, (BLOCK_SIZE % SD_SPI_CHUNK_SIZE));
                        memcpy(temp_buf+(byte_counter*SD_SPI_CHUNK_SIZE),read_buf, (BLOCK_SIZE % SD_SPI_CHUNK_SIZE));
                    }
                    memcpy(internal_buffer, temp_buf, BLOCK_SIZE);
                    memcpy(data, temp_buf, BLOCK_SIZE);

                    free(read_buf);
                    free(temp_buf);
                }
#endif
                /* Read CRC From SD Card */
                crc[0] = spi_bb_recieve(&(sdcard->spi_conf));
                crc[1] = spi_bb_recieve(&(sdcard->spi_conf));

                /* Adjust CRC Read From SD Card */
                Data_CRC = ((uint16_t)(crc[0]*256) + crc[1]);

                /* Calculare CRC From Block Read From SD Card */
                Cal_CRC = crc16_ccitt(internal_buffer, BLOCK_SIZE, 0);
                //for (byte_counter = 0; byte_counter<BLOCK_SIZE; byte_counter++)
                //{
                //    Skylo_Printf(SKYLO_DEBUG,"data- %x\n", data[byte_counter]);
                //}

                if ( Cal_CRC != Data_CRC ) {
                    Skylo_Printf(SKYLO_ERR,"[SD] CRC not match for sector - %d\n", (block_counter + sector));
                    RELEASE_SDCARD_LOCK(sdcard->mutex);
                    CS_DEASSERT((sdcard->spi_conf));
                    spi_bb_recieve(&(sdcard->spi_conf));
                    return QAPI_ERROR;
                }
            }

            /* Send Stop Command To SD Card When Multiple Block Read Ends */
            if ( command == MULTI_BLOCK_READ ) {
                if ( (sdcard_command(sdcard,command_stop,0x00,R1)) != 0x00 ) {
                    CS_ASSERT((sdcard->spi_conf));
                    /* Wait Till SD Card Keeps MISO Line LOW */
                    while ( (sd_response_stop[0] = spi_bb_recieve(&(sdcard->spi_conf))) == 0x00 );
                }

            }
        } else {

            Skylo_Printf(SKYLO_ERR,"[SD] SPI CMD24 response error 0x%X\n",response);
            RELEASE_SDCARD_LOCK(sdcard->mutex);
            CS_DEASSERT((sdcard->spi_conf));
            spi_bb_recieve(&(sdcard->spi_conf));
            free(read_buf);
            free(temp_buf);
            return QAPI_ERROR;
        }

        /* Make SD Card Listening Ready For Next Command */
        spi_bb_recieve(&(sdcard->spi_conf));
        CS_DEASSERT((sdcard->spi_conf));
        spi_bb_recieve(&(sdcard->spi_conf));

        /* Calculate Execution Time For Buffer Read */
        speed_time[1] = sapp_get_time_in_msec();
        sdcard->rx_speed =  (1000 * (total_block*BLOCK_SIZE))/(speed_time[1] - speed_time[0]);
        sdcard->rx_bytes += (total_block*BLOCK_SIZE);
        //Skylo_Printf(SKYLO_DEBUG,"[SD] Total Bytes read : %d \t", sdcard->rx_bytes);
        //Skylo_Printf(SKYLO_DEBUG,"[SD] Bytes read : %d in %d ms @ %d Bps\n"
        //        ,(total_block*BLOCK_SIZE),(speed_time[1] - speed_time[0]),sdcard->rx_speed);

        //Skylo_Printf(SKYLO_DEBUG,"[SD] Read Done : %s\n",data);
        RELEASE_SDCARD_LOCK(sdcard->mutex);
    } else {
        Skylo_Printf(SKYLO_ERR,"[SD] Unable to take the lock\n");
        return QAPI_ERROR;
    }
    return QAPI_OK;
}

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
qapi_Status_t sdcard_erase(sdcard_conf_t *sdcard,uint32_t start_addr, uint32_t end_addr)
{
    if ( sdcard->status == SD_UNINITIALIZED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Need to initialize the sd card first\n");
        return QAPI_ERROR;
    }

    if ( sdcard-> status == SD_FAILED_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] SD card was not initialized and has failed\n");
        return QAPI_ERROR;
    }


    if ( TAKE_SDCARD_LOCK(sdcard->mutex) ) {
        /* Skylo_Printf(SKYLO_DEBUG,"[SD] Got lock in function : %s\n",__func__); */
        uint32_t sd_arguement_start = start_addr;
        uint32_t sd_arguement_end = end_addr;
        uint32_t sd_arguement_erase = 0;
        uint8_t response = 0;

        /* Tell SD Card From Which Sector To Start Erase */
        if ( (response = sdcard_command(sdcard,ERASE_START,sd_arguement_start,R1)) != 0x00 ) {
            Skylo_Printf(SKYLO_ERR,"[SD] Wrong response received for start command -32 - %x\n", response);
            RELEASE_SDCARD_LOCK(sdcard->mutex);
            return QAPI_ERROR;
        }

        /* Tell SD Card At Which Sector To End Erase */
        if ( (response = sdcard_command(sdcard,ERASE_END,sd_arguement_end,R1)) != 0x00 ) {
            Skylo_Printf(SKYLO_ERR,"[SD] Wrong response received for end command -33 - %x\n", response);
            RELEASE_SDCARD_LOCK(sdcard->mutex);
            return QAPI_ERROR;
        }

        /* Tell SD Card To Initiate Block Erase */
        if ( (response = sdcard_command(sdcard,ERASE,sd_arguement_erase,R1)) != 0x00 ) {
            Skylo_Printf(SKYLO_ERR,"[SD] Wrong response received for erase command -38 - %x\n", response);
            RELEASE_SDCARD_LOCK(sdcard->mutex);
            return QAPI_ERROR;
        }

        CS_ASSERT((sdcard->spi_conf));

        /* Wait Till SD Card Keeps MISO Line LOW */
        while ( (response = spi_bb_recieve(&(sdcard->spi_conf))) == 0x00 );

        spi_bb_recieve(&(sdcard->spi_conf));
        CS_DEASSERT((sdcard->spi_conf));
        spi_bb_recieve(&(sdcard->spi_conf));

        /* speed_time[1] = sapp_get_time_in_msec(); */

        /* Skylo_Printf(SKYLO_DEBUG,"Erase %d sectors in %d ms time\n", */
        /* end_addr - start_addr,(speed_time[1] - speed_time[0])); */

        RELEASE_SDCARD_LOCK(sdcard->mutex);
    } else {
        Skylo_Printf(SKYLO_ERR,"[SD] Unable to take the SD lock\n");
        return QAPI_ERROR;
    }
    return QAPI_OK;
}

/**
 * @function    : Function to print the string for the response recieved if it has any errors.
 * @brief       : Functin to print string for response.
 * @param[in]   : response recieved.
 * @return      : None.
 * @retval      : None.
 */
void sdcard_response_string(uint8_t response)
{
#if 0
    uint8_t bit;

    for ( bit = 0; bit < 8;bit++ ) {
       if ( response & (1<<bit )) {
           Skylo_Printf(SKYLO_ERR,"[SD] %d Bit of %d Response : %s\n",bit,response);
       }
    }
    Skylo_Printf(SKYLO_DEBUG,"[SD] All errors have been printed\n");

#endif
}

/**
 * @function    : Generate CRC table for CRC7 in global array
 * @brief       : Generate CRC table for CRC7 in global array
 */
void GenerateCRCTable(void)
{
    // int i, j;

    // for ( i = 0; i < 256; i++ ) {
    //     CRCTable[i] = (i & 0x80) ? i ^ CRCPoly : i;
    //     for ( j = 1; j < 8; j++ ) {
    //         CRCTable[i] <<= 1;
    //         if ( CRCTable[i] & 0x80 )
    //             CRCTable[i] ^= CRCPoly;
    //     }
    // }

    int byt,bit;
    uint8_t crc = 0;

    /* Generate CRC7 table */
    for ( byt = 0U; byt < 256U; byt ++ ) {

        crc = byt;
        if ( (crc & 0x80U) != 0U )
            crc ^= 0x89U;

        for ( bit = 1U; bit < 8U; bit ++ ) {

            crc <<= 1;
            if ((crc & 0x80U) != 0U)
                crc ^= 0x89U;
        }
        CRCTable[byt] = (crc & 0x7FU);
    }
}

/**
 * @function    : CRCAdd
 * @brief       : Adds a message byte to the current CRC-7 to get a the new CRC-7
 * @param[in]   : CRC current CRC-7
 * @param[in]   : message_byte add to create new CRC - 7
 * @return      : New generated CRC-7
 * @retval      : New generated CRC-7
 */
uint8_t CRCAdd(uint8_t CRC, uint8_t message_byte)
{
    return CRCTable[(CRC << 1) ^ message_byte];
}

/**
 * @function    : getCRC
 * @brief       : Returns the CRC-7 for command and the arguement given.
 * @param[in]   : command to get CRC-7
 * @param[in]   : arguement of that command to create CRC - 7
 * @return      : New generated CRC-7
 * @retval      : New generated CRC-7
 */
uint8_t getCRC(uint8_t command,uint32_t arguement)
{
    char message[5];
    int i;
    unsigned char CRC = 0;

    message[0] = command;
    message[1] = arguement >> (3*8) & 0xFF;
    message[2] = arguement >> (2*8) & 0xFF;
    message[3] = arguement >> (1*8) & 0xFF;
    message[4] = arguement >> (0*8) & 0xFF;

    for (i = 0; i < 5; i++)
        CRC = CRCAdd(CRC, message[i]);

    return CRC;
}

/**
 * @function    : crc16_ccitt
 * @brief       : Function to generate CRC16-CCITT checksum
 * @param[in]   : block of data to be calculated for checksum
 * @param[in]   : blockLength length of data
 * @param[in]   : starting value of crc table (0)
 * @return      : New generated CRC16-CCITT
 * @retval      : New generated CRC16-CCITT
 */
unsigned short crc16_ccitt(const unsigned char block[], unsigned int blockLength, unsigned short crc)
{
    unsigned int i;

    for ( i=0U; i<blockLength; i++ ) {
        unsigned short tmp = (crc >> 8) ^ (unsigned short) block[i];
        crc = ((unsigned short)(crc << 8U)) ^ crc16_ccitt_table[tmp];
    }
    return crc;
}

/**
 * @function  : Function to set intrrupt for sdcard detection.
 * @brief     : Function to set intrrupt for sdcard detection.
 * @param[in] : NONE
 * @return    : On Success Returns QAPI_OK, On Failure returns Error Code.
 * @retval    : QAPI_OK on success.
 * @retval    : @see GPIO Error Codes
 */
qapi_Status_t sd_card_detect_init(void)
{
    qapi_Status_t status = QAPI_OK;
    qapi_GPIO_Value_t value;

    /* SD_DETECT_INT_PIN(GPIO_50) By Default Sets INT1 PIN With Active High with dual Edge interrupt */

    //Skylo_Printf(SKYLO_ERR,"[SD] Before gpio_config\n");
    if ( QAPI_OK != (status = gpio_config(&gpio_global[GLBL_GPIO_SDCARD_INT_PIN_E])) ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Unable to config SD Detect Pin!(E%d)\n", status);
        return status;
    }

    /* Initialize Event For GPIO External Interrupt. */
    if ( 0 != (status = (qurt_signal_init(&(gpio_global[GLBL_GPIO_SDCARD_INT_PIN_E].event)))) ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Not Able To Initialize External Interrupt On SD Detect Pin %d(E%d)\n",
                GLBL_GPIO_SDCARD_INT_PIN_E, status);
        gpio_release(&gpio_global[GLBL_GPIO_SDCARD_INT_PIN_E]);
        return status;
    }

    //Skylo_Printf(SKYLO_ERR,"[SD] After gpio_config\n");
    //sd_detect_gpio.cb = (qapi_GPIOINT_CB_t)sd_detect_callback;

    if ( QAPI_OK != (status = gpio_reg_interrupt(&gpio_global[GLBL_GPIO_SDCARD_INT_PIN_E])) ) {
        Skylo_Printf(SKYLO_ERR,"[SD] Not Able To Register External Interrupt On SD Detect Pin %d(E%d)\n",status);
        gpio_release(&gpio_global[GLBL_GPIO_SDCARD_INT_PIN_E]);
        return status;
    }

    //Skylo_Printf(SKYLO_ERR,"[SD] After gpio interrupt\n");
    value = gpio_read(&gpio_global[GLBL_GPIO_SDCARD_INT_PIN_E]);

    //Skylo_Printf(SKYLO_ERR,"[SD] After gpio read\n");
    if ( value == QAPI_GPIO_HIGH_VALUE_E ) {
        Skylo_Printf(SKYLO_ERR,"[SD] SD Card Is Not Found ...!!!\n");
        sdcard_conf.sd_detect_flag = SD_REMOVED;
        return QAPI_OK;
    }

    if ( value == QAPI_GPIO_LOW_VALUE_E ) {
        Skylo_Printf(SKYLO_INFO,"[SD] SDCARD Detected ...!!!\n");
        sdcard_conf.sd_detect_flag = SD_DETECT;
        return QAPI_OK;
    }

    return QAPI_OK;
}

/**
 * @function    : SD_CARD detect call back function.
 * @brief       : SD_CARD detect call back function.
 * @return      : None.
 * @retval      : None.
 */
void sd_detect_callback(qapi_GPIOINT_Callback_Data_t event)
{
    qapi_GPIO_Value_t value;

    value = gpio_read(&gpio_global[GLBL_GPIO_SDCARD_INT_PIN_E]);

    /*  SD Card Is Not Present */
    if ( value == QAPI_GPIO_HIGH_VALUE_E ) {
        sdcard_conf.sd_detect_flag = SD_REMOVED;
        return;
    }

    /*  SD Card Is Detected */
    if (value == QAPI_GPIO_LOW_VALUE_E ) {
        sdcard_conf.sd_detect_flag = SD_DETECT;
        return;
    }
}

/**
 * @function    : Function to read sd card detection flag.
 * @brief       : Function to read sd card detection flag.
 * @return      : Success value of sd card initialize flag.
 * @retval      : 1 if sdcard detect
 * @retval      : 0 if sdcard removed
 */
int is_sd_card_available(void)
{
    return sdcard_conf.sd_detect_flag;
}
