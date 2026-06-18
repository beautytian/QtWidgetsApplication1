#ifndef HGTCOLLECTION_H
#define HGTCOLLECTION_H

#include <string>
#include <map>
#include <memory>
#include "HGTReader.h"

class HGTCollection
{
public:
    explicit HGTCollection(const std::string& dir);
    ~HGTCollection() = default;

    void loadTiles();
    std::string findTilePath(double lon, double lat) const;
    double samplePoint(double lon, double lat, const std::string& method) const;
    size_t tileCount() const { return m_tileMap.size(); }

private:
    std::string m_hgtDir;
    std::map<std::string, std::string> m_tileMap;

    std::string tileNameFromLonLat(double lon, double lat) const;
    void tileSWCorner(double lon, double lat, int& lon0, int& lat0) const;
    void lonLatToXYInTile(double lon, double lat, int lon0, int lat0, double& x, double& y) const;
};

#endif // HGTCOLLECTION_H#pragma once
