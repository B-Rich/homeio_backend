#include "log_array.hpp"

LogArray::LogArray() {
  maxHistory = 40;
}

void LogArray::log(std::string c) {
  LogItem *li = new LogItem(c);
  addLogItem(*li);
}

void LogArray::logError(std::string c) {
  LogItem *li = new LogItem(c);
  li->error = true;
  addLogItem(*li);
}

void LogArray::addLogItem(LogItem li) {
  if (logBuffer.size() >= maxHistory) {
    logBuffer.erase( logBuffer.begin() );
  }

  logBuffer.push_back(li);
}

void LogArray::limit(unsigned int maxLines) {
  if (maxHistory > maxLines) {
    maxHistory = maxLines;
  }
}