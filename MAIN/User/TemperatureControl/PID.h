/**
  ******************************************************************************
  * @file    PID.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    24-April-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef PID_H
#define PID_H

#include <stdint.h>

/// Number of delta samples to
#define DELTA_SAMPLES 4 // PID::reset_state() assumes 4.

/// The PID controller module implements a simple PID controller.
class PID
{
private:
    float p_gain; ///< proportional gain
    float i_gain; ///< integral gain
    float d_gain; ///< derivative gain

    /// Data for approximating d (smoothing to handle discrete nature of sampling).
    /// See PID.cc for a description of why we do this.
    float delta_history[DELTA_SAMPLES];
    float delta_summation;      ///< ?
    uint8_t delta_idx;          ///< Current index in the delta history buffer
    float prev_error;             ///< Previous input for calculating next delta
    float error_acc;              ///< Accumulated error, for calculating integral

    int sp;                     ///< Process set point
    int last_output;            ///< Last output of the PID controller

    float temp_dState;
    float lastd_term;

public:
    /// Initialize the PID module
    PID();

    /// Set the P term of the PID controller
    /// \param[in] p_gain_in New proportional gain term
    void setPGain(const float p_gain_in) { p_gain = p_gain_in; }

    /// Set the I term of the PID controller
    /// \param[in] i_gain_in New integration gain term
    void setIGain(const float i_gain_in) { i_gain = i_gain_in; }

    /// Set the D term of the PID controller
    /// \param[in] d_gain_in New derivative gain term
    void setDGain(const float d_gain_in) { d_gain = d_gain_in; }

    /// Set the setpoint of the PID controller
    /// \param[in] target New PID controller target
    void setTarget(const int target);

    /// Get the current PID target
    /// \return Current setpoint
    int getTarget() const { return sp; }

    /// Reset the PID to board-on values
    void reset();

    /// Reset only the PID control loop variables
    void reset_state();

    /// Calculate the next cycle of the PID loop.
    /// \param[in] pv Process value (measured value from the sensor)
    /// \return output value (used to control the output)
    int calculate(const float pv);

    /// Get the current value of the error term
    /// \return Error term
    int getErrorTerm() { return (int)error_acc; }

    /// Get the last process output value
    /// \return Last process output value
    int getLastOutput() { return (int)last_output; }

    /// Get the current value of the delta term
    /// \return Delta term
    int getDeltaTerm() { return (int)delta_summation; }
};



#endif /* PID_H */
