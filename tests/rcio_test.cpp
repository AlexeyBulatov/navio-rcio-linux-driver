#include <gtest/gtest.h>

#include <drivers/NavioRCIO.h>
#include <drivers/common.h>
#include <drivers/NavioRCInput.h>

class NavioRCIO_Test : public ::testing::Test {
protected:

	NavioRCIO_Test()
    {
        common_init();
        auto interface = new NavioRCIO_serial{};
        interface->init();
        
        rcio = new NavioRCIO{interface};
        rcio->init();
	}

	virtual ~NavioRCIO_Test() {
	}

    NavioRCIO *rcio;
};

TEST_F(NavioRCIO_Test, init) {
    EXPECT_EQ(rcio->init(), true);
}

TEST_F(NavioRCIO_Test, detection) {
    EXPECT_EQ(rcio->detect(), true);
}

TEST_F(NavioRCIO_Test, debug_setting) {
    int ret;

    ret = rcio->ioctl(PX4IO_SET_DEBUG, 2);
    
    EXPECT_FALSE(ret < 0);

    unsigned long debug_level;

    ret = rcio->ioctl(PX4IO_GET_DEBUG, (unsigned long) &debug_level);

    EXPECT_FALSE(ret < 0);
    EXPECT_EQ(2, debug_level);
}

TEST_F(NavioRCIO_Test, adc1_get) {
    int ret;
    unsigned raw_adc1_sample;

    ret = rcio->ioctl(PX4IO_GET_RAW_ADC1, (unsigned long) &raw_adc1_sample);
    
    EXPECT_FALSE(ret < 0);

    EXPECT_TRUE(raw_adc1_sample > 4090);
}

TEST_F(NavioRCIO_Test, rc_input_source_unknown_get) {
    int ret;
    rc_input_values input_rc;

    ret = rcio->ioctl(RC_INPUT_GET, (unsigned long) &input_rc);

    EXPECT_FALSE(ret == -ENOTCONN);
}

TEST_F(NavioRCIO_Test, rc_input_source_ppm_get) {

    int ret;
    rc_input_values input_rc;

    ret = rcio->ioctl(RC_INPUT_GET, (unsigned long) &input_rc);

    EXPECT_TRUE(ret >= 0);

    EXPECT_TRUE(input_rc.input_source == RC_INPUT_SOURCE_PX4IO_PPM);
}

TEST_F(NavioRCIO_Test, rc_input_source_sbus_get) {

    int ret;
    rc_input_values input_rc;

    ret = rcio->ioctl(RC_INPUT_GET, (unsigned long) &input_rc);

    EXPECT_TRUE(ret >= 0);

    EXPECT_FALSE(input_rc.input_source == RC_INPUT_SOURCE_PX4IO_SBUS);
}

TEST_F(NavioRCIO_Test, rc_input_source_spektrum_get) {

    int ret;
    rc_input_values input_rc;

    ret = rcio->ioctl(RC_INPUT_GET, (unsigned long) &input_rc);

    EXPECT_TRUE(ret >= 0);

    EXPECT_FALSE(input_rc.input_source == RC_INPUT_SOURCE_PX4IO_SPEKTRUM);
}

TEST_F(NavioRCIO_Test, rc_input_source_st24_get) {

    int ret;
    rc_input_values input_rc;

    ret = rcio->ioctl(RC_INPUT_GET, (unsigned long) &input_rc);

    EXPECT_TRUE(ret >= 0);

    EXPECT_FALSE(input_rc.input_source == RC_INPUT_SOURCE_PX4IO_ST24);
}


TEST_F(NavioRCIO_Test, rc_pwm_set_arm) {
    int ret;

    ret = -1;

    EXPECT_TRUE(ret >= 0);
}

TEST_F(NavioRCIO_Test, rc_pwm_set_disarm) {
    int ret;

    ret = -1;

    EXPECT_TRUE(ret >= 0);
}

TEST_F(NavioRCIO_Test, rc_pwm_set_arm_ok) {
    int ret;

    ret = -1;

    EXPECT_TRUE(ret >= 0);
}

TEST_F(NavioRCIO_Test, rc_pwm_clear_arm_ok) {
    int ret;

    ret = -1;

    EXPECT_TRUE(ret >= 0);
}

TEST_F(NavioRCIO_Test, rc_output_pwm1) {

    int ret;

    ret = -1;

    EXPECT_TRUE(ret >= 0);
}
