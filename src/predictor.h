#ifndef PREDICTOR_H_
#define PREDICTOR_H_

#include "motion.h"

extern motion PredictML(
    float rock_score,
    float paper_score,
    float rest_score);

motion PredictThreshold(
    float extensor_value,
    float flexor_value,
    float rock_flexor_lower_limit,
    float rock_extensor_upper_limit,
    float paper_extensor_lower_limit,
    float paper_flexor_upper_limit);

#endif