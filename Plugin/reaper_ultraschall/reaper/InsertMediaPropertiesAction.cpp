////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 Ultraschall (http://ultraschall.fm)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>

#include <Framework.h>
#include <StringUtilities.h>

#include "InsertMediaPropertiesAction.h"
#include "CustomActionFactory.h"
#include "FileManager.h"
#include "NotificationWindow.h"
#include "ITagWriter.h"

namespace ultraschall {
namespace reaper {

class ErrorRecord
{
public:
   ErrorRecord(const std::string& target, const std::string& message) :
      target_(target), message_(message)
   {
   }

   inline const std::string& Target() const
   {
      return target_;
   }

   inline const std::string& Message() const
   {
      return message_;
   }

private:
   std::string target_;
   std::string message_;
};

static DeclareCustomAction<InsertMediaPropertiesAction> action;

ServiceStatus InsertMediaPropertiesAction::Execute()
{
   if(ConfigureAssets() == true)
   {
      std::vector<ErrorRecord> errorRecords;
      
      ProjectManager& projectManager = ProjectManager::Instance();
      Project currentProject = projectManager.CurrentProject();
      
      for(size_t i = 0; i < targetNames_.size(); i++)
      {
         const std::string& targetName = targetNames_[i];
         
         ITagWriter* pTagWriter = CreateTagWriter(targetNames_[i]);
         if(pTagWriter != nullptr)
         {
            BasicMediaInformation properties = BasicMediaInformation::ParseString(currentProject.Notes());
            if(pTagWriter->InsertStandardProperties(targetName, properties) == false)
            {
               errorRecords.push_back(ErrorRecord(targetName, ": Failed to insert tags."));
            }
            
            if(cover_.empty() == false)
            {
               if(pTagWriter->InsertCoverImage(targetName, cover_) == false)
               {
                  errorRecords.push_back(ErrorRecord(targetName, ": Failed to insert cover image."));
               }
            }
            
            if(chapters_.empty() == false)
            {
               if(pTagWriter->InsertChapterMarkers(targetName, chapters_, true) == false)
               {
                  errorRecords.push_back(ErrorRecord(targetName, ": Failed to insert chapter markers."));
               }
            }
            
            framework::SafeRelease(pTagWriter);
         }
      }
      
      if(errorRecords.size() > 0)
      {
         for(size_t j = 0; j < errorRecords.size(); j++)
         {
            NotificationWindow::Show(errorRecords[j].Message(), errorRecords[j].Target(), true);
         }
      }
      else
      {
         std::stringstream os;
         os << "The following media files have been updated successfully:\r\n\r\n";
         for(size_t k = 0; k < targetNames_.size(); k++)
         {
            const std::string::size_type offset = targetNames_[k].rfind(FileManager::PathSeparator());
            if(offset != std::string::npos)
            {
               const std::string& targetName = targetNames_[k].substr(offset + 1, targetNames_[k].size()); // skip separator
               os << targetName;
               if(k < (targetNames_.size() - 1))
               {
                  os << "\r\n";
               }
            }
         }
         
         os << "\r\n";
         
         NotificationWindow::Show(os.str());
      }
   }
   
   return SERVICE_SUCCESS;
}

std::vector<std::string> InsertMediaPropertiesAction::FindTargetFiles(const Project& project)
{
   std::vector<std::string> targetNames;

   const std::string projectFolder = project.FolderName();
   const std::string projectName = project.Name();

   PRECONDITION_RETURN(projectFolder.empty() == false, targetNames);
   PRECONDITION_RETURN(projectName.empty() == false, targetNames);

   static const size_t MAX_FILE_EXTENSIONS = 3;
   static const char* fileExtensions[MAX_FILE_EXTENSIONS] = {"mp3", "mp4", "m4a"};
   for(size_t i = 0; i < MAX_FILE_EXTENSIONS; i++)
   {
      std::string targetName = FileManager::AppendPath(projectFolder, projectName) + "." + fileExtensions[i];
      if(FileManager::FileExists(targetName) != false)
      {
         targetNames.push_back(targetName);
      }
   }

   return targetNames;
}

std::string InsertMediaPropertiesAction::FindCoverImage(const Project& project)
{
   const std::string projectFolder = project.FolderName();
   const std::string projectName = project.Name();

   PRECONDITION_RETURN(projectFolder.empty() == false, std::string());
   PRECONDITION_RETURN(projectName.empty() == false, std::string());

   std::string coverImage;

   std::vector<std::string> imageNames;
   imageNames.push_back(FileManager::AppendPath(projectFolder, "cover") + ".jpg");
   imageNames.push_back(FileManager::AppendPath(projectFolder, "cover") + ".jpeg");
   imageNames.push_back(FileManager::AppendPath(projectFolder, "cover") + ".png");
   imageNames.push_back(FileManager::AppendPath(projectFolder, projectName) + ".jpg");
   imageNames.push_back(FileManager::AppendPath(projectFolder, projectName) + ".jpeg");
   imageNames.push_back(FileManager::AppendPath(projectFolder, projectName) + ".png");
   const size_t imageIndex = FileManager::FileExists(imageNames);
   if(imageIndex != -1)
   {
      coverImage = imageNames[imageIndex];
   }

   return coverImage;
}

ITagWriter* InsertMediaPropertiesAction::CreateTagWriter(const std::string& targetName)
{
   PRECONDITION_RETURN(targetName.empty() == false, nullptr);
   PRECONDITION_RETURN(targetName.length() > 4, nullptr);

   ITagWriter* tagWriter = nullptr;

   const TARGET_TYPE targetType = EvaluateFileType(targetName);
   if(targetType == MP3_TARGET)
   {
      tagWriter = new MP3TagWriter();
   }
   else if(targetType == MP4_TARGET)
   {
      tagWriter = new MP4TagWriter();
   }

   return tagWriter;
}

std::string InsertMediaPropertiesAction::NormalizeTargetName(const std::string& targetName)
{
   std::string firstStage = targetName;
   std::string secondStage = framework::StringTrimRight(firstStage);
   return framework::StringLowercase(secondStage);
}

InsertMediaPropertiesAction::TARGET_TYPE InsertMediaPropertiesAction::EvaluateFileType(const std::string& targetName)
{
   PRECONDITION_RETURN(targetName.empty() == false, INVALID_TARGET_TYPE);

   TARGET_TYPE type = INVALID_TARGET_TYPE;

   const std::string cookedTargetName = NormalizeTargetName(targetName);
   const size_t extensionOffset = targetName.rfind(".");
   if(extensionOffset != std::string::npos)
   {
      const std::string fileExtension = targetName.substr(extensionOffset + 1, targetName.length() - extensionOffset);
      if(fileExtension.empty() == false)
      {
         if(fileExtension == "mp3")
         {
            type = MP3_TARGET;
         }
         else if(fileExtension == "mp4")
         {
            type = MP4_TARGET;
         }
         else if(fileExtension == "m4a")
         {
            type = MP4_TARGET;
         }
      }
   }

   return type;
}

bool InsertMediaPropertiesAction::ConfigureAssets()
{
   bool result = true;
   
   ResetAssets();
   
   ProjectManager& projectManager = ProjectManager::Instance();
   Project currentProject = projectManager.CurrentProject();
   
   targetNames_ = FindTargetFiles(currentProject);
   if(targetNames_.empty() == true)
   {
      const std::string message = "Can't find a single target file!";
      NotificationWindow::Show(message.c_str(), true); // target files are mandatory, at least one.

      result = false;
   }
   
   if(true == result)
   {
      id3v2_ = BasicMediaInformation::ParseString(currentProject.Notes());
      if(id3v2_.Validate() == false)
      {
         const std::string message = "ID3v2 tags have not been defined yet.";
         NotificationWindow::Show(message.c_str(), true); // id3v2 tags are mandatory
         
         result = false;
      }
   }
   
   if(true == result)
   {
      cover_ = FindCoverImage(currentProject);
      if(cover_.empty() == true)
      {
         const std::string message = "Cover image is missing.";
         NotificationWindow::Show(message.c_str(), false); // cover is optional
         
         result = true;
      }
   }
   
   if(true == result)
   {
      chapters_ = currentProject.QueryAllMarkers();
      if(chapters_.empty() == true)
      {
         const std::string message = "No chapters, you fool!";
         NotificationWindow::Show(message.c_str(), false); // chapters are optional
         
         result = false;
      }
   }
   
   return result;
}

void InsertMediaPropertiesAction::ResetAssets()
{
   targetNames_.clear();
   id3v2_.Reset();
   cover_.clear();
   chapters_.clear();
}
   
}
}


