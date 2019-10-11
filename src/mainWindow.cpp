/**
 * \file        mainWindow.cpp
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
 * This class is setting up the main window incl. all buttons and bars. It is the parent widget of all other widgets
 * (GraphicView, roomWidget, widgetLandmark).
 *
 **/


//mainWindow.cpp

#include "mainWindow.h"


MWindow :: MWindow()
{
    qDebug("Enter MWindow :: MWindow");
    setupUi(this);

    //WindowTitle
    this->setWindowTitle("JPSeditor");

    //Set-up view and scene
    mview = new jpsGraphicsView;
    mscene = new QGraphicsScene(this);
    mview->setScene(mscene);
    mview->setSceneRect(0, 0, 1920, 1080);
    setCentralWidget(mview);
    mview->showMaximized();

    dmanager = new jpsDatamanager(this,mview);
    mview->SetDatamanager(dmanager);

    //Setup ToolBar
    this->toolBar->setWindowTitle("Main ToolBar");
    this->toolBar->setMovable(false);
    drawing_toolbar_ = nullptr;

    //Roomwidget
    rwidget=nullptr;
    //Landmarkwidget
    lwidget=nullptr;
    //Snapping Options
    snappingOptions=nullptr;

    //StaturBar

    length_edit = new QLineEdit();
    length_edit->setMaximumWidth(75);

    x_edit = new QLineEdit();
    x_edit->setMaximumWidth(45);

    y_edit = new QLineEdit();
    y_edit->setMaximumWidth(45);

    infoLabel= new QLabel();
    infoLabel->setMinimumWidth(135);

    label_x = new QLabel();
    label_x->setMinimumWidth(10);
    label_x->setText("X :");

    label_y = new QLabel();
    label_y->setMinimumWidth(10);
    label_y->setText("Y :");

    label1 = new QLabel();
    label1->setMinimumWidth(90);
    label1->setText("Length of Line :");

    label2 = new QLabel();
    label2->setMinimumWidth(300);
    label2->setText("[m]");


    //filename of saved project
    _filename="";

    statusBar()->addPermanentWidget(infoLabel);
    statusBar()->addPermanentWidget(label_x);
    statusBar()->addPermanentWidget(x_edit);
    statusBar()->addPermanentWidget(label_y);
    statusBar()->addPermanentWidget(y_edit);

    // Timer needed for autosaving function
    // timer will trigger autosave every 5th minute
    timer = new QTimer(this);
    timer->setInterval(600000);
    timer->start();

    _cMapTimer = new QTimer(this);
    //_cMapTimer=nullptr;

    _statScale=false;

    // Signals and Slots
    // Menu - File
    connect(actionBeenden, SIGNAL(triggered(bool)),this,SLOT(close()));
    connect(action_ffnen,SIGNAL(triggered(bool)),this,SLOT(openFileDXF()));
    connect(action_ffnen_xml,SIGNAL(triggered(bool)),this,SLOT(openFileXML()));
    connect(action_ffnen_cogmap,SIGNAL(triggered(bool)),this,SLOT(openFileCogMap()));
    connect(actionSpeichern,SIGNAL(triggered(bool)),this,SLOT(saveAsXML()));
    connect(actionSpeichern_dxf,SIGNAL(triggered(bool)),this,SLOT(saveAsDXF()));
    connect(actionSettings,SIGNAL(triggered(bool)),this,SLOT(Settings()));

    // Tab Help
    connect(action_ber,SIGNAL(triggered(bool)),this,SLOT(info()));

    // Tab Tools
    connect(actionanglesnap,SIGNAL(triggered(bool)),this,SLOT(anglesnap()));
    connect(actiongridmode,SIGNAL(triggered(bool)),this,SLOT(gridmode()));
    connect(actionObjectsnap,SIGNAL(triggered(bool)),this,SLOT(objectsnap()));

    // Menu - Edit
    connect(actionDeleteAll,SIGNAL(triggered(bool)),this,SLOT(deleteAllContents()));
    connect(actionDelete_single_line,SIGNAL(triggered(bool)),this,SLOT(delete_marked_lines()));
    connect(actionUndo,SIGNAL(triggered(bool)),mview,SLOT(Undo()));
    connect(actionRedo,SIGNAL(triggered(bool)),mview,SLOT(Redo()));

    connect(actionRoom,SIGNAL(triggered(bool)),this,SLOT(define_room()));
    connect(actionAuto_Definition,SIGNAL(triggered(bool)),this,SLOT(autoDefine_room()));

    connect(actionScale,SIGNAL(triggered(bool)),this,SLOT(enableScale()));
    // Tab View
    connect(actionRotate_90_deg_clockwise,SIGNAL(triggered(bool)),this,SLOT(rotate()));
    connect(actionShow_Point_of_Origin,SIGNAL(triggered(bool)),this,SLOT(ShowOrigin()));

    // X Y edit
    connect(x_edit,SIGNAL(returnPressed()),this,SLOT(send_xy()));
    connect(y_edit,SIGNAL(returnPressed()),this,SLOT(send_xy()));

    // Mview
    connect(mview,SIGNAL(no_drawing()),this,SLOT(en_selectMode()));
    connect(mview,SIGNAL(set_focus_textedit()),length_edit,SLOT(setFocus()));
    connect(mview,SIGNAL(mouse_moved()),this,SLOT(show_coords()));
    connect(mview,SIGNAL(LineLengthChanged()),this,SLOT(ShowLineLength()));

    // Mark all lines
    QAction *str_a = new QAction(this);
    str_a->setShortcut(Qt::Key_A | Qt::CTRL);
    connect(str_a, SIGNAL(triggered(bool)), mview, SLOT(SelectAllLines()));
    this->addAction(str_a);

    // Remove all lines
//    QAction *str_del = new QAction(this);
//    str_del->setShortcut(Qt::Key_D | Qt::CTRL);
//    connect(str_del,SIGNAL(triggered(bool)),this,SLOT(remove_all_lines()));

    // Autosave
    connect(timer, SIGNAL(timeout()), this, SLOT(AutoSave()));

    // Landmark specifications
    connect(actionLandmarkWidget,SIGNAL(triggered(bool)),this,SLOT(define_landmark()));

    // CMap
//    connect(actionRun_visualisation,SIGNAL(triggered(bool)),this,SLOT(RunCMap()));
//    connect(_cMapTimer,SIGNAL(timeout()),this,SLOT(UpdateCMap()));
//    connect(actionSpeichern_cogmap,SIGNAL(triggered()),this,SLOT(SaveCogMapXML()));

    // Room type data gathering
    connect(actionGather_data,SIGNAL(triggered(bool)),this, SLOT(GatherData()));

    // Right dock widget
    propertyDockWidget = nullptr;

    // Left dock widget
    listDockWidget = nullptr;

    // Object snapping
    objectsnapping = {};
    bool endpoint = false;
    bool Intersections_point = false;
    bool Center_point = false;
    bool SelectedLine_point = false;
    objectsnapping.append(endpoint);
    objectsnapping.append(Intersections_point);
    objectsnapping.append(Center_point);
    objectsnapping.append(SelectedLine_point);

    // Main toolbar action group
    auto main_toolbar_actionGroup = new QActionGroup(this);
    main_toolbar_actionGroup->addAction(actionSelect_Mode); // select mode
    main_toolbar_actionGroup->addAction(actionDraw); // draw mode
    main_toolbar_actionGroup->addAction(actionZone); // assemble zone
    main_toolbar_actionGroup->addAction(actionMeasureLength); // measure mode

    connect(actionMeasureLength, SIGNAL(triggered(bool)), this, SLOT(measureLengthButtonClicked()));
    connect(actionSelect_Mode,SIGNAL(triggered(bool)),this,SLOT(en_selectMode()));
    connect(actionDraw, SIGNAL(triggered(bool)),this,SLOT(setupDrawingToolBar()));
    connect(actionZone, SIGNAL(triggered(bool)),this,SLOT(setupZoneToolBar()));

    // Drawing toolbar
    connect(actionWall,SIGNAL(triggered(bool)),this,SLOT(en_disableWall()));
    connect(actionCrossing,SIGNAL(triggered(bool)),this,SLOT(en_disableCrossing()));
    connect(actionHLine,SIGNAL(triggered(bool)),this,SLOT(en_disableHLine()));
    connect(actionLandmark,SIGNAL(triggered(bool)),this,SLOT(en_disableLandmark()));
    connect(actionSource, SIGNAL(triggered(bool)),this,SLOT(sourceButtonClicked()));
    connect(actionGoal,SIGNAL(triggered(bool)),this,SLOT(goalButtionClicked()));
    connect(actionTransition,SIGNAL(triggered(bool)),this,SLOT(transitionButtonClicked()));
    connect(actionTrack,SIGNAL(triggered(bool)),this,SLOT(trackButtonClicked()));

    drawingActionGroup = new QActionGroup(this);
    drawingActionGroup->addAction(actionWall);
    drawingActionGroup->addAction(actionCrossing);
    drawingActionGroup->addAction(actionTransition);
    drawingActionGroup->addAction(actionHLine);
    drawingActionGroup->addAction(actionLandmark);
    drawingActionGroup->addAction(actionSource);
    drawingActionGroup->addAction(actionGoal);
    drawingActionGroup->addAction(actionTrack);

    // Zone toolbar
    connect(actionCorridor, SIGNAL(triggered(bool)),this, SLOT(corridorButtonClicked()));
    connect(actionPlatform, SIGNAL(triggered(bool)),this, SLOT(platformButtonClicked()));
    connect(actionLobby, SIGNAL(triggered(bool)),this, SLOT(lobbyButtonClicked()));
    connect(actionOffice, SIGNAL(triggered(bool)),this, SLOT(officeButtonClicked()));
    connect(actionStairs, SIGNAL(triggered(bool)),this, SLOT(stairButtonClicked()));

    // Assemble actions group
    zoneActionGroup = new QActionGroup(this);
    zoneActionGroup->addAction(actionCorridor);
    zoneActionGroup->addAction(actionLobby);
    zoneActionGroup->addAction(actionOffice);
//    zoneActionGroup->addAction(actionEntrance);
    zoneActionGroup->addAction(actionStairs);
    zoneActionGroup->addAction(actionPlatform);

    // Set background
    connect(actionImportNewBackgorund, SIGNAL(triggered(bool)),this,SLOT(importBackground()));
    connect(actionShowHideBackground, SIGNAL(triggered(bool)), mview, SLOT(showHideBackground()));
    connect(actionScaleUpBackground, SIGNAL(triggered(bool)), mview, SLOT(scaleUpBackground()));
    connect(actionScaleDownBackground, SIGNAL(triggered(bool)), mview, SLOT(scaleDownBackground()));

    // Message on status bar
    connect(mview, SIGNAL(sendMsgToStatusBar(QString)), this, SLOT(msgReceived(QString)));

    // Layer
    connect(actionLayer, SIGNAL(triggered(bool)),this, SLOT(layerButtonClicked()));

    // Inifile widget
    inifileWidget = nullptr;

    // Running widget
    bottomDockWidget = nullptr;
    connect(actionRunSimulation, SIGNAL(triggered(bool)),this,SLOT(runSimulationButtonClicked()));
    qDebug("Leave MWindow :: MWindow");
}

MWindow::~MWindow()
{
    qDebug("Enter MWindow::~MWindow");

    delete dmanager;
    delete mview;

    delete length_edit;
    delete label1;
    delete label2;
    delete infoLabel;
    delete timer;
    delete _cMapTimer;
    delete drawing_toolbar_;
    qDebug("Leave MWindow::~MWindow");
}

void MWindow::setupDrawingToolBar()
{
    qDebug("Enter MWindow::setupDrawingToolBar");
    closeLeftToolBarArea(); // close running toolbar
    closeListDockWidget(); // close running list widget
    closePropertyDockWidget(); // close running property widget

    drawing_toolbar_ = new QToolBar("Drawing ToolBar", this);
    addToolBar(Qt::LeftToolBarArea, drawing_toolbar_);
    drawing_toolbar_->setMovable(false);
    drawing_toolbar_->setBackgroundRole(QPalette::HighlightedText);
    drawing_toolbar_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // drawing actions group
    drawing_toolbar_->addActions(drawingActionGroup->actions());
    qDebug("Leave MWindow::setupDrawingToolBar");
}

void MWindow::setupZoneToolBar()
{
    qDebug("Enter MWindow::setupZoneToolBar");
    closeLeftToolBarArea();
    closeListDockWidget();
    closePropertyDockWidget();

    zone_toolbar_ = new QToolBar("Zone ToolBar", this);

    addToolBar(Qt::LeftToolBarArea, zone_toolbar_);
    zone_toolbar_->setMovable(false);
    zone_toolbar_->setBackgroundRole(QPalette::HighlightedText);
    zone_toolbar_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    zone_toolbar_->addActions(zoneActionGroup->actions());
    qDebug("Leave MWindow::setupZoneToolBar");
}

void MWindow::closeLeftToolBarArea()
{
    qDebug("Enter MWindow::closeLeftToolBarArea");
    if(drawing_toolbar_ != nullptr)
    {
        drawing_toolbar_->close();
        drawing_toolbar_ = nullptr;
    }

    if(zone_toolbar_ != nullptr)
    {
        zone_toolbar_->close();
        zone_toolbar_ = nullptr;
    }
    qDebug("Leave MWindow::closeLeftToolBarArea");
}

void MWindow::AutoSave()
{
    qDebug("Enter MWindow::AutoSave");
    QMap<QString, QString> settingsmap = loadSettings();
    QString backupfolder = settingsmap["backupfolder"];

    QString filename = backupfolder + "/backup_untitled.xml";
    QFile file(filename);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //Save geometry
        dmanager->writeXML(file);

        //Save routing (hlines)
        QString fileNameRouting=filename.split(".").first()+"_routing.xml";
        QFile routingFile(fileNameRouting);
        if (routingFile.open(QIODevice::WriteOnly|QIODevice::Text))
            dmanager->writeRoutingXML(routingFile);

        //Save sources
        QString fileNameSource=filename.split(".").first()+"_sources.xml";
        QFile sourcesFile(fileNameSource);
        if(sourcesFile.open(QIODevice::WriteOnly|QIODevice::Text))
            dmanager->writeSourceXML(sourcesFile);

        //Save goals
        QString fileNameGoal=filename.split(".").first()+"_goals.xml";
        QFile goalsFile(fileNameGoal);
        if(goalsFile.open(QIODevice::WriteOnly|QIODevice::Text))
            dmanager->writeGoalXML(goalsFile);

        //Save traffic
        QString fileNameTraffic = filename.split(".").first()+"_traffic.xml";
        QFile trafficFile(fileNameTraffic);
        if(trafficFile.open(QIODevice::WriteOnly|QIODevice::Text))
            dmanager->writeTrafficXML(trafficFile);

        //Save transitions
        QString fileNameTransition=filename.split(".").first()+"_transitions.xml";
        QFile transitionFile(fileNameTransition);
        if(transitionFile.open(QIODevice::WriteOnly|QIODevice::Text))
            dmanager->writeTransitionXML(transitionFile);

        statusBar()->showMessage(tr("Backup file generated!"), 10000);
    }
    qDebug("Leave MWindow::AutoSave");
}

void MWindow::GatherData()
{
    qDebug("Enter MWindow::GatherData");
    if (rwidget==nullptr)
    {
        rwidget = new roomWidget(this,this->dmanager,this->mview);
        //rwidget->setGeometry(QRect(QPoint(5,75), rwidget->size()));
        rwidget->setAttribute(Qt::WA_DeleteOnClose);
        rwidget->GatherRTData();
        rwidget->close();
        rwidget=nullptr;
        actionRoom->setChecked(false);
        //rwidget->show();

    }
    else
    {
        rwidget->GatherRTData();
    }

    statusBar()->showMessage(tr("Data gathered"),10000);
    qDebug("Leave MWindow::GatherData");
}


//void MWindow::RunCMap()
//{

//    double frameRate = dmanager->GetCMapFrameRate();
//    _cMapFrame=1;
//    if (frameRate==0)
//    {
//        statusBar()->showMessage(tr("No cognitive map has been loaded!"),10000);
//        return;
//    }
//    _cMapTimer->setInterval(1/frameRate*1000);
//    _cMapTimer->start();
//}

//void MWindow::UpdateCMap()
//{
//    _cMapFrame++;
//    if (_cMapFrame>dmanager->GetLastCMapFrame())
//    {
//        _cMapTimer->stop();
//        dmanager->ShowCMapFrame(1);
//        return;
//    }
//    dmanager->ShowCMapFrame(_cMapFrame);
//}


void MWindow::Settings()
{
    qDebug("Enter MWindow::Settings");
    settingDialog = new SettingDialog;

//    QString backupfolder = settings.value("backupfolder").toString();
//    QString interval =  settings.value("interval").toString();
//
//    qDebug()<< settings.value("backupfolder");
//    qDebug()<< settings.value("interval");
//
//    QMap<QString, QString> defaultsetting;
//    defaultsetting["backupfolder"] = backupfolder;
//    defaultsetting["interval"] = interval;

    QMap<QString, QString> defaultsetting = loadSettings();
    connect(settingDialog,SIGNAL(sendSetting(QMap<QString, QString>)),
            this,SLOT(saveSettings(QMap<QString, QString>)));
    settingDialog->setCurrentSetting(defaultsetting);

    settingDialog->setModal(true);
    settingDialog->exec();
    qDebug("Leave MWindow::Settings");
}

void MWindow::ShowOrigin()
{
    qDebug("Enter MWindow::ShowOrigin");
    mview->ShowOrigin();
    qDebug("Leave MWindow::ShowOrigin");
}

void MWindow::openFileDXF(){
    qDebug("Enter MWindow::openFileDXF");

    QString fileName=QFileDialog::getOpenFileName(this,tr("Open DXF"),"",tr("DXF-Drawings (*.dxf)"));
    //QFile file(fileName);
    std::string fName= fileName.toStdString();

    if (!dmanager->readDXF(fName))
    {
        statusBar()->showMessage("DXF-File could not be parsed!",10000);
    }
    //if(file.open(QIODevice::ReadOnly|QIODevice::Text)) {
      //  textEdit->setPlainText(QString::fromUtf8(file.readAll()));
     //   statusBar()->showMessage(tr("Datei erfolgreich geladen"),5000);
    //}
    else
    {

        statusBar()->showMessage("DXF-File successfully loaded!",10000);
    }
    qDebug("Leave MWindow::openFileDXF");
}

/*
    since 0.8.8

    For "Load XML" menu button
 */
void MWindow::openFileXML()
{
    qDebug("Enter MWindow::openFileXML");
    closeLeftToolBarArea();
    closeListDockWidget();
    closePropertyDockWidget();

    QString fileName=QFileDialog::getOpenFileName(this,tr("Open XML"),"",tr("XML-Files (*.xml)"));

    // Open geometry
    QString error_geometry = openGeometry(fileName);

    // if load geometry file failed, stop read others files
    if(!error_geometry.isEmpty())
    {
        QMessageBox::critical(this,
                              "Open XML",
                              error_geometry,
                              QMessageBox::Ok);
        qDebug("Leave MWindow::openFileXML");
        return;
    }

    QString error_routing = openRouting(fileName);

    QString error_goal = openGoal(fileName);

    QString error_source = openSource(fileName);

    QString error_traffic = openTraffic(fileName);

    if(!error_goal.isEmpty() or !error_routing.isEmpty() or !error_source.isEmpty() or !error_traffic.isEmpty())
    {
        // Show error information when some files are locked
        QString error = "";
        QMessageBox msgBox;
        msgBox.setText("Geometry is loaded, but some files aren't loaded.");

        if(!error_routing.isEmpty())
            error += error_routing + "\n";

        if(!error_goal.isEmpty())
            error += error_goal + "\n";

        if(!error_source.isEmpty())
            error += error_source + "\n";

        if(!error_traffic.isEmpty())
            error += error_traffic + "\n";

        msgBox.setDetailedText(error);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

    } else
    {
        statusBar()->showMessage(tr("All files successfully loaded!"),10000);
    }

    //AutoZoom to drawing
    mview->AutoZoom();

    qDebug("Leave MWindow::openFileXML");
}

QString MWindow::openGeometry(QString fileName)
{
    qDebug("Enter MWindow::openGeometry");
    QFile file(fileName);

    if(fileName.isEmpty())
    {
        QString error = "Geometry file isn't found!";
        qDebug("Leave MWindow::openGeometry, returned error!");
        return error;
    }


    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString error = "Format error, open geometry file failed!";
        qDebug("Leave MWindow::openGeometry, returned Format error!");
        return error;
    }

    if (!dmanager->readXML(file))
    {
        QString error = "Content error, read geometry file failed";
        qDebug("Leave MWindow::openGeometry, returned Content error!");
        return error;
    }
    else
    {
        this->setWindowTitle(fileName);
        QString error = ""; // The empty error string means file is loaded successful!

        return error;
    }
    qDebug("Leave MWindow::openGeometry");
}

QString MWindow::openRouting(QString fileName)
{
    qDebug("Enter MWindow::openRouting");
    QString fileNameRouting= fileName.split(".").first()+"_routing.xml";
    QFile fileRouting(fileNameRouting);


    if (!fileRouting.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString error = "Routing file isn't found!";
        qDebug("Leave MWindow::openRouting, Routing file is not found!");
        return error;
    }

    //Start to read routing
    if (!dmanager->readRoutingXML(fileRouting))
    {
        QString error = "Content error, read routing file failed!";
        qDebug("Leave MWindow::openRouting, read routing file failed!");
        return error;
    }
    else
    {
        QString error = ""; // file is loaded successful!
        qDebug("Leave MWindow::openRouting");
        return error;
    }
    qDebug("Leave MWindow::openRouting");
}

QString MWindow::openSource(QString fileName)
{
    qDebug("Enter MWindow::openSource");
    QString fileNameSource= fileName.split(".").first()+"_sources.xml";
    QFile fileSource(fileNameSource);

    if(!fileSource.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString error = "Source file isn't found!";
        qDebug("Leave MWindow::openSource, source file isn't found!");
        return error;
    }

    SourceReader sourceReader(mview);

    if(!sourceReader.read(&fileSource))
    {
        QString error = "Content error, read source file failed";
        qDebug("Leave MWindow::openSource, read source file failed");
        return error;
    } else
    {
        QString error = ""; // file is loaded successful!
        qDebug("Leave MWindow::openSource");
        return error;
    }
    qDebug("Leave MWindow::openSource");
}

QString MWindow::openGoal(QString fileName)
{
    qDebug("Enter QString MWindow::openGoal");
    QString fileNameGoal= fileName.split(".").first()+"_goals.xml";
    QFile fileGoal(fileNameGoal);

    if(!fileGoal.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString error = "Goal file isn't found!";
        qDebug("Leave QString MWindow::openGoal, goal file isn't found!");
        return error;
    }

    GoalReader goalReader(mview);

    if(!goalReader.read(&fileGoal))
    {
        QString error = "Content error, read source file failed";
        qDebug("Leave QString MWindow::openGoal, read source file failed");
        return error;
    } else
    {
        QString error = ""; // file is loaded successful!
        qDebug("Leave QString MWindow::openGoal");
        return error;
    }
    qDebug("Leave QString MWindow::openGoal");
}

void MWindow::openFileCogMap()
{
    qDebug("Enter MWindow::openFileCogMap");
    QString fileName=QFileDialog::getOpenFileName(this,tr("Open XML"),"",tr("XML-Files (*.xml)"));
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this,
                              "OpenFileXML",
                              "Couldn't open xml-file",
                              QMessageBox::Ok);
        qDebug("Leave MWindow::openFileCogMap");
        return;
    }


    if (!dmanager->ParseCogMap(file))
    {
        statusBar()->showMessage("XML-File could not be parsed!",10000);
    }

    else
    {

        statusBar()->showMessage("Cognitive map successfully loaded!",10000);
    }
    file.close();
    qDebug("Leave MWindow::openFileCogMap");
}

void MWindow::OpenLineFile()
{
    qDebug("Enter MWindow::OpenLineFile");
    QString fileName=QFileDialog::getOpenFileName(this,tr("Open Lines"),"",tr("txt-File (*.txt)"));
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        statusBar()->showMessage("Line-File could not be opened!",10000);
        qDebug("Leave MWindow::OpenLineFile, open Line-File failed!");
        return;

    }

    if (!dmanager->ReadLineFile(file))
    {
        statusBar()->showMessage("Line-File could not be parsed!",10000);
        qDebug("Leave MWindow::OpenLineFile, parsing Line-File failed!");
    }
    //if(file.open(QIODevice::ReadOnly|QIODevice::Text)) {
      //  textEdit->setPlainText(QString::fromUtf8(file.readAll()));
     //   statusBar()->showMessage(tr("Datei erfolgreich geladen"),5000);
    //}
    else
    {

        statusBar()->showMessage("Line-File successfully loaded!",10000);
        qDebug("Leave MWindow::OpenLineFile");
    }
    file.close();
    qDebug("Leave MWindow::OpenLineFile");
}

void MWindow::saveAsXML(){
    qDebug("Enter MWindow::saveAsXML");
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save XML"),"",tr("XML-Files (*.xml)"));
    _filename=fileName;
    if (fileName.isEmpty()) return;
    QFile file(fileName);

    if(file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        //Save geometry
        dmanager->writeXML(file);

        //Save routing (hlines)
        QString fileNameRouting=fileName.split(".").first()+"_routing.xml";
        QFile routingFile(fileNameRouting);
        if (routingFile.open(QIODevice::WriteOnly|QIODevice::Text))
            dmanager->writeRoutingXML(routingFile);

        //Save sources
        QString fileNameSource=fileName.split(".").first()+"_sources.xml";
        QFile sourcesFile(fileNameSource);
        if(sourcesFile.open(QIODevice::WriteOnly|QIODevice::Text))
            dmanager->writeSourceXML(sourcesFile);

        //Save goals
        QString fileNameGoal=fileName.split(".").first()+"_goals.xml";
        QFile goalsFile(fileNameGoal);
        if(goalsFile.open(QIODevice::WriteOnly|QIODevice::Text))
            dmanager->writeGoalXML(goalsFile);

        //Save traffic
        QString fileNameTraffic = fileName.split(".").first()+"_traffic.xml";
        QFile trafficFile(fileNameTraffic);
        if(trafficFile.open(QIODevice::WriteOnly|QIODevice::Text))
            dmanager->writeTrafficXML(trafficFile);

        //Save transitions
        QString fileNameTransition=fileName.split(".").first()+"_transitions.xml";
        QFile transitionFile(fileNameTransition);
        if(transitionFile.open(QIODevice::WriteOnly|QIODevice::Text))
            dmanager->writeTransitionXML(transitionFile);

        statusBar()->showMessage(tr("XML-File successfully saved!"),10000);
    }
    qDebug("Leave MWindow::saveAsXML");
}

void MWindow::saveAsDXF()
{
    qDebug("Enter MWindow::saveAsDXF");
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save DXF"),"",tr("DXF-Drawings (*.dxf)"));
    if (fileName.isEmpty()) return;
    QFile file(fileName);

    if(file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        //QString coord_string=mview->build_coordString();
        std::string fName= fileName.toStdString();
        dmanager->writeDXF(fName);
        //file.write(coord_string.toUtf8());//textEdit->toPlainText().toUtf8());
        statusBar()->showMessage(tr("DXF-File successfully saved!"),10000);
    }
    qDebug("Leave MWindow::saveAsDXF");
}

void MWindow::SaveCogMapXML()
{
    qDebug("Enter MWindow::SaveCogMapXML");
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save CognitiveMap XML"),"",tr("XML-Files (*.xml)"));
    if (fileName.isEmpty()) return;
    QFile file(fileName);

    if(file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        dmanager->WriteCognitiveMapXML(file);
        //file.write(coord_string.toUtf8());//textEdit->toPlainText().toUtf8());
        statusBar()->showMessage(tr("XML-File successfully saved!"),10000);
    }
    qDebug("Leave MWindow::SaveCogMapXML");
}


void MWindow::info()
{
    qDebug("Enter MWindow::info");
    QString gittext = QMessageBox::tr(
            "<h1><p style=\"line-height:0.7\">JPSeditor</p></h1><p style=\"line-height:1.4\" style=\"color:Gray;"
            "\"><small><i>Version %1</i></small></p>"
            "<p style=\"line-height:0.4\" style=\"color:Gray;\"><i>Commit Hash</i> %2</p>"
            "<p style=\"line-height:0.4\" style=\"color:Gray;\"><i>Commmit Date</i> %3</p>"
            "<p style=\"line-height:0.4\" style=\"color:Gray;\"><i>Branch</i> %4</p><hr>"
    ).arg(JPSEDITOR_VERSION).arg(GIT_COMMIT_HASH).arg(GIT_COMMIT_DATE);

    QString text = QMessageBox::tr(
            "<p style=\"color:Gray;\"><small><i> &copy; 2009-2019  FZ Jülich <br><a href=\"http://jupedsim"
            ".org\">jupedsim"
            ".org</a></i></small></p>"
    );

    QMessageBox msg(QMessageBox::Information,"About JPSeditor", gittext+text, QMessageBox::Ok);
    msg.setIconPixmap(QPixmap(":/inAppIcons/jpseditor.icns"));

    //Change font
    QFont font( "Tokyo" );
    font.setPointSize( 10 );
// font.setWeight( QFont::Bold );
//font.setItalic( TRUE );
    msg.setFont(font);
//msg.setStandardButtons(0);
    msg.exec();
    qDebug("Leave MWindow::info");
}

void MWindow::anglesnap()
{
    qDebug("Enter MWindow::anglesnap");
   mview->change_stat_anglesnap();
   qDebug("Leave MWindow::anglesnap");
}

void MWindow::en_disableWall()
{
    qDebug("Enter MWindow::en_disableWall");
    closePropertyDockWidget();

    mview->en_disableWall();
    qDebug("Leave MWindow::en_disableWall");
}

void MWindow::en_disableCrossing()
{
    qDebug("Enter MWindow::en_disableCrossing");
    closePropertyDockWidget();

    mview->en_disableCrossing();
    qDebug("Leave MWindow::en_disableCrossing");
}

void MWindow::en_disableLandmark()
{
    qDebug("Enter MWindow::en_disableLandmark");
    closePropertyDockWidget();

    mview->en_disableLandmark();
    qDebug("Leave MWindow::en_disableLandmark");
}

void MWindow::en_disableHLine()
{
    qDebug("Enter MWindow::en_disableHLine");
    closePropertyDockWidget();

    mview->en_disableHLine();
    qDebug("Leave MWindow::en_disableHLine");
}

void MWindow::transitionButtonClicked()
{
    qDebug("Enter MWindow::transitionButtonClicked");
    closePropertyDockWidget();

    mview->enableTransition();

    propertyDockWidget = new QDockWidget(tr("Transitions"), this);
    propertyDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    propertyDockWidget->setAllowedAreas( Qt::RightDockWidgetArea);

    auto *transitionWidget = new TransitionWidget(this, this->dmanager, mview);


    addDockWidget(Qt::RightDockWidgetArea, propertyDockWidget);
    propertyDockWidget->setWidget(transitionWidget);
    qDebug("Leave MWindow::transitionButtonClicked");
}

void MWindow::trackButtonClicked()
{
    qDebug("Enter MWindow::trackButtonClicked");
    closePropertyDockWidget();

    mview->enableTrack();

/*  For now Track property Widget isn't needed
    propertyDockWidget = new QDockWidget(tr("Tracks"), this);
    propertyDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    propertyDockWidget->setAllowedAreas( Qt::RightDockWidgetArea);

    auto *trackPropertyWidget = new TrackPropertyWidget(this, this->dmanager);
    connect(mview, SIGNAL(tracksChanged()), trackPropertyWidget, SLOT(updateWallListWidget()));

    addDockWidget(Qt::RightDockWidgetArea, propertyDockWidget);
    propertyDockWidget->setWidget(trackPropertyWidget);
*/
    qDebug("Leave MWindow::trackButtonClicked");
}

void MWindow::objectsnap()
{
    qDebug("Enter MWindow::objectsnap");
    if(snappingOptions==nullptr)
    {
        snappingOptions = new SnappingOptions(this);
        snappingOptions->setGeometry(QRect(QPoint(75,75), snappingOptions->size()));
        snappingOptions->setAttribute(Qt::WA_DeleteOnClose);
        snappingOptions->setState(objectsnapping);
        snappingOptions->show();

        connect(snappingOptions,SIGNAL(snapStart_endpoint(bool)),mview,SLOT(changeStart_endpoint(bool)));
        connect(snappingOptions,SIGNAL(snapIntersections_point(bool)),mview,SLOT(changeIntersections_point(bool)));
        connect(snappingOptions,SIGNAL(snapCenter_point(bool)),mview,SLOT(changeCenter_point(bool)));
        connect(snappingOptions,SIGNAL(snapSelectedLine_point(bool)),mview,SLOT(changeLine_point(bool)));
    }
    else
    {
        objectsnapping.clear();
        objectsnapping = snappingOptions->getState();
        snappingOptions->close();
        snappingOptions=nullptr;
        actionObjectsnap->setChecked(false);
    }
    qDebug("Leave MWindow::objectsnap");
}

void MWindow::gridmode()
{
    qDebug("Enter MWindow::gridmode");
    mview->change_gridmode();
    qDebug("Leave MWindow::gridmode");
}

void MWindow::show_coords()
{
    QPointF point = mview->return_Pos();
    QString string = "";
    string.sprintf("(%.2f, %5.2f)", point.x(), point.y());
    infoLabel->setText(string);
}

void MWindow::deleteAllContents()
{
    qDebug("Enter MWindow::deleteAllContents");

    // Delete all JPSelements in datamanager
    dmanager->remove_all();

    // Delete all QGraphicsitem in view
    mview->delete_all();

    emit mview->sourcesChanged(); // emit to update source widget
    emit mview->goalsChanged(); // emit to update goal widget
    emit mview->transitonsChanged(); // emit to update transition widget
    emit mview->layersChanged(); // emit to update layer widget
    emit allContentsDeleted(); // emit to property & roomlist widget

    statusBar()->showMessage(tr("All lines are deleted!"),10000);
    qDebug("Leave MWindow::deleteAllContents");
}

void MWindow::delete_marked_lines()
{
    qDebug("Enter MWindow::delete_marked_lines");

    mview->delete_marked_lines();
    dmanager->remove_marked_lines();
    mview->clearMarkedLineList();
    mview->deleteMarkedLandmark();

    emit mview->markedLineDeleted();

    statusBar()->showMessage(tr("Marked lines are deleted!"),10000);
    qDebug("Leave MWindow::delete_marked_lines");
}

void MWindow::send_length()
{
    qDebug("Enter MWindow::send_length");
    qreal length = length_edit->text().toDouble();
    if(length != 0)
    {
         mview->take_l_from_lineEdit(length);
    }
    length_edit->clear();
    qDebug("Leave MWindow::send_length");
}

void MWindow::send_xy()
{
    qDebug("Enter MWindow::send_xy");
    qreal x = x_edit->text().toDouble();
    qreal y = y_edit->text().toDouble();

    QPointF endpoint;
    endpoint.setX(x);
    endpoint.setY(y);

    mview->take_endpoint_from_xyEdit(endpoint);

    x_edit->clear();
    qDebug("Leave MWindow::send_xy");
}


void MWindow::define_room()
{
    qDebug("Enter MWindow::define_room");
    if (rwidget==nullptr)
    {
        rwidget = new roomWidget(this,this->dmanager,this->mview);
        rwidget->setGeometry(QRect(QPoint(75,75), rwidget->size()));
        rwidget->setAttribute(Qt::WA_DeleteOnClose);
        rwidget->show();
    }
    else
    {
        rwidget->close();
        rwidget=nullptr;
        actionRoom->setChecked(false);
    }
    qDebug("Leave MWindow::define_room");
}

void MWindow::autoDefine_room()
{
    qDebug("Enter autoDefine_room");
    if (rwidget==nullptr)
    {
        rwidget = new roomWidget(this,this->dmanager,this->mview);
        //rwidget->setGeometry(QRect(QPoint(5,75), rwidget->size()));
        rwidget->setAttribute(Qt::WA_DeleteOnClose);
        rwidget->StartAutoDef();
        rwidget->close();
        rwidget=nullptr;
        actionRoom->setChecked(false);
        //rwidget->show();

    }
    else
    {
        rwidget->StartAutoDef();
    }

    statusBar()->showMessage(tr("Rooms and doors are set."),10000);
    qDebug("Leave autoDefine_room");
}

void MWindow::define_landmark()
{
    qDebug("Enter MWindow::define_landmark");
    if (lwidget==nullptr)
    {
        lwidget = new widgetLandmark(this,this->dmanager,this->mview);
        lwidget->setGeometry(QRect(QPoint(5,75), lwidget->size()));
        lwidget->setAttribute(Qt::WA_DeleteOnClose);
        lwidget->show();
    }
    else
    {
        lwidget->close();
        lwidget=nullptr;
        actionLandmarkWidget->setChecked(false);
    }
    qDebug("Leave MWindow::define_landmark");
}

void MWindow::en_selectMode()
{
    qDebug("Enter MWindow::en_selectMode");
    mview->disable_drawing();
    length_edit->clearFocus();

    if(drawing_toolbar_ != nullptr)
    {
        drawing_toolbar_->close();
        drawing_toolbar_ = nullptr;
    }
    qDebug("Leave MWindow::en_selectMode");
}

void MWindow::dis_selectMode()
{
    qDebug("Enter MWindow::dis_selectMode");
/*
 if (actionWall->isChecked()==true || actionCrossing->isChecked()==true || actionExit->isChecked()==true
            || actionLandmark->isChecked()==true)
    {
        actionSelect_Mode->setChecked(false);
    }
*/

    if(drawingActionGroup->checkedAction() != actionSelect_Mode)
        actionSelect_Mode->setChecked(false);
    qDebug("Leave MWindow::dis_selectMode");
}

//void MWindow::lines_deleted()
//{
//    qDebug("Enter MWindow::lines_deleted");
//    dmanager->remove_marked_lines();
//    qDebug("Leave MWindow::lines_deleted");
//}

void MWindow::ShowLineLength()
{

    length_edit->setText(QString::number(mview->ReturnLineLength()));
    length_edit->selectAll();

}

void MWindow::ScaleLines()
{
    qDebug("Enter MWindow::ScaleLines");
    if (_statScale)
    {
        qreal factor = length_edit->text().toDouble();
        mview->ScaleLines(factor);
        length_edit->clear();
        _statScale=false;
    }
    qDebug("Leave MWindow::ScaleLines");
}

void MWindow::enableScale()
{
    qDebug("Enter MWindow::enableScale");
    _statScale=true;
    qDebug("Leave MWindow::enableScale");
}

void MWindow::rotate()
{   qDebug("Enter MWindow::rotate");
    mview->rotate(-90);
    qDebug("Leave MWindow::rotate");
}

void MWindow::closeEvent(QCloseEvent *event)
{
    qDebug("Enter MWindow::closeEvent");
    int ret = QMessageBox::warning(
                this, "Quit?",
                "Do you really want to quit?",
                QMessageBox::Yes | QMessageBox::No );

    if (ret == QMessageBox::Yes)
    {
        QMainWindow::closeEvent(event);
    }
    else
    {
        event->ignore();
    }
    qDebug("Leave MWindow::closeEvent");
}


void MWindow::on_actionCopy_triggered()
{
    qDebug("Enter on_actionCopy_triggered");
    actionCopy->setChecked(true);
    mview->start_Copy_function();
    qDebug("Leave on_actionCopy_triggered");
}

void MWindow::on_actionOnline_Help_triggered()
{
    qDebug("Enter MWindow::on_actionOnline_Help_triggered");
    QString JPSeditor = "http://www.jupedsim.org/jpseditor/";
    QDesktopServices::openUrl(QUrl(JPSeditor));
    qDebug("Leave MWindow::on_actionOnline_Help_triggered");
}

void MWindow::on_actionClear_all_Rooms_and_Doors_triggered()
{
    qDebug("Enter MWindow::on_actionClear_all_Rooms_and_Doors_triggered");
    dmanager->remove_all();

    if(rwidget!= nullptr){
        rwidget->show_rooms();
        rwidget->showLayersInfo();
        rwidget->show_crossings();
        rwidget->show_obstacles();
    }

    statusBar()->showMessage(tr("All rooms and doors are cleared!"),10000);
    qDebug("Leave MWindow::on_actionClear_all_Rooms_and_Doors_triggered");
}

void MWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Escape:
            mview->disable_drawing();
            en_selectMode();
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

// Default settings
void MWindow::saveSettings(QMap<QString, QString> settingsmap)
{
    QSettings settings("FZJ","JPSeditor");
    settings.beginGroup("backup");
    settings.setValue("backupfolder", settingsmap["backupfolder"]);
    settings.setValue("interval", settingsmap["interval"]);
    timer->setInterval(settingsmap["interval"].toInt());
    settings.endGroup();
}

QMap<QString, QString> MWindow::loadSettings()
{
    qDebug("Enter MWindow::loadSettings");
    QSettings settings("FZJ","JPSeditor");

    settings.beginGroup("backup");
    QString value = settings.value("backupfolder", "../").toString();
    QString interval = settings.value("interval", "600000").toString();
    settings.endGroup();

    QMap<QString, QString> settingsmap;
    settingsmap["backupfolder"] = value;
    settingsmap["interval"] = interval;

    qDebug("Leave MWindow::loadSettings");
    return settingsmap;
}

void MWindow::on_actionNew_Inifile_triggered()
{
    qDebug("Enter MWindow::on_actionNew_Inifile_triggered");
    inifileWidget = new InifileWidget(this, dmanager);
    inifileWidget->show();
    qDebug()<< "MWindow::on_actionNew_Inifile_triggered(): inifile widget is showed!";
    qDebug("Leave MWindow::on_actionNew_Inifile_triggered");
}

void MWindow::on_actionBack_to_Origin_triggered()
{
    qDebug("Enter MWindow::on_actionBack_to_Origin_triggered");
    mview->centerOn(QPointF(0.0,0.0));
    qDebug("Leave MWindow::on_actionBack_to_Origin_triggered");
}

void MWindow::on_actionZoom_Windows_triggered()
{
    qDebug("Enter MWindow::on_actionZoom_Windows_triggered");
    en_selectMode();
    mview->selectedWindows();
    qDebug("Leave MWindow::on_actionZoom_Windows_triggered");
}

void MWindow::on_actionZoom_Extents_triggered()
{
    qDebug("Enter MWindow::on_actionZoom_Extents_triggered");
    mview->AutoZoom();
    qDebug("Leave MWindow::on_actionZoom_Extents_triggered");
}

void MWindow::sourceButtonClicked()
{
    qDebug("Enter MWindow::sourceButtonClicked");
    closePropertyDockWidget();

    //source widget off, dockwidget off -> open source widget
    mview->enableSourceMode();

    propertyDockWidget = new QDockWidget(tr("Sources"), this);
    propertyDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    propertyDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);

    auto *sourceWidget = new SourceWidget(this, mview, this->dmanager);
    addDockWidget(Qt::RightDockWidgetArea, propertyDockWidget);
    propertyDockWidget->setWidget(sourceWidget);
    qDebug("Leave MWindow::sourceButtonClicked");

}

// Goal drawing mode
/*
    since 0.8.8

    Build widget for goal if there is no, or destory widget.
 */

void MWindow::goalButtionClicked()
{
    qDebug("Enter MWindow::goalButtionClicked");
    closePropertyDockWidget();

    mview->enableGoalMode();

    propertyDockWidget = new QDockWidget(tr("Goals"), this);
    propertyDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    propertyDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto *goalWidget = new GoalWidget(this, mview, this->dmanager);
    addDockWidget(Qt::RightDockWidgetArea, propertyDockWidget);
    propertyDockWidget->setWidget(goalWidget);

    qDebug("Leave MWindow::goalButtionClicked");
}

/*
    Since v0.8.8

    Open traffic file
 */

QString MWindow::openTraffic(QString fileName)
{
    qDebug("Enter MWindow::openTraffic");
    QString fileNameTraffic = fileName.split(".").first() + "_traffic.xml";
    QFile fileTraffic(fileNameTraffic);

    if (!fileTraffic.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString error = "Traffic file isn't found!";
        qDebug("Leave MWindow::openTraffic, traffic file isn't found!");
        return error;
    }

    if(!dmanager->readTrafficXML(fileTraffic))
    {
        QString error = "Content error, read traffic file failed";
        qDebug("Leave MWindow::openTraffic, read traffic file failed!");
        return error;
    } else
    {
        QString error = ""; // file is loaded successful!
        qDebug("Leave MWindow::openTraffic");
        return error;
    }
    qDebug("Leave MWindow::openTraffic");
}

/*
    since v0.8.8

    Receive message from other widgets, show on status bar
 */
void MWindow::showStatusBarMessage(QString msg, int duration)
{
    qDebug("Enter MWindow::showStatusBarMessage");
    statusBar()->showMessage(tr(msg.toStdString().data()), duration);
    qDebug("Leave MWindow::showStatusBarMessage");
}

/*
    since v0.8.9

    measure Length of walls
 */

void MWindow::measureLengthButtonClicked()
{
    qDebug("Enter MWindow::measureLengthButtonClicked");
    mview->enableMeasureLengthMode();
    qDebug("Leave MWindow::measureLengthButtonClicked");
}

/*
    since v0.8.9

    receive msg from others widgets
 */

void MWindow::msgReceived(QString Msg)
{
    qDebug("Enter MWindow::msgReceived");
    statusBar()->showMessage(Msg, 10000);
    qDebug("Leave MWindow::msgReceived");
}


/*
    since v0.8.9

    Import a image (JPG or PNG) as Background
 */

void MWindow::importBackground()
{
    qDebug("Enter MWindow::importBackground");
    QString fileName = QFileDialog::getOpenFileName(
            this, "open image file",
            ".",
            "Image files (*.bmp *.jpg *.pbm *.pgm *.png *.ppm *.xbm *.xpm);;All files (*.*)");
    mview->setBackground(fileName);
    qDebug("Leave MWindow::importBackground");
}

/*
    since v0.8.9

    create a listDockwidget and propertyDockwidget
 */

void MWindow::corridorButtonClicked()
{
    qDebug("Enter MWindow::corridorButtonClicked");
    closeListDockWidget();
    closePropertyDockWidget();

    addListDockWidget("Corridor");
    qDebug("Leave MWindow::corridorButtonClicked");
}

void MWindow::closePropertyDockWidget()
{
    qDebug("Enter MWindow::closePropertyDockWidget");
    if(propertyDockWidget != nullptr)
    {
        propertyDockWidget->close(); //close() has deleted pointer
        propertyDockWidget = nullptr;
    }
    qDebug("Leave MWindow::closePropertyDockWidget");
}

void MWindow::closeListDockWidget()
{
    qDebug("Enter MWindow::closeListDockWidget");
    if(listDockWidget != nullptr)
    {
        listDockWidget->close();
        listDockWidget = nullptr;
    }
    qDebug("Leave MWindow::closeListDockWidget");
}

void MWindow::addListDockWidget(const QString &type)
{
    qDebug("Enter MWindow::addListDockWidget");
    closeListDockWidget();
    closePropertyDockWidget();

    // create dock widget
    listDockWidget = new QDockWidget(type, this);
    listDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    listDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);

    // create list widget
    auto *listWidget = new RoomListWidget(this, this->dmanager);
    listWidget->setLabel(type);

    // add list widget into dock widget
    addDockWidget(Qt::LeftDockWidgetArea, listDockWidget);
    listDockWidget->setWidget(listWidget);

    qDebug("Leave MWindow::addListDockWidget");
}

void MWindow::addPropertyDockWidget(JPSZone *zone)
{
    qDebug("Enter MWindow::addPropertyDockWidget");

    if(zone == nullptr)
    {
        qDebug("Zone is null point, Leave MWindow::addPropertyDockWidget");
        return;
    }

    closePropertyDockWidget();

    propertyDockWidget = new QDockWidget(tr("Property"), this);
    propertyDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    propertyDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);

    auto *propertyWidget = new PropertyWidget(this, this->dmanager,
                                              zone, mview);

    addDockWidget(Qt::RightDockWidgetArea, propertyDockWidget);
    propertyDockWidget->setWidget(propertyWidget);

    qDebug("Leave MWindow::addPropertyDockWidget");
}

void MWindow::platformButtonClicked()
{
    qDebug("Enter MWindow::platformButtonClicked");
    closeListDockWidget();
    closePropertyDockWidget();

    addListDockWidget("Platform");
    qDebug("Leave MWindow::platformButtonClicked");
}

void MWindow::layerButtonClicked()
{
    qDebug("Enter MWindow::layerButtonClicked");
    closeLeftToolBarArea();
    closeListDockWidget();
    closePropertyDockWidget();

    // create dock widget
    listDockWidget = new QDockWidget("Layer", this);
    listDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    listDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);

    auto *layerWidget = new LayerWidget(this, mview);

    addDockWidget(Qt::LeftDockWidgetArea, listDockWidget);
    listDockWidget->setWidget(layerWidget);

    qDebug("Leave MWindow::layerButtonClicked");
}

void MWindow::lobbyButtonClicked()
{
    qDebug("Enter MWindow::lobbyButtonClicked");
    closeListDockWidget();
    closePropertyDockWidget();

    addListDockWidget("Lobby");
    qDebug("Leave MWindow::lobbyButtonClicked");
}

void MWindow::officeButtonClicked()
{
    qDebug("Enter MWindow::officeButtonClicked");
    closeListDockWidget();
    closePropertyDockWidget();

    addListDockWidget("Office");
    qDebug("Leave MWindow::officeButtonClicked");
}

void MWindow::stairButtonClicked()
{
    qDebug("Enter MWindow::stairButtonClicked");
    closeListDockWidget();
    closePropertyDockWidget();

    addListDockWidget("Stair");
    qDebug("Leave MWindow::stairButtonClicked");
}

void MWindow::runSimulationButtonClicked()
{
    closeLeftToolBarArea();
    closeListDockWidget();
    closePropertyDockWidget();
    closeBottomDockWidget();

    if(bottomDockWidget == nullptr)
    {
        qDebug("Enter MWindow::runSimulationButtonClicked");
        bottomDockWidget = new QDockWidget("Run Simulation", this);
        bottomDockWidget->setFeatures(QDockWidget::DockWidgetClosable);
        bottomDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);

        auto *runningWidget = new RunningWidget(this);
        addDockWidget(Qt::BottomDockWidgetArea, bottomDockWidget);
        bottomDockWidget->setWidget(runningWidget);
        qDebug("Leave MWindow::runSimulationButtonClicked");
    }
}

void MWindow::closeBottomDockWidget()
{
    qDebug("Enter MWindow::closeBottomDockWidget");
    if(bottomDockWidget != nullptr)
    {
        bottomDockWidget->close(); //close() has deleted pointer
        bottomDockWidget = nullptr;
    }
    qDebug("Leave MWindow::closeBottomDockWidget");
}
