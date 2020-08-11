#pragma once

//System includes
#include <vector>

template<typename T>
class SampleBuffer {
private:
    std::vector<T> samples;
    size_t frontPosition;
    size_t backPosition;
    size_t capacity;
    size_t currentSize;
public:
    
    SampleBuffer(size_t capacity) {
        
        samples.resize(capacity);
        
        this->capacity = capacity;
        
        frontPosition = 0;
        backPosition = 0;
        currentSize = 0;
    }
    
    T pop() {
        
        if (currentSize == 0) {
            return 0;
        }
        
        frontPosition++;
        frontPosition = frontPosition % capacity;
        
        currentSize--;
        
        T sample = samples[frontPosition];
        
        samples[frontPosition] = 0;
        
        return sample;
    }
    
    
    void push(T sample) {
        
        samples[backPosition] = sample;
        
        backPosition++;
        
        backPosition = backPosition % capacity;
        
        currentSize++;
    }
    
    size_t size() {
        return currentSize;
    }
};
