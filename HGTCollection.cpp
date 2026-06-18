#include "HGTCollection.h"
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>
#include <cstdio>

namespace fs = std::filesystem;

const double HGT_STEP = 1.0 / 3600.0;

HGTCollection::HGTCollection(const std::string& dir)
    : m_hgtDir(dir)
{
    loadTiles();
}

void HGTCollection::loadTiles()
{
    if (!fs::exists(m_hgtDir) || !fs::is_directory(m_hgtDir)) {
        std::cerr << "HGT directory does not exist: " << m_hgtDir << std::endl;
        return;
    }
    
    std::cout << "Scanning directory: " << m_hgtDir << std::endl;

    for (const auto& entry : fs::directory_iterator(m_hgtDir)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            std::cout << "Found file: " << filename << std::endl;
            std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper);
            if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".HGT") {
                m_tileMap[filename] = entry.path().string();
            }
        }
    }
}

void HGTCollection::tileSWCorner(double lon, double lat, int& lon0, int& lat0) const
{
    lon0 = static_cast<int>(std::floor(lon));
    lat0 = static_cast<int>(std::floor(lat));
}

std::string HGTCollection::tileNameFromLonLat(double lon, double lat) const
{
    int lon0, lat0;
    tileSWCorner(lon, lat, lon0, lat0);

    std::string ns = (lat0 >= 0) ? "N" : "S";
    std::string ew = (lon0 >= 0) ? "E" : "W";

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s%02d%s%03d.hgt",
        ns.c_str(), std::abs(lat0), ew.c_str(), std::abs(lon0));
    return std::string(buffer);
}

void HGTCollection::lonLatToXYInTile(double lon, double lat, int lon0, int lat0, double& x, double& y) const
{
    x = (lon - lon0) / HGT_STEP;
    y = (lat0 + 1.0 - lat) / HGT_STEP;
}

std::string HGTCollection::findTilePath(double lon, double lat) const
{
    std::string tileName = tileNameFromLonLat(lon, lat);
    std::transform(tileName.begin(), tileName.end(), tileName.begin(), ::toupper);
    auto it = m_tileMap.find(tileName);
    if (it != m_tileMap.end()) {
        return it->second;
    }
    return "";
}

double HGTCollection::samplePoint(double lon, double lat, const std::string& method) const
{
    std::string hgtPath = findTilePath(lon, lat);
    if (hgtPath.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    HGTReader reader(hgtPath);
    if (!reader.isLoaded()) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    int lon0, lat0;
    tileSWCorner(lon, lat, lon0, lat0);

    double x, y;
    lonLatToXYInTile(lon, lat, lon0, lat0, x, y);

    x = std::min(std::max(x, 0.0), static_cast<double>(HGT_SIZE - 1));
    y = std::min(std::max(y, 0.0), static_cast<double>(HGT_SIZE - 1));

    if (method == "nearest") {
        int col = static_cast<int>(std::round(x));
        int row = static_cast<int>(std::round(y));
        int16_t z = reader.getValue(row, col);
        if (z == HGT_NODATA) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return static_cast<double>(z);
    }

    if (method == "bilinear") {
        int c0 = static_cast<int>(std::floor(x));
        int r0 = static_cast<int>(std::floor(y));
        int c1 = std::min(c0 + 1, HGT_SIZE - 1);
        int r1 = std::min(r0 + 1, HGT_SIZE - 1);

        double wx = x - c0;
        double wy = y - r0;

        int16_t z00 = reader.getValue(r0, c0);
        int16_t z01 = reader.getValue(r0, c1);
        int16_t z10 = reader.getValue(r1, c0);
        int16_t z11 = reader.getValue(r1, c1);

        std::vector<double> vals = {
            static_cast<double>(z00), static_cast<double>(z01),
            static_cast<double>(z10), static_cast<double>(z11)
        };
        std::vector<double> wts = {
            (1.0 - wx) * (1.0 - wy),
            wx * (1.0 - wy),
            (1.0 - wx) * wy,
            wx * wy
        };

        double sumVals = 0.0;
        double sumWts = 0.0;

        for (size_t i = 0; i < vals.size(); ++i) {
            if (vals[i] != HGT_NODATA) {
                sumVals += vals[i] * wts[i];
                sumWts += wts[i];
            }
        }

        if (sumWts == 0.0) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return sumVals / sumWts;
    }

    return std::numeric_limits<double>::quiet_NaN();
}