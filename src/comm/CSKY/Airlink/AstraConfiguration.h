#ifndef ASTRA_H
#define ASTRA_H

#include "AirlinkConfiguration.h"

namespace CSKY {

class AstraConfiguration : public AirlinkConfiguration
{
    Q_OBJECT
public:
    AstraConfiguration(const QString& name);
    AstraConfiguration(AstraConfiguration* source);
#ifdef QGC_AIRLINK_ENABLED
    LinkType    type                 (void) override { return LinkConfiguration::TypeAstra; }
    QString     hostName             (void) override { return "astra.csky.space"; }
    QString     modemType            (void) override {return "Astra";}
    //QString     modemType
#endif
};

}

#endif // AIRLINKLINK_H
