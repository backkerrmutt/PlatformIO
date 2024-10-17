#include "filters.h"
#include <cmath>

// MinMaxAvgStatistic implementation
MinMaxAvgStatistic::MinMaxAvgStatistic() :
  min_(NAN), max_(NAN), sum_(0), count_(0) {}

void MinMaxAvgStatistic::process(float value) {  
  min_ = std::isnan(min_) ? value : std::min(min_, value);
  max_ = std::isnan(max_) ? value : std::max(max_, value);
  sum_ += value;
  count_++;
}

void MinMaxAvgStatistic::reset() {
  min_ = NAN;
  max_ = NAN;
  sum_ = 0;
  count_ = 0;
}

float MinMaxAvgStatistic::minimum() const {
  return min_;
}

float MinMaxAvgStatistic::maximum() const {
  return max_;
}

float MinMaxAvgStatistic::average() const {
  return sum_/count_;
}

// HighPassFilter implementation
HighPassFilter::HighPassFilter(float samples) :
  kX(exp(-1/samples)),
  kA0((1+kX)/2),
  kA1(-kA0),
  kB1(kX),
  last_filter_value_(NAN),
  last_raw_value_(NAN) {}

HighPassFilter::HighPassFilter(float cutoff, float sampling_frequency) :
  HighPassFilter(sampling_frequency/(cutoff*2*M_PI)) {}

float HighPassFilter::process(float value) { 
  if(std::isnan(last_filter_value_) || std::isnan(last_raw_value_)) {
    last_filter_value_ = 0.0;
  } else {
    last_filter_value_ = kA0 * value + kA1 * last_raw_value_ + kB1 * last_filter_value_;
  }
  last_raw_value_ = value;
  return last_filter_value_;
}

void HighPassFilter::reset() {
  last_raw_value_ = NAN;
  last_filter_value_ = NAN;
}

// LowPassFilter implementation
LowPassFilter::LowPassFilter(float samples) :
  kX(exp(-1/samples)),
  kA0(1-kX),
  kB1(kX),
  last_value_(NAN) {}

LowPassFilter::LowPassFilter(float cutoff, float sampling_frequency) :
  LowPassFilter(sampling_frequency/(cutoff*2*M_PI)) {}

float LowPassFilter::process(float value) {  
  if(std::isnan(last_value_)) {
    last_value_ = value;
  } else {  
    last_value_ = kA0 * value + kB1 * last_value_;
  }
  return last_value_;
}

void LowPassFilter::reset() {
  last_value_ = NAN;
}

// Differentiator implementation
Differentiator::Differentiator(float sampling_frequency) :
  kSamplingFrequency(sampling_frequency),
  last_value_(NAN) {}

float Differentiator::process(float value) {  
  float diff = (value-last_value_)*kSamplingFrequency;
  last_value_ = value;
  return diff;
}

void Differentiator::reset() {
  last_value_ = NAN;
}
