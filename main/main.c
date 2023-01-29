/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "mpu6050.h"
#include "imu.h"
#include "filter.h"
#include "structconfig.h"

void app_main(void)
{
    int ifwork;
    I2C_Init();
    ifwork = MPU_Init();
    printf("init state:%d\n", ifwork);

    uint8_t flag = 1;
    float pitch, roll, yaw;
    float gravity_x, gravity_y, gravity_z;
    int ax, ay, az;
    int gx, gy, gz;
    int ret, ret1;

    while (1)
    {
        Prepare_Data();                             // 获取姿态解算所需数据
        IMUupdate(&Gyr_rad, &Acc_filt, &Att_Angle); // 四元数姿态解算

        printf("x:%f,%f,%f,%f,%f,%f\n", Att_Angle.rol, Att_Angle.pit, Att_Angle.yaw,Gyr_rad.X, 
                                                                     Gyr_rad.Y, Gyr_rad.Z);
        // printf("--------------------------------------------------\n");
        // printf("x:%f,y:%f,z:%f \n", Att_Angle.rol, Att_Angle.pit, Att_Angle.yaw);

        // MPU_Get_Accelerometer(&ax, &ay, &az);
        // gravity_x=ax/2048.0*9.8;
        // gravity_y=ay/2048.0*9.8;
        // gravity_z=az/2048.0*9.8;
        //  printf("x:%f,y:%f,z:%f \n",gravity_x,gravity_y,gravity_z);
        // MPU_Get_Gyroscope(&gx, &gy, &gz);
        // printf("%d,%d,%d\n", ax, ay, az);
        // printf("--------------------------------------------------\n");
        // printf("%d,%d,%d\n", gx, gy, gz);
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}
