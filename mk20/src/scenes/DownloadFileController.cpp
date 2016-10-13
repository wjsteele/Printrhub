//
// Created by Phillip Schuster on 13.05.16.
//

#include "DownloadFileController.h"
#include "SD.h"
#include "projects/ProjectsScene.h"
//#include "print/PrintStatusSceneController.h"
//#include "print/CleanPlasticSceneController.h"
//#include "ConfirmSceneController.h"
#include "framework/views/ProgressBar.h"
#include "MainSceneController.h"
#include "UIBitmaps.h"
#include "font_LiberationSans.h"

extern UIBitmaps uiBitmaps;

DownloadFileController::DownloadFileController(String url, String localFilePath) :
  SidebarSceneController::SidebarSceneController(),
  _fileSize(0),
  _bytesRead(0),
  _previousPercent(0),
  _localFilePath(localFilePath),
  _url(url)
{

}

DownloadFileController::DownloadFileController() :
  SidebarSceneController::SidebarSceneController(),
  _fileSize(0),
  _bytesRead(0),
  _previousPercent(0)
{

}

DownloadFileController::~DownloadFileController()
{

}

uint16_t DownloadFileController::getBackgroundColor()
{
  return Application.getTheme()->getColor(BackgroundColor);
}

String DownloadFileController::getName()
{
  return "DownloadFileController";
}



UIBitmap * DownloadFileController::getSidebarIcon() {
  return &uiBitmaps.btn_exit;
}


UIBitmap * DownloadFileController::getSidebarBitmap() {
  return &uiBitmaps.sidebar_downloading;
}

void DownloadFileController::onWillAppear()
{
  BitmapView* icon = new BitmapView(Rect(0,0,uiBitmaps.downloading_scene.width, uiBitmaps.downloading_scene.height));
  icon->setBitmap(&uiBitmaps.downloading_scene);
  addView(icon);

  _progressBar = new ProgressBar(Rect(0,228,270,12));
  //_progressBar->setTrackColor(ILI9341_WHITE);
	_progressBar->setTrackColor(Application.getTheme()->getColor(HighlightBackgroundColor));
  _progressBar->setValue(0.0f);
  addView(_progressBar);

  //Trigger file download
  const char* url = _url.c_str();
  Application.getESPStack()->requestTask(TaskID::DownloadFile,strlen(url),(uint8_t*)url);

  SidebarSceneController::onWillAppear();
}

void DownloadFileController::onSidebarButtonTouchUp()
{
  ProjectsScene* scene = new ProjectsScene();
  Application.pushScene(scene);
}

bool DownloadFileController::handlesTask(TaskID taskID)
{
  switch(taskID) {
    case TaskID::GetJobWithID:
    case TaskID::FileSaveData:
    case TaskID::FileClose:
    case TaskID::SaveProjectWithID:
    case TaskID::FileSetSize:
    case TaskID::Error:
    case TaskID::DownloadFile:
      return true;
      break;
    default:
      return false;
  }
}

bool DownloadFileController::runTask(CommHeader &header, const uint8_t *data, size_t dataSize, uint8_t *responseData, uint16_t *responseDataSize, bool* sendResponse, bool* success)
{
  LOG_VALUE("DownloadFileController handling task",header.getCurrentTask());

  if (header.getCurrentTask() == TaskID::FileSetSize) {
    uint32_t contentLength;
    memcpy(&contentLength,data,sizeof(uint32_t));
    _fileSize = contentLength;
    _bytesRead = 0;
    *sendResponse = true;
    *responseDataSize = 0;

  }
  else if (header.getCurrentTask() == TaskID::DownloadFile)
  {
    LOG("Handling GetJobWithID Task");

    if (header.commType == ResponseSuccess)
    {
      if (dataSize != sizeof(uint32_t))
      {
        LOG_VALUE("Expected content of GetJobWithID to be uint32_t with the number of bytes to receive, but received a content length of",*responseDataSize);
      }
      else
      {
        uint32_t contentLength;
        memcpy(&contentLength,data,sizeof(uint32_t));
        _fileSize = contentLength;
        _bytesRead = 0;

        LOG_VALUE("Expected file size of",_fileSize);

        //Open a file on SD card
        //char * fp[_localFilePath.length() + 1];
        //_localFilePath.toCharArray(fp, _localFilePath.length());
        //SD.remove(fp);
        _file = SD.open(_localFilePath.c_str(),O_WRITE | O_CREAT | O_TRUNC);
        if (!_file.available())
        {
          //TODO: We should handle that. For now we will have to read data from ESP to clean the pipe but there should be better ways to handle errors
          //Application.getESPStack()->requestTask(Error);
          //return false;
        }

        LOG("File opened for writing. Now waiting for number of bytes to read");
        LOG("Now Expecting file chunks with FileSaveData tasks");
      }
    }
    else
    {
      //TODO: Show an error message
      ProjectsScene* scene = new ProjectsScene();
      Application.pushScene(scene);
    }
  }
  else if (header.getCurrentTask() == TaskID::FileSaveData)
  {
    LOG("Handling FileSaveData Task");
    if (header.commType == Request)
    {
      LOG_VALUE("Received Chunk of Data with Size", dataSize);
      int numBytesWritten = _file.write(data, dataSize);
      LOG_VALUE("Written number of bytes to file",numBytesWritten);

      *sendResponse = true;
      *responseDataSize = 0;

      //Add number of bytes received to total bytes read
      _bytesRead += dataSize;

      float fraction = (float)_bytesRead / (float)_fileSize;
      int percent = (int)(fraction*100.0f);

      if (percent != _previousPercent)
      {
        if (percent % 5 == 0)
        {
          //Only update the progress bar once in a while as the communication is blocked while doing so and we don't want to loose too much speed

          // TODO: Currently locks up the communication, disabled until fixed
          _progressBar->setValue(fraction);
        }
      }

      _previousPercent = percent;

    }
  }
  else if (header.getCurrentTask() == TaskID::FileClose)
  {
    LOG("Handling FileClose Task");
    LOG_VALUE("Bytes read",_bytesRead);
    _file.close();

    //PrintStatusSceneController* scene = new PrintStatusSceneController();
    ProjectsScene* scene = new ProjectsScene();
    Application.pushScene(scene);
  }
  else if (header.getCurrentTask() == TaskID::SaveProjectWithID)
  {
    char _fp[header.contentLength+1];
    memset(_fp, 0, header.contentLength+1);
    memcpy(_fp, data, header.contentLength);
    _localFilePath = String(_fp);
    _localFilePath = "/projects/" + _localFilePath;
    //Open a file on SD card
    SD.remove(_fp);
    //SD.remove(_localFilePath.c_str());
    _file = SD.open(_localFilePath.c_str(), FILE_WRITE);

    *sendResponse = true;
    *responseDataSize = 0;
  }

  return true;
}

#pragma mark ButtonDelegate Implementation

void DownloadFileController::buttonPressed(void *button)
{
  SidebarSceneController::buttonPressed(button);
}
