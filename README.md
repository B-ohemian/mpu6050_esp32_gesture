# MPU6050使用ESP32进行姿态解算
该项目是基于小马哥四轴飞控的姿态解算部分移植到了ESP32S3上面并能够很好的运行。该项目在ESP32\ESP32S3上成功测试。
## 如何使用
``` C
#define SCL 15 /*!< I2C SCL pin number  */
#define SDA 16 /*!< I2C SDA pin number  */
```
在`mpu6050.c`文件中修改IIC引脚即可使用
