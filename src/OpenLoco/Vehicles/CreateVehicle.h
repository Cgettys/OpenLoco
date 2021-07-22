#pragma once

#include "../Date.h"
namespace OpenLoco::Vehicles
{
    static inline int32_t calculateInitialBogieReliability(const VehicleObject& vehObject)
    {
        int32_t reliability = vehObject.reliability * 256;
        if (getCurrentYear() + 2 > vehObject.designed)
        {
            // Reduce reliability by an eighth after 2 years past design
            reliability -= reliability / 8;
            if (getCurrentYear() + 3 > vehObject.designed)
            {
                // Reduce reliability by a further eighth (quarter total) after 3 years past design
                reliability -= reliability / 8;
            }
        }
        if (reliability != 0)
        {
            reliability += 255;
        }
        return reliability;
    }
}