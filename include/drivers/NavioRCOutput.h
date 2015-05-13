#ifndef _NAVIORCOUTPUT_H
#define _NAVIORCOUTPUT_H

#include <stdint.h>
#include <sys/ioctl.h>

#include <drivers/common.h>

#define PWM_OUTPUT_MAX_CHANNELS	16

/**
 * Lowest minimum PWM in us
 */
#define PWM_LOWEST_MIN 900

/**
 * Default minimum PWM in us
 */
#define PWM_DEFAULT_MIN 1000 
/**
 * Highest PWM allowed as the minimum PWM
 */
#define PWM_HIGHEST_MIN 1600

/**
 * Highest maximum PWM in us
 */
#define PWM_HIGHEST_MAX 2100

/**
 * Default maximum PWM in us
 */
#define PWM_DEFAULT_MAX 2000

/**
 * Lowest PWM allowed as the maximum PWM
 */
#define PWM_LOWEST_MAX 1400

/**
 * Do not output a channel with this value
 */
#define PWM_IGNORE_THIS_CHANNEL UINT16_MAX

/**
 * Servo output signal type, value is actual servo output pulse
 * width in microseconds.
 */
typedef uint16_t	servo_position_t;

/**
 * Servo output status structure.
 *
 * May be published to output_pwm, or written to a PWM output
 * device.
 */
struct pwm_output_values {
	/** desired pulse widths for each of the supported channels */
	servo_position_t	values[PWM_OUTPUT_MAX_CHANNELS];
	unsigned			channel_count;
};


/**
 * RC config values for a channel
 *
 * This allows for PX4IO_PAGE_RC_CONFIG values to be set without a
 * param_get() dependency
 */
struct pwm_output_rc_config {
	uint8_t channel;
	uint16_t rc_min;
	uint16_t rc_trim;
	uint16_t rc_max;
	uint16_t rc_dz;
	uint16_t rc_assignment;
	bool     rc_reverse;
};

/*
 * ioctl() definitions
 *
 * Note that ioctls and ORB updates should not be mixed, as the
 * behaviour of the system in this case is not defined.
 */
#define _PWM_SERVO_BASE		0x2a00

/** arm all servo outputs handle by this driver */
#define PWM_SERVO_ARM		_NAVIO_IOC(_PWM_SERVO_BASE, 0)

/** disarm all servo outputs (stop generating pulses) */
#define PWM_SERVO_DISARM	_NAVIO_IOC(_PWM_SERVO_BASE, 1)

/** get default servo update rate */
#define PWM_SERVO_GET_DEFAULT_UPDATE_RATE _NAVIO_IOC(_PWM_SERVO_BASE, 2)

/** set alternate servo update rate */
#define PWM_SERVO_SET_UPDATE_RATE _NAVIO_IOC(_PWM_SERVO_BASE, 3)

/** get alternate servo update rate */
#define PWM_SERVO_GET_UPDATE_RATE _NAVIO_IOC(_PWM_SERVO_BASE, 4)

/** get the number of servos in *(unsigned *)arg */
#define PWM_SERVO_GET_COUNT	_NAVIO_IOC(_PWM_SERVO_BASE, 5)

/** selects servo update rates, one bit per servo. 0 = default (50Hz), 1 = alternate */
#define PWM_SERVO_SET_SELECT_UPDATE_RATE _NAVIO_IOC(_PWM_SERVO_BASE, 6)

/** check the selected update rates */
#define PWM_SERVO_GET_SELECT_UPDATE_RATE _NAVIO_IOC(_PWM_SERVO_BASE, 7)

/** set the 'ARM ok' bit, which activates the safety switch */
#define PWM_SERVO_SET_ARM_OK	_NAVIO_IOC(_PWM_SERVO_BASE, 8)

/** clear the 'ARM ok' bit, which deactivates the safety switch */
#define PWM_SERVO_CLEAR_ARM_OK	_NAVIO_IOC(_PWM_SERVO_BASE, 9)

/** start DSM bind */
#define DSM_BIND_START	_NAVIO_IOC(_PWM_SERVO_BASE, 10)

#define DSM2_BIND_PULSES 3	/* DSM_BIND_START ioctl parameter, pulses required to start dsm2 pairing */
#define DSMX_BIND_PULSES 7	/* DSM_BIND_START ioctl parameter, pulses required to start dsmx pairing */
#define DSMX8_BIND_PULSES 9 	/* DSM_BIND_START ioctl parameter, pulses required to start 8 or more channel dsmx pairing */

/** power up DSM receiver */
#define DSM_BIND_POWER_UP _NAVIO_IOC(_PWM_SERVO_BASE, 11)

/** set the PWM value for failsafe */
#define PWM_SERVO_SET_FAILSAFE_PWM	_NAVIO_IOC(_PWM_SERVO_BASE, 12)

/** get the PWM value for failsafe */
#define PWM_SERVO_GET_FAILSAFE_PWM	_NAVIO_IOC(_PWM_SERVO_BASE, 13)

/** set the PWM value when disarmed - should be no PWM (zero) by default */
#define PWM_SERVO_SET_DISARMED_PWM	_NAVIO_IOC(_PWM_SERVO_BASE, 14)

/** get the PWM value when disarmed */
#define PWM_SERVO_GET_DISARMED_PWM	_NAVIO_IOC(_PWM_SERVO_BASE, 15)

/** set the minimum PWM value the output will send */
#define PWM_SERVO_SET_MIN_PWM	_NAVIO_IOC(_PWM_SERVO_BASE, 16)

/** get the minimum PWM value the output will send */
#define PWM_SERVO_GET_MIN_PWM	_NAVIO_IOC(_PWM_SERVO_BASE, 17)

/** set the maximum PWM value the output will send */
#define PWM_SERVO_SET_MAX_PWM	_NAVIO_IOC(_PWM_SERVO_BASE, 18)

/** get the maximum PWM value the output will send */
#define PWM_SERVO_GET_MAX_PWM	_NAVIO_IOC(_PWM_SERVO_BASE, 19)

/** set the number of servos in (unsigned)arg - allows change of
 * split between servos and GPIO */
#define PWM_SERVO_SET_COUNT	_NAVIO_IOC(_PWM_SERVO_BASE, 20)

/** set the lockdown override flag to enable outputs in HIL */
#define PWM_SERVO_SET_DISABLE_LOCKDOWN		_NAVIO_IOC(_PWM_SERVO_BASE, 21)

/** get the lockdown override flag to enable outputs in HIL */
#define PWM_SERVO_GET_DISABLE_LOCKDOWN		_NAVIO_IOC(_PWM_SERVO_BASE, 22)

/** force safety switch off (to disable use of safety switch) */
#define PWM_SERVO_SET_FORCE_SAFETY_OFF		_NAVIO_IOC(_PWM_SERVO_BASE, 23)

/** force failsafe mode (failsafe values are set immediately even if failsafe condition not met) */
#define PWM_SERVO_SET_FORCE_FAILSAFE		_NAVIO_IOC(_PWM_SERVO_BASE, 24)

/** make failsafe non-recoverable (termination) if it occurs */
#define PWM_SERVO_SET_TERMINATION_FAILSAFE	_NAVIO_IOC(_PWM_SERVO_BASE, 25)

/** force safety switch on (to enable use of safety switch) */
#define PWM_SERVO_SET_FORCE_SAFETY_ON		_NAVIO_IOC(_PWM_SERVO_BASE, 26)

/** set RC config for a channel. This takes a pointer to pwm_output_rc_config */
#define PWM_SERVO_SET_RC_CONFIG			_NAVIO_IOC(_PWM_SERVO_BASE, 27)

/** set the 'OVERRIDE OK' bit, which allows for RC control on FMU loss */
#define PWM_SERVO_SET_OVERRIDE_OK		_NAVIO_IOC(_PWM_SERVO_BASE, 28)

/** clear the 'OVERRIDE OK' bit, which allows for RC control on FMU loss */
#define PWM_SERVO_CLEAR_OVERRIDE_OK		_NAVIO_IOC(_PWM_SERVO_BASE, 29)

/** setup OVERRIDE_IMMEDIATE behaviour on FMU fail */
#define PWM_SERVO_SET_OVERRIDE_IMMEDIATE	_NAVIO_IOC(_PWM_SERVO_BASE, 30)

/*
 *
 *
 * WARNING WARNING WARNING! DO NOT EXCEED 31 IN IOC INDICES HERE!
 *
 *
 */

/** set a single servo to a specific value */
#define PWM_SERVO_SET(_servo)	_NAVIO_IOC(_PWM_SERVO_BASE, 0x20 + _servo)

/** get a single specific servo value */
#define PWM_SERVO_GET(_servo)	_NAVIO_IOC(_PWM_SERVO_BASE, 0x40 + _servo)

/** get the _n'th rate group's channels; *(uint32_t *)arg returns a bitmap of channels
 *  whose update rates must be the same.
 */
#define PWM_SERVO_GET_RATEGROUP(_n) _NAVIO_IOC(_PWM_SERVO_BASE, 0x60 + _n)

#endif 
