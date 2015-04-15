TcpCommand::TcpCommand() {
}

void TcpCommand::logInfo(string log) {
  logWithColor(log, CYAN);
}

// sample command: "meas;batt_u;0;100;"
string TcpCommand::processCommand(string command) {
  logInfo("TCP command: " + command);
  
  string commandName, response;
  response = "{}";
  
  commandName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);

  if (commandName == "measIndex") {
    response = processMeasIndexCommand(command);
  }
  if (commandName == "measNameList") {
    response = processMeasNameListCommand(command);
  }
  if (commandName == "measShow") {
    response = processMeasShowCommand(command);
  }
  if (commandName == "measRawForTime") {
    response = processMeasRawForTimeCommand(command);
  }
  if (commandName == "measRawForIndex") {
    response = processMeasRawForIndexCommand(command);
  }
  if (commandName == "measStorage") {
    response = processMeasStorageCommand(command);
  }
  if (commandName == "actionIndex") {
    response = processActionIndexCommand(command);
  }
  if (commandName == "actionShow") {
    response = processActionShowCommand(command);
  }
  if (commandName == "actionExecute") {
    response = processActionExecuteCommand(command);
  }
  if (commandName == "actionHistory") {
    response = processActionHistoryCommand(command);
  }
  if (commandName == "overseerIndex") {
    response = processOverseerIndexCommand(command);
  }
  if (commandName == "overseerShow") {
    response = processOverseerShowCommand(command);
  }
  
  
  logInfo("TCP response: " + response);
  
  return response;
}

// meas#index
string TcpCommand::processMeasIndexCommand(string command) {
  string response, detailsResponse;
  
  response = "{\"status\":0,\"array\":[";
  
  for(std::vector<MeasType>::iterator it = measTypeArray->measTypes.begin(); it != measTypeArray->measTypes.end(); ++it) {
    detailsResponse += it->toJson() + ",";
  }
  
  // remove last coma
  if (detailsResponse[detailsResponse.size() - 1] == ',') {
    detailsResponse.resize(detailsResponse.size() - 1);
  }
  
  response += detailsResponse + "]}";
    
  return response;
}

// meas#name_list
string TcpCommand::processMeasNameListCommand(string command) {
  string response;
  
  response = "{\"status\":0,\"array\":[";
  
  for(std::vector<MeasType>::iterator it = measTypeArray->measTypes.begin(); it != measTypeArray->measTypes.end(); ++it) {
    response += "\"" + it->name + "\",";
  }
  
  // remove last coma
  if (response[response.size() - 1] == ',') {
    response.resize(response.size() - 1);
  }
  response += "]}";
    
  return response;
}

// meas#show
string TcpCommand::processMeasShowCommand(string command) {
  string response, measName;
  measName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  MeasType *foundMeasType = measTypeArray->byName(measName);
  
  if (foundMeasType) {
    response = "{\"status\":0,\"object\":" + foundMeasType->toJson() + "}";
  }
  else {
    response = "{\"status\":1,\"meas_type\":\"" + measName + "\",\"reason\":\"meas_not_found\"}";
  }
  
  return response;
}

// meas#raw_for_time
string TcpCommand::processMeasRawForTimeCommand(string command) {
  string measName, fromString, toString, response;
  unsigned long long tFrom, tTo;
  
  measName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  fromString = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  toString = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  stringstream(fromString) >> tFrom;
  stringstream(toString) >> tTo;
  
  MeasType *foundMeasType = measTypeArray->byName(measName);
  
  if (foundMeasType) {
    logInfo("TCP command: processMeasRawForTimeCommand(" + measName + ", " + to_string(tFrom) + ", " + to_string(tTo) + ")" );
    
    string bufferString = foundMeasType->rawForTimeJson(tFrom, tTo);
    response = "{\"status\":0,\"meas_type\":\"" + measName + "\"";
    response += ",\"lastTime\":" + to_string( foundMeasType->buffer->lastTime );
    response += ",\"firstTime\":" + to_string( foundMeasType->buffer->firstTime );
    response += ",\"interval\":" + to_string( foundMeasType->buffer->calcInterval() );
    response += ",\"count\":" + to_string( foundMeasType->buffer->count );
    response += ",\"usingMiliSeconds\":1";
    response += ",\"data\": " + bufferString;
    response += ",\"fromTime\":" + to_string( tFrom );
    response += ",\"toTime\":" + to_string( tTo );
    response += "}";
  }
  else {
    response = "{\"status\":1,\"meas_type\":\"" + measName + "\",\"reason\":\"meas_not_found\"}";
  }
  
  return response;
}

// meas#raw_for_index
string TcpCommand::processMeasRawForIndexCommand(string command) {
  string measName, fromString, toString, response;
  unsigned long int iFrom, iTo;
  
  measName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  fromString = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  toString = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  stringstream(fromString) >> iFrom;
  stringstream(toString) >> iTo;
  
  MeasType *foundMeasType = measTypeArray->byName(measName);
  
  if (foundMeasType) {
    logInfo("TCP command: processMeasRawForIndexCommand(" + measName + ", " + to_string(iFrom) + ", " + to_string(iTo) + ")" );
    
    string bufferString = foundMeasType->rawForIndexJson(iFrom, iTo);
    response = "{\"status\":0,\"meas_type\":\"" + measName + "\"";
    response += ",\"lastTime\":" + to_string( foundMeasType->buffer->lastTime );
    response += ",\"firstTime\":" + to_string( foundMeasType->buffer->firstTime );
    response += ",\"interval\":" + to_string( foundMeasType->buffer->calcInterval() );
    response += ",\"count\":" + to_string( foundMeasType->buffer->count );
    response += ",\"usingMiliSeconds\":1";
    response += ",\"data\": " + bufferString;
    response += ",\"fromIndex\":" + to_string( iFrom );
    response += ",\"toIndex\":" + to_string( iTo );
    response += "}";
  }
  else {
    response = "{\"status\":1,\"meas_type\":\"" + measName + "\",\"reason\":\"meas_not_found\"}";
  }
  
  return response;
}

// meas#storage
string TcpCommand::processMeasStorageCommand(string command) {
  string measName, fromString, toString, response;
  unsigned long long from, to;
  
  measName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  fromString = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  toString = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  stringstream(fromString) >> from;
  stringstream(toString) >> to;
  
  MeasType *foundMeasType = measTypeArray->byName(measName);
  
  if (foundMeasType) {
    string bufferString = foundMeasType->storageJson(from, to);
    response = "{\"status\":0,\"meas_type\":\"" + measName + "\",";
    response +=  "\"data\":" + bufferString;
    response += "}";
  }
  else {
    response = "{\"status\":1,\"meas_type\":\"" + measName + "\",\"reason\":\"meas_not_found\"}";
  }  
  return response;
}

// actions#index
string TcpCommand::processActionIndexCommand(string command) {
  string response, detailsResponse;
  
  response = "{\"status\":0,\"array\":[";
  
  for(std::vector<ActionType>::iterator it = actionTypeArray->actionTypes.begin(); it != actionTypeArray->actionTypes.end(); ++it) {
    detailsResponse += it->toJson() + ",";
  }  
  
  // remove last coma
  if (detailsResponse[detailsResponse.size() - 1] == ',') {
    detailsResponse.resize(detailsResponse.size() - 1);
  }
  
  response += detailsResponse + "]}";
    
  return response;
}

// action#show
string TcpCommand::processActionShowCommand(string command) {
  string response, actionName;
  actionName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  ActionType *foundActionType = actionTypeArray->byName(actionName);
  
  if (foundActionType) {
    response = "{\"status\":0,\"object\":" + foundActionType->toJson() + "}";
  }
  else {
    response = "{\"status\":1,\"action_type\":\"" + actionName + "\",\"reason\":\"action_not_found\"}";
  }
  
  return response;
}

// action#execute
string TcpCommand::processActionExecuteCommand(string command) {
  string actionName;
  actionName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  string response;
  
  ActionType *foundActionType = actionTypeArray->byName(actionName);
  
  if (foundActionType) {
    foundActionType->execute();
    response = "{\"status\":0,\"action\":" + foundActionType->toJson() + "}";
  }
  else {
    response = "{\"status\":1,\"action\":\"" + actionName + "\",\"reason\":\"action_not_found\"}";
  }
  
  return response;  
}

// action#history
string TcpCommand::processActionHistoryCommand(string command) {
  string actionName;
  actionName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  
  string response;
  
  ActionType *foundActionType = actionTypeArray->byName(actionName);
  
  if (foundActionType) {
    response = "{\"status\":0,\"action\":" + foundActionType->historyToJson() + "}";
  }
  else {
    response = "{\"status\":1,\"action\":\"" + actionName + "\",\"reason\":\"action_not_found\"}";
  }
  
  return response;  
}


// overseers#index
string TcpCommand::processOverseerIndexCommand(string command) {
  string response, detailsResponse;
  
  response = "{\"status\":0,\"array\":[";
  
  for(std::vector<Overseer>::iterator it = overseerArray->overseers.begin(); it != overseerArray->overseers.end(); ++it) {
    detailsResponse += it->toJson() + ",";
  }  
  
  // remove last coma
  if (detailsResponse[detailsResponse.size() - 1] == ',') {
    detailsResponse.resize(detailsResponse.size() - 1);
  }
  
  response += detailsResponse + "]}";
    
  return response;
}

// overseers#show
string TcpCommand::processOverseerShowCommand(string command) {
  string response, overseerName;
  overseerName = command.substr(0, command.find(";"));
  command = command.substr(command.find(";") + 1);
  Overseer *foundOverseer = overseerArray->byName(overseerName);
  
  if (foundOverseer) {
    response = "{\"status\":0,\"object\":" + foundOverseer->toJson() + "}";
  }
  else {
    response = "{\"status\":1,\"overseer\":\"" + overseerName + "\",\"reason\":\"overseer_not_found\"}";
  }
  
  return response;
}





//








