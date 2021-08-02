#include "mktime.h"

uint32_t
unix_time_in_seconds( uint8_t sec, uint8_t min, uint8_t hrs, uint8_t day, uint8_t mon, uint16_t year )
{
  uint32_t ts = 0;
	uint8_t sec_dec = 0;
	uint8_t sec_u = 0;
	
	/*sec_u = (sec << 4)>>4;
	sec_dec = sec>>4;
	sec = (sec_dec)*10+(sec_u);*/
	
  //  Add up the seconds from all prev years, up until this year.
  uint8_t years = 0;
  uint8_t leap_years = 0;
	year = 2000+year;
  for( uint16_t y_k = EPOCH_YEAR; y_k<year; y_k++ )
  {
    if( IS_LEAP_YEAR( y_k ) )
    {
      leap_years++;
    } else {
      years++;
    }
  }
  ts += ( (years*days_per_year[0]) + (leap_years*days_per_year[1]) ) * SEC_PER_DAY;

  //  Add up the seconds from all prev days this year, up until today.
  uint8_t year_index = (IS_LEAP_YEAR( year )) ? 1 : 0;
  for( uint8_t mo_k = 0; mo_k<(mon); mo_k++ )
  { //  days from previous months this year
    ts += days_per_month[ year_index ][ mo_k ] * SEC_PER_DAY;
  }
  ts += (day-1) * SEC_PER_DAY; // days from this month

  //  Calculate seconds elapsed just today.
  ts += (hrs-1) * SEC_PER_HOUR;
  ts += min * SEC_PER_MIN;
  ts += sec;

  return ts;
}