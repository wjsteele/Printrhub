//
// Created by Phillip Schuster on 20.01.16.
//

#include "MainSceneController.h"
#include "Bitmaps.h"
#include "LabelButton.h"
#include "Application.h"
#include "NavBar.h"
#include "ChoosePrintSceneController.h"
#include "WiFiSetupSceneController.h"
#include "VirtualKeyboardSceneController.h"
#include "MachineControlSceneController.h"
#include "ColorTheme.h"

MainSceneController::MainSceneController():
SceneController::SceneController()
{
    _printButton = new LabelButton("PRINT",Rect(0,0,159,119));
    _printButton->setName("Print Button");
    _printButton->setDelegate(this);
    addView(_printButton);

    _hotendButton = new LabelButton("HOTEND",Rect(160,0,159,119));
    _hotendButton->setName("Hotend Button");
    _hotendButton->setDelegate(this);
    addView(_hotendButton);

    _filamentButton = new LabelButton("FILAMENT",Rect(0,120,159,120));
    addView(_filamentButton);

    _settingsButton= new LabelButton("SETTINGS",Rect(160,120,159,120));
    _settingsButton->setDelegate(this);
    addView(_settingsButton);
}

MainSceneController::~MainSceneController()
{

}


uint16_t MainSceneController::getBackgroundColor()
{
    return Application.getTheme()->getColor(SpacerColor);
}

void MainSceneController::display()
{

}

String MainSceneController::getName()
{
    return "MainSceneController";
}

void MainSceneController::loop()
{
    SceneController::loop();
}

void MainSceneController::onWillAppear()
{
    SceneController::onWillAppear();

    Display.setScrollInsets(0,0);
}

#pragma mark ButtonDelegate Implementation

void MainSceneController::buttonPressed(void *button)
{
    LOG("MainSceneController::buttonPressed");

    if (button == _printButton)
    {
        ChoosePrintSceneController* choosePrintSceneController = new ChoosePrintSceneController();
        Application.pushScene(choosePrintSceneController);
    }
    else if (button == _settingsButton)
    {
        VirtualKeyboardSceneController* scene = new VirtualKeyboardSceneController();
        Application.pushScene(scene);
    }
    else if (button == _hotendButton)
    {
        MachineControlSceneController* scene = new MachineControlSceneController();
        Application.pushScene(scene);
    }
    else
    {
        MainSceneController* scene = new MainSceneController();
        Application.pushScene(scene);
    }
}
