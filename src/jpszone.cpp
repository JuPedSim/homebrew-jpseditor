/**
 * \file        rooms.cpp
 * \date        Oct-01-2018
 * \version     v0.8.4
 * \copyright   <2009-2018> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * This class represents a (sub)room. Walls and/or doors belonging to a certain room have to defined with the help of roomwidget.
 *
 *
 **/

#include "rooms.h"
#include <QtGui>
#include <iostream>
#include <fstream>
#include <QGraphicsLineItem>
#include "jpscrossing.h"
#include <QDebug>



jpsRoom::jpsRoom(int id_room)
{
    id=id_room;
    QString string="Room ";
    name=string.append(QString::number(id));
    highlighted=false;
    area_=0.0;

    A_x=0.0; //plane equation: A_x . x + B_y . y + C_z = z
    B_y=0.0;
    C_z=0.0;
    _elevation=0;
    visible=true;
}


void jpsRoom::addWall(QList <jpsLineItem *> newWalls)
{
    qDebug("Enter jpsRoom::removeWall(newWall)");
    for (int i=0; i<newWalls.size(); i++)
    {
        addWall(newWalls[i]);
    }
    qDebug("Leave jpsRoom::removeWall(newWall)");
}

void jpsRoom::addWall(jpsLineItem *newWall)
{
    if (newWall->is_Wall() && !wall_list.contains(newWall))
    {
        wall_list.push_back(newWall);
    }
}

void jpsRoom::addinnerWall(QList<jpsLineItem *> newWalls, int id_polygon)
{
    for (int i=0; i<newWalls.size(); ++i)
    {
        addinnerWall(newWalls[i],id_polygon);
    }
}

void jpsRoom::addinnerWall(jpsLineItem *newWall, int id_polygon)
{
    if (newWall->is_Wall() && !wall_list.contains(newWall))
    {
        if (id_polygon >= inner_polygons.size())
            inner_polygons.push_back(QVector<QLineF>{});

        wall_list.push_back(newWall);
        inner_polygons[id_polygon].push_back(newWall->get_line()->line());
    }
}

void jpsRoom::removeWall(QList <jpsLineItem *> wall)
{
     qDebug("Enter jpsRoom::removeWall(wall)");
    for (int i=0; i<wall.size(); i++)
    {
        wall_list.removeOne(wall[i]);
    }
}

const QList<jpsCrossing *>& jpsRoom::GetDoors() const
{
    return doorList_;
}

void jpsRoom::removeDoor(jpsCrossing *crossing)
{
    if (doorList_.contains(crossing))
    {
        doorList_.removeOne(crossing);
    }
     qDebug("Leave jpsRoom::removeWall");
}

QString jpsRoom::get_name()
{
    return name;
}

void jpsRoom::change_name(QString name)
{
    this->name=name;
}

QList<jpsLineItem *> jpsRoom::get_listWalls()
{
    return wall_list;
}

void jpsRoom::activate()
{
    for (signed int i=0; i<wall_list.size(); i++)
    {
        wall_list[i]->get_line()->setPen(QPen(Qt::yellow));
    }
}

QVector<QPointF> jpsRoom::get_vertices() const
{
    QVector<QPointF> vertices;

    for (int i=0; i<wall_list.size(); i++)
    {
        if (vertices.contains(wall_list[i]->get_line()->line().p1())==false)
        {
            vertices.push_back(wall_list[i]->get_line()->line().p1());
        }
        if (vertices.contains(wall_list[i]->get_line()->line().p2())==false)
        {
            vertices.push_back(wall_list[i]->get_line()->line().p2());
        }
    }
    for (jpsCrossing* crossing:doorList_)
    {
        if (vertices.contains(crossing->get_cLine()->get_line()->line().p1())==false)
        {
            vertices.push_back(crossing->get_cLine()->get_line()->line().p1());
        }

        if (vertices.contains(crossing->get_cLine()->get_line()->line().p1())==false)
        {
            vertices.push_back(crossing->get_cLine()->get_line()->line().p1());
        }

    }

    return vertices;
}

QPointF jpsRoom::get_center()
{
    qDebug() << "Enter jpsRoom::get_center";
    IdentifyInnerOuter(); // write outer_polygon

    if(outer_polygon.size()==0){
        QPointF  origin(0,0);
        return origin;
    }

    QVector<QPointF> vertices = RoomAsSortedPolygon(outer_polygon);
    qreal sum_x=0;
    qreal sum_y=0;

    for (int i=0; i<vertices.size(); ++i)
    {
        sum_x+=vertices[i].x();
        sum_y+=vertices[i].y();
    }
    QPointF mean;
    mean.setX(sum_x/vertices.size());
    mean.setY(sum_y/vertices.size());

    qDebug() << "Leave jpsRoom::get_center";
    return mean;
}

void jpsRoom::highlight(const QString& color)
{
     qDebug("Enter jpsRoom::highlight. highlighted=<%d>", highlighted);

    if (!highlighted)
    {
        QPen pen;
        if (color=="darkMagenta")
            pen = QPen(Qt::darkMagenta,4);
        else if (color=="darkGreen")
            pen = QPen(Qt::darkGreen,4);
        else
            pen = QPen(rndColors(qrand()%100),4);
        pen.setCosmetic(true);

        for (int i=0; i<wall_list.size(); ++i)
        {
            wall_list[i]->get_line()->setPen(pen);
            wall_list[i]->set_defaultColor(color);
        }

        highlighted=true;
    }
    else
    {
        QPen pen = QPen(Qt::black,2);
        pen.setCosmetic(true);
        for (int i=0; i<wall_list.size(); i++)
        {
            wall_list[i]->get_line()->setPen(pen);
            wall_list[i]->set_defaultColor("black");
        }

        highlighted=false;
    }
    qDebug("Enter jpsRoom::highlight. highlight=<%d>", highlighted);
}

void jpsRoom::switchVisibility()
{
    qDebug("Enter jpsRoom::switchVisibility. visible=<%d>", visible);
    if(!visible)
    {
        QColor color(0,0,0,0);
        QPen pen(color);

        for (int i=0; i<wall_list.size(); i++)
        {
            wall_list[i]->get_line()->setPen(pen);
            wall_list[i]->set_defaultColor("white");
        }
    }
    else
    {
        QPen pen = QPen(Qt::black,2);
        pen.setCosmetic(true);
        for (int i=0; i<wall_list.size(); i++)
        {
            wall_list[i]->get_line()->setPen(pen);
            wall_list[i]->set_defaultColor("black");
        }
    }
    qDebug("Exit jpsRoom::switchVisibility. visible=<%d>", visible);
}


QString jpsRoom::get_type() const
{
    return _type;
}

void jpsRoom::set_type(const QString &string)
{
    _type=string;
}

QList<QPointF> jpsRoom::GetDoorVertices() const
{
    QList<QPointF> vertices;

    for (int i=0; i<wall_list.size(); i++)
    {
        int counterP1=0;
        int counterP2=0;
        for (int j=0; j<wall_list.size(); j++)
        {

            if (j!=i)
            {
                if (wall_list[i]->get_line()->line().p1()==wall_list[j]->get_line()->line().p1()
                        || wall_list[i]->get_line()->line().p1()==wall_list[j]->get_line()->line().p2())
                {
                    counterP1++;
                }
                else if (wall_list[i]->get_line()->line().p2()==wall_list[j]->get_line()->line().p1()
                        || wall_list[i]->get_line()->line().p2()==wall_list[j]->get_line()->line().p2())
                {
                    counterP2++;
                }
            }
        }
        if (counterP1==0)
        {
            if (!vertices.contains(wall_list[i]->get_line()->line().p1()))
            vertices.push_back(wall_list[i]->get_line()->line().p1());
        }
        else if (counterP2==0)
        {
            if (!vertices.contains(wall_list[i]->get_line()->line().p2()))
            vertices.push_back(wall_list[i]->get_line()->line().p2());
        }
    }

    return vertices;
}

void jpsRoom::AddDoor(jpsCrossing *door)
{
    if (!doorList_.contains(door))
        doorList_.push_back(door);
    //outer_polygon.push_back(door->get_cLine()->get_line()->line());
}

void jpsRoom::AddInnerDoor(jpsCrossing *door, int id_polygon)
{

    if (!doorList_.contains(door))
    {
        doorList_.push_back(door);
        if (id_polygon >= inner_polygons.size())
            inner_polygons.push_back(QVector<QLineF>{});

        inner_polygons[id_polygon].push_back(door->get_cLine()->get_line()->line());
    }
}

//QPolygonF jpsRoom::RoomAsPolygon() const
//{
//    return QPolygonF(get_vertices());
//}

QVector<QPointF> jpsRoom::RoomAsSortedPolygon(const QVector<QLineF>& lines) const
{
     qDebug("Enter jpsRoom::RoomAsSortedPolygon");
    QVector<QLineF> clines=lines;
    QVector<QPointF> points = {};

    if(lines.empty())
    {
         qDebug("\t empty lines!!");
         qDebug("Leave jpsRoom::RoomAsSortedPolygon with ERROR");         
         return points;
         
    }

    points.push_back(lines.first().p1());
    points.push_back(lines.first().p2());

    clines.pop_front();

    for (int i=0; i<clines.size(); ++i)
    {
        if (clines[i].p1() == points.last())
        {
            //points.push_back(line->get_line()->line().p1());
            points.push_back(clines[i].p2());
            clines.remove(i);
            i=-1;
        }
        else if (clines[i].p2() == points.last())
        {
            //points.push_back(line->get_line()->line().p1());
            points.push_back(clines[i].p1());
            clines.remove(i);
            i=-1;
        }
    }


//    for (QPointF point:points)
//    {
//        std::cout << point.x() << " " << point.y() << std::endl;

//    }
//    std::cout << "----------------------------" << std::endl;

    qDebug("Leave jpsRoom::RoomAsSortedPolygon");         
    return points;
}

const QVector<QLineF> &jpsRoom::GetOuterPolygon() const
{
    return outer_polygon;
}

qreal jpsRoom::GetArea() const
{
    return area_;
}


void jpsRoom::IdentifyInnerOuter()
{
    qDebug() << "Enter jpsRoom::IdentifyInnerOuter";
    QList<QLineF> lines;


    if (!wall_list.isEmpty()){
        for (jpsLineItem* lineItem:wall_list)
        {
            lines.append(lineItem->get_line()->line());
        }
        for (jpsCrossing* crossing:doorList_)
        {
            lines.append(crossing->get_cLine()->get_line()->line());
        }
    } else{
        outer_polygon = {};
        return;
    }


    QVector<QVector<QLineF>> polygons;

    while (!lines.empty())
    {
        polygons.push_back(QVector<QLineF>{});

        polygons.last().push_back(lines.first());
        lines.pop_front();

        bool found=true;

        while (found==true)
        {
            found=false;

            for (const QLineF& line:lines)
            {
                if (EqualsPoint(polygons.last().front().p2(),line.p1()))
                {
                    polygons.last().push_front(line);
                    lines.removeOne(line);
                    found=true;
                    break;
                }
                else if (EqualsPoint(polygons.last().front().p2(), line.p2()))
                {
                    polygons.last().push_front(QLineF(line.p2(),line.p1()));
                    lines.removeOne(line);
                    found=true;
                    break;
                }
                else if (EqualsPoint(polygons.last().front().p1(), line.p1()))
                {
                    polygons.last().push_front(QLineF(line.p2(),line.p1()));
                    lines.removeOne(line);
                    found=true;
                    break;
                }
                else if (EqualsPoint(polygons.last().front().p1(), line.p2()))
                {
                    polygons.last().push_front(line);
                    lines.removeOne(line);
                    found=true;
                    break;
                }
            }


        }
    }
    qreal maxArea=0.0;
    qreal sumArea=0.0;
    int n=0;
    int maxN=0;
    for (const QVector<QLineF>& poly:polygons)
    {
        qreal cArea=CalculateArea(poly);
        if (cArea>maxArea)
        {
            maxArea=cArea;
            maxN=n;
        }
        sumArea+=std::fabs(cArea);
        ++n;

    }

    area_=std::fabs(maxArea)-(sumArea-std::fabs(maxArea));
    //std::cout << area_ << std::endl;
    outer_polygon=polygons[maxN];
    polygons.remove(maxN);

    inner_polygons=polygons;

    qDebug() << "Leave jpsRoom::IdentifyInnerOuter";
}


QRectF jpsRoom::CalculateBoundingBox() const
{
    if (outer_polygon.empty())
        return QRectF();

    //xmin/xmax/ymin/ymax
    qreal x_min= outer_polygon.first().p1().x();
    qreal x_max= outer_polygon.first().p1().x();
    qreal y_min= outer_polygon.first().p1().y();
    qreal y_max= outer_polygon.first().p1().y();

    for (const QLineF& line:outer_polygon)
    {
        if (line.p1().x()<x_min)
            x_min=line.p1().x();
        else if (line.p1().x()>x_max)
            x_max=line.p1().x();
        if (line.p1().y()<y_min)
            y_min=line.p1().y();
        else if (line.p1().y()>y_max)
            y_max=line.p1().y();
    }

    //std::cout << x_min << x_max << y_min << y_max << std::endl;

    return QRectF(QPointF(x_min,y_min),QPointF(x_max,y_max));
}

qreal jpsRoom::CalculateArea(const QVector<QLineF>& poly) const
{
    if (poly.isEmpty())
        return 0.0;

//    QVector<QPointF> points = RoomAsSortedPolygon(poly);

    qreal area=0.0;
    int i, j=poly.size()-1;

    for (i=0; i<poly.size(); ++i)
    {
        area+=(poly[j].p1().x()+poly[i].p1().x())*(poly[j].p1().y()-poly[i].p1().y());
        j=i;
    }

    area=std::fabs(area*0.5);

    return area;
}


void JPSZone::set_id(int id_room)
{
    id=id_room;
}

int jpsRoom::get_id()
{
    return id;
}
float jpsRoom::get_ax()
{
     return A_x;
}
void jpsRoom::set_ax(float AX)
{
     A_x=AX;
}

float jpsRoom::get_by()
{
     return B_y;
}
void jpsRoom::set_by(float BY)
{
     B_y=BY;
}

float jpsRoom::get_cz()
{
     return C_z;
}
void jpsRoom::set_cz(float CZ)
{
     C_z=CZ;
}


bool EqualsPoint(const QPointF& point1, const QPointF& point2, double eps)
{
    if (std::fabs(point1.x()-point2.x())<eps && std::fabs(point1.y()-point2.y())<eps)
        return true;
    else
        return false;
}

inline QColor rndColors(int seed){
    QColor color;
    float currentHue = 0.0;
    for (int i = 0; i < seed; i++){
        color= QColor::fromHslF(currentHue, 1.0, 0.5);
        currentHue += 0.618033988749895f;
        currentHue = std::fmod(currentHue, 1.0f);
    }
    return color;
}

QPointF jpsRoom::get_up()
{
     return up_;
}

void jpsRoom::set_up(QPointF up)
{
     up_ = up;
}

QPointF jpsRoom::get_down()
{
     return down_;
}

void jpsRoom::set_down(QPointF down)
{
     down_ = down;
}

void jpsRoom::switchHighlight()
{
    highlighted = !highlighted;
}

float jpsRoom::get_elevation()
{
     return elevation_;
}

void jpsRoom::set_elevation(float elevation)
{
     if(this->get_type().toUpper() != "STAIR") // only for horizontal floors
          for (auto crossing: _doorList)
               crossing->set_elevation(elevation);

     elevation_ = elevation;
}

void jpsRoom::correctPlaneCoefficients()
{
     qDebug("Enter correctPlaneCoefficients");
     qDebug("\t room=<%s> of type=<%s> has %d doors",
            this->get_name().toStdString().c_str(),
            this->get_type().toStdString().c_str(),
            (int)_doorList.size());
    if(_doorList.size() == 0 || this->get_type().toUpper() != "Stair")
    {
        this->set_ax(0);
        this->set_by(0);
        this->set_cz(this->get_elevation());
        return;
    }

    QPointF P1(0,0), P2(0,0), P3(0,0); // plane is defined by three non-collinear points
    float elevation_1=0, elevation_2=0;
    // P1 P2 are the points on the first door of a room
    P1 = doorList_[0]->get_cLine()->get_line()->line().p1();
    P2 = doorList_[0]->get_cLine()->get_line()->line().p2();
    elevation_1 = doorList_[0]->get_elevation();

    //from _doortList get one more point on a door with different elevation as P3
    for (int i=1; i<doorList_.size(); i++)
    {
      if(doorList_[i]->get_elevation() != doorList_[0]->get_elevation()){
           P3 = doorList_[i]->get_cLine()->get_line()->line().p1();
           elevation_2 = doorList_[i]->get_elevation();
           break;
      }
    }

     // variables for convenience
     float P1_x = P1.x();
     float P1_y = P1.y();
     float P1_z = elevation_1;

     float P2_x = P2.x();
     float P2_y = P2.y();
     float P2_z = elevation_1;

     float P3_x = P3.x();
     float P3_y = P3.y();
     float P3_z = elevation_2;
     qDebug("\t P1=(%.2f, %.2f, %.2f), P2=(%.2f, %.2f, %.2f), P3=(%.2f, %.2f, %.2f)",
            P1_x, P1_y, P1_z,
            P2_x, P2_y, P2_z,
            P3_x, P3_y, P3_z
          );
     float d = 1.0;
     // Thanks M. Osterkamp
     float c = (((1-P3_x/P1_x*1)-((P3_y-P3_x/P1_x*P1_y)/(P2_y-P2_x/P1_x*P1_y))
                 *(1-P2_x/P1_x*1))/((P3_z-P3_x/P1_x*P1_z)
                                    -((P3_y-P3_x/P1_x*P1_y)/(P2_y-P2_x/P1_x*P1_y))
                                    *(P2_z-P2_x/P1_x*P1_z)))*d;

     float b = ((1-P3_x/P1_x*1)*d-(P3_z-P3_x/P1_x*P1_z)*c)/(P3_y-P3_x/P1_x*P1_y);
     float a = (d-P1_z*c-P1_y*b)/P1_x;
     qDebug("\t a = %.2f, b = %.2f, c= %.2f", a, b, c);
     set_ax(-a/c);
     set_by(-b/c);
     set_cz(d/c);
     qDebug("\t ax = %.2f, by = %.2f, cz= %.2f", -a/c, -b/c, d/c);
     qDebug("Leave correctPlaneCoefficients");
}

bool jpsRoom::is_highlighted()
{
     return highlighted;
}

bool jpsRoom::isVisible()
{
    return visible;
}

void jpsRoom::setVisible(bool visibility)
{
    visible = visibility;
}

JPSZone *JPSZone::getFatherZone() const
{
    qDebug("Enter JPSZone::getFatherZone");
    return father_zone;
    qDebug("Leave JPSZone::getFatherZone");
}

void JPSZone::setFatherZone(JPSZone *fatherZone)
{
    qDebug("Enter JPSZone::setFatherZone");
    father_zone = fatherZone;
    qDebug("Leave JPSZone::setFatherZone");
}

const QString &JPSZone::getName() const {
    return name;
}

void JPSZone::setName(const QString &name) {
    JPSZone::name = name;
}
