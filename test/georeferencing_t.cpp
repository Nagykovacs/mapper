/*
 *    Copyright 2012-2015 Kai Pastor
 *
 *    This file is part of OpenOrienteering.
 *
 *    OpenOrienteering is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    OpenOrienteering is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with OpenOrienteering.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "georeferencing_t.h"

#include <proj_api.h>

#include "../src/core/crs_template.h"
#include "../src/file_format_xml.h"


int XMLFileFormat::active_version = 6;


namespace
{
	QString epsg5514_spec = QLatin1String("+init=epsg:5514");
	QString gk2_spec   = QLatin1String("+proj=tmerc +lat_0=0 +lon_0=6 +k=1.000000 +x_0=2500000 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs");
	QString gk3_spec   = QLatin1String("+proj=tmerc +lat_0=0 +lon_0=9 +k=1.000000 +x_0=3500000 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs");
	QString utm32_spec = QLatin1String("+proj=utm +zone=32 +datum=WGS84");
	
	
	/**
	 * Returns the radian value of a value given in degree or degree/minutes/seconds.
	 */
	double degFromDMS(double d, double m=0.0, double s=0.0)
	{
		return d + m/60.0 + s/3600.0;
	}
}


void GeoreferencingTest::initTestCase()
{
	// nothing
}


void GeoreferencingTest::testEmptyProjectedCRS()
{
	Georeferencing new_georef;
	QVERIFY(new_georef.getState() == Georeferencing::Local);
}


void GeoreferencingTest::testCRS_data()
{
	QTest::addColumn<QString>("id");
	QTest::addColumn<QString>("spec");
	
	QTest::newRow("EPSG:4326") << "EPSG:4326" << QString("+init=epsg:4326");
	QTest::newRow("UTM")       << "UTM"       << utm32_spec;
}

void GeoreferencingTest::testCRS()
{
	QFETCH(QString, id);
	QFETCH(QString, spec);
	QVERIFY2(georef.setProjectedCRS(id, spec), georef.getErrorText().toLatin1());
}


void GeoreferencingTest::testCRSTemplates()
{
	auto epsg_template = CRSTemplateRegistry().find("EPSG");
	QCOMPARE(epsg_template->parameters().size(), (std::size_t)1);
	
	QCOMPARE(epsg_template->coordinatesName(), QString{"EPSG @code@ coordinates"});
	QCOMPARE(epsg_template->coordinatesName({ "4326" }), QString{"EPSG 4326 coordinates"});
	
	georef.setProjectedCRS("EPSG", epsg_template->specificationTemplate().arg("5514"), { "5514" });
	QVERIFY(georef.isValid());
	QCOMPARE(georef.getProjectedCoordinatesName(), QString{"EPSG 5514 coordinates"});
}


void GeoreferencingTest::testProjection_data()
{
	QTest::addColumn<QString>("proj");
	QTest::addColumn<double>("easting");
	QTest::addColumn<double>("northing");
	QTest::addColumn<double>("latitude");
	QTest::addColumn<double>("longitude");
	
	// Record name                               CRS spec      Easting      Northing     Latitude (radian)           Longitude (radian)
	// Selected from http://www.lvermgeo.rlp.de/index.php?id=5809
	QTest::newRow("LVermGeo RLP Koblenz UTM") << utm32_spec <<  398125.0 << 5579523.0 << degFromDMS(50, 21, 32.2) << degFromDMS( 7, 34, 4.0);
	QTest::newRow("LVermGeo RLP Koblenz GK3") << gk3_spec   << 3398159.0 << 5581315.0 << degFromDMS(50, 21, 32.2) << degFromDMS( 7, 34, 4.0);
	QTest::newRow("LVermGeo RLP Pruem UTM")   << utm32_spec <<  316464.0 << 5565150.0 << degFromDMS(50, 12, 36.1) << degFromDMS( 6, 25, 39.6);
	QTest::newRow("LVermGeo RLP Pruem GK2")   << gk2_spec   << 2530573.0 << 5563858.0 << degFromDMS(50, 12, 36.1) << degFromDMS( 6, 25, 39.6);
	QTest::newRow("LVermGeo RLP Landau UTM")  << utm32_spec <<  436705.0 << 5450182.0 << degFromDMS(49, 12,  4.2) << degFromDMS( 8,  7, 52.0);
	QTest::newRow("LVermGeo RLP Landau GK3")  << gk3_spec   << 3436755.0 << 5451923.0 << degFromDMS(49, 12,  4.2) << degFromDMS( 8,  7, 52.0);
	
	// Selected from http://geoportal.cuzk.cz/geoprohlizec/?lng=EN, source Bodová pole, layer Bod ZPBP určený v ETRS
	// http://dataz.cuzk.cz/gu.php?1=25&2=08&3=024&4=a&stamp=7oexTQLNPE8ri5SXY04ARS7vIMnJu3N2
	QTest::newRow("EPSG 5514 ČÚZK Dolní Temenice") << epsg5514_spec   << -563714.79 << -1076943.54 << degFromDMS(49, 58, 37.5577) << degFromDMS(16, 57, 35.5493);
}


void GeoreferencingTest::testProjection()
{
#if PJ_VERSION >= 480
	const double max_dist_error = 2.2; // meter
	const double max_angl_error = 0.00004; // degrees
#else
	const double max_dist_error = 5.5; // meter
	const double max_angl_error = 0.00007; // degrees
#endif
	
	QFETCH(QString, proj);
	QVERIFY2(georef.setProjectedCRS(proj, proj), proj.toLatin1());
	QCOMPARE(georef.getErrorText(), QString(""));
	
	QFETCH(double, easting);
	QFETCH(double, northing);
	QFETCH(double, latitude);
	QFETCH(double, longitude);
	
	// geographic to projected
	LatLon lat_lon(latitude, longitude);
	bool ok;
	QPointF proj_coord = georef.toProjectedCoords(lat_lon, &ok);
	QVERIFY(ok);
	
	if (fabs(proj_coord.x() - easting) > max_dist_error)
		QCOMPARE(QString::number(proj_coord.x(), 'f'), QString::number(easting, 'f'));
	if (fabs(proj_coord.y() - northing) > max_dist_error)
		QCOMPARE(QString::number(proj_coord.y(), 'f'), QString::number(northing, 'f'));
	
	// projected to geographic
	proj_coord = QPointF(easting, northing);
	lat_lon = georef.toGeographicCoords(proj_coord, &ok);
	QVERIFY(ok);
	if (fabs(lat_lon.latitude() - latitude) > max_angl_error)
		QCOMPARE(QString::number(lat_lon.latitude(), 'f'), QString::number(latitude, 'f'));
	if (fabs(lat_lon.longitude() - longitude) > (max_angl_error / cos(latitude)))
		QCOMPARE(QString::number(lat_lon.longitude(), 'f'), QString::number(longitude, 'f'));
}



QTEST_GUILESS_MAIN(GeoreferencingTest)
