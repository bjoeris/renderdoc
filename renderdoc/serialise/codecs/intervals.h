/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2017-2018 Baldur Karlsson
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
******************************************************************************/
#pragma once

#include <algorithm>
#include <map>

namespace vk_cpp_codec
{
template <typename T>
struct Intervals;

template <typename T>
struct IntervalsIter
{
  friend struct Intervals<T>;

private:
  typename std::map<uint64_t, T>::iterator iter;
  const std::map<uint64_t, T> *owner;
  IntervalsIter(std::map<uint64_t, T> *owner, typename std::map<uint64_t, T>::iterator iter)
      : iter(iter), owner(owner)
  {
  }
  typename std::map<uint64_t, T>::iterator unwrap() { return iter; }
public:
  IntervalsIter(const IntervalsIter &src) : owner(src.owner), iter(src.iter) {}
  IntervalsIter &operator++()
  {
    ++iter;
    return *this;
  }
  IntervalsIter operator++(int)
  {
    IntervalsIter tmp(*this);
    operator++();
    return tmp;
  }
  IntervalsIter &operator--()
  {
    --iter;
    return *this;
  }
  IntervalsIter operator--(int)
  {
    IntervalsIter tmp(*this);
    operator--();
    return tmp;
  }
  bool operator==(const IntervalsIter &rhs) const { return iter == rhs.iter && owner == rhs.owner; }
  bool operator!=(const IntervalsIter &rhs) const { return iter != rhs.iter && owner == rhs.owner; }
  IntervalsIter &operator=(const IntervalsIter &rhs)
  {
    iter = rhs.iter;
    owner = rhs.owner;
    return *this;
  }
  inline uint64_t start() { return iter->first; }
  inline uint64_t end()
  {
    typename std::map<uint64_t, T>::iterator next = iter;
    next++;
    if(next == owner->end())
    {
      return UINT64_MAX;
    }
    return next->first;
  }
  inline T &value() { return iter->second; }
};

template <typename T>
struct Intervals
{
private:
  std::map<uint64_t, T> StartPoints;

public:
private:
  IntervalsIter<T> Wrap(typename std::map<uint64_t, T>::iterator iter)
  {
    return IntervalsIter<T>(&StartPoints, iter);
  }

public:
  Intervals() : StartPoints{{0, T()}} {}
  IntervalsIter<T> begin() { return Wrap(StartPoints.begin()); }
  IntervalsIter<T> end() { return Wrap(StartPoints.end()); }
  // finds the interval containing `x`.
  IntervalsIter<T> find(uint64_t x)
  {
    // Find the first interval which starts AFTER `x`
    typename std::map<uint64_t, T>::iterator it = StartPoints.upper_bound(x);
    // Because the first interval always starts at 0, the found interval cannot be the first
    // interval
    RDCASSERT(it != StartPoints.begin());
    // Move back 1 interval, to find the last interval that starts no later than `range.start`.
    // This is the interval that contains the point
    it--;
    return Wrap(it);
  }

  // IntervalsUpdate adds the interval [start, end) to the family of intervals.
  // Overlapping intervals' values are updated according to the `trans` function.
  void Update(uint64_t start, uint64_t end, std::function<T(T)> trans)
  {
    if(start >= end)
    {
      return;
    }
    IntervalsIter<T> start_it = find(start);
    // Loop over the intervals that overlap the [start, end).
    while(start < end)
    {
      T oldValue = start_it.value();
      T newValue = trans(oldValue);
      // Check if the value of this overlapping interval actually changes; otherwise we don't do
      // anything.
      if(newValue != oldValue)
      {
        // Add the new start point
        start_it = Wrap(StartPoints.insert(std::pair<uint64_t, T>(start, newValue)).first);
        start_it.value() = newValue;
        if(end < start_it.end())
        {
          // if the end point is also contained in this overlapping interval,
          // then we need to put in a new point; after this point, the value goes back to this
          // interval's original value.
          StartPoints.insert(std::pair<uint64_t, T>(end, oldValue));
        }

        // Check if the new interval's value is equal to the previous interval, and merge if
        // necessary.
        if(start_it != this->begin())
        {
          IntervalsIter<T> prev_it = start_it;
          prev_it--;
          if(prev_it.value() == start_it.value())
          {
            StartPoints.erase(start_it.unwrap());
            start_it = prev_it;
          }
        }

        // Check if the new interval's value is equal to the next interval, and merge if necessary.
        IntervalsIter<T> next_it = start_it;
        next_it++;
        if(next_it != this->end())
        {
          if(next_it.value() == start_it.value())
          {
            StartPoints.erase(next_it.unwrap());
          }
        }
      }
      // Move on to the next overlapping interval.
      start_it++;
      if(start_it == this->end())
      {
        return;
      }
      // update `start`, so that [start, end) only overlaps the intervals that have not yet been
      // updated.
      start = start_it.start();
    }
  }
};

}    // namespace vk_cpp_codec