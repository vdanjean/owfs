#include <config.h>
#include "owfs_config.h"
#include "ow.h"
#include "ow_counters.h"
#include "ow_connection.h"

#define EtherWeather_COMMAND_RESET	'R'
#define EtherWeather_COMMAND_ACCEL	'A'
#define EtherWeather_COMMAND_BYTES	'B'
#define EtherWeather_COMMAND_BITS		'b'
#define EtherWeather_COMMAND_POWER	'P'

int EtherWeather_detect(struct connection_in *in);

static int EtherWeather_command(struct connection_in *in, char command, int datalen, const BYTE * idata, BYTE * odata) {
	ssize_t res;
	ssize_t left = datalen;
	BYTE * packet;

	struct timeval tvnet = { 0, 200000, };

	// The packet's length field includes the command byte.
	packet = malloc(datalen + 2);
	packet[0] = datalen + 1;
	packet[1] = command;
	memcpy(packet + 2, idata, datalen);

	left = datalen + 2;
/*
	char prologue[2];
	prologue[0] = datalen + 1;
	prologue[1] = command;

	res = write(in->fd, prologue, 2);
	if (res < 1) {
		ERROR_CONNECT("Trouble writing data to EtherWeather: %s\n",
			SAFESTRING(in->name));
		return -EIO;
	}
*/
        while (left > 0) {
		res = write(in->fd, &packet[datalen + 2 - left], left);
		if (res < 0) {
			if (errno == EINTR) {
				STAT_ADD1_BUS(BUS_write_interrupt_errors, in);
				continue;
			}
			ERROR_CONNECT("Trouble writing data to EtherWeather: %s\n",
				SAFESTRING(in->name));
			break;
		}
		left -= res;
        }

	tcdrain(in->fd);
	gettimeofday(&(in->bus_write_time), NULL);

	if (left > 0) {
		STAT_ADD1_BUS(BUS_write_errors, in);
		free(packet);
		return -EIO;
	}

	// Allow extra time for powered bytes
	if (command == 'P') {
		tvnet.tv_sec += 2;
	}

	// Read the response header
	if (tcp_read(in->fd, packet, 2, &tvnet) != 2) {
		LEVEL_CONNECT("EtherWeather_command header read error\n");
		free(packet);
		return -EIO;
        }

	// Make sure it was echoed properly
	if (packet[0] != (datalen + 1) || packet[1] != command) {
		LEVEL_CONNECT("EtherWeather_command invalid header\n");
		free(packet);
		return -EIO;
	}

	// Then read any data
	if (datalen > 0) {
		if (tcp_read(in->fd, odata, datalen, &tvnet) != (ssize_t) datalen) {
			LEVEL_CONNECT("EtherWeather_command data read error\n");
			free(packet);
			return -EIO;
        	}
	}

	free(packet);
	return 0;
}

static int EtherWeather_sendback_data(const BYTE * data, BYTE * resp, const size_t size, const struct parsedname *pn) {

	if (EtherWeather_command(pn->in, EtherWeather_COMMAND_BYTES, size, data, resp)) {
		return -EIO;
	}

	return 0;
}

static int EtherWeather_sendback_bits(const BYTE * data, BYTE * resp, const size_t size, const struct parsedname *pn) {

	if (EtherWeather_command(pn->in, EtherWeather_COMMAND_BITS, size, data, resp)) {
		return -EIO;
	}

	return 0;
}

static int EtherWeather_next_both(struct device_search *ds, const struct parsedname *pn) {
	BYTE sendbuf[9];

	// if the last call was not the last one
	if (!pn->in->AnyDevices)
		ds->LastDevice = 1;
	if (ds->LastDevice)
		return -ENODEV;

	memcpy(sendbuf, ds->sn, 8);
	if (ds->LastDiscrepancy == -1) {
		sendbuf[8] = 0x40;
	} else {
		sendbuf[8] = ds->LastDiscrepancy;
	}
	if (ds->search == 0xEC) sendbuf[8] |= 0x80;

	if (EtherWeather_command(pn->in, EtherWeather_COMMAND_ACCEL, 9, sendbuf, sendbuf)) {
		return -EIO;
	}

	if (sendbuf[8] == 0xFF) {
		/* No devices */
		return -ENODEV;
	}

	memcpy(ds->sn, sendbuf, 8);

	if (CRC8(ds->sn, 8) || (ds->sn[0] == 0)) {
		/* Bus error */
		return -EIO;
	}

	if ((ds->sn[0] & 0x7F) == 0x04) {
		/* We found a DS1994/DS2404 which require longer delays */
		pn->in->ds2404_compliance = 1;
	}

	/* 0xFE indicates no discrepancies */
	ds->LastDiscrepancy = sendbuf[8];
	if (ds->LastDiscrepancy == 0xFE) ds->LastDiscrepancy = -1;

	ds->LastDevice = (sendbuf[8] == 0xFE);

	LEVEL_DEBUG("EtherWeather_next_both SN found: " SNformat "\n", SNvar(ds->sn));

	return 0;
}

static int EtherWeather_PowerByte(const BYTE byte, BYTE * resp, const UINT delay, const struct parsedname *pn) {
	BYTE pbbuf[2];

	/* PowerByte command specifies delay in 500ms ticks, not milliseconds */
	pbbuf[0] = (delay + 499) / 500;
	pbbuf[1] = byte;
	LEVEL_DEBUG("SPU: %d %d\n", pbbuf[0], pbbuf[1]);
	if (EtherWeather_command(pn->in, EtherWeather_COMMAND_POWER, 2, pbbuf, pbbuf)) {
		return -EIO;
	}

	*resp = pbbuf[1];
	return 0;
}


static void EtherWeather_close(struct connection_in *in) {
	if (in->fd >= 0) {
		close(in->fd);
		in->fd = -1;
	}
	FreeClientAddr(in);
}

static int EtherWeather_reset(const struct parsedname *pn) {
	if (EtherWeather_command(pn->in, EtherWeather_COMMAND_RESET, 0, NULL, NULL)) {
		STAT_ADD1_BUS(BUS_reset_errors, pn->in);
		return -EIO;
	}

	return 0;
}

static void EtherWeather_setroutines(struct interface_routines *f) {
	f->detect = EtherWeather_detect;
	f->reset = EtherWeather_reset;
	f->next_both = EtherWeather_next_both;
//    f->overdrive = ;
//    f->testoverdrive = ;
	f->PowerByte = EtherWeather_PowerByte;
//    f->ProgramPulse = ;
	f->sendback_data = EtherWeather_sendback_data;
	f->sendback_bits = EtherWeather_sendback_bits;
	f->select = NULL;
	f->reconnect = NULL;
	f->close = EtherWeather_close;
	f->transaction = NULL;
	f->flags =
		ADAP_FLAG_overdrive | ADAP_FLAG_dirgulp | ADAP_FLAG_2409path;
}

int EtherWeather_detect(struct connection_in *in) {

	struct parsedname pn;

	FS_ParsedName(NULL, &pn); // minimal parsename -- no destroy needed
	pn.in = in;

	LEVEL_CONNECT("Connecting to EtherWeather\n");

	/* Set up low-level routines */
	EtherWeather_setroutines(&(in->iroutines));

	if (in->name == NULL)
		return -1;

	/* Add the port if it isn't there already */
	if (strchr(in->name, ':') == NULL) {
		ASCII *temp = realloc(in->name, strlen(in->name) + 3);
		if (temp == NULL)
			return -ENOMEM;
		in->name = temp;
		strcat(in->name, ":15862");
	}

	if (ClientAddr(in->name, in))
		return -1;
	if ((pn.in->fd = ClientConnect(in)) < 0)
		return -EIO;


	/* TODO: probe version, and confirm that it's actually an EtherWeather */

	LEVEL_CONNECT("Connected to EtherWeather at %s\n", in->name);

	in->Adapter = adapter_EtherWeather;

	in->adapter_name = "EtherWeather";
	in->busmode = bus_etherweather;
	in->AnyDevices = 1;

	return 0;
}
