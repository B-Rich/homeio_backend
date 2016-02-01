#ifndef PLANT_MONITOR_ITEM_HPP
#define	PLANT_MONITOR_ITEM_HPP

#include <string>
#include <memory>

#include "../utils/helper.hpp"
#include "../meas/meas_type.hpp"

class PlantMonitorItem {
 public:
  std::string measName;
  std::shared_ptr<MeasType> measType;
};

#endif