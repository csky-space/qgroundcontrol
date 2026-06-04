#include "RCMappingSourceType.h"

QString toString(RCMappingSourceType sourceType) {
    switch(sourceType) {
        case RCMappingSourceType::JoystickAxis: return "JoystickAxis";
        case RCMappingSourceType::JoystickButton: return "JoystickButton";
        default: return "InvalidSourceType";
    }
}
