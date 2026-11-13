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

#include "platform/CCPlatformConfig.h"
#include "HelperMacros.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_OPENHARMONY

#include "platform/CCDevice.h"
#include "platform/openharmony/napi/NapiHelper.h"

NS_CC_BEGIN

int Device::getDPI() {
    auto value = NapiHelper::napiCallFunction("getDPI");
    if (value.IsNumber()) {
        return value.As<Napi::Number>().FloatValue();
    }
    return 0.F;
}

int Device::getDevicePixelRatio() {
    // float value;
    // NapiHelper::napiCallFunction("getPixelRation", &value);
    // return value;
    // TODO(qgh):openharmony does support this interface, but returning a value of 1.5 will cause the entire page to zoom in.
    return 1;
}

void Device::setKeepScreenOn(bool value) {
    CC_UNUSED_PARAM(value);
}

cocos2d::Vec4 Device::getSafeAreaEdge() {
    // 引擎原有：获取方向和刘海尺寸
    auto value = NapiHelper::napiCallFunction("getDeviceOrientation");
    auto height = NapiHelper::napiCallFunction("getCutoutHeight");
    auto width = NapiHelper::napiCallFunction("getCutoutWidth");
    
    // --- 新增：尝试通过 NAPI 获取真实的底部小白条高度（为未来扩展留出接口） ---
    auto nav_height_val = NapiHelper::napiCallFunction("getNavigationIndicatorHeight");

    int32_t orientation = 0;
    int32_t cutout_height = 0;
    int32_t cutout_width = 0;
    int32_t nav_height = 0;

    if (value.IsNumber()) {
        orientation = value.As<Napi::Number>().Int32Value();
    }
    if (height.IsNumber()) {
        cutout_height = height.As<Napi::Number>().Int32Value();
    }
    if (width.IsNumber()) {
        cutout_width = width.As<Napi::Number>().Int32Value();
    }
    // 读取 ArkTS 层传递的底部高度（如果未在 JS/ETS 注册该函数，会安全跳过）
    if (nav_height_val.IsNumber()) {
        nav_height = nav_height_val.As<Napi::Number>().Int32Value();
    }

    float safearea_top = 0.0f;
    float safearea_left = 0.0f;
    float safearea_bottom = 0.0f;
    float safearea_right = 0.0f;

    // 根据屏幕方向分配安全区
    if(0 == orientation) { 
        // 竖屏
        safearea_top += cutout_height;
        
        // --- 核心修复：竖屏底部留白逻辑 ---
        if (nav_height > 0) {
            safearea_bottom = nav_height;
        } else {
            // 像 Android 一样，增加一个通用的 20dp 防呆安全留白
            float dpi = getDPI(); // 调用同文件中的 getDPI() 方法
            if (dpi <= 0) dpi = 160.0f; // 容错兜底
            safearea_bottom = 20.0f * (dpi / 160.0f);
        }
        
    } else if(1 == orientation) { 
        // 横屏（顺时针旋转）[cite: 5]
        safearea_right += cutout_width;
        
        // 横屏寸土寸金，如果没有明确传入小白条真实高度，则不做 20dp 的兜底留白
        if (nav_height > 0) {
            safearea_bottom = nav_height;
        }
        
    } else if(2 == orientation) { 
        // 倒立竖屏
        safearea_bottom += cutout_height;
        
    } else if(3 == orientation) { 
        // 反向横屏
        safearea_left += cutout_width;
        
        if (nav_height > 0) {
            safearea_bottom = nav_height;
        }
    }

    return cocos2d::Vec4(safearea_top, safearea_left, safearea_bottom, safearea_right);
}

Device::Rotation Device::getDeviceRotation() {
    auto value = NapiHelper::napiCallFunction("getDeviceOrientation");
    int32_t result = 0;
    if (value.IsNumber()) {
        result = value.As<Napi::Number>().Int32Value();
    }
    if(result == 0) {
        return cocos2d::Device::Rotation::_0;
    } else if(result == 1) {
        // TODO(qgh): The openharmony platform is rotated clockwise.
        return cocos2d::Device::Rotation::_270;
    } else if(result == 2) {
        return cocos2d::Device::Rotation::_180;
    } else if(result == 3) {
        // TODO(qgh): The openharmony platform is rotated clockwise.
        return cocos2d::Device::Rotation::_90;
    }
    CC_ASSERT(false);
    return cocos2d::Device::Rotation::_0;
}

Device::NetworkType Device::getNetworkType() {
    auto value = NapiHelper::napiCallFunction("getNetworkType");
    int32_t result;
    if (value.IsNumber()) {
        result = value.As<Napi::Number>().Int32Value();
    }
    if(result == 0) {
        return cocos2d::Device::NetworkType::WWAN;
    } else if(result == 1 or result == 3) {
        return cocos2d::Device::NetworkType::LAN;
    }
    return cocos2d::Device::NetworkType::NONE;
}

float Device::getBatteryLevel() {
    auto value = NapiHelper::napiCallFunction("getBatteryLevel");
    if (value.IsNumber()) {
        return value.As<Napi::Number>().FloatValue();
    }
    return 0.F;
}

std::string Device::getDeviceModel() {
    std::string str;
    auto ret = NapiHelper::napiCallFunction("getDeviceModel");
    if (ret.IsString()) {
        str = ret.As<Napi::String>().Utf8Value();
    }
    return str;
}

void Device::vibrate(float duration) {
    NapiHelper::napiCallFunction("vibrate", duration);
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_OPENHARMONY
