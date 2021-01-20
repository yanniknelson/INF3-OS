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
	uint8_t readCMOSOffset(uint8_t offset)
	{
		//specify CMOS offset
		__outb(CMOSOFFSET, offset);
		//get and return the data from that offset
		return __inb(CMOSDATA);
	}

	/**
	 * Gets all the time and data data from the CMOS, with no concern for validity
	 * and stores it in the passed in RTCTimePoint
	 * @param tp The RTCTimePoint to be populated with data from the CMOS
	 */
	void read_CMOSTime(RTCTimePoint &tp) {
		tp.seconds = readCMOSOffset(secs);
		tp.minutes = readCMOSOffset(mins);
		tp.hours = readCMOSOffset(hrs);
		tp.day_of_month = readCMOSOffset(day);
		tp.month = readCMOSOffset(mnth);
		tp.year = readCMOSOffset(yr);
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

		//flag to indicate the first read has occured
		bool first_read = false;
		//flag to indicate the last attempted read was successful
		bool last_success = false;
		//status register A
		uint8_t statA;
		//ignore interupts
		UniqueIRQLock l;
		//status register B
		uint8_t statB;
		//stores the data from the last read
		uint8_t last_seconds;
		uint8_t last_minutes;
		uint8_t last_hours;
		uint8_t last_day_of_month;
		uint8_t last_month;
		uint8_t last_year;
		//until we have successfully read the data
		do {
			//get the A register and only if an update is not in progress read the data
			//this doesn't ensure an update doesn't start in the middle of the reads though
			statA = readCMOSOffset(0x0A);
			if (!(statA & 0x80))
			{
				read_CMOSTime(tp);
				first_read = true;
			}
		} while (!first_read);

		//once we have read once, we read again (ensureing we don't read on an update) 
		//and ensure the two reads are the same, if so then the data is valid, if not
		//then we must repeat holding the most recent read for comparison
		do {
			//store the last read in the appropriate variables
			last_seconds = tp.seconds;
			last_minutes = tp.minutes;
			last_hours = tp.hours;
			last_day_of_month = tp.day_of_month;
			last_month = tp.month;
			last_year = tp.year;

			//again ensure the update bit isn't set, if not, read the data again (and the B resgister)
			statA = readCMOSOffset(0x0A);
			if (!(statA & 0x80))
			{	
				//we must also get the B register to ensure we can properly decode the new data
				statB = readCMOSOffset(0x0B);
				read_CMOSTime(tp);
				//we must also ensure we flag when the new read attempt occured during an update
				//this is because in this case the contents of tp is not updated and so by neccessity
				//our last and new will be equal, in this case we must not use the data
				last_success = true;
			} else {
				last_success = false;
			}
		//so we exit the check once all the data is the same, and the last read attempt was successful
		} while (((last_seconds != tp.seconds) || (last_minutes != tp.minutes) || 
		(last_hours != tp.hours) || (last_day_of_month != tp.day_of_month) || 
		(last_month != tp.month) || (last_year != tp.year)) && last_success);	
		
		//if the data is BCD (bit 3 is 0) then decode and restore the data now stored in tp
		if (!(statB & 0x04))
		{
			tp.seconds = BCDtoShort(tp.seconds);
			tp.minutes = BCDtoShort(tp.minutes);
			//we or the converted hours with the uncoverted hours to preserve the am/pm indicator for 12/hour time
			//this is valid as in 12 hour BCD bit 7 will never be set (12 = 00010010) 
			tp.hours = BCDtoShort(tp.hours) | (tp.hours & 0x80);
			tp.day_of_month = BCDtoShort(tp.day_of_month);
			tp.month = BCDtoShort(tp.month);
			tp.year = BCDtoShort(tp.year);
		}
		//if the the time is in 12 hour format and bit 7 of the hours is set (the hour is pm) convert the hour
		if ((statB & 0x02) && (tp.hours & 0x80))
		{
			tp.hours = (((tp.hours & 0x7F) + 12) % 24);
		}
	}
};

const DeviceClass CMOSRTC::CMOSRTCDeviceClass(RTC::RTCDeviceClass, "cmos-rtc");

RegisterDevice(CMOSRTC);
