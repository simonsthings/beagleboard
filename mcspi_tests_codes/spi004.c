#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "ads1258.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
        perror(s);
        abort();
}

static const char *device = "/dev/spidev3.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 100000;
static uint16_t delay;
static int fd;

static void transfer(unsigned char *inbuf, unsigned char *outbuf, unsigned int length)
{
        int ret;
		int i;
		unsigned char uc1[256];
		unsigned char uc2[256];
		if(outbuf!=NULL) for(i=0;i<length;i++) uc1[i] = *outbuf++;
        struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)uc1,
                .rx_buf = (unsigned long)uc2,
                .len = length,
                .delay_usecs = delay,
                .speed_hz = speed,
                .bits_per_word = bits,
        };
		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
		//printf("ret: %d\n",ret);
		if (ret == -1) pabort("can't send spi message");
		if(inbuf!=NULL) for(i=0;i<length;i++) *inbuf++ = uc2[i];
}

// vorläufige Routinen für ADS1258
unsigned char ads_read_reg( int reg ){
	unsigned char tx[2];
	unsigned char rx[2];
	if(reg > REG_ID) reg = REG_ID;
	tx[0] = CMD_REG_READ_SINGLE | reg;
	transfer( rx, tx, 2 );
	return rx[1];
}

void ads_write_reg( unsigned char byte, int reg ){
	unsigned char tx[2];
	if( reg > REG_ID ) reg = REG_ID;
	tx[0] = CMD_REG_WRITE_SINGLE | reg;
	tx[1] = byte;
	transfer( (unsigned char*)NULL,tx,2 );
}

void ads_reset(){
	unsigned char *pc,tx = CMD_RESET;
	pc=&tx;
	transfer( (unsigned char*)NULL, pc,1 );
}

void ads_pulse_convert(unsigned char *data){	// unsigned char data[4]
	unsigned char tx[5];
	unsigned char rx[5];
	int i;
	tx[0] = CMD_PULSE_CONVERT;
	transfer( rx, tx, 1);
	usleep(1);
	tx[0]=0;
	transfer( rx, tx,4);	
	for( i=0; i<4;i++) *data++ = rx[i];
}

void ads_channel_read(unsigned char *data){	// unsigned char data[4]
	unsigned char tx[5];
	unsigned char rx[5];
	int i;
	tx[0] = CMD_CHANNEL_READ_COMMAND;
	transfer( rx, tx, 5);
	for( i=1; i<5; i++ ) *data++ = rx[i];
}

void print_usage(const char *prog)
{
        printf("Usage: %s [-DsbdlHOLC3]\n", prog);
        puts("  -D --device   device to use (default /dev/spidev1.1)\n"
             "  -s --speed    max speed (Hz)\n"
             "  -d --delay    delay (usec)\n"
             "  -b --bpw      bits per word \n"
             "  -l --loop     loopback\n"
             "  -H --cpha     clock phase\n"
             "  -O --cpol     clock polarity\n"
             "  -L --lsb      least significant bit first\n"
             "  -C --cs-high  chip select active high\n"
             "  -3 --3wire    SI/SO signals shared\n");
        exit(1);
}

void parse_opts(int argc, char *argv[])
{
        while (1) {
                static const struct option lopts[] = {
                        { "device",  1, 0, 'D' },
                        { "speed",   1, 0, 's' },
                        { "delay",   1, 0, 'd' },
                        { "bpw",     1, 0, 'b' },
                        { "loop",    0, 0, 'l' },
                        { "cpha",    0, 0, 'H' },
                        { "cpol",    0, 0, 'O' },
                        { "lsb",     0, 0, 'L' },
                        { "cs-high", 0, 0, 'C' },
                        { "3wire",   0, 0, '3' },
                        { NULL, 0, 0, 0 },
                };
                int c;
                c = getopt_long(argc, argv, "D:s:d:b:lHOLC3", lopts, NULL);
                if (c == -1) break;
                switch (c) {
                case 'D': device = optarg; break;
                case 's': speed = atoi(optarg); break;
                case 'd': delay = atoi(optarg); break;
                case 'b': bits = atoi(optarg); break;
                case 'l': mode |= SPI_LOOP; break;
                case 'H': mode |= SPI_CPHA; break;
                case 'O': mode |= SPI_CPOL; break;
                case 'L': mode |= SPI_LSB_FIRST; break;
                case 'C': mode |= SPI_CS_HIGH; break;
                case '3': mode |= SPI_3WIRE; break;
                default: print_usage(argv[0]); break;
                }
        }
}

int main(int argc, char *argv[])
{
        int ret = 0;
		int i,j;
	unsigned char ucdata[4];
	int iadval;
	unsigned char *puc;
        parse_opts(argc, argv);
        fd = open(device, O_RDWR);
        if (fd < 0) pabort("can't open device");
        // spi mode
        ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
        if (ret == -1) pabort("can't set spi mode");
        ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
        if (ret == -1) pabort("can't get spi mode");
        // bits per word
        ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        if (ret == -1) pabort("can't set bits per word");
        ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
        if (ret == -1) pabort("can't get bits per word");
        // max speed hz
        ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        if (ret == -1) pabort("can't set max speed hz");
        ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
        if (ret == -1) pabort("can't get max speed hz");
        printf("spi mode: %d\n", mode);
        printf("bits per word: %d\n", bits);
        printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
        //transfer(fd);
		//while(){
			ads_reset();
			ads_write_reg(0,REG_GPIOC);
			ads_write_reg(0x55,REG_GPIOD);
			for( i=0; i<10; i++ ) printf("%x ",(int)ads_read_reg( i));
			printf("\n");
	//	}
 		getchar();
		while(0xff){
			ads_pulse_convert(ucdata);
			if(ucdata[0]<0x88 || ucdata[0]>0x97) printf("%x\n",ucdata[0]);
			//printf("S: %x, \tH: %x, \tM: %x, \tL: %x\n",ucdata[0],ucdata[1],ucdata[2],ucdata[3]);
			usleep(200);
		}
       	close(fd);
        return ret;
}
