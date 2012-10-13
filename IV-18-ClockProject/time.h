/*
 * time.c
 *  Huge thanks to oPossum from 43oh.com!  http://www.43oh.com/forum/viewtopic.php?f=10&t=2477&p=21670#p21670
 */

#ifndef TIME_H_
#define TIME_H_


struct btm                                              // BCD time structure
{
    unsigned int sec;
    unsigned int min;
    unsigned int hour;
    unsigned int wday;
    unsigned int mday;
    unsigned int yday;
    unsigned int mon;
    unsigned int year;
    char time_change; //indicates what values have changed
};

struct alarm_bcd
{
	unsigned int min;
	unsigned int hour;
	char daysOfWeek; //allows selection of specific days
};



unsigned is_leap_year_bcd(unsigned year)
{
    if(year & 3) return 0;
    switch(year & 0x03FF) {
        case 0x0100: case 0x0200: case 0x0300: return 0;
    }
    return 1;
}

static const unsigned short dim[18][2] = {                 // Number of days in month for non-leap year and leap year
        0x31,   0x31,                                   // 00 January
        0x28,   0x29,                                   // 01 February
        0x31,   0x31,                                   // 02 March
        0x30,   0x30,                                   // 03 April
        0x31,   0x31,                                   // 04 May
        0x30,   0x30,                                   // 05 June
        0x31,   0x31,                                   // 06 July
        0x31,   0x31,                                   // 07 August
        0x30,   0x30,                                   // 08 September
        0x31,   0x31,                                   // 09 October
        0,      0,                                      // 0A
        0,      0,                                      // 0B
        0,      0,                                      // 0C
        0,      0,                                      // 0D
        0,      0,                                      // 0E
        0,      0,                                      // 0F
        0x30,   0x30,                                   // 10 November
        0x31,   0x31                                    // 11 December
    };

void rtc_tick_bcd(struct btm *t)
{
                                                      //
                                                        //
    t->sec = _bcd_add_short(t->sec, 1);                 // Increment seconds
    if(t->sec > 0x59) {                                 // Check for overflow
        t->sec = 0;                                     // Reset seconds
        t->min =  _bcd_add_short(t->min, 1);            // Increment minutes
        t->time_change = 1;								// Minutes have change
        if(t->min > 0x59) {                             // Check for overflow
            t->min = 0;                                 // Reset minutes
            t->hour = _bcd_add_short(t->hour, 1);       // Increment hours
            if(t->hour > 0x23) {                        // Check for overflow
                t->hour = 0;                            // Reset hours
                t->yday = _bcd_add_short(t->yday, 1);   // Increment day of year
                t->wday = _bcd_add_short(t->wday, 1);   // Increment day of week
                if(t->wday > 0x06)                      // Check for overflow
                    t->wday = 0;                        // Reset day of week
                t->mday = _bcd_add_short(t->mday, 1);   // Increment day of month, check for overflow
                if(t->mday > dim[t->mon][is_leap_year_bcd(t->year)]) {
                    t->mday = 0x01;                     // Reset day of month
                    t->mon = _bcd_add_short(t->mon, 1); // Increment month
                    if(t->mon > 0x11) {                 // Check for overflow
                        t->mon = 0;                     // Reset month
                        t->yday = 0;                    // Reset day of year
                        t->year = _bcd_add_short(t->year, 1); // Increment year
                    }                                   // - year
                }                                       // - month
            }                                           // - day
        }                                               // - hour
    }                                                   // - minute
}                                                       //
                                       //
char check_alarm(struct btm *t, volatile struct alarm_bcd *a)
{
	//check day of the week first
	if (1<<t->wday & a->daysOfWeek)
	{
		if (t->hour == a->hour)
			if(t->min == a->min)
				return 1;
	}
	//if we don't satisfy any of the alarm conditions
	return 0;
}




#endif /* TIME_H_ */
