#include <utility>
#include <string>
#include "hpack_table.h"

void
Table::set_dynamic_table_size(uint32_t size) {
    dynamic_table_size = size;
}

std::pair<std::string, std::string> // temporally
Table::get_header(uint32_t index) {
    std::pair<std::string, std::string> header;    
    return header;
}

std::string // temporally
Table::parse_string(uint8_t* buf) {
    return "";
}

std::pair<std::string, std::string>
Table::parse_header(uint32_t index, uint8_t* buf, bool isIndexed) {
    std::pair<std::string, std::string> header;    
    std::string val_tmp;
    if (!isIndexed) {
        if (index == 0) {
            header.first = this->parse_string(buf); //temporally
        }
        val_tmp = this->parse_string(buf); //temporally
    }
    
    if (index > 0) {
        header = this->get_header(index);
        if (val_tmp.length() > 0) {
            header.second = val_tmp;
        }
    }
    return header;
}
