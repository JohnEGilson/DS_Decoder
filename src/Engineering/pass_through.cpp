#include "pass_through.h"
#include <iostream>
#include <map>
//#include <format>
#include "../output/write_log.h"

void pass_through::parse( std::vector<uint8_t> d ) {
  //std::map<int,std::string> sensor_name = {{0,"CTD"},{1,"DO"},{2,"pH"},{3,"ECO"},{4,"OCR"},{5,"SUNA"}};
  std::map<int,std::string> sensor_name = {{0,"CTD"}};

  int ptype = d[0] & 0xF;
  int length = ((d[1] & 0xF) << 8) + d[2]; // sensor pass-through string length

  //std::cout << "Packet[{:2X}] {:s} Pass Through" << ptype << " " <<  sensor_name[d[4]] << std::endl;

  for( int i = 3; i < length; i++ ) {
    if (d[i]==13)
      continue; // remove carriage return [0x13] from pass-through sensor strings
    if (d[i]==0x09)
      d[i]=' '; // replace tab character [0x09] with a space; tabs are not allowed within JSON strings
    switch ( d[0] & 0xF ) {
      case 15: ctd_info += d[i]; break; // "DF"
      //case 1: do_info += d[i]; break;
      //case 2: ph_info += d[i]; break;
      //case 3: eco_info += d[i]; break;
      //case 4: ocr_info += d[i]; break;
      //case 5: suna_info += d[i]; break;
    }
  }
}
