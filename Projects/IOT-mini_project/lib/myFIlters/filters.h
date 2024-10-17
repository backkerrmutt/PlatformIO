#ifndef FILTERS_H
#define FILTERS_H

#include <cmath>

/**
 * @brief Statistic block for min/nax/avg
 */
class MinMaxAvgStatistic {
 float min_;
 float max_;
 float sum_;
 int count_;
public:
 MinMaxAvgStatistic();
 void process(float value);
 void reset();
 float minimum() const;
 float maximum() const;
 float average() const;
};

/**
 * @brief High Pass Filter 
 */
class HighPassFilter {
 const float kX;
 const float kA0;
 const float kA1;
 const float kB1;
 float last_filter_value_;
 float last_raw_value_;
public:
 HighPassFilter(float samples);
 HighPassFilter(float cutoff, float sampling_frequency);
 float process(float value);
 void reset();
};

/**
 * @brief Low Pass Filter 
 */
class LowPassFilter {
 const float kX;
 const float kA0;
 const float kB1;
 float last_value_;
public:
 LowPassFilter(float samples);
 LowPassFilter(float cutoff, float sampling_frequency);
 float process(float value);
 void reset();
};

/**
 * @brief Differentiator
 */
class Differentiator {
 const float kSamplingFrequency;
 float last_value_;
public:
 Differentiator(float sampling_frequency);
 float process(float value);
 void reset();
};

/**
 * @brief MovingAverageFilter
 * @tparam buffer_size Number of samples to average over
 */
template<int kBufferSize> class MovingAverageFilter {
 int index_;
 int count_;
 float values_[kBufferSize];
public:
 MovingAverageFilter();
 float process(float value);
 void reset();
 int count() const;
};

// Include template implementation directly in the header file
template<int kBufferSize>
MovingAverageFilter<kBufferSize>::MovingAverageFilter() :
  index_(0), count_(0) {}

template<int kBufferSize>
float MovingAverageFilter<kBufferSize>::process(float value) {  
  values_[index_] = value;
  index_ = (index_ + 1) % kBufferSize;
  if(count_ < kBufferSize) {
    count_++;  
  }
  float sum = 0.0;
  for(int i = 0; i < count_; i++) {
    sum += values_[i];
  }
  return sum/count_;
}

template<int kBufferSize>
void MovingAverageFilter<kBufferSize>::reset() {
  index_ = 0;
  count_ = 0;
}

template<int kBufferSize>
int MovingAverageFilter<kBufferSize>::count() const {
  return count_;
}

#endif // FILTERS_H
