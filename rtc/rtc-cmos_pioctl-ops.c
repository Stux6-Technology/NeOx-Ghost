/* ==========================================================================
 *   PROJECT: NeOx Ghost (Advanced Microkernel Architecture Project)
 *   COPYRIGHT: (C) 2020 - 2026 Stux6 Technology Team. All Rights Reserved.
 *   DEVELOPER: Stux6 Tech. Lead Eng. Alperen ERKAN <erkanalperen54 [at] gmail.com> 
 *              or <stux6.team@gmail.com>
 * ==========================================================================
 *   LICENSE SUMMARY (STUX6 GENERAL PRIVATE PROJECT LICENSE - SGPPL-v1.0)
 * 
 *   1. This software and its kernel architecture are officially registered 
 *      intellectual property of the STUX6 TECHNOLOGY team.
 *   2. This code is made available strictly under "source-available" status 
 *      for personal research and local laboratory development only.
 *   3. ANY DISTRIBUTION, FORKING, OR RE-PUBLISHING ON ANY INTERNET PLATFORM 
 *      (INCLUDING GITHUB, GITLAB, BITBUCKET) IS STRICTLY PROHIBITED.
 *   4. Commercial enterprise, government network, or military deployment 
 *      requires express, hand-signed written authorization from the team captain.
 *   5. This header, copyright notices, and license text MUST remain untouched.
 * 
 *   FOR THE FULL TERMS AND CONDITIONS, REFER TO THE 'LICENSE' FILE.
 * ========================================================================== */

/* This implementation is largely based on sys-utils/hwclock-cmos.c from
   util-linux.  */

/* A struct tm has int fields (it is defined in POSIX)
   tm_sec	0-59, 60 or 61 only for leap seconds
   tm_min	0-59
   tm_hour	0-23
   tm_mday	1-31
   tm_mon	0-11
   tm_year	number of years since 1900
   tm_wday	0-6, 0=Sunday
   tm_yday	0-365
   tm_isdst	>0: yes, 0: no, <0: unknown  */

#include "rtc_pioctl_S.h"
#include <hurd/rtc.h>
#include <hurd/hurd_types.h>
#include <sys/io.h>
#include <stdbool.h>

/* Conversions to and from RTC internal format.  */
#define BCD_TO_BIN(val) ((val)=((val)&15) + (((val)>>4)&15)*10 + \
	((val)>>8)*100)
#define BIN_TO_BCD(val) ((val)=(((val)/100)<<8) + \
	((((val)/10)%10)<<4) + (val)%10)

/* POSIX uses 1900 as epoch for a struct tm, and 1970 for a time_t.  */
#define TM_EPOCH 1900

#define CLOCK_CTL_ADDR 0x70
#define CLOCK_DATA_ADDR 0x71

#define is_leap(year) \
              ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

static const int mon_yday[2][13] =
{
  /* Normal years.  */
  { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
  /* Leap years.  */
  { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};


static inline unsigned char
cmos_read (unsigned char reg)
{
  outb_p (reg, CLOCK_CTL_ADDR);
  return inb_p (CLOCK_DATA_ADDR);
}

static inline void
cmos_write (unsigned char reg, unsigned char val)
{
  outb_p (reg, CLOCK_CTL_ADDR);
  outb_p (val, CLOCK_DATA_ADDR);
}

static inline int
cmos_clock_busy (void)
{
  /* Poll bit 7 (UIP) of Control Register A.  */
  return (cmos_read (10) & 0x80);
}

/* Calculate day of year based on month, day of month, and year.  The value
   it returns is in binary format.  */
static int
calculate_yday (const struct rtc_time *tm)
{
  return mon_yday[is_leap (tm->tm_year)][tm->tm_mon] + tm->tm_mday - 1;
}

/* 3 RTC_UIE_ON -- Enable update-ended interrupt.  */
kern_return_t
rtc_S_pioctl_rtc_uie_on (struct trivfs_protid *cred)
{
  return EOPNOTSUPP;
}

/* 4 RTC_UIE_OFF -- Disable update-ended interrupt.  */
kern_return_t
rtc_S_pioctl_rtc_uie_off (struct trivfs_protid *cred)
{
  return EOPNOTSUPP;
}

/* 9 RTC_RD_TIME -- Read RTC time.  */
kern_return_t
rtc_S_pioctl_rtc_rd_time (struct trivfs_protid *cred, struct rtc_time *tm)
{
  unsigned char status = 0;
  unsigned char pmbit = 0;
  int time_passed_in_milliseconds = 0;
  bool read_rtc_successfully = false;

  if (!cred)
    return EOPNOTSUPP;
  if (!(cred->po->openmodes & O_READ))
    return EBADF;

  /* When we wait for 100 ms (it takes too long), we exit with error.  */
  while (time_passed_in_milliseconds < 100)
    {
      if (!cmos_clock_busy ())
	{
	  tm->tm_sec = cmos_read (0);
	  tm->tm_min = cmos_read (2);
	  tm->tm_hour = cmos_read (4);
	  tm->tm_wday = cmos_read (6);
	  tm->tm_mday = cmos_read (7);
	  tm->tm_mon = cmos_read (8);
	  tm->tm_year = cmos_read (9);
	  status = cmos_read (11);
	  /* Unless the clock changed while we were reading, consider this
	     a good clock read.  */
	  if (tm->tm_sec == cmos_read (0))
	    {
	      read_rtc_successfully = true;
	      break;
	    }
	}
      usleep (1000);
      time_passed_in_milliseconds++;
    }

  if (!read_rtc_successfully)
    return EBUSY;

  /* If the data we just read is in BCD format, convert it to binary
     format.  */
  if (!(status & 0x04))
    {
      BCD_TO_BIN (tm->tm_sec);
      BCD_TO_BIN (tm->tm_min);
      pmbit = (tm->tm_hour & 0x80);
      tm->tm_hour &= 0x7f;
      BCD_TO_BIN (tm->tm_hour);
      BCD_TO_BIN (tm->tm_wday);
      BCD_TO_BIN (tm->tm_mday);
      BCD_TO_BIN (tm->tm_mon);
      BCD_TO_BIN (tm->tm_year);
    }

  /* We don't use the century byte of the Hardware Clock since we
     don't know its address (usually 50 or 55).  Here, we follow the
     advice of the X/Open Base Working Group: "if century is not
     specified, then values in the range [69-99] refer to years in the
     twentieth century (1969 to 1999 inclusive), and values in the
     range [00-68] refer to years in the twenty-first century (2000 to
     2068 inclusive)".  */
  tm->tm_wday -= 1;
  tm->tm_mon -= 1;
  if (tm->tm_year < 69)
    tm->tm_year += 100;

  /* Calculate day of year.  */
  tm->tm_yday = calculate_yday (tm);

  if (pmbit)
    {
      tm->tm_hour += 12;
      if (tm->tm_hour == 24)
        tm->tm_hour = 0;
    }

  /* We don't know whether it's daylight.  */
  tm->tm_isdst = -1;

  return KERN_SUCCESS;
}

/* 10 RTC_SET_TIME -- Set RTC time.  */
kern_return_t
rtc_S_pioctl_rtc_set_time (struct trivfs_protid *cred, struct rtc_time tm)
{
  unsigned char save_control, save_freq_select, pmbit = 0;

  if (!cred)
    return EOPNOTSUPP;
  if (!(cred->po->openmodes & O_WRITE))
    return EBADF;

  /* CMOS byte 10 (clock status register A) has 3 bitfields:
    bit 7: 1 if data invalid, update in progress (read-only bit)
            (this is raised 224 us before the actual update starts)
     6-4    select base frequency
            010: 32768 Hz time base (default)
            111: reset
            all other combinations are manufacturer-dependent
            (e.g.: DS1287: 010 = start oscillator, anything else = stop)
     3-0    rate selection bits for interrupt
            0000 none (may stop RTC)
            0001, 0010 give same frequency as 1000, 1001
            0011 122 microseconds (minimum, 8192 Hz)
            .... each increase by 1 halves the frequency, doubles the period
            1111 500 milliseconds (maximum, 2 Hz)
            0110 976.562 microseconds (default 1024 Hz).  */

  /* Tell the clock it's being set.  */
  save_control = cmos_read (11);
  cmos_write (11, (save_control | 0x80));
  /* Stop and reset prescaler.  */
  save_freq_select = cmos_read (10);
  cmos_write (10, (save_freq_select | 0x70));

  tm.tm_year %= 100;
  tm.tm_mon += 1;
  tm.tm_wday += 1;

  /* 12hr mode; the default is 24hr mode.  */
  if (!(save_control & 0x02))
    {
      if (tm.tm_hour == 0)
	tm.tm_hour = 24;
      if (tm.tm_hour > 12)
	{
	  tm.tm_hour -= 12;
	  pmbit = 0x80;
	}
    }

  /* BCD mode - the default.  */
  if (!(save_control & 0x04))
    {
      BIN_TO_BCD (tm.tm_sec);
      BIN_TO_BCD (tm.tm_min);
      BIN_TO_BCD (tm.tm_hour);
      BIN_TO_BCD (tm.tm_wday);
      BIN_TO_BCD (tm.tm_mday);
      BIN_TO_BCD (tm.tm_mon);
      BIN_TO_BCD (tm.tm_year);
    }

  cmos_write (0, tm.tm_sec);
  cmos_write (2, tm.tm_min);
  cmos_write (4, tm.tm_hour | pmbit);
  cmos_write (6, tm.tm_wday);
  cmos_write (7, tm.tm_mday);
  cmos_write (8, tm.tm_mon);
  cmos_write (9, tm.tm_year);

  /* The kernel sources, linux/arch/i386/kernel/time.c, have the
  following comment:

  The following flags have to be released exactly in this order,
  otherwise the DS12887 (popular MC146818A clone with integrated
  battery and quartz) will not reset the oscillator and will not
  update precisely 500 ms later. You won't find this mentioned in
  the Dallas Semiconductor data sheets, but who believes data
  sheets anyway ... -- Markus Kuhn.  */
  cmos_write (11, save_control);
  cmos_write (10, save_freq_select);

  return KERN_SUCCESS;
}
