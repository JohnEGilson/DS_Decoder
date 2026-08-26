#include "Engineering_Data.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
//#include <format>
#include "../output/write_log.h"

#include "../json/json.hpp"
using json = nlohmann::ordered_json;
extern json config;

extern std::string DS_PATH;


#define PARAMETER_VALUE_WIDTH 8
#define PARAMETER_NAME_WIDTH  8
#define PARAMETER_UNIT_WIDTH  6

// Parses SBE_Error_Data [0xf1]
// Requires a separate pfile.json that describes variable names, order, types, descriptions
// Only parameters sent. Use pfile.json to interpret parameter data
void SBE_Error_Data::parse_pfile(std::vector<uint8_t> d,std::string pfile) {

	int ptype = d[0] & 0xF;
	int EngVer  = d[3];
	//std::cout << "SBE_Error_Data " << ptype << EngVer << std::endl;
        if ( ptype == 1 ) { //if Engineering 0xF1 change the version :: JG addition
		size_t pos = pfile.rfind('.');
                pfile = pfile.substr(0,pos) + "_" + std::to_string(EngVer) + ".json";
        }
	//std::cout << "SBE Config " << pfile << std::endl;

    	std::ifstream f(pfile);
        json params = json::parse(f);

        int dval;
	int n = 4; //int n = 6;
	double val;
	int prec;
	std::stringstream unit,vstr,pnamestr;

	pnamestr << std::setw(PARAMETER_NAME_WIDTH) << "\"Eng_ver\"";
	vstr << std::setw(PARAMETER_VALUE_WIDTH) << (uint16_t)d[3];
	int vs = d[3];

	unit << std::setw(PARAMETER_UNIT_WIDTH) << "\"1\"";
	list.push_back({pnamestr.str(),unit.str(),"Engineering Packet software version",vstr.str()});
	pnamestr.str("");
	vstr.str("");
	unit.str("");

	for(auto &[pname,patts] : params.items()) {
		prec = 0;
		unit.str("");
		unit << "\"";
		//std::cout << "SBE Parsing: " << pname << std::endl;
		if (patts["type"] == "U8") {
		          if (patts.contains("mask")) {
			    dval=d[n];
                            std::string hexbits = patts["mask"];
                            int bits = std::stoi(hexbits,nullptr,16);
                            bool lsbbits = (bits & 1);
			    if ( lsbbits ) {
			      val = ( ( dval & bits ) );
                              n++;
			    } else {
			      int bitshift = log2( bits & (-1)*bits );
			      val = ( ( dval & bits ) >> bitshift );
			    }
			  } else {
			    val = d[n];
			    n++;
		          }
		}
		else if (patts["type"] == "U16") {
		          if (patts.contains("mask")) {
			    dval=(d[n]<<8) + d[n+1];
                            std::string hexbits = patts["mask"];
                            int bits = std::stoi(hexbits,nullptr,16);
                            bool lsbbits = (bits & 1);
			    if ( lsbbits ) {
			      val = ( ( dval & bits ) );
                              n+=2;
			    } else {
			      int bitshift = log2( bits & (-1)*bits );
			      val = ( ( dval & bits ) >> bitshift );
			    }
			  } else {
			    val = (d[n]<<8) + d[n+1];
			    n+=2;
		          }
		}
		else if (patts["type"] == "I16") {
			val = (d[n]<<8) + d[n+1];
			if (val > 32767) // 2025/09/10 BG convert uint16 to int16
				val = val - 65536;
			n+=2;
		}
		else if (patts["type"] == "U24") {
			val = (d[n]<<16) + (d[n+1]<<8) + d[n+2];
			n+=3;
		}

		else {
			std::cout << "unknown parameter" << std::endl;
			val = (d[n]<<8) + d[n+1];
			n+=2;
		}
		if (patts.contains("scale")) {
			if (patts["scale"] == "pres")
				val /= double(config["prof"]["CTD Discrete"]["PRES"]["gain"]);
			else if (patts["scale"] == "temp")
				val /= double(config["prof"]["CTD Discrete"]["TEMP"]["gain"]);
			else if (patts["scale"] == "psal")
				val /= double(config["prof"]["CTD Discrete"]["PSAL"]["gain"]);
                        else
				val *= (float)patts["scale"];
			prec = (int)patts["prec"]; // precision for output (list files and json); every parameter with scale needs "prec" defined
		}
		if (patts.contains("offset")) {
			if (patts["offset"] == "pres")
				val -= double(config["prof"]["CTD Discrete"]["PRES"]["offset"]);
			else if (patts["offset"] == "temp")
				val -= double(config["prof"]["CTD Discrete"]["TEMP"]["offset"]);
			else if (patts["offset"] == "psal")
				val -= double(config["prof"]["CTD Discrete"]["PSAL"]["offset"]);
                        else
			        val -= (float)patts["offset"];
		}
		if (patts.contains("units")) {
			unit << (std::string)patts["units"] << "\"";
		} else {
			unit << "1\"";
		}

		vstr.str("");
		pnamestr.str("");
		pnamestr << "\"" << pname << "\"";
		vstr << std::setw(PARAMETER_VALUE_WIDTH) << std::fixed << std::setprecision(prec) << val;
		list.push_back({pnamestr.str(),unit.str(),(std::string)patts["description"],vstr.str()});
               
		if ( ( ptype == 0 ) && ( pname == "nQueued" ) ) { //in DS there is a gap
			n+=6;
		}

	}
}
