#include <errno.h>
#include <cstdio>
#include <unistd.h>
#include <err.h>

#include <drivers/NavioRCIO.h>
#include <drivers/NavioRCInput.h>
#include <drivers/common.h>

#define debug(fmt, args ...) fprintf(stderr, "[RCIO]: " fmt "\n", ##args)
#define log(fmt, args ...) debug(fmt, ##args)

NavioRCIO::NavioRCIO(NavioRCIO_serial *interface):
    _interface(interface),
    _hardware(0),
    _max_actuators(0),
    _max_controls(0),
    _max_rc_input(0),
    _max_relays(0),
    _max_transfer(16),
    _battery_amp_per_volt(90.0f / 5.0f), // this matches the 3DR current sensor
    _battery_amp_bias(0),
    _battery_mamphour_total(0),
    _battery_last_timestamp(0),
    _initialized(false)
{
}

bool NavioRCIO::init()
{
    if (_initialized) {
        return true;
    }

    unsigned protocol;
    uint32_t start_try_time = micros64();

    do {
        usleep(2000);
        protocol = io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_PROTOCOL_VERSION);
    } while (protocol == _io_reg_get_error && (get_elapsed_time(&start_try_time) < 700U * 1000U));

    /* if the error still persists after timing out, we give up */
    if (protocol == _io_reg_get_error) {
        log("Failed to communicate with IO, abort.");
        return false;
    }

    if (protocol != PX4IO_PROTOCOL_VERSION) {
        log("IO protocol/firmware mismatch, abort.");
        return false;
    }

    _hardware      = io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_HARDWARE_VERSION);
    _max_actuators = io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_ACTUATOR_COUNT);
    _max_controls  = io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_CONTROL_COUNT);
    _max_relays    = io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_RELAY_COUNT);
    _max_transfer  = io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_MAX_TRANSFER) - 2;
    _max_rc_input  = io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_RC_INPUT_COUNT);

    if ((_max_actuators < 1) || (_max_actuators > 255) ||
        (_max_relays > 32)   ||
        (_max_transfer < 16) || (_max_transfer > 255)  ||
        (_max_rc_input < 1)  || (_max_rc_input > 255)) {

        log("config read error");
        log("[IO] config read fail, abort.");
        return false;
    }

    if (_max_rc_input > RC_INPUT_MAX_CHANNELS)
        _max_rc_input = RC_INPUT_MAX_CHANNELS;

    _initialized = true;

#if 0
    log("_hardware: %d", _hardware);
    log("_max_actuators: %d", _max_actuators);
    log("_max_controls: %d", _max_controls);
    log("_max_relays: %d", _max_relays);
    log("_max_transfer: %d", _max_transfer);
    log("_max_rc_input: %d", _max_rc_input);
    
    while(true)
        ;
#endif

    return true;
}

NavioRCIO::~NavioRCIO()
{
    if (_interface != nullptr) {
        delete _interface;
    }
}

void NavioRCIO::poll()
{
    static const size_t ALPHABET_SIZE = 50;

    static int i = 0;

    char *buffer = new char[i];

    for (int j = 0; j < i; j++) {
        buffer[j] = 'a' + j;
    }

    _interface->write(0x5a, buffer, i);

    i = (i + 1) % ALPHABET_SIZE;

    delete [] buffer;
}

bool NavioRCIO::detect()
{
     unsigned protocol = io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_PROTOCOL_VERSION);

     if (protocol != PX4IO_PROTOCOL_VERSION) {
         if (protocol == _io_reg_get_error) {
             log("IO not installed\n");
         } else {
             log("IO version error\n");
         }
         return false;
     }
     return true;
}


int NavioRCIO::io_reg_get(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values)
{
    /* range check the transfer */
    if (num_values > ((_max_transfer) / sizeof(*values))) {
        debug("io_reg_get: too many registers (%u, max %u)", num_values, _max_transfer / 2);
        return -EINVAL;
    }
    int ret = _interface->read((page << 8) | offset, reinterpret_cast<void *>(values), num_values);
    if (ret != (int)num_values) {
        debug("io_reg_get(%u,%u,%u): data error %d", page, offset, num_values, ret);
        return -1;
    }
    return OK;

}

uint32_t NavioRCIO::io_reg_get(uint8_t page, uint8_t offset)
{
    uint16_t value;
    if (io_reg_get(page, offset, &value, 1) != OK)
        return _io_reg_get_error;
    return value;
}


int NavioRCIO::io_reg_set(uint8_t page, uint8_t offset, const uint16_t *values, unsigned num_values)
{
    /* range check the transfer */
    if (num_values > ((_max_transfer) / sizeof(*values))) {
        debug("io_reg_set: too many registers (%u, max %u)", num_values, _max_transfer / 2);
        return -EINVAL;
    }

    int ret =  _interface->write((page << 8) | offset, (void *)values, num_values);

    if (ret != (int)num_values) {
        debug("io_reg_set(%u,%u,%u): error %d", page, offset, num_values, ret);
        return -1;
    }

    return OK;
}

int NavioRCIO::io_reg_set(uint8_t page, uint8_t offset, uint16_t value)
{
    return io_reg_set(page, offset, &value, 1);
}

int NavioRCIO::io_reg_modify(uint8_t page, uint8_t offset, uint16_t clearbits, uint16_t setbits)
{
    int ret;
    uint16_t value;

    ret = io_reg_get(page, offset, &value, 1);

    if (ret != OK)
        return ret;

    value &= ~clearbits;
    value |= setbits;

    return io_reg_set(page, offset, value);
}

void NavioRCIO::print_status(bool extended_status)
{
    /* basic configuration */
    printf("protocol %u hardware %u bootloader %u buffer %uB crc 0x%04x%04x\n",
           io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_PROTOCOL_VERSION),
           io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_HARDWARE_VERSION),
           io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_BOOTLOADER_VERSION),
           io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_MAX_TRANSFER),
           io_reg_get(PX4IO_PAGE_SETUP,  PX4IO_P_SETUP_CRC),
           io_reg_get(PX4IO_PAGE_SETUP,  PX4IO_P_SETUP_CRC+1));
    printf("%u controls %u actuators %u R/C inputs %u analog inputs %u relays\n",
           io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_CONTROL_COUNT),
           io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_ACTUATOR_COUNT),
           io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_RC_INPUT_COUNT),
           io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_ADC_INPUT_COUNT),
           io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_RELAY_COUNT));

    /* status */
    printf("%u bytes free\n",
           io_reg_get(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_FREEMEM));
    uint16_t flags = io_reg_get(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_FLAGS);
    uint16_t io_status_flags = flags;
    printf("status 0x%04x%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
           flags,
           ((flags & PX4IO_P_STATUS_FLAGS_OUTPUTS_ARMED) ? " OUTPUTS_ARMED" : ""),
           ((flags & PX4IO_P_STATUS_FLAGS_SAFETY_OFF) ? " SAFETY_OFF" : " SAFETY_SAFE"),
           ((flags & PX4IO_P_STATUS_FLAGS_OVERRIDE) ? " OVERRIDE" : ""),
           ((flags & PX4IO_P_STATUS_FLAGS_RC_OK)    ? " RC_OK" : " RC_FAIL"),
           ((flags & PX4IO_P_STATUS_FLAGS_RC_PPM)   ? " PPM" : ""),
           ((flags & PX4IO_P_STATUS_FLAGS_RC_DSM)   ? " DSM" : ""),
           ((flags & PX4IO_P_STATUS_FLAGS_RC_ST24)   ? " ST24" : ""),
           ((flags & PX4IO_P_STATUS_FLAGS_RC_SBUS)  ? " SBUS" : ""),
           ((flags & PX4IO_P_STATUS_FLAGS_FMU_OK)   ? " FMU_OK" : " FMU_FAIL"),
           ((flags & PX4IO_P_STATUS_FLAGS_RAW_PWM)  ? " RAW_PWM_PASSTHROUGH" : ""),
           ((flags & PX4IO_P_STATUS_FLAGS_MIXER_OK) ? " MIXER_OK" : " MIXER_FAIL"),
           ((flags & PX4IO_P_STATUS_FLAGS_ARM_SYNC) ? " ARM_SYNC" : " ARM_NO_SYNC"),
           ((flags & PX4IO_P_STATUS_FLAGS_INIT_OK)  ? " INIT_OK" : " INIT_FAIL"),
           ((flags & PX4IO_P_STATUS_FLAGS_FAILSAFE)  ? " FAILSAFE" : ""));
    uint16_t alarms = io_reg_get(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_ALARMS);
    printf("alarms 0x%04x%s%s%s%s%s%s%s%s\n",
           alarms,
           ((alarms & PX4IO_P_STATUS_ALARMS_VBATT_LOW)     ? " VBATT_LOW" : ""),
           ((alarms & PX4IO_P_STATUS_ALARMS_TEMPERATURE)   ? " TEMPERATURE" : ""),
           ((alarms & PX4IO_P_STATUS_ALARMS_SERVO_CURRENT) ? " SERVO_CURRENT" : ""),
           ((alarms & PX4IO_P_STATUS_ALARMS_ACC_CURRENT)   ? " ACC_CURRENT" : ""),
           ((alarms & PX4IO_P_STATUS_ALARMS_FMU_LOST)      ? " FMU_LOST" : ""),
           ((alarms & PX4IO_P_STATUS_ALARMS_RC_LOST)       ? " RC_LOST" : ""),
           ((alarms & PX4IO_P_STATUS_ALARMS_PWM_ERROR)     ? " PWM_ERROR" : ""),
           ((alarms & PX4IO_P_STATUS_ALARMS_VSERVO_FAULT)  ? " VSERVO_FAULT" : ""));
    /* now clear alarms */
    io_reg_set(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_ALARMS, 0xFFFF);

    if (_hardware == 1) {
        printf("vbatt mV %u ibatt mV %u vbatt scale %u\n",
               io_reg_get(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_VBATT),
               io_reg_get(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_IBATT),
               io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_VBATT_SCALE));
        printf("amp_per_volt %.3f amp_offset %.3f mAh discharged %.3f\n",
               (double)_battery_amp_per_volt,
               (double)_battery_amp_bias,
               (double)_battery_mamphour_total);

    } else if (_hardware == 2) {
        printf("vservo %u mV vservo scale %u\n",
               io_reg_get(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_VSERVO),
               io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_VSERVO_SCALE));
        printf("vrssi %u\n", io_reg_get(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_VRSSI));
    }

    printf("actuators (including S.BUS)");

    for (unsigned i = 0; i < _max_actuators; i++)
        printf(" %hi", int16_t(io_reg_get(PX4IO_PAGE_ACTUATORS, i)));

    printf("\n");
    printf("hardware servo ports");

    for (unsigned i = 0; i < _max_actuators; i++)
        printf(" %u", io_reg_get(PX4IO_PAGE_SERVOS, i));

    printf("\n");
    uint16_t raw_inputs = io_reg_get(PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_COUNT);
    printf("%d raw R/C inputs", raw_inputs);

    for (unsigned i = 0; i < raw_inputs; i++)
        printf(" %u", io_reg_get(PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_BASE + i));

    printf("\n");

    flags = io_reg_get(PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_FLAGS);
    printf("R/C flags: 0x%04x%s%s%s%s%s\n", flags,
        (((io_status_flags & PX4IO_P_STATUS_FLAGS_RC_DSM) && (!(flags & PX4IO_P_RAW_RC_FLAGS_RC_DSM11))) ? " DSM10" : ""),
        (((io_status_flags & PX4IO_P_STATUS_FLAGS_RC_DSM) && (flags & PX4IO_P_RAW_RC_FLAGS_RC_DSM11)) ? " DSM11" : ""),
        ((flags & PX4IO_P_RAW_RC_FLAGS_FRAME_DROP) ? " FRAME_DROP" : ""),
        ((flags & PX4IO_P_RAW_RC_FLAGS_FAILSAFE) ? " FAILSAFE" : ""),
        ((flags & PX4IO_P_RAW_RC_FLAGS_MAPPING_OK) ? " MAPPING_OK" : "")
           );

    if ((io_status_flags & PX4IO_P_STATUS_FLAGS_RC_PPM)) {
        int frame_len = io_reg_get(PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_DATA);
        printf("RC data (PPM frame len) %u us\n", frame_len);

        if ((frame_len - raw_inputs * 2000 - 3000) < 0) {
            printf("WARNING  WARNING  WARNING! This RC receiver does not allow safe frame detection.\n");
        }
    }

    uint16_t mapped_inputs = io_reg_get(PX4IO_PAGE_RC_INPUT, PX4IO_P_RC_VALID);
    printf("mapped R/C inputs 0x%04x", mapped_inputs);

    for (unsigned i = 0; i < _max_rc_input; i++) {
        if (mapped_inputs & (1 << i))
            printf(" %u:%d", i, REG_TO_SIGNED(io_reg_get(PX4IO_PAGE_RC_INPUT, PX4IO_P_RC_BASE + i)));
    }

    printf("\n");
    uint16_t adc_inputs = io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_ADC_INPUT_COUNT);
    printf("ADC inputs");

    for (unsigned i = 0; i < adc_inputs; i++)
        printf(" %u", io_reg_get(PX4IO_PAGE_RAW_ADC_INPUT, i));

    printf("\n");

    /* setup and state */
    uint16_t features = io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_FEATURES);
    printf("features 0x%04x%s%s%s%s\n", features,
        ((features & PX4IO_P_SETUP_FEATURES_SBUS1_OUT) ? " S.BUS1_OUT" : ""),
        ((features & PX4IO_P_SETUP_FEATURES_SBUS2_OUT) ? " S.BUS2_OUT" : ""),
        ((features & PX4IO_P_SETUP_FEATURES_PWM_RSSI) ? " RSSI_PWM" : ""),
        ((features & PX4IO_P_SETUP_FEATURES_ADC_RSSI) ? " RSSI_ADC" : "")
        );
    uint16_t arming = io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING);
    printf("arming 0x%04x%s%s%s%s%s%s%s%s%s%s\n",
           arming,
           ((arming & PX4IO_P_SETUP_ARMING_FMU_ARMED)        ? " FMU_ARMED" : " FMU_DISARMED"),
           ((arming & PX4IO_P_SETUP_ARMING_IO_ARM_OK)        ? " IO_ARM_OK" : " IO_ARM_DENIED"),
           ((arming & PX4IO_P_SETUP_ARMING_MANUAL_OVERRIDE_OK)    ? " MANUAL_OVERRIDE_OK" : ""),
           ((arming & PX4IO_P_SETUP_ARMING_FAILSAFE_CUSTOM)        ? " FAILSAFE_CUSTOM" : ""),
           ((arming & PX4IO_P_SETUP_ARMING_INAIR_RESTART_OK)    ? " INAIR_RESTART_OK" : ""),
           ((arming & PX4IO_P_SETUP_ARMING_ALWAYS_PWM_ENABLE)    ? " ALWAYS_PWM_ENABLE" : ""),
           ((arming & PX4IO_P_SETUP_ARMING_LOCKDOWN)        ? " LOCKDOWN" : ""),
           ((arming & PX4IO_P_SETUP_ARMING_FORCE_FAILSAFE)        ? " FORCE_FAILSAFE" : ""),
           ((arming & PX4IO_P_SETUP_ARMING_TERMINATION_FAILSAFE) ? " TERM_FAILSAFE" : ""),
           ((arming & PX4IO_P_SETUP_ARMING_OVERRIDE_IMMEDIATE) ? " OVERRIDE_IMMEDIATE" : "")
           );
#ifdef CONFIG_ARCH_BOARD_PX4FMU_V1
    printf("rates 0x%04x default %u alt %u relays 0x%04x\n",
           io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_RATES),
           io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_DEFAULTRATE),
           io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_ALTRATE),
           io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_RELAYS));
#endif
#ifdef CONFIG_ARCH_BOARD_PX4FMU_V2
    printf("rates 0x%04x default %u alt %u\n",
           io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_RATES),
           io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_DEFAULTRATE),
           io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_ALTRATE));
#endif
    printf("debuglevel %u\n", io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_SET_DEBUG));
    for (unsigned group = 0; group < 4; group++) {
        printf("controls %u:", group);

        for (unsigned i = 0; i < _max_controls; i++)
            printf(" %d", (int16_t) io_reg_get(PX4IO_PAGE_CONTROLS, group * PX4IO_PROTOCOL_MAX_CONTROL_COUNT + i));

        printf("\n");
    }

    if (extended_status) {
        for (unsigned i = 0; i < _max_rc_input; i++) {
            unsigned base = PX4IO_P_RC_CONFIG_STRIDE * i;
            uint16_t options = io_reg_get(PX4IO_PAGE_RC_CONFIG, base + PX4IO_P_RC_CONFIG_OPTIONS);
            printf("input %u min %u center %u max %u deadzone %u assigned %u options 0x%04x%s%s\n",
                   i,
                   io_reg_get(PX4IO_PAGE_RC_CONFIG, base + PX4IO_P_RC_CONFIG_MIN),
                   io_reg_get(PX4IO_PAGE_RC_CONFIG, base + PX4IO_P_RC_CONFIG_CENTER),
                   io_reg_get(PX4IO_PAGE_RC_CONFIG, base + PX4IO_P_RC_CONFIG_MAX),
                   io_reg_get(PX4IO_PAGE_RC_CONFIG, base + PX4IO_P_RC_CONFIG_DEADZONE),
                   io_reg_get(PX4IO_PAGE_RC_CONFIG, base + PX4IO_P_RC_CONFIG_ASSIGNMENT),
                   options,
                   ((options & PX4IO_P_RC_CONFIG_OPTIONS_ENABLED) ? " ENABLED" : ""),
                   ((options & PX4IO_P_RC_CONFIG_OPTIONS_REVERSE) ? " REVERSED" : ""));
        }
    }

    printf("failsafe");

    for (unsigned i = 0; i < _max_actuators; i++)
        printf(" %u", io_reg_get(PX4IO_PAGE_FAILSAFE_PWM, i));

    printf("\ndisarmed values");

    for (unsigned i = 0; i < _max_actuators; i++)
        printf(" %u", io_reg_get(PX4IO_PAGE_DISARMED_PWM, i));

    printf("\n");
}


int NavioRCIO::ioctl(int cmd, unsigned long arg)
{
    int ret = OK;

    /* regular ioctl? */
    switch (cmd) {

        case PX4IO_SET_DEBUG:
        /* set the debug level */
        ret = io_reg_set(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_SET_DEBUG, arg);
        break;

        case PX4IO_GET_DEBUG:
            *(unsigned long*)arg = io_reg_get(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_SET_DEBUG);
            ret = OK;
        break;

        case PX4IO_GET_RAW_ADC1:
            *(unsigned*)arg = io_reg_get(PX4IO_PAGE_RAW_ADC_INPUT, 0);
            ret = OK;
        break;


        case RC_INPUT_GET: {
            uint16_t status;
            rc_input_values *rc_val = (rc_input_values *)arg;

            ret = io_reg_get(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_FLAGS, &status, 1);

            if (ret != OK)
                break;

            /* if no R/C input, don't try to fetch anything */
            if (!(status & PX4IO_P_STATUS_FLAGS_RC_OK)) {
                ret = -ENOTCONN;
                break;
            }

            /* sort out the source of the values */
            if (status & PX4IO_P_STATUS_FLAGS_RC_PPM) {
                rc_val->input_source = RC_INPUT_SOURCE_PX4IO_PPM;

            } else if (status & PX4IO_P_STATUS_FLAGS_RC_DSM) {
                rc_val->input_source = RC_INPUT_SOURCE_PX4IO_SPEKTRUM;

            } else if (status & PX4IO_P_STATUS_FLAGS_RC_SBUS) {
                rc_val->input_source = RC_INPUT_SOURCE_PX4IO_SBUS;

            } else if (status & PX4IO_P_STATUS_FLAGS_RC_ST24) {
                rc_val->input_source = RC_INPUT_SOURCE_PX4IO_ST24;

            } else {
                rc_val->input_source = RC_INPUT_SOURCE_UNKNOWN;
            }

            /* read raw R/C input values */
            ret = io_reg_get(PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_BASE, &(rc_val->values[0]), _max_rc_input);
            break;
        }

        default:
        ret = -EINVAL;
    }

    return ret;
}
