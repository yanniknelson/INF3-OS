/*
 * CMOS Real-time Clock
 * SKELETON IMPLEMENTATION -- TO BE FILLED IN FOR TASK (1)
 */

/*
 * STUDENT NUMBER: s
 */
#include <infos/drivers/timer/rtc.h>
#include <infos/util/lock.h>
#include <arch/x86/pio.h>

using namespace infos::drivers;
using namespace infos::drivers::timer;
using namespace infos::util;
using namespace infos::arch::x86;

class CMOSRTC : public RTC
{

private:
	//defines for clarity
	//port to specify CMOS offset
	#define CMOSOFFSET 0x70
	//port to get CMOS data
	#define CMOSDATA 0x71

	//offsets for cmos data
	#define secs 0x00
	#define mins 0x02
	#define hrs 0x04
	#define day 0x07
	#define mnth 0x08
	#define yr 0x09

	/**
	 * Converts 8-bit BCD to unsigned short
	 * @param BCD The BCD to be converted
	 * @return Unsigned short conversion of passed in 8-bit BCD digit
	 */
	unsigned short BCDtoShort(uint8_t BCD)
	{
		//add the value of top 4 bits of the BCD * 10 to the value of the bottom 4 BCD bits
		return (BCD & 0x0F) + ((BCD >> 4) & 0x0F) * 10;
	}

	/**
	 * Gets the data from the CMOS at the passed in offset
	 * @param offset The offset of the desired data
	 * @return The date stored in the CMOS at the provided offset 
	 */
	uint8_t readCMOS(uint8_t offset){
		//specify CMOS offset
		__outb(CMOSOFFSET, offset);
		//get and return the data from that offset
		return __inb(CMOSDATA);
	}

public:
	static const DeviceClass CMOSRTCDeviceClass;

	const DeviceClass &device_class() const override
	{
		return CMOSRTCDeviceClass;
	}

	/**
	 * Interrogates the RTC to read the current date & time.
	 * @param tp Populates the tp structure with the current data & time, as
	 * given by the CMOS RTC device.
	 */
	void read_timepoint(RTCTimePoint &tp) override
	{
		// FILL IN THIS METHOD - WRITE HELPER METHODS IF NECESSARY

		//flag to indicate the update bit has been set
		bool set = false;
		//flag to indicate the update flag has been unset
		bool unset = false;
		//status register A
		uint8_t statA;
		//ignore interupts
		UniqueIRQLock l;
		//get the A status register and wait until an update has started (bit 7 is 1)
		//once it's started wait for it to end (bit 7 is 0)
		//end the loop
		do {
			statA = readCMOS(0xA);
			if (statA & 0x80) {
				set = true;
			} else {
				if (set){
					unset = true;
				}
			}
		} while(!unset);
		//get the B status register to get register format (bit 3)
		uint8_t statB = readCMOS(0x0B);
		//get all of the data from the CMOS
		tp.seconds = readCMOS(secs);
		tp.minutes = readCMOS(mins);
		tp.hours = readCMOS(hrs);
		tp.day_of_month = readCMOS(day);
		tp.month = readCMOS(mnth);
		tp.year = readCMOS(yr);
		//if the data is BCD (bit 3 is 0) then decode and restore the data now stored in tp
		if (!(statB & 0x04)) {
			tp.seconds = BCDtoShort(tp.seconds);
			tp.minutes = BCDtoShort(tp.minutes);
			tp.hours = BCDtoShort(tp.hours);
			tp.day_of_month = BCDtoShort(tp.day_of_month);
			tp.month = BCDtoShort(tp.month);
			tp.year = BCDtoShort(tp.year);
		}
	}
	
};

const DeviceClass CMOSRTC::CMOSRTCDeviceClass(RTC::RTCDeviceClass, "cmos-rtc");

RegisterDevice(CMOSRTC);
