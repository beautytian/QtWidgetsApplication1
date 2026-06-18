#include "HGTReader.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>

HGTReader::HGTReader(const std::string& path)
    : m_filepath(path)
    , m_loaded(false)
{
    load();
}

bool HGTReader::load()
{
    std::ifstream file(m_filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open HGT file: " << m_filepath << std::endl;
        m_loaded = false;
        return false;
    }

    // 获取文件大小
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    size_t expectedSize = HGT_SIZE * HGT_SIZE * sizeof(int16_t);

    if (fileSize != expectedSize) {
        std::cerr << "HGT file size incorrect: " << m_filepath
            << " (expected: " << expectedSize << ", actual: " << fileSize << ")" << std::endl;
        file.close();
        m_loaded = false;
        return false;
    }

    file.seekg(0, std::ios::beg);
    m_data.resize(HGT_SIZE * HGT_SIZE);
    file.read(reinterpret_cast<char*>(m_data.data()), expectedSize);
    file.close();

    // 大端转小端 (SRTM 数据是大端存储)
    for (auto& val : m_data) {
        val = (val >> 8) | ((val & 0xFF) << 8);
    }

    m_loaded = true;
    return true;
}

int16_t HGTReader::getValue(int row, int col) const
{
    if (!m_loaded || row < 0 || row >= HGT_SIZE || col < 0 || col >= HGT_SIZE) {
        return HGT_NODATA;
    }
    return m_data[row * HGT_SIZE + col];
}