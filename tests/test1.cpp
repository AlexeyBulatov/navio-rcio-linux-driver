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

TEST_F(NavioRCIO_Test, Init) {
    EXPECT_EQ(rcio.init(), true);
}

TEST_F(NavioRCIO_Test, Detection) {
    EXPECT_EQ(rcio.detect(), true);
}
