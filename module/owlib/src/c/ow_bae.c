/*
$Id$
    OWFS -- One-Wire filesystem
    OWHTTPD -- One-Wire Web Server
    Written 2003 Paul H Alfille
    email: palfille@earthlink.net
    Released under the GPL
    See the header file: ow.h for full attribution
    1wire/iButton system from Dallas Semiconductor
*/

/* General Device File format:
    This device file corresponds to a specific 1wire/iButton chip type
    ( or a closely related family of chips )

    The connection to the larger program is through the "device" data structure,
      which must be declared in the acompanying header file.

    The device structure holds the
      family code,
      name,
      device type (chip, interface or pseudo)
      number of properties,
      list of property structures, called "filetype".

    Each filetype structure holds the
      name,
      estimated length (in bytes),
      aggregate structure pointer,
      data format,
      read function,
      write funtion,
      generic data pointer

    The aggregate structure, is present for properties that several members
    (e.g. pages of memory or entries in a temperature log. It holds:
      number of elements
      whether the members are lettered or numbered
      whether the elements are stored together and split, or separately and joined
*/

/* Pascal Baerten's BAE device -- preliminary */

#include <config.h>
#include "owfs_config.h"
#include "ow_bae.h"

/* ------- Prototypes ----------- */

/* BAE */
READ_FUNCTION(FS_r_mem);
WRITE_FUNCTION(FS_w_mem);
WRITE_FUNCTION(FS_w_flash);
WRITE_FUNCTION(FS_w_extended);
WRITE_FUNCTION(FS_writebyte);

WRITE_FUNCTION(FS_w_date);
READ_FUNCTION(FS_r_date);
WRITE_FUNCTION(FS_w_counter);
READ_FUNCTION(FS_r_counter);

READ_FUNCTION(FS_version_state) ;
READ_FUNCTION(FS_version) ;
READ_FUNCTION(FS_version_device) ;
READ_FUNCTION(FS_version_bootstrap) ;

READ_FUNCTION(FS_type_state) ;
READ_FUNCTION(FS_localtype) ;
READ_FUNCTION(FS_type_device) ;
READ_FUNCTION(FS_type_chip) ;

/* ------- Structures ----------- */

#define _FC02_MEMORY_SIZE 192
#define _FC02_FUNCTION_FLASH_SIZE 4096

struct filetype BAE[] = {
	F_STANDARD,
	{"memory", _FC02_MEMORY_SIZE, NULL, ft_binary, fc_stable, FS_r_mem, FS_w_mem, NO_FILETYPE_DATA,},
	{"flash", _FC02_MEMORY_SIZE, NULL, ft_binary, fc_stable, NO_READ_FUNCTION, FS_w_flash, NO_FILETYPE_DATA,},
	{"command", 32, NULL, ft_binary, fc_stable, NO_READ_FUNCTION, FS_w_extended, NO_FILETYPE_DATA,},
  {"udate", PROPERTY_LENGTH_UNSIGNED, NON_AGGREGATE, ft_unsigned, fc_link, FS_r_counter, FS_w_counter, NO_FILETYPE_DATA,},
  {"date", PROPERTY_LENGTH_DATE, NON_AGGREGATE, ft_date, fc_second, FS_r_date, FS_w_date, NO_FILETYPE_DATA,},
  {"writebyte", PROPERTY_LENGTH_UNSIGNED, NON_AGGREGATE, ft_unsigned, fc_stable, NO_READ_FUNCTION, FS_writebyte, NO_FILETYPE_DATA, },
  {"versionstate", PROPERTY_LENGTH_HIDDEN, NON_AGGREGATE, ft_unsigned, fc_volatile, FS_version_state, NO_WRITE_FUNCTION, NO_FILETYPE_DATA, },
  {"version", 5, NON_AGGREGATE, ft_ascii, fc_link, FS_version, NO_WRITE_FUNCTION, NO_FILETYPE_DATA,},
  {"device_version", PROPERTY_LENGTH_UNSIGNED, NON_AGGREGATE, ft_unsigned, fc_link, FS_version_device, NO_WRITE_FUNCTION, NO_FILETYPE_DATA,},
  {"bootstrap_version", PROPERTY_LENGTH_UNSIGNED, NON_AGGREGATE, ft_unsigned, fc_link, FS_version_bootstrap, NO_WRITE_FUNCTION, NO_FILETYPE_DATA,},
  {"typestate", PROPERTY_LENGTH_HIDDEN, NON_AGGREGATE, ft_unsigned, fc_volatile, FS_type_state, NO_WRITE_FUNCTION, NO_FILETYPE_DATA, },
  {"localtype", 5, NON_AGGREGATE, ft_ascii, fc_link, FS_localtype, NO_WRITE_FUNCTION, NO_FILETYPE_DATA,},
  {"device_type", PROPERTY_LENGTH_UNSIGNED, NON_AGGREGATE, ft_unsigned, fc_link, FS_type_device, NO_WRITE_FUNCTION, NO_FILETYPE_DATA,},
  {"chip_type", PROPERTY_LENGTH_UNSIGNED, NON_AGGREGATE, ft_unsigned, fc_link, FS_type_chip, NO_WRITE_FUNCTION, NO_FILETYPE_DATA,},
};

DeviceEntryExtended(FC, BAE, DEV_resume | DEV_alarm );

/* BAE command codes */
#define _1W_ERASE_FIRMWARE 0xBB
#define _1W_FLASH_FIRMWARE 0xBA

#define _1W_READ_VERSION 0x11
#define _1W_READ_TYPE 0x12
#define _1W_EXTENDED_COMMAND 0x13
#define _1W_READ_BLOCK_WITH_LEN 0x14
#define _1W_WRITE_BLOCK_WITH_LEN 0x15

#define _1W_CONFIRM_WRITE 0xBC

/* BAE registers */
#define _FC02_ADC  50    /* u8 */
#define _FC02_ADCAN  24    /* u16 */
#define _FC02_ADCAP  22    /* u16 */
#define _FC02_ADCC  2    /* u8 */
#define _FC02_ADCTOTN  36    /* u32 */
#define _FC02_ADCTOTP  32    /* u32 */
#define _FC02_ALAN  66    /* u16 */
#define _FC02_ALAP  64    /* u16 */
#define _FC02_ALARM  52    /* u8 */
#define _FC02_ALARMC  6    /* u8 */
#define _FC02_ALCD  68    /* u16 */
#define _FC02_ALCT  70    /* u32 */
#define _FC02_ALRT  74    /* u32 */
#define _FC02_CNT  51    /* u8 */
#define _FC02_CNTC  3    /* u8 */
#define _FC02_COUNT  44    /* u32 */
#define _FC02_DUTY1  14    /* u16 */
#define _FC02_DUTY2  16    /* u16 */
#define _FC02_DUTY3  18    /* u16 */
#define _FC02_DUTY4  20    /* u16 */
#define _FC02_MAXAN  28    /* u16 */
#define _FC02_MAXAP  26    /* u16 */
#define _FC02_MOD1  10    /* u16 */
#define _FC02_MOD2  12    /* u16 */
#define _FC02_MSIZE  0    /* u16 */
#define _FC02_OUT  48    /* u8 */
#define _FC02_OUTC  4    /* u8 */
#define _FC02_PIO  49    /* u8 */
#define _FC02_PIOC  5    /* u8 */
#define _FC02_RTC  40    /* u32 */
#define _FC02_RTCC  7    /* u8 */
#define _FC02_TPM1C  8    /* u8 */
#define _FC02_TPM2C  9    /* u8 */

/* Note, read and write page sizes are differnt -- 32 bytes for write and no page boundary. 8 Bytes for read */

/* ------- Functions ------------ */

static int OW_w_mem(BYTE * data, size_t size, off_t offset, struct parsedname *pn);
static int OW_w_extended(BYTE * data, size_t size, UINT * return_code, struct parsedname *pn);
static int OW_version( UINT * version, struct parsedname * pn ) ;
static int OW_type( UINT * localtype, struct parsedname * pn ) ;
static int OW_r_mem(BYTE *bytes, size_t size, off_t offset, struct parsedname * pn);

static int OW_initiate_flash(BYTE * data, struct parsedname *pn);
static int OW_write_flash(BYTE * data, struct parsedname *pn);

static uint16_t BAE_uint16(BYTE * p);
static uint32_t BAE_uint32(BYTE * p);
static void BAE_uint16_to_bytes( uint16_t num, unsigned char * p );
static void BAE_uint32_to_bytes( uint32_t num, unsigned char * p );

/* BAE memory functions */
static int FS_r_mem(struct one_wire_query *owq)
{
	if (OW_r_mem( (BYTE *) OWQ_buffer(owq), OWQ_size(owq), OWQ_offset(owq), PN(owq))) {
		return -EINVAL;
	}
	OWQ_length(owq) = OWQ_size(owq) ;
	return 0;
}

static int FS_w_mem(struct one_wire_query *owq)
{
	size_t pagesize = 32; // different from read page size
	size_t remain = OWQ_size(owq) ;
	BYTE * data = (BYTE *) OWQ_buffer(owq) ;
	off_t location = OWQ_offset(owq) ;
	
	// Write data 32 bytes at a time ignoring page boundaries
	while ( remain > 0 ) {
		size_t bolus = remain ;
		if ( bolus > pagesize ) {
			bolus = pagesize ;
		}
		if ( OW_w_mem(data, bolus, location, PN(owq)  ) ) {
			return -EINVAL ;
		}
		remain -= bolus ;
		data += bolus ;
		location += bolus ;
	}
	
	return 0;
}

/* BAE flash functions */
static int FS_w_flash(struct one_wire_query *owq)
{
	struct parsedname * pn = PN(owq) ;
	BYTE * rom_image = (BYTE *) OWQ_buffer(owq) ;

	size_t rom_offset ;

	// test size
	if ( OWQ_size(owq) != _FC02_FUNCTION_FLASH_SIZE ) {
		LEVEL_DEBUG("Flash size of %d is not the expected %d.\n", (int)OWQ_size(owq), (int)_FC02_FUNCTION_FLASH_SIZE ) ;
		return -EBADMSG ;
	}

	// start flash process
	if ( OW_initiate_flash( rom_image, pn ) ) {
		LEVEL_DEBUG("Unsuccessful flash initialization\n");
		return -EFAULT ;
	}

	// loop though pages, up to 5 attempts for each page
	for ( rom_offset=0 ; rom_offset<_FC02_FUNCTION_FLASH_SIZE ; rom_offset += 32 ) {
		int tries = 0 ;
		LEVEL_DEBUG("Flash up to %d bytes.\n",rom_offset);
		while ( OW_write_flash( &rom_image[rom_offset], pn ) ) {
			++tries ;
			if ( tries > 4 ) {
				LEVEL_DEBUG( "Too many attempts writing flash at offset %d.\n", rom_offset ) ;
				return -EIO ;
			}
		}
	}
	
	LEVEL_DEBUG("Successfully flashed full rom.\n") ;
	return 0;
}

/* BAE flash/counter functions */
static int FS_r_date(struct one_wire_query *owq)
{
	UINT counter;
	BYTE data[4] ; //register representation
	if (OW_r_mem( data, 4, _FC02_RTC, PN(owq))) {
		return -EINVAL;
	}
	counter = BAE_uint32(data) ;
	OWQ_D(owq) = (_DATE) counter ;
	LEVEL_DEBUG("Counter Data: %.2X %.2X %.2X %.2X (%Ld) \n", data[0], data[1], data[2], data[3],  counter );
	return 0;
}

static int FS_w_date(struct one_wire_query *owq)
{
	BYTE data[4] ; //register representation

	BAE_uint32_to_bytes( OWQ_D(owq), data ) ;
	if (OW_w_mem( data, 4, _FC02_RTC, PN(owq))) {
		return -EINVAL;
	}
	return 0;
}

int FS_r_counter(struct one_wire_query *owq)
{
	_DATE D ;
	
	if ( FS_r_sibling_D( &D, "date", owq ) ) {
		return -EINVAL ;
	}
	
	OWQ_U(owq) = (UINT) D ;
	return 0;
}

static int FS_w_counter(struct one_wire_query *owq)
{
	_DATE D = (_DATE) OWQ_D(owq);
	
	return FS_w_sibling_D( D, "date", owq ) ;
}

/* BAE extended command */
static int FS_w_extended(struct one_wire_query *owq)
{
	UINT ret ;
	// Write data 32 bytes maximum
	if ( OW_w_extended( (BYTE *) OWQ_buffer(owq), OWQ_size(owq), &ret, PN(owq)  ) ) {
		return -EINVAL ;
	}

	if ( ret == 0xFFFF ) {
		LEVEL_DEBUG("Bad return code for extended command\n") ;
		return -EINVAL ;
	}
	
	return 0;
}

static int FS_writebyte(struct one_wire_query *owq)
{
	off_t location = OWQ_U(owq)>>8 ;
	BYTE data = OWQ_U(owq) & 0xFF ;
	
	// Write 1 byte ,
	if ( OW_w_mem( &data, 1, location, PN(owq)  ) ) {
		return -EINVAL ;
	}
	
	return 0;
}

/* BAE version */
static int FS_version_state(struct one_wire_query *owq)
{
	UINT v ;
	if ( OW_version( &v, PN(owq) ) ) {
		return -EINVAL ;
	}
	OWQ_U(owq) = v ;
	return 0 ;
}

static int FS_version(struct one_wire_query *owq)
{
	char v[6];
	UINT version ;
	
	if ( FS_r_sibling_U( &version, "versionstate", owq ) ) {
		return -EINVAL ;
	}
	
	UCLIBCLOCK;
	snprintf(v,6,"%.2X.%.2X",version&0xFF, (version>>8)&0xFF);
	UCLIBCUNLOCK;
	
	return Fowq_output_offset_and_size(v, 5, owq);
}

static int FS_version_device(struct one_wire_query *owq)
{
	UINT version ;
	
	if ( FS_r_sibling_U( &version, "versionstate", owq ) ) {
		return -EINVAL ;
	}
	
	OWQ_U(owq) = version & 0xFF ;
	
	return 0 ;
}

static int FS_version_bootstrap(struct one_wire_query *owq)
{
	UINT version ;
	
	if ( FS_r_sibling_U( &version, "versionstate", owq ) ) {
		return -EINVAL ;
	}
	
	OWQ_U(owq) = (version>>8) & 0xFF ;
	
	return 0 ;
}

/* BAE type */
static int FS_type_state(struct one_wire_query *owq)
{
	UINT t ;
	if ( OW_type( &t, PN(owq) ) ) {
		return -EINVAL ;
	}
	OWQ_U(owq) = t ;
	return 0 ;
}

static int FS_localtype(struct one_wire_query *owq)
{
	char t[6];
	UINT localtype ;
	
	if ( FS_r_sibling_U( &localtype, "typestate", owq ) ) {
		return -EINVAL ;
	}
	
	UCLIBCLOCK;
	snprintf(t,6,"%.2X.%.2X",localtype&0xFF, (localtype>>8)&0xFF);
	UCLIBCUNLOCK;
	
	return Fowq_output_offset_and_size(t, 5, owq);
}

static int FS_type_device(struct one_wire_query *owq)
{
	UINT t ;
	
	if ( FS_r_sibling_U( &t, "typestate", owq ) ) {
		return -EINVAL ;
	}
	
	OWQ_U(owq) = t & 0xFF ;
	
	return 0 ;
}

static int FS_type_chip(struct one_wire_query *owq)
{
	UINT t ;
	
	if ( FS_r_sibling_U( &t, "typestate", owq ) ) {
		return -EINVAL ;
	}
	
	OWQ_U(owq) = (t>>8) & 0xFF ;
	
	return 0 ;
}

/* Lower level functions */
static int OW_w_mem(BYTE * data, size_t size, off_t offset, struct parsedname *pn)
{
	BYTE p[1 + 2 + 1 + 32 + 2] = { _1W_WRITE_BLOCK_WITH_LEN, LOW_HIGH_ADDRESS(offset), BYTE_MASK(size), };
	BYTE q[] = { _1W_CONFIRM_WRITE, } ;
	struct transaction_log t[] = {
		TRXN_START,
		TRXN_WR_CRC16(p, 1+ 2 + 1 + size, 0),
		TRXN_WRITE1(q),
		TRXN_END,
	};
	
	/* Copy to scratchpad */
	memcpy(&p[4], data, size);
	
	return BUS_transaction(t, pn) ;
}

static int OW_w_extended(BYTE * data, size_t size, UINT * return_code, struct parsedname *pn)
{
	BYTE p[1 + 1 + 32 + 2] = { _1W_EXTENDED_COMMAND, BYTE_MASK(size), };
	BYTE q[] = { _1W_CONFIRM_WRITE, } ;
	BYTE r[2] ;
	int ret ;
	struct transaction_log t[] = {
		TRXN_START,
		TRXN_WR_CRC16(p, 1+ 1 + size, 0),
		TRXN_WRITE1(q),
		TRXN_DELAY(10),
		TRXN_READ2(r),
		TRXN_END,
	};
	
	/* Copy to scratchpad */
	memcpy(&p[4], data, size);
	
	ret = BUS_transaction(t, pn) ;
	if (ret) {
		return 1 ;
	}
	return_code[0] = (r[1]<<8) + r[0] ;
	return 0 ;
}

//read bytes[size] from position
static int OW_r_mem(BYTE * data, size_t size, off_t offset, struct parsedname * pn)
{
	BYTE p[1+2+1 + size + 2] ;
	struct transaction_log t[] = {
		TRXN_START,
		TRXN_WR_CRC16(p, 4, size),
		TRXN_END,
	};
	
	p[0] = _1W_READ_BLOCK_WITH_LEN ;
	p[1] = BYTE_MASK(offset) ;
	p[2] = BYTE_MASK(offset>>8) ; ;
	p[3] = BYTE_MASK(size) ;
	
	if (BUS_transaction(t, pn)) {
		return 1;
	}
	memcpy(data, &p[4], size);
	return 0;
}

static int OW_version( UINT * version, struct parsedname * pn )
{
	BYTE p[5] = { _1W_READ_VERSION, } ;
	struct transaction_log t[] = {
		TRXN_START,
		TRXN_WR_CRC16(p, 1, 2),
		TRXN_END,
	} ;
		
	if (BUS_transaction(t, pn)) {
		return 1;
	}

	version[0] = BAE_uint16(&p[1]) ;
	return 0 ;
};

static int OW_type( UINT * localtype, struct parsedname * pn )
{
	BYTE p[5] = { _1W_READ_TYPE, } ;
	struct transaction_log t[] = {
		TRXN_START,
		TRXN_WR_CRC16(p, 1, 2),
		TRXN_END,
	} ;
	
	if (BUS_transaction(t, pn)) {
		return 1;
	}
	
	localtype[0] = BAE_uint16(&p[1]) ;
	return 0 ;
};

/* Routines to play with byte <-> integer */

static uint16_t BAE_uint16(BYTE * p)
{
	return (((uint16_t) p[0]) << 8) | ((uint16_t) p[1]);
}

static uint32_t BAE_uint32(BYTE * p)
{
	return (((uint32_t) p[0]) << 24) | (((uint32_t) p[1]) << 16) | (((uint32_t) p[2]) << 8) | ((uint32_t) p[3]);
}

static void BAE_uint16_to_bytes( uint16_t num, unsigned char * p )
{
	p[1] = num&0xFF ;
	p[0] = (num>>8)&0xFF ;
}

static void BAE_uint32_to_bytes( uint32_t num, unsigned char * p )
{
	p[3] = num&0xFF ;
	p[2] = (num>>8)&0xFF ;
	p[1] = (num>>16)&0xFF ;
	p[0] = (num>>24)&0xFF ;
}

static int OW_initiate_flash( BYTE * data, struct parsedname * pn )
{
	BYTE p[1+1+1+4+2] = { _1W_EXTENDED_COMMAND, _1W_ERASE_FIRMWARE, 4, } ;
	BYTE q[] = { _1W_CONFIRM_WRITE, } ;
	struct transaction_log t[] = {
		TRXN_START,
		TRXN_WR_CRC16(p, 1+1+1+4, 0),
		TRXN_WRITE1(q),
		TRXN_DELAY(100),
		TRXN_END,
	} ;

	memcpy(&p[3], data, 4 ) ;
	if (BUS_transaction(t, pn)) {
		return 1;
	}
	return 0 ;
}

static int OW_write_flash( BYTE * data, struct parsedname * pn )
{
	BYTE p[1+1+1+32+2] = { _1W_EXTENDED_COMMAND, _1W_FLASH_FIRMWARE, 4,  } ;
	BYTE q[] = { _1W_CONFIRM_WRITE, } ;
	struct transaction_log t[] = {
		TRXN_START,
		TRXN_WR_CRC16(p, 1+1+1+32, 0),
		TRXN_WRITE1(q),
		TRXN_END,
	} ;
	
	memcpy(&p[3], data, 32 ) ;
	if (BUS_transaction(t, pn)) {
		return 1;
	}
	return 0 ;
}
