#include "hexfile/hexfile.h"
#include "output/write_log.h"
#include <cstdlib>
#include <string>
#include <fstream>
//#include <format>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <chrono>
#include <thread>

using namespace boost::filesystem;

#include "json/json.hpp"
using json = nlohmann::ordered_json;
json config;

std::string DS_PATH;
int MAJOR_VERSION = 1;
int MINOR_VERSION = 0;

int main( int argc, char **argv) {

  // Read decoder config file
  const char *ds_path = getenv("DS_PATH");
  if (ds_path == NULL) {
	// If DS_PATH env variable is not defined, exit. Write to stdout becuase log() requires DS_PATH
    std::cout << "Unable to load DS_PATH environment variable; exiting." << std::endl;
    return 0;
  }
  DS_PATH = ds_path;

  int sn = std::stoi(argv[1]);
  //if (sn <= 6020 | sn == 6022 | sn == 6023 | sn == 6024 ) { return 0; }
  
  std::ifstream f(DS_PATH + "/config/config.json");
  if (!f) {
	// If config.json is not found, exit. Write to stdout because log() requires DS_PATH
    std::cout << "Unable to open config.json; exiting." << std::endl;
	return 0;
  }
  config = json::parse(f);

  //std::cout << "argc " << argc << std::endl;

  //JG addition use config in directory if present
  if (argc >= 2) {
    //std::cout << sn << std::endl;
    std::ostringstream ssjg;
    ssjg.str("");
    ssjg.clear();
    ssjg << DS_PATH << "/data/" << std::setw(4) << std::setfill('0') << std::to_string(sn) << "/modified_" << std::setw(4) << std::setfill('0') << std::to_string(sn) << "_config.json";
    std::string config_filename = ssjg.str();
    std::ifstream f2(config_filename);
    if (f2) {
      //std::cout << "Float specific config.json " << config_filename << std::endl;
      config = json::parse(f2);
    }
    ssjg.str("");
    ssjg.clear();
  }

  //log("Starting DS-SOLO");

  std::vector<std::filesystem::path> hexfiles;
  std::string incoming_dir = DS_PATH + "/incoming";

  //std::cout << "argc " << argc << std::endl;

  // Command-line options
  if (argc == 3) {
    std::string filter_filename, filter_filepath;
    int sn = std::stoi(argv[1]);
    int cycle = std::stoi(argv[2]);

    //std::cout << "cycle " << cycle << std::endl;
    //std::cout << "Pausing" << std::endl;
    //std::this_thread::sleep_for(std::chrono::seconds(10));
    //filter_filename = std::format("{:04d}_{:03d}.hex",sn,cycle); // DS_Decoder [sn] [cycle]
    //filter_filepath = std::format("{}/data/{:d}/hex/{}",DS_PATH,sn,filter_filename);
    std::ostringstream ss;
    ss.str("");
    ss.clear();
    if (cycle<0) {
      ss << std::setw(4) << std::setfill('0') << std::to_string(sn) << "_-001.hex"; // DS_Decoder [sn] [cycle]
    } else {
      ss << std::setw(4) << std::setfill('0') << std::to_string(sn) << "_" << std::setw(4) << std::setfill('0') << std::to_string(cycle) << ".hex"; // DS_Decoder [sn] [cycle]
    }
    filter_filename = ss.str();
    ss.str("");
    ss.clear();
    ss << DS_PATH << "/data/" << std::setw(4) << std::setfill('0') << std::to_string(sn) << "/hex/" << filter_filename;
    filter_filepath = ss.str();
    ss.str("");
    ss.clear();
    if (std::filesystem::exists(filter_filepath)) {
        //log( std::format("* Processing {} using command-line filter",filter_filename));
        std::filesystem::copy(filter_filepath,incoming_dir); // copy hexfile from float subdirectory to incoming
    }
    else {
        //log( std::format("* warning - unable to find {}",filter_filename) );
    }
  }

  // read and process each hex file in hex directory
  std::copy(std::filesystem::directory_iterator(incoming_dir),std::filesystem::directory_iterator(),std::back_inserter(hexfiles));
  std::sort(hexfiles.begin(),hexfiles.end());
  for (const std::filesystem::path &filepath : hexfiles) {
    //std::cout << "Processing main " << std::endl;
    hexfile h(filepath.string());
    h.Decode();
    h.archive();
    h.write_JSON();

//	if (h.cycle == -1 && config.contains("email") ) {
//     string cmd = std::format("python3 {} 'DS #{:d} startup' '{}' '{}'", std::string(config["email"]["python_script"]), h.sn, h.jsonpath, std::string(config["email"]["alert_recipients"]) );
//      system(cmd.c_str());
//      log( std::format("* Send startup message SUBJECT: 'DS #{:d} startup' to {}",h.sn,std::string(config["email"]["alert_recipients"])));
//	}
//	if (h.cycle == 0 && config.contains("email") ) {
//      string cmd = std::format("python3 {} 'DS #{:d} cycle 0' '{}' '{}'", std::string(config["email"]["python_script"]), h.sn, h.jsonpath, std::string(config["email"]["alert_recipients"]) );
//      system(cmd.c_str());
//      log( std::format("* Send startup message SUBJECT: 'DS #{:d} cycle 0' to {}",h.sn,std::string(config["email"]["alert_recipients"])));
//	}

  }

  //log("Finished.");
  return 0;
}
