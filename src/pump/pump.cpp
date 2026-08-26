#include "pump.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

//#include <format>
#include "../output/write_log.h"

#include "../json/json.hpp"
using json = nlohmann::ordered_json;
extern json config;

void Pump_Data::Decode(std::vector<uint8_t> &data) {
	received = 1;
	PumpTimeSeries s;
	unsigned int i;         // temporary indexing variable
	unsigned int index = 0; // index of pump sample in current packet
	unsigned int nbyte;     // Number of bytes for this pump packet

	s.phase = -9;
	int pid = data[0] & 0x0F;
	version = (data[1] & 0xF0) >> 4;
	nbyte = ((data[1] & 0x0F) << 8) + data[2];
	s.packet = data[2] & 0x0F;

	i = 3;

	while (i < nbyte -1) {
		s.index = index++;
		if ( version == 0 ) {
		  s.phase = -9;
		  s.p_cnt = ((data[i] & 0x0F) << 8) + (data[i+1]);
		  i += 2;
		} else {
		  s.phase = (data[i] & 0xF0) >> 4;
		  s.p_cnt = ((data[i] & 0x0F) << 16) + (data[i+1] << 8) + data[i+2];
		  i += 3;
                }
		s.pres = s.p_cnt * (float)config["PRESSURE_GAIN"] - (float)config["PRESSURE_OFFSET"];
		if (s.pres > 7500)
			s.pres = -999; // blank out bad pressure scans
		if ( version == 2 ) { //Quartzdyne
		  s.p_cnt = ( data[i]  << 8 ) + data[i+1];
		  s.presQ = s.p_cnt * (float)config["PRESSURE_GAIN"] - (float)config["PRESSURE_OFFSET"];
		  if (s.presQ > 7500)
			s.presQ = -999; // blank out bad pressure scans
		  i += 2;
                }
		s.pump_time = (data[i]<<8) + data[i+1];
		i += 2;
		if (s.pump_time > 0x8000)
			s.pump_time -= 0x10000;

		s.v_cnt = (data[i] << 8) + data[i+1];
		i += 2;
		s.volt = s.v_cnt * (float)config["VOLTAGE_GAIN"] - (float)config["VOLTAGE_OFFSET"];
		s.c_cnt = (data[i] << 8) + data[i+1];
		i += 2;
		s.curr = s.c_cnt * (float)config["CURRENT_GAIN"] - (float)config["CURRENT_OFFSET"];
		s.vac_strt = data[i];
		i += 1;
		s.vac_end = data[i];
		i += 1;
		//s.time_sc = 60 * ( ( data[i+11] << 8 ) + data[i+12] );
		s.time_sc = ( ( data[i] << 8 ) + data[i+1] );
		i += 2;

		Scan.push_back(s);
	}

	std::sort(Scan.begin(),Scan.end());
	//log(std::format("Packet[{:2X}] Pump Data ({:d}) format: {:d}",pid,s.packet,version) );
}

std::ostream & operator << ( std::ostream &os, Pump_Data &p ) {
	os << "# Pump Time Series" << std::endl;
	os << std::setfill(' ');
	for (auto s : p.Scan) {
		os << std::setw(4) << s.pump_time << " ";
		os << std::setw(8) << std::setprecision(3) << s.pres << " ";
		os << std::setw(4) << std::setprecision(0) << s.curr << " ";
		os << std::setw(4) << std::setprecision(0) << s.volt << " ";
		os << std::setw(3) << s.vac_strt << " " << std::setw(3) << s.vac_end << " ";
		os << std::setw(2) << s.phase << std::endl;
	}
	return os;
}
