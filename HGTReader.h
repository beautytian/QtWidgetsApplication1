#ifndef HGTREADER_H
#define HGTREADER_H

#include <string>
#include <vector>
#include <cstdint>

const int HGT_SIZE = 3601;
const int16_t HGT_NODATA = -32768;

class HGTReader
{
public:
    explicit HGTReader(const std::string& path);
    ~HGTReader() = default;

    bool load();
    bool isLoaded() const { return m_loaded; }
    int16_t getValue(int row, int col) const;

private:
    std::string m_filepath;
    std::vector<int16_t> m_data;
    bool m_loaded;
};

#endif // HGTREADER_H
