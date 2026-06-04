#include "RCRangeMapper.h"

#include <cmath>

RCRangeMapper::RCRangeMapper(float inMin, float inMax, float outMin, float outMax) 
    : _inMin(inMin)
    , _inMax(inMax)
    , _inRange(inMax - inMin)
    , _outMin(outMin)
    , _outMax(outMax)
    , _outRange(outMax - outMin) {}

float RCRangeMapper::inMin() const {
    return _inMin;
}

float RCRangeMapper::inMax() const {
    return _inMax;
}

float RCRangeMapper::inRange() const {
    return _inRange;
}

float RCRangeMapper::outMin() const {
    return _outMin;
}  

float RCRangeMapper::outMax() const {
    return _outMax;
}

float RCRangeMapper::outRange() const {
    return _outRange;
}

float RCRangeMapper::rawValue() const {
    return _rawValue;
}

float RCRangeMapper::normalizedValue() const {
    return _normalizedValue;
}

float RCRangeMapper::mappedValue() const {
    return _mappedValue;
}

bool RCRangeMapper::IsClamping() const {
    return _IsClamping;
}

bool RCRangeMapper::isReversed() const {
    return _isReversed;
}

bool RCRangeMapper::isValueMapped() const {
    return _isValueMapped;
}

void RCRangeMapper::setInMin(float value) {
    invalidate();

    _inMin = value;
    _inRange = _inMax - _inMin;

    emit inMinChanged();
    emit inRangeChanged();
}

void RCRangeMapper::setInMax(float value) {
    invalidate();

    _inMax = value;
    _inRange = _inMax - _inMin;

    emit inMaxChanged();
    emit inRangeChanged();
}

void RCRangeMapper::setOutMin(float value) {
    invalidate();

    _outMin = value;
    _outRange = _outMax - _outMin;

    emit outMinChanged();
    emit outRangeChanged();
}

void RCRangeMapper::setOutMax(float value) {
    invalidate();

    _outMax = value;
    _outRange = _outMax - _outMin;

    emit outMaxChanged();
    emit outRangeChanged();
}

void RCRangeMapper::setRanges(float inMin, float inMax, float outMin, float outMax) {
    invalidate();

    _inMin = inMin;
    _inMax = inMax;
    _inRange = _inMax - _inMin;

    _outMin = outMin;
    _outMax = outMax;
    _outRange = _outMax - _outMin;

    emit inMinChanged();
    emit inMaxChanged();
    emit inRangeChanged();
    emit outMinChanged();
    emit outMaxChanged();
    emit outRangeChanged();
}

void RCRangeMapper::setInRange(float min, float max) {
    invalidate();

    _inMin = min;
    _inMax = max;
    _inRange = _inMax - _inMin;

    emit inMinChanged();
    emit inMaxChanged();
    emit inRangeChanged();
}

void RCRangeMapper::setOutRange(float min, float max) {
    invalidate();

    _outMin = min;
    _outMax = max;
    _outRange = _outMax - _outMin;
    
    emit outMinChanged();
    emit outMaxChanged();
    emit outRangeChanged();
}

void RCRangeMapper::setIsClamping(bool state) {
    if (_IsClamping == state) {
        return;
    }

    invalidate();

    bool shouldEmit = _IsClamping != state;
    _IsClamping = state;

    if (shouldEmit) {
        emit IsClampingChanged();
    }
}

void RCRangeMapper::setIsReversed(bool state) {
    if (_isReversed == state) {
        return;
    }

    invalidate();

    bool shouldEmit = _isReversed != state;
    _isReversed = state;

    if (shouldEmit) {
        emit isReversedChanged();
    }
}

void RCRangeMapper::invalidate() {
    if (_isValueMapped) {
        _isValueMapped = false;
        emit isValueMappedChanged();
    }
}

void RCRangeMapper::resetValue() {
    invalidate();

    _rawValue = _inMin;
    if (_isReversed) {
        _normalizedValue = 1.0f;
        _mappedValue = _outMax;
    }
    else {
        _normalizedValue = 0.0f;
        _mappedValue = _outMin;
    }

    emit rawValueChanged();
    emit normalizedValueChanged();
    emit mappedValueChanged();
}

void RCRangeMapper::remapValue() {
    float normalized = (_rawValue - _inMin) / _inRange;
    if (!std::isfinite(normalized)) {
        invalidate();
        return;
    }

    if (_isReversed) {
        normalized = 1.0f - normalized;
    }

    float mapped = _outMin + normalized * _outRange;
    if (_IsClamping) {
        mapped = std::min(_outMax, std::max(_outMin, mapped));
    }

    if (!std::isfinite(mapped)) {
        invalidate();
        return;
    }

    normalized = std::min(1.0f, std::max(0.0f, normalized));

    _normalizedValue = normalized;
    _mappedValue = mapped;

    emit normalizedValueChanged();
    emit mappedValueChanged();

    if (!_isValueMapped) {
        _isValueMapped = true;
        emit isValueMappedChanged();
    }
}

void RCRangeMapper::setRawValue(float value) {
    _rawValue = value;

    emit rawValueChanged();

    remapValue();
}

bool RCRangeMapper::isInInputRange(float value) {
    return (value >= _inMin) && (value <= _inMax);
}

bool RCRangeMapper::isInOutputRange  (float value) {
    return (value >= _outMin) && (value <= _outMax);
}

void RCRangeMapper::expandInputToFit (float value) {
    if (isInInputRange(value)) {
        return;
    }
    setInRange(std::min(value, _inMin), std::max(value, _inMax));
}
