#include <string>
#include "HPACK.h"
#include "hpack_table.h"


static const header STATIC_TABLE[] = {
    header("", ""),
    header(":authority", ""),
    header(":method", "GET"),
    header(":method", "POST"),
    header(":path", "/"),
    header(":path", "/index.html"),
    header(":scheme", "http"),
    header(":scheme", "https"),
    header(":status", "200"),
    header(":status", "204"),
    header(":status", "206"),
    header(":status", "304"),
    header(":status", "400"),
    header(":status", "404"),
    header(":status", "500"),
    header("accept-charset", ""),
    header("accept-encoding", "gzip, deflate"),
    header("accept-language", ""),
    header("accept-ranges", ""),
    header("accept", ""),
    header("access-control-allow-origin", ""),
    header("age", ""),
    header("allow", ""),
    header("authorization", ""),
    header("cache-control", ""),
    header("content-disposition", ""),
    header("content-encoding", ""),
    header("content-language", ""),
    header("content-length", ""),
    header("content-location", ""),
    header("content-range", ""),
    header("content-type", ""),
    header("cookie", ""),
    header("date", ""),
    header("etag", ""),
    header("expect", ""),
    header("expires", ""),
    header("from", ""),
    header("host", ""),
    header("if-match", ""),
    header("if-modified-since", ""),
    header("if-none-match", ""),
    header("if-range", ""),
    header("if-unmodified-since", ""),
    header("last-modified", ""),
    header("link", ""),
    header("location", ""),
    header("max-forwards", ""),
    header("proxy-authenticate", ""),
    header("proxy-authorization", ""),
    header("range", ""),
    header("referer", ""),
    header("refresh", ""),
    header("retry-after", ""),
    header("server", ""),
    header("set-cookie", ""),
    header("strict-transport-security", ""),
    header("transfer-encoding", ""),
    header("user-agent", ""),
    header("vary", ""),
    header("via", ""),
    header("www-authenticate", "")
};

static const uint8_t STATIC_TABLE_NUM = 62;

void
Table::set_dynamic_table_size(uint32_t size) {
    dynamic_table_size = size;
}

header // temporally
Table::get_header(uint32_t index) {
    if (0 < index && index < STATIC_TABLE_NUM) {
        return STATIC_TABLE[index];
    } else if (STATIC_TABLE_NUM <= index && index <= STATIC_TABLE_NUM + entry_num) {
        RingTable* ring = head;
        for (int i = 0; i < index-STATIC_TABLE_NUM; i++) {
            ring = ring->nxt;
        }
        return ring->h;
    }
    header h; // temporally
    return h;
}

uint32_t // temporally
Table::parse_string(std::string& content, uint8_t* buf) {
    uint32_t dst;
    uint64_t l = decode_int(dst, buf, 7);
    if ((*buf & 0x80) > 0) {
        content = "";// huffman decoding
    } else {
        for (int i = 0; i < dst; i++) {
            content += (char)(*(buf++));
        }
    }
    return l;
}

header
Table::parse_header(uint32_t index, uint8_t* buf, bool isIndexed) {
    header h;
    std::string val_tmp;
    if (!isIndexed) {
        // l is used temporally
        if (index == 0) {
            uint64_t l = this->parse_string(h.first, buf);
        }
        uint64_t l = this->parse_string(val_tmp, buf);
    }
    
    if (index > 0) {
        h = this->get_header(index);
        if (val_tmp.length() > 0) {
            h.second = val_tmp;
        }
    }
    return h;
}
