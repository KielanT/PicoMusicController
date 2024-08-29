#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "bsp/board_api.h"
#include "tusb.h"

static void echo_serial_port(uint8_t itf, uint8_t buf[], uint32_t count)
{
  uint8_t const case_diff = 'a' - 'A';

  for(uint32_t i=0; i<count; i++)
  {
    if (itf == 0)
    {
      // echo back 1st port as lower case
      if (isupper(buf[i])) buf[i] += case_diff;
    }
    else
    {
      // echo back 2nd port as upper case
      if (islower(buf[i])) buf[i] -= case_diff;
    }

    tud_cdc_n_write_char(itf, buf[i]);
  }
  tud_cdc_n_write_flush(itf);
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
static void cdc_task(std::string& data)
{
    uint8_t itf = 0;

    if ( tud_cdc_n_available(itf) )
    {
        uint8_t buf[64];
        uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

         if (count > 0)
        {
            // Convert the buffer to a std::string
            data = {reinterpret_cast<char*>(buf), count};
        }
    }   
}