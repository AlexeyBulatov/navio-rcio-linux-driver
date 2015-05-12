#include <gtest/gtest.h>

#include <drivers/NavioRCIO.h>

class NavioRCIO_Test : public ::testing::Test {
protected:

	NavioRCIO_Test() {
        rcio.init();
	}

	virtual ~NavioRCIO_Test() {
	}

    NavioRCIO rcio{};
};

TEST_F(NavioRCIO_Test, init) {
    EXPECT_EQ(rcio.init(), true);
}

TEST_F(NavioRCIO_Test, detection) {
    EXPECT_EQ(rcio.detect(), true);
}

TEST_F(NavioRCIO_Test, debug_setting) {
    int ret;

    ret = rcio.ioctl(PX4IO_SET_DEBUG, 2);
    
    EXPECT_FALSE(ret < 0);

    unsigned long debug_level;

    ret = rcio.ioctl(PX4IO_GET_DEBUG, (unsigned long) &debug_level);

    EXPECT_FALSE(ret < 0);
    EXPECT_EQ(2, debug_level);
}

TEST_F(NavioRCIO_Test, adc1_get) {
    int ret;
    unsigned raw_adc1_sample;

    ret = rcio.ioctl(PX4IO_GET_RAW_ADC1, (unsigned long) &raw_adc1_sample);
    
    EXPECT_FALSE(ret < 0);

    EXPECT_TRUE(raw_adc1_sample > 4090);
}
