/*
  Copyright (C) 2007 National Institute For Space Research (INPE) - Brazil.

  This file is part of TerraMA2 - a free and open source computational
  platform for analysis, monitoring, and alert of geo-environmental extremes.

  TerraMA2 is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License,
  or (at your option) any later version.

  TerraMA2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with TerraMA2. See LICENSE. If not, write to
  TerraMA2 Team at <terrama2-team@dpi.inpe.br>.
*/

/*!
  \file terrama2/services/analysis/core/grid/zonal/Operator.cpp

  \brief Contains grid zonal analysis operators.

  \author Jano Simas
*/



#include "Operator.hpp"
#include "../../Operator.hpp"
#include "../../Utils.hpp"
#include "../../../../utility/Utils.hpp"
#include "../../../../utility/Verify.hpp"
#include "../../../../python/PythonInterpreter.hpp"
#include "../../../../ContextManager.hpp"
#include "../../../../MonitoredObjectContext.hpp"

#include "../../../../../../../core/data-model/Filter.hpp"
#include "../../../../../../../core/utility/TimeUtils.hpp"
#include "../../../../../../../core/utility/Logger.hpp"

// TerraLib
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/vp/BufferMemory.h>
#include <terralib/geometry/MultiPolygon.h>
#include <terralib/geometry/Utils.h>
#include <terralib/raster/PositionIterator.h>

double terrama2::services::analysis::core::grid::zonal::history::accum::getAbsTimeFromString(const std::string& timeStr)
{
  auto begin = timeStr.find_first_of("0123456789");
  auto end = timeStr.find_last_of("0123456789");

  if(begin == std::string::npos || end == std::string::npos)
    return std::nan("");

  auto time = timeStr.substr(begin, end-begin);
  return std::stod(time);
}

double terrama2::services::analysis::core::grid::zonal::history::accum::operatorImpl(terrama2::services::analysis::core::StatisticOperation statisticOperation,
    const std::string& dataSeriesName, const std::string& dateDiscardBefore, const std::string& dateDiscardAfter, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  OperatorCache cache;
  terrama2::services::analysis::core::python::readInfoFromDict(cache);
  // After the operator lock is released it's not allowed to return any value because it doesn' have the interpreter lock.
  // In case an exception is thrown, we need to set this boolean. Once the code left the lock is acquired we should return NAN.
  bool exceptionOccurred = false;

  auto& contextManager = ContextManager::getInstance();
  auto analysis = cache.analysisPtr;

  try
  {
    terrama2::services::analysis::core::verify::analysisMonitoredObject(analysis);
  }
  catch(const terrama2::core::VerifyException&)
  {
    contextManager.addError(cache.analysisHashCode, QObject::tr("Use of invalid operator for analysis %1.").arg(analysis->id).toStdString());
    return std::nan("");
  }

  terrama2::services::analysis::core::MonitoredObjectContextPtr context;
  try
  {
    context = ContextManager::getInstance().getMonitoredObjectContext(cache.analysisHashCode);
  }
  catch(const terrama2::Exception& e)
  {
    TERRAMA2_LOG_ERROR() << boost::get_error_info<terrama2::ErrorDescription>(e)->toStdString();
    return std::nan("");
  }


  try
  {
    // In case an error has already occurred, there is nothing to do.
    if(context->hasError())
      return std::nan("");

    auto valuesMap = utils::getAccumulatedMap<double>(dataSeriesName, dateDiscardBefore, dateDiscardAfter, band, buffer,
                                                      context, cache);

    if(valuesMap.empty() && statisticOperation != StatisticOperation::COUNT)
    {
      return std::nan("");
    }
    std::vector<double> values;
    values.reserve(valuesMap.size());

    for(const auto& pair : valuesMap)
      values.push_back(pair.second.first);

    terrama2::services::analysis::core::calculateStatistics(values, cache);
    return terrama2::services::analysis::core::getOperationResult(cache, statisticOperation);
  }
  catch(const terrama2::Exception& e)
  {
    context->addLogMessage(BaseContext::MessageType::ERROR_MESSAGE, boost::get_error_info<terrama2::ErrorDescription>(e)->toStdString());
    return std::nan("");
  }
  catch(const std::exception& e)
  {
    context->addLogMessage(BaseContext::MessageType::ERROR_MESSAGE, e.what());
    return std::nan("");
  }
  catch(...)
  {
    QString errMsg = QObject::tr("An unknown exception occurred.");
    context->addLogMessage(BaseContext::MessageType::ERROR_MESSAGE, errMsg.toStdString());
    return std::nan("");
  }
}

double terrama2::services::analysis::core::grid::zonal::history::accum::min(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::MIN, dataSeriesName, dateDiscardBefore, "0s", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::history::accum::max(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::MAX, dataSeriesName, dateDiscardBefore, "0s", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::history::accum::mean(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::MEAN, dataSeriesName, dateDiscardBefore, "0s", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::history::accum::median(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::MEDIAN, dataSeriesName, dateDiscardBefore, "0s", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::history::accum::standardDeviation(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::STANDARD_DEVIATION, dataSeriesName, dateDiscardBefore, "0s", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::history::accum::variance(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::VARIANCE, dataSeriesName, dateDiscardBefore, "0s", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::history::accum::sum(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::SUM, dataSeriesName, dateDiscardBefore, "0s", band, buffer);
}
