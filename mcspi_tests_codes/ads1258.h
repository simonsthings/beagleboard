// ADS1258 Registers
#define REG_CONFIG0 0
#define REG_CONFIG1 1
#define REG_MUXSCH	2
#define REG_MUXDIF	3
#define REG_MUXSG0	4
#define REG_MUXSG1	5
#define REG_SYSRED	6
#define REG_GPIOC	7
#define REG_GPIOD	8
#define REG_ID		9

// ADS1258 Commands
#define CMD_CHANNEL_READ_DIRECT 	0x10
#define CMD_CHANNEL_READ_COMMAND	0x30
#define CMD_REG_READ_SINGLE			0x40
#define CMD_REG_READ_MULTI			0x50
#define CMD_REG_WRITE_SINGLE		0x60
#define CMD_REG_WRITE_MULTI			0x70
#define CMD_PULSE_CONVERT			0x80
#define CMD_RESET					0xC0

// Status Byte's Bits
#define STATUS_CHANNEL	0x1F
#define STATUS_SUPPLY	0x20
#define STATUS_OVF		0x40
#define STATUS_NEW		0x80

