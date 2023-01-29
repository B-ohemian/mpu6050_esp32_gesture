
#include "mpu6050.h"


#define SCL 15 /*!< I2C SCL pin number  */
#define SDA 16 /*!< I2C SDA pin number  */


static uint8_t    MPU6050_buff[14];                  //���ٶ� ������ �¶� ԭʼ����
INT16_XYZ	 GYRO_OFFSET_RAW,ACC_OFFSET_RAW;		 //��Ư����
INT16_XYZ	 MPU6050_ACC_RAW,MPU6050_GYRO_RAW;	     //��ȡֵԭʼ����
uint8_t    	 SENSER_OFFSET_FLAG;                     //������У׼��־λ

// #define SCL 26 /*!< I2C SCL pin number  */
// #define SDA 27 /*!< I2C SDA pin number  */

/**
 * I2C��ʼ��
 */
void I2C_Init()
{
	esp_err_t esp_err;
	i2c_config_t conf = {
			.mode = I2C_MODE_MASTER,
			.sda_io_num = SDA, // select GPIO specific to your project
			.sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_io_num = SCL, // select GPIO specific to your project
			.scl_pullup_en = GPIO_PULLUP_ENABLE,
			.master.clk_speed = 200000, // select frequency specific to your project
																	// .clk_flags = 0,         
	};
	esp_err = i2c_param_config(0, &conf);
	printf("i2c_param_config: %d \n", esp_err);

	esp_err = i2c_driver_install(0, I2C_MODE_MASTER, 0, 0, 0);
	printf("i2c_driver_install: %d \n", esp_err);
}

/**
 * @brief MPU-6050 initial
 */
uint8_t MPU_Init()
{
	uint8_t res = 0;
	// ��ʼ��ESP32Ӳ��I2C
	// I2C_Init();
	// �ȴ���ʼ�����
	// vTaskDelay(200 / portTICK_RATE_MS);
	// ��λMPU6050, �Ĵ���0x6B bit7д1ʵ��
	MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0x80);
	vTaskDelay(200 / portTICK_RATE_MS);
	// ����MPU6050
	MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0x00);
	// ����������������,2000dps
	MPU_Set_Gyro_FSR(3);
	// ���ü��ٶȼ������̷�Χ,+-16g
	MPU_Set_Accel_FSR(3);
	// ���ò�����
	MPU_Set_Rate(200);
	MPU_Write_Byte(MPU_INT_EN_REG, 0X00);		 // �ر������ж�
	MPU_Write_Byte(MPU_USER_CTRL_REG, 0X00); // I2C��ģʽ�ر�
	MPU_Write_Byte(MPU_FIFO_EN_REG, 0X00);	 // �ر�FIFO
	MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X80); // INT���ŵ͵�ƽ��Ч
	MPU_Read_Byte(MPU_DEVICE_ID_REG, &res);
	if (res == MPU_ADDR) // ����ID��ȷ
	{
		MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X01); // ����CLKSEL,PLL X��Ϊ�ο�
		MPU_Write_Byte(MPU_PWR_MGMT2_REG, 0X00); // ���ٶ��������Ƕ�����
		MPU_Set_Rate(200);											 // ���ò�����Ϊ50Hz
		printf("ok\n");
	}
	else
		return 1;

	return 0;
}

/*****************************************************************************
* ��  ����uint8_t MPU6050_WriteMultBytes(uint8_t reg,uint8_t len,uint8_t *buf)
* ��  �ܣ���ָ���Ĵ���д��ָ����������
* ��  ����reg���Ĵ�����ַ
*         len��д�����ݳ��� 
*         buf: д�����ݴ�ŵĵ�ַ
* ����ֵ��0�ɹ� 1ʧ��
* ��  ע��MPU6050������ֲֻ���I2C�����޸ĳ��Լ��ļ���
*****************************************************************************/
uint8_t MPU6050_WriteMultBytes(uint8_t reg,uint8_t len,uint8_t *buf)
{
	if(MPU_Write_Len(reg,buf,len))
		return 1;
	else
		return 0;
}

/*****************************************************************************
* ��  ����uint8_t MPU6050_ReadMultBytes(uint8_t reg,uint8_t len,uint8_t *buf)
* ��  �ܣ���ָ���Ĵ�����ȡָ����������
* ��  ����reg���Ĵ�����ַ
*         len����ȡ���ݳ��� 
*         buf: ��ȡ���ݴ�ŵĵ�ַ
* ����ֵ��0�ɹ� 0ʧ��
* ��  ע��MPU6050������ֲֻ���I2C�����޸ĳ��Լ��ļ���
*****************************************************************************/
uint8_t MPU6050_ReadMultBytes(uint8_t reg,uint8_t len,uint8_t *buf)
{
	if(MPU_Read_Len(reg,buf,len))
		return 1;
	else
		return 0;
}

/**
 * @brief Set the Gyroscope full-scale range of ��250, ��500, ��1000, and ��2000��/sec (dps)
 *
 * @param fsr the number of register, it could be 0, 1, 2, 3
 *
 * @return
 *     - 0 is Success
 *     - 1 is Error
 */
uint8_t MPU_Set_Gyro_FSR(uint8_t fsr)
{
	return MPU_Write_Byte(MPU_GYRO_CFG_REG, fsr << 3);
}

/**
 * @brief Set the Accelerometer full-scale range of ��2g, ��4g, ��8g, and ��16g
 *
 * @param fsr the number of register, it could be 0, 1, 2, 3
 *
 * @return
 *     - 0 is Success
 *     - 1 is Error
 */
uint8_t MPU_Set_Accel_FSR(uint8_t fsr)
{
	return MPU_Write_Byte(MPU_ACCEL_CFG_REG, fsr << 3);
}

/**
 * @brief Set the Sample rate of Gyroscope, Accelerometer, DMP, etc.
 *
 * @param rate parameter is the sample rate of Gyroscope, Accelerometer, DMP, etc.
 *
 * @return
 *     - 0 is Success
 *     - 1 is Error
 */
uint8_t MPU_Set_Rate(uint16_t rate)
{
	uint8_t data;
	if (rate > 1000)
		rate = 1000;
	if (rate < 4)
		rate = 4;
	data = 1000 / rate - 1;
	data = MPU_Write_Byte(MPU_SAMPLE_RATE_REG, data);
	return MPU_Set_LPF(rate / 2); /*!< set low pass filter the half of the rate */
}

/**
 * @brief ͨ��I2Cдһ���ֽ����ݵ�MPU6050
 *
 * @param reg parameter is a register of MPU-6050 //�Ĵ�����ַ
 * @param data parameter will be written to the register of MPU-6050 //��Ҫд��Ĵ���������
 *
 * @return
 *     - 0 is Success
 *     - 1 is Error
 */
uint8_t MPU_Write_Byte(uint8_t reg, uint8_t data)
{
	esp_err_t error;
	// ����I2C���ӣ��������Ӿ��
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	// д�����ź�
	i2c_master_start(cmd);
	// �㲥��ַ����˵����д�����ȴ���Ӧ
	error = i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_WRITE, 1);
	if (error != ESP_OK)
		return 1;
	// д��Ҫд�����ݵļĴ�����ַ
	error = i2c_master_write_byte(cmd, reg, 1);
	if (error != ESP_OK)
		return 1;
	// ����һ��ָ���ļĴ���д�����ݡ�
	error = i2c_master_write_byte(cmd, data, 1);
	if (error != ESP_OK)
		return 1;
	// ֹͣ
	i2c_master_stop(cmd);
	// ��������
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
	// ɾ������
	i2c_cmd_link_delete(cmd);

	return 0;
}


/**
 * @brief Write a buffer to MPU-6050 through I2C
 *
 * @param reg parameter is a register of MPU-6050
 * @param data parameter is a buffer which will be written to a register of MPU-6050
 * @param len parameter is the length of data
 *
 * @return
 *     - 0 is Success
 *     - 1 is Error
 */
uint8_t MPU_Write_Len(uint8_t reg, uint8_t *data, uint8_t len)
{
	esp_err_t error;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	error = i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_WRITE, 1);
	if (error != ESP_OK)
		return 1;

	error = i2c_master_write_byte(cmd, reg, 1);
	if (error != ESP_OK)
		return 1;
	// ������������д��Ĵ���
	error = i2c_master_write(cmd, data, len, 1);
	if (error != ESP_OK)
		return 1;

	i2c_master_stop(cmd);

	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);

	i2c_cmd_link_delete(cmd);

	return 0;
}

/**
 * @brief Read a byte from MPU-6050 through I2C ��һ���ֽ�����
 * �Ը���ģʽ��ȡ����
 * @param reg parameter is a register of MPU-6050
 * @param res the data read will be stored in this parameter
 *
 * @return
 *     - 0 is Success
 *     - 1 is Error
 */
uint8_t MPU_Read_Byte(uint8_t reg, uint8_t *res)
{

	esp_err_t error;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	// ��ַ�㲥
	error = i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_WRITE, 1);
	if (error != ESP_OK)
		return 1;
	// д��Ҫ��ȡ�ļĴ�����ַ
	error = i2c_master_write_byte(cmd, reg, 1);
	if (error != ESP_OK)
		return 1;
	// ��ʼ�ź�
	i2c_master_start(cmd);
	// �㲥��ַ����˵��Ҫ������
	error = i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_READ, 1);
	if (error != ESP_OK)
		return 1;
	// �������Ĵ�������
	error = i2c_master_read_byte(cmd, res, I2C_MASTER_LAST_NACK);
	if (error != ESP_OK)
		return 1;
	// ֹͣ�ź�
	i2c_master_stop(cmd);
	// ��������
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);

	i2c_cmd_link_delete(cmd);
	return 0;
}

/**
 * @brief Read a buffer from MPU-6050 through I2C
 *
 * @param reg parameter is a register of MPU-6050
 * @param buf parameter is a buf witch will store the data
 * @param len parameter is the length of buf
 *
 * @return
 *     - 0 is Success
 *     - 1 is Error
 */
uint8_t MPU_Read_Len(uint8_t reg, uint8_t *buf, uint8_t len)
{
	esp_err_t error;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	error = i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_WRITE, 1);
	if (error != ESP_OK)
		return 1;

	error = i2c_master_write_byte(cmd, reg, 1);
	if (error != ESP_OK)
		return 1;

	i2c_master_start(cmd);
	error = i2c_master_write_byte(cmd, (MPU_ADDR << 1) | I2C_MASTER_READ, 1);
	if (error != ESP_OK)
		return 1;

	error = i2c_master_read(cmd, buf, len, I2C_MASTER_LAST_NACK);
	if (error != ESP_OK)
		return 1;

	i2c_master_stop(cmd);

	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);

	i2c_cmd_link_delete(cmd);
	return 0;
}

/**
 * @brief Set the band of low pass filter
 *
 * @param lps parameter is the band of low pass filter
 *
 * @return
 *     - 0 is Success
 *     - 1 is Error
 */
uint8_t MPU_Set_LPF(uint16_t lpf)
{
	uint8_t data = 0;
	if (lpf >= 188)
		data = 1;
	else if (lpf >= 98)
		data = 2;
	else if (lpf >= 42)
		data = 3;
	else if (lpf >= 20)
		data = 4;
	else if (lpf >= 10)
		data = 5;
	else
		data = 6;
	return MPU_Write_Byte(MPU_CFG_REG, data);
}

/**
 * @brief Get the temperature of the MPU-6050
 *
 * @return
 *     - temp is the temperature of the MPU-6050
 *     - 1 is Error
 */
int16_t MPU_Get_Temperature()
{
	uint8_t buf[2];
	int16_t raw;
	float temp;
	if (MPU_Read_Len(MPU_TEMP_OUTH_REG, buf, 2) == 0)
		return 1;
	raw = ((uint16_t)(buf[1] << 8)) | buf[0];
	temp = 36.53 + ((double)raw / 340);
	return temp * 100;
}

/**
 * @brief Get the Gyroscope data of the MPU-6050
 *
 * @param gx parameter is the x axis data of Gyroscope
 * @param gy parameter is the y axis data of Gyroscope
 * @param gz parameter is the z axis data of Gyroscope
 *
 * @return
 *     - 0 is Success
 *     - 1 is Error
 */
uint8_t MPU_Get_Gyroscope(int16_t *gx, int16_t *gy, int16_t *gz)
{
	uint8_t buf[6], res;
	res = MPU_Read_Len(MPU_GYRO_XOUTH_REG, buf, 6);
	if (res == 0)
	{
		*gx = ((uint16_t)buf[0] << 8) | buf[1];
		*gy = ((uint16_t)buf[2] << 8) | buf[3];
		*gz = ((uint16_t)buf[4] << 8) | buf[5];
	}
	return res;
}

/**
 * @brief Get the Accelerometer data of the MPU-6050
 *
 * @param ax parameter is the x axis data of Accelerometer
 * @param ay parameter is the y axis data of Accelerometer
 * @param az parameter is the z axis data of Accelerometer
 *
 * @return
 *     - 0 is Success
 *     - 1 is Error
 */
uint8_t MPU_Get_Accelerometer(int16_t *ax, int16_t *ay, int16_t *az)
{
	uint8_t buf[6], res;
	res = MPU_Read_Len(MPU_ACCEL_XOUTH_REG, buf, 6);
	if (res == 0)
	{
		*ax = ((uint16_t)buf[0] << 8) | buf[1];
		*ay = ((uint16_t)buf[2] << 8) | buf[3];
		*az = ((uint16_t)buf[4] << 8) | buf[5];
	}
	return res;
}

/******************************************************************************
* ��  ����void MPU6050_Read(void)
* ��  �ܣ���ȡ�����Ǽ��ٶȼƵ�ԭʼ����
* ��  ������
* ����ֵ����
* ��  ע����
*******************************************************************************/
void MPU6050_Read(void)
{
	MPU6050_ReadMultBytes(MPU_ACCEL_XOUTH_REG, 14, MPU6050_buff);// ��ѯ����ȡMPU6050��ԭʼ����
}

/******************************************************************************
* ��  ����uint8_t MPU6050_OffSet(INT16_XYZ value,INT16_XYZ *offset,uint16_t sensivity)
* ��  �ܣ�MPU6050��ƫУ׼
* ��  ����value�� 	 MPU6050ԭʼ����
*         offset��	 У׼�����ƫֵ
*         sensivity�����ٶȼƵ�������
* ����ֵ��1У׼��� 0У׼δ���
* ��  ע����
*******************************************************************************/
uint8_t MPU6050_OffSet(INT16_XYZ value,INT16_XYZ *offset,uint16_t sensivity)
{
	static int32_t tempgx=0,tempgy=0,tempgz=0; 
	static uint16_t cnt_a=0;//ʹ��static���εľֲ������������α������о�̬�洢���ڣ�Ҳ����˵�ú���ִ������ͷ��ڴ�
	if(cnt_a==0)
	{
		value.X=0;
		value.Y=0;
		value.Z=0;
		tempgx = 0;
		tempgy = 0;
		tempgz = 0;
		cnt_a = 1;
		sensivity = 0;
		offset->X = 0;
		offset->Y = 0;
		offset->Z = 0;
	}
	tempgx += value.X;
	tempgy += value.Y; 
	tempgz += value.Z-sensivity ;//���ٶȼ�У׼ sensivity ���� MPU6050��ʼ��ʱ���õ�������ֵ��8196LSB/g��;������У׼ sensivity = 0��
	if(cnt_a==200)               //200����ֵ��ƽ��
	{
		offset->X=tempgx/cnt_a;
		offset->Y=tempgy/cnt_a;
		offset->Z=tempgz/cnt_a;
		cnt_a = 0;
		return 1;
	}
	cnt_a++;
	return 0;
}	

/******************************************************************************
* ��  ����void MPU6050_DataProcess(void)
* ��  �ܣ���MPU6050����ȥ��ƫ����
* ��  ������
* ����ֵ����
* ��  ע����
*******************************************************************************/
void MPU6050_Offset(void)
{
	//���ٶ�ȥ��ƫADֵ 
	MPU6050_ACC_RAW.X =((((int16_t)MPU6050_buff[0]) << 8) | MPU6050_buff[1]) - ACC_OFFSET_RAW.X;
	MPU6050_ACC_RAW.Y =((((int16_t)MPU6050_buff[2]) << 8) | MPU6050_buff[3]) - ACC_OFFSET_RAW.Y;
	MPU6050_ACC_RAW.Z =((((int16_t)MPU6050_buff[4]) << 8) | MPU6050_buff[5]) - ACC_OFFSET_RAW.Z;
	//������ȥ��ƫADֵ 
	MPU6050_GYRO_RAW.X =((((int16_t)MPU6050_buff[8]) << 8) | MPU6050_buff[9]) - GYRO_OFFSET_RAW.X;
	MPU6050_GYRO_RAW.Y =((((int16_t)MPU6050_buff[10]) << 8) | MPU6050_buff[11]) - GYRO_OFFSET_RAW.Y;
	MPU6050_GYRO_RAW.Z =((((int16_t)MPU6050_buff[12]) << 8) | MPU6050_buff[13]) - GYRO_OFFSET_RAW.Z;
	
	if(GET_FLAG(GYRO_OFFSET)) //�����ǽ�����ƫУ׼
	{
		if(MPU6050_OffSet(MPU6050_GYRO_RAW,&GYRO_OFFSET_RAW,0))
		{
			
			 SENSER_FLAG_RESET(GYRO_OFFSET);
			//  PID_WriteFlash(); //���������ǵ���ƫ����
			//  GYRO_Offset_LED();
		     SENSER_FLAG_SET(ACC_OFFSET);//У׼���ٶ�
			
//			 printf("GYRO_OFFSET_RAW Value :X=%d  Y=%d  Z=%d\n",GYRO_OFFSET_RAW.X,GYRO_OFFSET_RAW.Y,GYRO_OFFSET_RAW.Z);
//			 printf("\n");
		}
	}
	if(GET_FLAG(ACC_OFFSET)) //���ٶȼƽ�����ƫУ׼ 
	{
		if(MPU6050_OffSet(MPU6050_ACC_RAW,&ACC_OFFSET_RAW,8196))
		{
			 SENSER_FLAG_RESET(ACC_OFFSET);
			//  PID_WriteFlash(); //������ٶȼƵ���ƫ����
			//  ACC_Offset_LED();
//			 printf("ACC_OFFSET_RAW Value X=%d  Y=%d  Z=%d\n",ACC_OFFSET_RAW.X,ACC_OFFSET_RAW.Y,ACC_OFFSET_RAW.Z); 
//			 printf("\n");
		}
	}
}