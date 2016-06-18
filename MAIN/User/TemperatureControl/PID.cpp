/**
  ******************************************************************************
  * @file    PID.cpp
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    24-April-2015
  * @brief
  *
  ******************************************************************************
  */
#include "PID.h"

#define ERR_ACC_MAX 256
#define ERR_ACC_MIN -ERR_ACC_MAX

// scale the output term to account for our fixed-point bounds
#define OUTPUT_SCALE 2

#define abs(x) ((x)>0?(x):-(x))

PID::PID()
{
    reset();
}

void PID::reset()
{
	sp = 0;
	p_gain = i_gain = d_gain = 0;

	reset_state();
}

void PID::reset_state()
{
	error_acc = 0;
	prev_error = 0;
	delta_idx = 0;

// Assume that DELTA_SAMPLES is 4 -- saves 24 bytes to eliminate this loop
	for (delta_idx = 0; delta_idx < DELTA_SAMPLES; delta_idx++)
		delta_history[delta_idx] = 0;

	delta_idx = 0;
	delta_summation = 0;

	last_output = 0;
}

// We're modifying the way we compute delta by averaging the deltas over a
// series of samples.  This helps us get a reasonable delta despite the discrete
// nature of the samples; on average we will get a delta of maybe 1/deg/second,
// which will give us a delta impulse for that one calculation round and then
// the D term will immediately disappear.  By averaging the last N deltas, we
// allow changes to be registered rather than get subsumed in the sampling noise.
    #define K1 0.99
    #define K2 (1.0-K1)
int PID::calculate(const float pv)
{
	float e = sp - pv;
	error_acc += e;
	// Clamp the error accumulator at accepted values.
	// This will help control overcorrection for accumulated error during the run-up
	// and allow the I term to be integrated away more quickly as we approach the
	// setpoint.
	if (error_acc > ERR_ACC_MAX)
		error_acc = ERR_ACC_MAX;
	else if (error_acc < ERR_ACC_MIN)
		error_acc = ERR_ACC_MIN;
	float p_term = (float)e * p_gain;
	float i_term = (float)error_acc * i_gain;
	float delta = e - prev_error;

//    float d_term=(d_gain*(pv - temp_dState))*K2+(K1*lastd_term);
//    last_output = ((int)(p_term + i_term - d_term))*OUTPUT_SCALE;
//    lastd_term = d_term;
//    temp_dState = pv;
	// Add to delta history
	delta_summation -= delta_history[delta_idx];
	delta_history[delta_idx] = delta;
	delta_summation += (float)delta;
	delta_idx = (delta_idx+1) % DELTA_SAMPLES;

	// Use the delta over the whole window
	float d_term = delta_summation * d_gain;

	prev_error = e;

	last_output = ((int)(p_term + i_term + d_term))*OUTPUT_SCALE;

	return last_output;
}

void PID::setTarget(const int target)
{
    if (abs(sp - target) > 10)
        reset_state();
    sp = target;
}
