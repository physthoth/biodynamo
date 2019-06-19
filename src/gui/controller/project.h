// Author: Lukasz Stempniewicz 25/05/19

// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// This File contains the declaration of the Project-class              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef GUI_PROJECT_H_
#define GUI_PROJECT_H_

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <RQ_OBJECT.h>
#include <TApplication.h>
#include <TClass.h>
#include <TEnv.h>
#include <TFile.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TVirtualX.h>
#include "TObject.h"

#include "core/util/io.h"
#include "gui/model/model.h"
#include "gui/view/log.h"
#include "gui/controller/project_object.h"

namespace gui {

enum class SimulationType { kBuild, kRun };

/// Project represents the intermediate communication mechanism
/// between the GUI and its models. `gui.h` will contain only 1 instance
/// of Project and will fetch as well as update the model through this instance.
/// This design will allow the GUI to be more-or-less independent
/// of the model implementation.

class Project {
  /// Using the singleton design pattern
 public:
  static Project& GetInstance() {
    static Project instance;  // Instantiated on first use.
    return instance;
  }

  ~Project() = default;

  /// Creates a new project
  /// @ param path path and filename of the project root file
  /// this file will contain a ProjectObject
  /// e.g. `/home/user/ProjectName.root`
  /// @return None
  void NewProject(const char* path, const char* name) {
    Log::Info("Creating new project at: ", path);
    Log::Info("  with name: ", name);

    fProjectObject.SetProjectName(name);
    fProjectPath.assign(path);
    fIsLoaded = kTRUE;

    SaveProject();
  }

  /// Loads project
  /// @param path path and filename of the project root file
  /// @return None
  void LoadProject(const char* path) {
    Log::Info("Loading project file: ", path);
    ReadProject();
  }

  /// Saves Project and all of its models.
  /// @param modelName optional input to specify
  /// saving a single model. Default "" will save all models
  /// @return None
  void SaveProject() {
    Log::Info("Attempting to save project");
    try {
      Log::Info("Saving project ...");
      bdm::WritePersistentObject(fProjectPath.c_str(), "ProjectObject",
                               fProjectObject, "recreate");
      Log::Info("Successfully saved!");
    } catch (...) {
      Log::Error("Couldn't save project!");
    }
    
  }

  /// Saves Project under a different name. Copies over models from original
  /// project.
  /// @param path and filename of the project root file
  /// @return None
  void SaveAsProject(const char* path) {
    Log::Info("Saving project as: ", path);
    WriteProject();
  }

  void CreateModel(const char* name) {
    Log::Info("Creating model within `Project`");
    if (fModels == nullptr) {
      fModels = new TList;
    }
    if (GetModel(name) != 0) {
      Log::Warning("Cannot create model with the same name!");
    } else {
      Model* tmp = new Model(name);
      //fModels->Add(tmp);
    }
  }

  Bool_t CreateModelElement(const char* modelName, const char* parent,
                            const char* elementName, int type) {
    Model* tmp = GetModel(modelName);
    if (tmp == 0) {
      Log::Warning("Cannot create element `", elementName,
                   "` within non-existent model `", modelName, "`");
    }
    return tmp->CreateElement(parent, elementName, type);
  }

  Model* GetModel(const char* name) {
    Model* tmp = (Model*)(fModels->FindObject(name));
    return tmp;
  }

  TList* GetAllModels() { return fModels; }

  void SaveAllModels() {}

  /// Starts BioDynaMo simulation on user-defined models.
  /// Initially it calls GenerateCode, the runs the command
  /// `biodynamo run` or `biodynamo build` on the generated code.
  /// @param simType signifies whether building or running.
  /// @param modelName optional input to specify simulation a
  /// single model. Default "" simulates all models.
  /// @return None
  void SimulateModel(SimulationType simType, const char* modelName = "") {}

  /// Starts BioDynaMo code generation on user-defined models.
  /// The `Project` class description explains where code will reside.
  /// @param modelName optional input to specify generate code for
  /// a single model. Default "" generates for all models.
  /// @return None
  void GenerateCode(const char* modelName = "") {}

  Bool_t fIsLoaded;  // kTRUE upon successful loading or new project creation
  TList* fModels = nullptr;

 private:
  /// Constructor
  Project() : fIsLoaded(kFALSE) {}
  Project(Project const&) = delete;
  Project& operator=(Project const&) = delete;

  void ReadProject() {}

  void WriteProject() {}

  std::string GetProjectDir() { return std::string(""); }

  std::string RunCmd(const char* cmd) {
    Log::Info("Running cmd:", cmd);
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
      throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      std::string tmp = buffer.data();
      Log::Info(tmp);
      result += tmp;
    }
    return result;
  }

  /// Contains all files and subfolders relevant to the project
  TDirectory fProjectDirectory;

  /// To be contained within fProjectFile
  ProjectObject fProjectObject;
  std::string fProjectPath;
};

}  // namespace gui

#endif  // GUI_PROJECT_H_