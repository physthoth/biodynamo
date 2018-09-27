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

#ifndef MY_CELL_H_
#define MY_CELL_H_

#include <biodynamo.h>

namespace bdm {

// Define my custom cell, which extends Cell by adding an extra
// data member cell_type.
class MyCell : public Cell {
// BDM_SIM_OBJECT(MyCell, Cell) {
  // BDM_SIM_OBJECT_HEADER(MyCellExt, 1, cell_type_);

 public:
  MyCell() {}
  explicit MyCell(const std::array<double, 3>& position) : Cell(position) {}

  void SetCellType(int t) { cell_type_ = t; }
  int GetCellType() const { return cell_type_; }

 private:
  int cell_type_;
};

}  // namespace bdm

#endif  // MY_CELL_H_