/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/

#ifndef RS_ARC_H
#define RS_ARC_H

#include "rs_atomicentity.h"
class LC_Quadratic;


/**
 * Holds the data that defines an arc.
 */
struct RS_ArcData {
	RS_ArcData() = default;
	~RS_ArcData() = default;

	RS_ArcData(const RS_Vector& center,
			   double radius,
			   double angle1, double angle2,
			   bool reversed);

	void reset();

	bool isValid() const;

	RS_Vector center;
	double radius;
	double angle1;
	double angle2;
	bool reversed;
};

std::ostream& operator << (std::ostream& os, const RS_ArcData& ad);


/**
 * Class for an arc entity. All angles are in Rad.
 *
 * @author Andrew Mustun
 */
class RS_Arc : public RS_AtomicEntity {
public:
	RS_Arc()=default;
    RS_Arc(RS_EntityContainer* parent,
           const RS_ArcData& d);
	virtual ~RS_Arc() = default;

	virtual RS_Entity* clone() const;

    /**	@return RS2::EntityArc */
    virtual RS2::EntityType rtti() const {
        return RS2::EntityArc;
    }
    /** @return true */
    virtual bool isEdge() const {
        return true;
    }

    /** @return Copy of data that defines the arc. **/
    RS_ArcData getData() const {
        return data;
    }

	virtual RS_VectorSolutions getRefPoints() const;

    /** Sets new arc parameters. **/
    void setData(RS_ArcData d) {
        data = d;
    }

    /** @return The center point (x) of this arc */
    virtual RS_Vector getCenter() const {
        return data.center;
    }
    /** Sets new center. */
    void setCenter(const RS_Vector& c) {
        data.center = c;
    }

    /** @return The radius of this arc */
    virtual double getRadius() const {
        return data.radius;
    }
    /** Sets new radius. */
    void setRadius(double r) {
        data.radius = r;
    }

    /** @return The start angle of this arc */
    double getAngle1() const {
        return data.angle1;
    }
    /** Sets new start angle. */
    void setAngle1(double a1) {
        data.angle1 = a1;
    }
    /** @return The end angle of this arc */
    double getAngle2() const {
        return data.angle2;
    }
    /** Sets new end angle. */
    void setAngle2(double a2) {
        data.angle2 = a2;
    }
    /** get angle relative arc center*/
    double getArcAngle(const RS_Vector& vp) {
        return (vp - data.center).angle();
    }
    /**
     * @return Direction 1. The angle at which the arc starts at
     * the startpoint.
     */
	double getDirection1() const;
    /**
     * @return Direction 2. The angle at which the arc starts at
     * the endpoint.
     */
	double getDirection2() const;

    /**
     * @retval true if the arc is reversed (clockwise),
     * @retval false otherwise
     */
    bool isReversed() const {
        return data.reversed;
    }
    /** sets the reversed status. */
    void setReversed(bool r) {
        data.reversed = r;
    }

    /** @return Start point of the entity. */
    virtual RS_Vector getStartpoint() const {
        return startpoint;
    }
    /** @return End point of the entity. */
    virtual RS_Vector getEndpoint() const {
        return endpoint;
    }
	virtual std::vector<RS_Entity* > offsetTwoSides(const double& distance) const;
    /**
          * implementations must revert the direction of an atomic entity
          */
    virtual void revertDirection();
    virtual void correctAngles();//make sure angleLength() is not more than 2*M_PI
    virtual void moveStartpoint(const RS_Vector& pos);
    virtual void moveEndpoint(const RS_Vector& pos);
    virtual bool offset(const RS_Vector& position, const double& distance);

    virtual void trimStartpoint(const RS_Vector& pos);
    virtual void trimEndpoint(const RS_Vector& pos);

    virtual RS2::Ending getTrimPoint(const RS_Vector& coord,
                                     const RS_Vector& trimPoint);
    /** choose an intersection to trim to based on mouse point */
    virtual RS_Vector prepareTrim(const RS_Vector& mousePoint,
                                  const RS_VectorSolutions& trimSol);

    virtual void reverse();

    virtual RS_Vector getMiddlePoint() const;
    double getAngleLength() const;
    virtual double getLength() const;
    double getBulge() const;

    bool createFrom3P(const RS_Vector& p1, const RS_Vector& p2,
                      const RS_Vector& p3);
    bool createFrom2PDirectionRadius(const RS_Vector& startPoint, const RS_Vector& endPoint,
                                     double direction1, double radius);
    bool createFrom2PDirectionAngle(const RS_Vector& startPoint, const RS_Vector& endPoint,
                                     double direction1, double angleLength);
    bool createFrom2PBulge(const RS_Vector& startPoint, const RS_Vector& endPoint,
                           double bulge);

	virtual RS_Vector getNearestEndpoint(const RS_Vector& coord,
										 double* dist = nullptr) const;
	virtual RS_Vector getNearestPointOnEntity(const RS_Vector& coord,
											  bool onEntity = true,
											  double* dist = nullptr,
											  RS_Entity** entity=nullptr) const;
    virtual RS_Vector getNearestCenter(const RS_Vector& coord,
									   double* dist = nullptr) const;
    virtual RS_Vector getNearestMiddle(const RS_Vector& coord,
									   double* dist = nullptr,
                                       int middlePoints = 1
									   ) const;
    virtual RS_Vector getNearestDist(double distance,
                                     const RS_Vector& coord,
									 double* dist = nullptr) const;
    virtual RS_Vector getNearestDist(double distance,
									 bool startp) const;
    virtual RS_Vector getNearestOrthTan(const RS_Vector& coord,
                    const RS_Line& normal,
					bool onEntity = false) const;
    virtual RS_VectorSolutions getTangentPoint(const RS_Vector& point) const;//find the tangential points seeing from given point
	virtual RS_Vector getTangentDirection(const RS_Vector& point) const;
    virtual void move(const RS_Vector& offset);
    virtual void rotate(const RS_Vector& center, const double& angle);
    virtual void rotate(const RS_Vector& center, const RS_Vector& angleVector);
    virtual void scale(const RS_Vector& center, const RS_Vector& factor);
    virtual void mirror(const RS_Vector& axisPoint1, const RS_Vector& axisPoint2);
    virtual void moveRef(const RS_Vector& ref, const RS_Vector& offset);
    virtual void stretch(const RS_Vector& firstCorner,
                         const RS_Vector& secondCorner,
                         const RS_Vector& offset);

    /** find the visible part of the arc, and call drawVisible() to draw */
    virtual void draw(RS_Painter* painter, RS_GraphicView* view, double& patternOffset);
    /** directly draw the arc, assuming the whole arc is within visible window */
    virtual void drawVisible(RS_Painter* painter, RS_GraphicView* view, double& patternOffset);

    friend std::ostream& operator << (std::ostream& os, const RS_Arc& a);

    virtual void calculateEndpoints();
    virtual void calculateBorders();
    /** return the equation of the entity
for quadratic,

return a vector contains:
m0 x^2 + m1 xy + m2 y^2 + m3 x + m4 y + m5 =0

for linear:
m0 x + m1 y + m2 =0
**/
    virtual LC_Quadratic getQuadratic() const;
    /**
     * @brief areaLineIntegral, line integral for contour area calculation by Green's Theorem
     * Contour Area =\oint x dy
     * @return line integral \oint x dy along the entity
     * \oint x dy = c_x r \sin t + \frac{1}{4}r^2\sin 2t +  \frac{1}{2}r^2 t
     */
    virtual double areaLineIntegral() const;

protected:
    RS_ArcData data;

    /**
     * Startpoint. This is redundant but stored for performance
     * reasons.
     */
    RS_Vector startpoint;
    /**
     * Endpoint. This is redundant but stored for performance
     * reasons.
     */
    RS_Vector endpoint;
};

#endif
