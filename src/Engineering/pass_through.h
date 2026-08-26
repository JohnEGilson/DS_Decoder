#include <string>
#include <vector>
#include <cstdint>

class pass_through {

public:
  std::string ctd_info;

  void parse( std::vector<uint8_t> d);

};
