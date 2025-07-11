#include <string>
#include "params.h"
#include "gtest/gtest.h"

using std::string;

TEST(ParamsTest, basic) {
    char buff[256];
    params_t req;
    request_init(&req, buff, sizeof(buff));

    ASSERT_EQ(TRUE, params_append_string(&req, "id", "xxx"));
    ASSERT_EQ(TRUE, params_append_string(&req, "name", "aaa"));
    ASSERT_EQ(TRUE, params_append_int(&req, "age", 100)); 
    ASSERT_EQ(TRUE, params_append_float(&req, "weigt", 55.5));
    ASSERT_EQ(TRUE, params_append_double(&req, "net weiget", 50.5));
    ASSERT_EQ(req.size, 62);
    ASSERT_EQ(req.capacity, sizeof(buff));

    int32_t age = 0;
    float  weiget = 0;
    double net_weiget = 0;

    ASSERT_EQ(params_get(&req, "id"), string("xxx"));
    ASSERT_EQ(params_get(&req, "name"), string("aaa"));

    ASSERT_EQ(params_get_int(&req, "age", &age), TRUE);
    ASSERT_EQ(age, 100);

    ASSERT_EQ(params_get_float(&req, "weigt", &weiget), TRUE); 
    ASSERT_EQ(weiget, 55.5);

    ASSERT_EQ(params_get_double(&req, "net weiget", &net_weiget), TRUE);
    ASSERT_EQ(net_weiget, 50.5);
}

TEST(ParamsTest, json) {
    char buff[256];
    params_t req;
    request_init(&req, buff, sizeof(buff));

    params_append_string(&req, "user", "{\"name\":\"Jack\",\"age\":27,\"weiget\":75.5}");
   
    char value[256]={0};
    ASSERT_EQ(params_get_json_string(&req, "user", "name",value,sizeof(value)), TRUE);
    ASSERT_EQ(value, string("Jack"));

    int32_t age = 0;
    ASSERT_EQ(params_get_json_int(&req, "user", "age", &age), TRUE);
    ASSERT_EQ(age, 27);

    double weiget = 0;
    ASSERT_EQ(params_get_json_double(&req, "user", "weiget", &weiget), TRUE);
    ASSERT_EQ(weiget, 75.5);

    printf("value:%s,age:%d,weiget:%lf\n",value, age, weiget);
}

TEST(ParamsTest, error) {
    char buff[6];
    params_t req;
    request_init(&req, buff, sizeof(buff));

    ASSERT_EQ(FALSE, params_append_string(&req, NULL, NULL));
    ASSERT_EQ(FALSE, params_append_string(&req, "id", "xxxxx"));
}

