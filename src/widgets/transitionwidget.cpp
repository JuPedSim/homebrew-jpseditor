/***************************************************************
 *
 * \file transitionwidget.cpp
 * \date 2019-06-26
 * \version 0.8.9
 * \author Tao Zhong
 * \copyright <2009-2019> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section Lincense
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
 *
 * This class is for transition widget
****************************************************************/


#include "transitionwidget.h"
#include "ui_transitionwidget.h"

TransitionWidget::TransitionWidget(QWidget *parent, jpsDatamanager *dmanager, jpsGraphicsView *mview)
    : QWidget(parent), ui(new Ui::TransitionWidget)
{
    qDebug("Enter TransitionWidget::TransitionWidget");
    ui->setupUi(this);

    data = dmanager;
    view = mview;

    updateListWidget();

    connect(ui->listWidget_transitions, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(highlightWall(QListWidgetItem *)));
    connect(ui->listWidget_transitions,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(showRoomsinButton(QListWidgetItem*)));
    connect(ui->pushButton_apply, SIGNAL(clicked()),this,SLOT(applyRooms()));
    connect(ui->pushButton_delete, SIGNAL(clicked()),this,SLOT(deleteButtonClicked()));

    connect(view, SIGNAL(transitonsChanged()), this, SLOT(updateListWidget()));
    connect(view, SIGNAL(markedLineDeleted()), this, SLOT(updateListWidget()));
    qDebug("Leave TransitionWidget::TransitionWidget");
}

TransitionWidget::~TransitionWidget()
{
    qDebug("Enter TransitionWidget::~TransitionWidget");
    delete ui;
    qDebug("Leave TransitionWidget::~TransitionWidget");
}

void TransitionWidget::updateListWidget()
{
    qDebug("Enter TransitionWidget::updateWallListWidget");
    ui->listWidget_transitions->clear();

    QList<jpsTransition *> transition_list = data->getTransitionList();

    if(transition_list.isEmpty())
        return;

    for (int i=0; i<transition_list.size(); i++)
    {
        QString string = "";
        string.sprintf("[%+06.3f, %+06.3f] - [%+06.3f, %+06.3f]",
                       transition_list[i]->get_cLine()->get_line()->line().x1(),
                       transition_list[i]->get_cLine()->get_line()->line().x2(),
                       transition_list[i]->get_cLine()->get_line()->line().y1(),
                       transition_list[i]->get_cLine()->get_line()->line().y2());

        ui->listWidget_transitions->addItem(string);
    }
    qDebug("Leave TransitionWidget::updateWallListWidget");
}

void TransitionWidget::applyRooms()
{
    qDebug("Enter applyRooms");
    if(ui->listWidget_transitions->currentItem() == nullptr)
        return;

    int cRow = ui->listWidget_transitions->currentRow();
    jpsTransition *cTran = data->getTransitionList()[cRow];

    JPSZone *room1 = nullptr;
    JPSZone *room2 = nullptr;

    for(JPSZone * room : data->getRoomlist())
    {
        if(room->getName() == ui->comboBox_from->currentText())
            room1 = room;

        if(room->getName() == ui->comboBox_to->currentText())
            room2 = room;
    }

    for(JPSZone * stair : data->getStair_list())
    {
        if(stair->getName() == ui->comboBox_from->currentText())
            room1 = stair;

        if(stair->getName() == ui->comboBox_to->currentText())
            room2 = stair;
    }

    for(JPSZone * platform : data->getPlatform_list())
    {
        if(platform->getName() == ui->comboBox_from->currentText())
            room1 = platform;

        if(platform->getName() == ui->comboBox_to->currentText())
            room2 = platform;
    }

    cTran->set_rooms(room1, room2);

    // Set elevation of transitions when setting room
    if(room1 != nullptr && room1->getType() != Stair)
    {
        cTran->setElevation(room1->get_elevation());
    }

    if(room2 != nullptr && room2->getType() != Stair)
    {
        cTran->setElevation(room2->get_elevation());
    }

    qDebug("Leave applyRooms");
}

void TransitionWidget::showRoomsinButton(QListWidgetItem *item)
{
    qDebug("Enter TransitionWidget::showRoomsinButton");
    // Add room list
    ui->comboBox_from->clear();
    ui->comboBox_to->clear();
    ui->comboBox_from->addItem("Outside");
    ui->comboBox_to->addItem("Outside");

    for(JPSZone *room : data->getRoomlist())
    {
        ui->comboBox_from->addItem(room->getName());
        ui->comboBox_to->addItem(room->getName());
    }

    for(JPSZone *stair : data->getStair_list())
    {
        ui->comboBox_from->addItem(stair->getName());
        ui->comboBox_to->addItem(stair->getName());
    }

    for(JPSZone *platform : data->getPlatform_list())
    {
        ui->comboBox_from->addItem(platform->getName());
        ui->comboBox_to->addItem(platform->getName());
    }
    // Set current room
    int cRow = ui->listWidget_transitions->currentRow();
    auto *transition = data->getTransitionList()[cRow];

    if(!transition->get_roomList().isEmpty())
    {
        if(transition->get_roomList()[0] == nullptr)
        {
            ui->comboBox_from->setCurrentText("Outside");
        } else
        {
            ui->comboBox_from->setCurrentText(transition->get_roomList()[0]->getName());
        }

        if(transition->get_roomList()[1] == nullptr)
        {
            ui->comboBox_to->setCurrentText("Outside");
        } else
        {
            ui->comboBox_to->setCurrentText(transition->get_roomList()[1]->getName());
        }
    }

    qDebug("Leave TransitionWidget::showRoomsinButton");
}

void TransitionWidget::highlightWall(QListWidgetItem *item)
{
    qDebug("Enter PropertyWidget::highlightWall");
    int cRow = ui->listWidget_transitions->currentRow();

    auto *line = data->getTransitionList()[cRow];
    view->unmark_all_lines();
    view->markLine(line->get_cLine());

    qDebug("Leave PropertyWidget::highlightWall");
}

void TransitionWidget::deleteButtonClicked()
{
    qDebug("Enter TransitionWidget::deleteButtonClicked");
    if(ui->listWidget_transitions->currentItem() == nullptr)
    {
        return;
    }
    else
    {
        // Delete transition in data manager
        int row = ui->listWidget_transitions->currentRow();
        auto *transition = data->getTransitionList()[row];
        data->removeTransition(transition);

        // Delete transition in view
        view->delete_marked_lines();

        // Update widget
        updateListWidget();
    }
    qDebug("Leave TransitionWidget::deleteButtonClicked");
}
