#include "meas_buffer.hpp"

MeasBuffer::MeasBuffer() {
  maxSize = 1000000;
  elementSize = sizeof(meas_buffer_element);
  clearAndResize(maxSize);
  removeSpikes = false;
}

MeasBuffer::MeasBuffer(meas_buffer_index _maxSize) {
  maxSize = _maxSize;
  elementSize = sizeof(meas_buffer_element);
  clearAndResize(maxSize);
  removeSpikes = false;
}

void MeasBuffer::clearAndResize(meas_buffer_index _maxSize) {
  maxSize = _maxSize;

  offset = 0;
  count = 0;
  firstTime = 0;
  responseIndexInterval = 1;

  buffer.clear();
  buffer.reserve(maxSize);
  buffer.resize(maxSize);
  // http://www.cplusplus.com/reference/vector/vector/reserve/
}

meas_buffer_index MeasBuffer::add(meas_buffer_element raw) {
  // special algorithm to remove spikes
  // update spike with raw before
  if ( (removeSpikes) && (wasSpike(raw)) ) {
    buffer[offset] = at(1);
  }

  // must be here because in other case first measurement is equal 0
  if (count == 0) {
    buffer[offset] = raw;
  }

  offset++;
  if (offset >= maxSize) {
    offset = 0;
  }
  if (offset > count) {
    count = offset;
    lastTimeForCount = Helper::mTime();
  }
  if (firstTime == 0) {
    firstTime = Helper::mTime();
  }
  lastTime = Helper::mTime();

  buffer[offset] = raw;

  return offset;
}



meas_buffer_index MeasBuffer::calcInterval() {
  if ((offset == 0) && (count == 0)) {
    return 1; // not to fuck up all this shit somewhere
  } else {
    meas_time timeCount = lastTimeForCount - firstTime;
    return timeCount / count;
  }
}

meas_time MeasBuffer::earliestTime() {
  return lastTime - ((meas_time) calcInterval() * (meas_time) count);
}

// detect if b is spike near a and c
bool MeasBuffer::isSpike(meas_buffer_element a, meas_buffer_element b, meas_buffer_element c) {
  int absA = abs( (int) a - (int) c );
  int absB = abs( (int) a - (int) b );

  if (absB > 10 * (absA + 1) ) {
    logArray->log("MeasBuffer", "found SPIKE " + std::to_string(a) + " - " + std::to_string(b) + " - " + std::to_string(c) );
    return true;
  } else {
    return false;
  }

}

bool MeasBuffer::wasSpike(meas_buffer_element latestRaw) {
  // not enough data
  if (count <= 3) {
    return false;
  }

  return isSpike(at(1), at(0), latestRaw);
}

void MeasBuffer::filterStoredSpikes() {
  // not enough data
  if (count <= 3) {
    return;
  }

  meas_buffer_index i;

  // iterate buffer
  for (i = 1; i < (count - 1); i++) {
    // check if spike within closest raws
    if ( isSpike( at(i+1), at(i), at(i-1)) ) {
      // overwrite
      buffer[index(i)] = buffer[index(i+1)];
    }
  }
}

meas_buffer_element MeasBuffer::at(meas_buffer_index i) {
  return buffer[index(i)];
}

meas_buffer_element MeasBuffer::last() {
  return at(0);
}

meas_buffer_index MeasBuffer::index(meas_buffer_index i) {
  long int tmpI = (long int) offset - (long int) i;
  if (tmpI >= 0) {
    return (unsigned long int) tmpI;
  } else {
    return maxSize + offset - i;
  }
}

unsigned long int MeasBuffer::memorySize() {
  return buffer.capacity() * elementSize;
}

bool MeasBuffer::stored(meas_buffer_index i) {
  if (i>count) {
    return false;
  } else {
    return true;
  }
}

meas_buffer_index MeasBuffer::tempResponseIndexInterval() {
  return responseIndexInterval;
}

meas_buffer_index MeasBuffer::calculateIndexInterval(meas_buffer_index lower, meas_buffer_index higher, meas_buffer_index responseMaxSize) {
  if (responseMaxSize == 0) {
    return 1;
  }

  meas_buffer_index distance = higher - lower;

  if (distance <= responseMaxSize) {
    return 1;
  } else {
    return (meas_buffer_index) ceil((double) distance / (double) responseMaxSize);
  }
}

std::vector < meas_buffer_element > MeasBuffer::getFromBuffer(meas_buffer_index from, meas_buffer_index to, meas_buffer_index responseMaxSize) {
  meas_buffer_index i;
  std::vector < meas_buffer_element > result;

  if (from <= to) {
    // safety
    if (to >= maxSize) {
      to = maxSize - 1;
    }

    logArray->log("MeasBuffer", "UP getFromBuffer(" + std::to_string(from) + ", " + std::to_string(to) + ", " + std::to_string(responseMaxSize) + ")");

    responseIndexInterval = calculateIndexInterval(from, to, responseMaxSize);
    for (i = from; i <= to; i += responseIndexInterval) {
      if (stored(i)) {
        result.push_back(at(i));
      }
    }
    return result;
  } else {
    // safety
    if (from >= maxSize) {
      from = maxSize - 1;
    }

    logArray->log("MeasBuffer", "DOWN getFromBuffer(" + std::to_string(from) + ", " + std::to_string(to) + ", " + std::to_string(responseMaxSize) + ")");

    responseIndexInterval = calculateIndexInterval(to, from, responseMaxSize);
    for (i = from; (i > to) && (i >= responseIndexInterval); i -= responseIndexInterval) {
      if (stored(i)) {
        result.push_back(at(i));
      }
    }

    return result;
  }
}

std::string MeasBuffer::jsonArray(meas_buffer_index from, meas_buffer_index to, meas_buffer_index responseMaxSize) {
  std::string s = "[";
  std::vector < meas_buffer_element > tmp = getFromBuffer(from, to, responseMaxSize);
  for(std::vector<meas_buffer_element>::iterator it = tmp.begin(); it != tmp.end(); ++it) {
    s += std::to_string(*it);
    s += ",";
  }

  // remove last coma
  if (s[s.size() - 1] == ',') {
    s.resize(s.size() - 1);
  }

  s += "]";

  return s;
}

std::string MeasBuffer::toJson() {
  std::string response;
  response = "{";
  response += "\"interval\":" + std::to_string(calcInterval()) + ",";
  response += "\"count\":" + std::to_string(count) + ",";
  response += "\"offset\":" + std::to_string(offset) + ",";
  response += "\"maxSize\":" + std::to_string(maxSize) + ",";
  response += "\"lastTime\":" + std::to_string(lastTime) + ",";
  response += "\"firstTime\":" + std::to_string(firstTime); //+ ",";
  //response += "\"earliestTime\":" + to_string(earliestTime());
  response += "}";
  return response;
}
