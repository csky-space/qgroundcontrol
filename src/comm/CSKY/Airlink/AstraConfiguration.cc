#include "AstraConfiguration.h"

namespace CSKY {

AstraConfiguration::AstraConfiguration(const QString& name) : AirlinkConfiguration(name){

}

AstraConfiguration::AstraConfiguration(AstraConfiguration* source) : AirlinkConfiguration(source){

}

}
