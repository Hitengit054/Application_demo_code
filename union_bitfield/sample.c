
#include<stdio.h>
#include<stdint.h>

// 16 bit ( 2 byte of data)

//0x5454 = 0b0101 0100 0101 0100


struct bitData
{
    uint8_t bit_2 : 2;
    uint8_t bit_4 : 2;
    uint8_t bit_8 : 4;
    uint8_t bit_16 : 8;
};

typedef union u_bitData
{
    uint16_t u_value;
    struct bitData s_v;
}u;


int main()
{
    struct bitData p;
    uint16_t value = 0x5454;
    
    p.bit_2 = (uint8_t) (  (value) & 0x03 );
    p.bit_4= (uint8_t) (  (value >> 2 ) & 0x03 );
    p.bit_8 = (uint8_t) (  (value >> 4 ) & 0x0F );
    p.bit_16 = (uint8_t) (  (value >>  8 ) & 0xFF );


    printf("%#X\n", p.bit_2);
    printf("%#X\n", p.bit_4);
    printf("%#X\n", p.bit_8);
    printf("%#X\n", p.bit_16);

    u u_p;
    u_p.u_value = 0x5454;

    printf("%#X\n", u_p.s_v.bit_2);
    printf("%#X\n", u_p.s_v.bit_4);
    printf("%#X\n", u_p.s_v.bit_8);
    printf("%#X\n", u_p.s_v.bit_16);

}


