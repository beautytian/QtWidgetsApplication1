#include "GeoidQuery.h"
#include <iostream>
#include <cmath>
#include <limits>
#include <vector>

GeoidQuery::GeoidQuery(const std::string& path)
    : m_gridPath(path)
    , m_dataset(nullptr)
    , m_width(0)
    , m_height(0)
    , m_nodata(0)
    , m_loaded(false)
{
    load();
}

GeoidQuery::~GeoidQuery()
{
    if (m_dataset) {
        GDALClose(m_dataset);
        m_dataset = nullptr;
    }
}

bool GeoidQuery::load()
{
    GDALAllRegister();
    m_dataset = static_cast<GDALDataset*>(GDALOpen(m_gridPath.c_str(), GA_ReadOnly));
    if (!m_dataset) {
        std::cerr << "Cannot open Geoid file: " << m_gridPath << std::endl;
        m_loaded = false;
        return false;
    }

    if (m_dataset->GetRasterCount() < 1) {
        std::cerr << "Geoid file has no raste band" << std::endl;
        GDALClose(m_dataset);
        m_dataset = nullptr;
        m_loaded = false;
        return false;
    }

    if (m_dataset->GetGeoTransform(m_geotransform) != CE_None) {
        std::cerr << "Cannot get geotransform parameters" << std::endl;
        GDALClose(m_dataset);
        m_dataset = nullptr;
        m_loaded = false;
        return false;
    }

    m_width = m_dataset->GetRasterXSize();
    m_height = m_dataset->GetRasterYSize();
    m_nodata = m_dataset->GetRasterBand(1)->GetNoDataValue();

    m_loaded = true;
    return true;
}

double GeoidQuery::query(double lon, double lat) const
{
    if (!m_loaded) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    double lon_use = lon;
    // 处理经度范围
    if (lon_use < m_geotransform[0]) {
        lon_use += 360.0;
    }
    else if (lon_use > m_geotransform[0] + m_width * m_geotransform[1]) {
        lon_use -= 360.0;
    }

    // 像素坐标
    double col_f = (lon_use - m_geotransform[0]) / m_geotransform[1];
    double row_f = (lat - m_geotransform[3]) / m_geotransform[5];

    if (col_f < 0.0 || col_f > m_width - 1 || row_f < 0.0 || row_f > m_height - 1) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    int c0 = static_cast<int>(std::floor(col_f));
    int r0 = static_cast<int>(std::floor(row_f));
    int c1 = std::min(c0 + 1, m_width - 1);
    int r1 = std::min(r0 + 1, m_height - 1);

    double wx = col_f - c0;
    double wy = row_f - r0;

    GDALRasterBand* band = m_dataset->GetRasterBand(1);

    // 读取4个角点的值
    float vals[4];
    band->RasterIO(GF_Read, c0, r0, 1, 1, &vals[0], 1, 1, GDT_Float32, 0, 0);
    band->RasterIO(GF_Read, c1, r0, 1, 1, &vals[1], 1, 1, GDT_Float32, 0, 0);
    band->RasterIO(GF_Read, c0, r1, 1, 1, &vals[2], 1, 1, GDT_Float32, 0, 0);
    band->RasterIO(GF_Read, c1, r1, 1, 1, &vals[3], 1, 1, GDT_Float32, 0, 0);

    std::vector<double> dvals(4);
    for (int i = 0; i < 4; ++i) {
        dvals[i] = static_cast<double>(vals[i]);
    }

    std::vector<double> wts = {
        (1.0 - wx) * (1.0 - wy),
        wx * (1.0 - wy),
        (1.0 - wx) * wy,
        wx * wy
    };

    double sumVals = 0.0;
    double sumWts = 0.0;

    for (int i = 0; i < 4; ++i) {
        bool isValid;
        if (std::isnan(m_nodata) || m_nodata == 0) {
            isValid = !std::isnan(dvals[i]);
        }
        else {
            isValid = std::abs(dvals[i] - m_nodata) > 1e-6;
        }

        if (isValid) {
            sumVals += dvals[i] * wts[i];
            sumWts += wts[i];
        }
    }

    if (sumWts == 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    return sumVals / sumWts;
}