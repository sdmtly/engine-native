/****************************************************************************
 Copyright (c) 2022-2023 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
****************************************************************************/
import display from '@ohos.display';
import I18n from '@ohos.i18n';
import deviceInfo from '@ohos.deviceInfo';
import batteryInfo from '@ohos.batteryInfo';
import sensor from '@ohos.sensor';
import connection from '@ohos.net.connection'
import vibrator from '@ohos.vibrator';
import process from '@ohos.process';

let pro = new process.ProcessManager();
globalThis.getSystemLanguage = function () {
    return I18n.System.getSystemLanguage();
}

globalThis.getOSFullName = function () {
    return deviceInfo.osFullName;
}

globalThis.getDeviceModel = function () {
    return deviceInfo.productModel;
}

globalThis.getBatteryLevel = function () {
    return batteryInfo.batterySOC;
}

globalThis.getDPI = function () {
    var displayClass = display.getDefaultDisplaySync();
    return displayClass.densityDPI;
}

globalThis.getPixelRation = function () {
    var displayClass = display.getDefaultDisplaySync();
    return displayClass.densityPixels;
}

globalThis.getDeviceOrientation = function () {
    var displayClass = display.getDefaultDisplaySync();
    return displayClass.rotation;
}

function radiansToDegrees(radians)  {
    var pi = Math.PI;
    return radians * (180/pi);
}

let sDeviceMotionValues = [];
try {
    sensor.on(sensor.SensorId.ACCELEROMETER, function (data) {
        sDeviceMotionValues[0] = data.x * -1;
        sDeviceMotionValues[1] = data.y * -1;
        sDeviceMotionValues[2] = data.z;
    },
        { interval: 200000000 }
    );
} catch (err) {
    sDeviceMotionValues[0] = 0;
    sDeviceMotionValues[1] = 0;
    sDeviceMotionValues[2] = 0;
}

try {
    sensor.on(sensor.SensorId.LINEAR_ACCELEROMETER, function(data){
        sDeviceMotionValues[3] = data.x;
        sDeviceMotionValues[4] = data.y;
        sDeviceMotionValues[5] = data.z;
    },
        {interval: 200000000}
    );
} catch (err) {
    sDeviceMotionValues[3] = 0;
    sDeviceMotionValues[4] = 0;
    sDeviceMotionValues[5] = 0;
}
try {
    sensor.on(sensor.SensorId.GYROSCOPE, function(data){
        sDeviceMotionValues[6] = radiansToDegrees(data.x);
        sDeviceMotionValues[7] = radiansToDegrees(data.y);
        sDeviceMotionValues[8] = radiansToDegrees(data.z);
    },
        {interval: 200000000}
    );
} catch (err) {
    sDeviceMotionValues[6] = 0;
    sDeviceMotionValues[7] = 0;
    sDeviceMotionValues[8] = 0;
}

globalThis.getDeviceMotionValue = function () {
    return sDeviceMotionValues;
}


globalThis.getNetworkType = function () {
    connection.getDefaultNet().then((netHandle)=>
       connection.getNetCapabilities(netHandle).then((data)=>{
            return data.bearerTypes;
        }).catch(err => { 
            console.log('getDefaultNet error.' + JSON.stringify(err));
            return -1; // 未获取到网络连接类型
        })
    ).catch(err => { 
        console.log('getDefaultNet error.' + JSON.stringify(err));
        return -1; // 网络未连接
    });
}

globalThis.vibrate = function (duration) {
    console.log('begin to vibrate, duration is.' + duration);
    try {
        vibrator.startVibration({
            type: 'time',
            duration: duration * 1000
        }, {
            id: 0,
            usage: 'alarm'
        }, (error) => {
            if (error) {
                console.error('vibrate fail, error.code: ' + error.code + 'error.message: ', + error.message);
                return error.code;
            }
            console.log('Vibration start sucessful.');
            return 0;
        });
      } catch (err) {
        console.error('errCode: ' + err.code + ' ,msg: ' + err.message);
      }
}

globalThis.terminateProcess = function () {
    this.pro.exit(0);
}
