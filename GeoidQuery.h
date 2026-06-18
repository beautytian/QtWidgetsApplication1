#pragma once
#ifndef GEOIDQUERY_H
#define GEOIDQUERY_H

#include <string>
#include <gdal_priv.h>

class GeoidQuery
{
public:
    explicit GeoidQuery(const std::string& path);
    ~GeoidQuery();

    bool load();
    bool isLoaded() const { return m_loaded; }
    double query(double lon, double lat) const;

private:
    std::string m_gridPath;
    GDALDataset* m_dataset;
    double m_geotransform[6];
    int m_width;
    int m_height;
    double m_nodata;
    bool m_loaded;
};

#endif // GEOIDQUERY_H