/// @file RCMappingSourceType.h

#pragma once

#include <cstdint>

#include <QString>

enum class RCMappingSourceType : int32_t {
    JoystickAxis = 0,
    JoystickButton,
    COUNT
};

QString toString(RCMappingSourceType sourceType);
