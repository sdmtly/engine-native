/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "platform/CCDevice.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_OPENHARMONY
#include "HelperMacros.h"
#include "sensors/oh_sensor.h"
NS_CC_BEGIN

constexpr Sensor_Type SENSOR_ID[]{SENSOR_TYPE_ACCELEROMETER, SENSOR_TYPE_LINEAR_ACCELERATION, SENSOR_TYPE_GYROSCOPE};
int64_t SENSOR_SAMPLE_PERIOD = 200000000;
constexpr int64_t INVALID_VALUE = -1;

Sensor_Subscriber *g_user[3] = {nullptr, nullptr, nullptr};
Sensor_SubscriptionId *g_id[3] = {nullptr, nullptr, nullptr};
Sensor_SubscriptionAttribute *g_attr[3] = {nullptr, nullptr, nullptr};

static cocos2d::Device::MotionValue motionValue;

float radiansToDegrees(float radians) {
    const float pi = M_PI;
    return radians * (180.0 / pi);
}

void SensorDataCallbackImpl(Sensor_Event *event) {
    if (event == nullptr) {
        LOGI("event is null");
        return;
    }
    int64_t timestamp = INVALID_VALUE;
    int32_t ret = OH_SensorEvent_GetTimestamp(event, &timestamp);
    if (ret != SENSOR_SUCCESS) {
        return;
    }
    Sensor_Type sensorType;
    ret = OH_SensorEvent_GetType(event, &sensorType);
    if (ret != SENSOR_SUCCESS) {
        return;
    }
    Sensor_Accuracy accuracy = SENSOR_ACCURACY_UNRELIABLE;
    ret = OH_SensorEvent_GetAccuracy(event, &accuracy);
    if (ret != SENSOR_SUCCESS) {
        return;
    }
    float *data = nullptr;
    uint32_t length = 0;
    ret = OH_SensorEvent_GetData(event, &data, &length);
    if (ret != SENSOR_SUCCESS) {
        return;
    }
    switch (sensorType) {
    case SENSOR_TYPE_ACCELEROMETER:
        motionValue.accelerationIncludingGravityX = data[0];
        motionValue.accelerationIncludingGravityY = data[1];
        motionValue.accelerationIncludingGravityZ = -data[2];
        break;
    case SENSOR_TYPE_LINEAR_ACCELERATION:
        motionValue.accelerationX = data[0];
        motionValue.accelerationY = data[1];
        motionValue.accelerationZ = data[2];
        break;
    case SENSOR_TYPE_GYROSCOPE:
        motionValue.rotationRateAlpha = radiansToDegrees(data[0]);
        motionValue.rotationRateBeta = radiansToDegrees(data[1]);
        motionValue.rotationRateGamma = radiansToDegrees(data[2]);
        break;
    }
}

static void enableSensor(int index) {
    Sensor_Subscriber *gUser = OH_Sensor_CreateSubscriber();
    int32_t ret = OH_SensorSubscriber_SetCallback(gUser, SensorDataCallbackImpl);
    if (ret != SENSOR_SUCCESS) {
        return;
    }

    Sensor_SubscriptionId *id = OH_Sensor_CreateSubscriptionId();
    ret = OH_SensorSubscriptionId_SetType(id, SENSOR_ID[index]);
    if (ret != SENSOR_SUCCESS) {
        return;
    }

    Sensor_SubscriptionAttribute *attr = OH_Sensor_CreateSubscriptionAttribute();
    ret = OH_SensorSubscriptionAttribute_SetSamplingInterval(attr, SENSOR_SAMPLE_PERIOD);
    if (ret != SENSOR_SUCCESS) {
        return;
    }

    ret = OH_Sensor_Subscribe(id, attr, gUser);
    if (ret != SENSOR_SUCCESS) {
        return;
    }
    g_user[index] = gUser;
    g_id[index] = id;
    g_attr[index] = attr;

    LOGI("Subscriber successful");
}

static void disableSensor(int index) {
    int32_t ret = -1;
    Sensor_SubscriptionId *it = g_id[index];
    Sensor_SubscriptionAttribute *at = g_attr[index];
    Sensor_Subscriber *gUser = g_user[index];
    if(it == nullptr || at == nullptr || gUser == nullptr){
        return;
    }
    ret = OH_Sensor_Unsubscribe(it, gUser);
    if (ret != SENSOR_SUCCESS) {
        return;
    }
    if (it != nullptr) {
        OH_Sensor_DestroySubscriptionId(it);
    }
    if (at != nullptr) {
        OH_Sensor_DestroySubscriptionAttribute(at);
    }
    if (gUser != nullptr) {
        OH_Sensor_DestroySubscriber(gUser);
        gUser = nullptr;
    }
}

void cocos2d::Device::setAccelerometerEnabled(bool isEnabled) {
    constexpr int length = sizeof(SENSOR_ID) / sizeof(SENSOR_ID[0]);
    if (isEnabled) {
        for (uint32_t i = 0; i < length; i++) {
            disableSensor(i);
        }
        for (uint32_t i = 0; i < length; i++) {
            enableSensor(i);
        }
    } else {
        for (uint32_t i = 0; i < length; i++) {
            disableSensor(i);
        }
    }
}

void cocos2d::Device::setAccelerometerInterval(float interval) {
    SENSOR_SAMPLE_PERIOD = static_cast<int64_t>(interval*1000000000);
    setAccelerometerEnabled(true);
}

const cocos2d::Device::MotionValue &cocos2d::Device::getDeviceMotionValue() { return motionValue; }

NS_CC_END
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_OPENHARMONY
