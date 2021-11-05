#include <Wire.h>

class Sensor
{
private:
    /* data */
    const int i2c_addr = 0x68;

    //Variables for Gyroscope
    int gyro_x, gyro_y, gyro_z;
    long gyro_x_cal, gyro_y_cal, gyro_z_cal;
    bool set_gyro_angles;

    long acc_x, acc_y, acc_z, acc_total_vector;
    float angle_roll_acc, angle_pitch_acc;

    float angle_pitch, angle_roll;
    int angle_pitch_buffer, angle_roll_buffer;
    float angle_pitch_output, angle_roll_output;

    // Setup timers and temp variables
    long loop_timer;
    int temp;

    void setup_mpu_6050_registers()
    {

        //Activate the MPU-6050

        //Start communicating with the MPU-6050
        Wire.beginTransmission(this->i2c_addr);
        //Send the requested starting register
        Wire.write(0x6B);
        //Set the requested starting register
        Wire.write(0x00);
        //End the transmission
        Wire.endTransmission();

        //Configure the accelerometer (+/-8g)

        //Start communicating with the MPU-6050
        Wire.beginTransmission(this->i2c_addr);
        //Send the requested starting register
        Wire.write(0x1C);
        //Set the requested starting register
        Wire.write(0x10);
        //End the transmission
        Wire.endTransmission();

        //Configure the gyro (500dps full scale)

        //Start communicating with the MPU-6050
        Wire.beginTransmission(this->i2c_addr);
        //Send the requested starting register
        Wire.write(0x1B);
        //Set the requested starting register
        Wire.write(0x08);
        //End the transmission
        Wire.endTransmission();
    }

    void read_mpu_6050_data()
    {

        //Read the raw gyro and accelerometer data

        //Start communicating with the MPU-6050
        Wire.beginTransmission(this->i2c_addr);
        //Send the requested starting register
        Wire.write(0x3B);
        //End the transmission
        Wire.endTransmission();
        //Request 14 bytes from the MPU-6050
        Wire.requestFrom(this->i2c_addr, 14);
        //Wait until all the bytes are received
        while (Wire.available() < 14)
            ;

        //Following statements left shift 8 bits, then bitwise OR.
        //Turns two 8-bit values into one 16-bit value
        this->acc_x = Wire.read() << 8 | Wire.read();
        this->acc_y = Wire.read() << 8 | Wire.read();
        this->acc_z = Wire.read() << 8 | Wire.read();
        this->temp = Wire.read() << 8 | Wire.read();
        this->gyro_x = Wire.read() << 8 | Wire.read();
        this->gyro_y = Wire.read() << 8 | Wire.read();
        this->gyro_z = Wire.read() << 8 | Wire.read();
    }

public:
    Sensor(/* args */);
    ~Sensor();

    void setup()
    {
        //Setup the registers of the MPU-6050
        setup_mpu_6050_registers();

        //Read the raw acc and gyro data from the MPU-6050 1000 times
        for (int cal_int = 0; cal_int < 1000; cal_int++)
        {
            this->read_mpu_6050_data();
            //Add the gyro x offset to the gyro_x_cal variable
            this->gyro_x_cal += this->gyro_x;
            //Add the gyro y offset to the gyro_y_cal variable
            this->gyro_y_cal += this->gyro_y;
            //Add the gyro z offset to the gyro_z_cal variable
            this->gyro_z_cal += this->gyro_z;
            //Delay 3us to have 250Hz for-loop
            delay(3);
        }

        // Divide all results by 1000 to get average offset
        this->gyro_x_cal /= 1000;
        this->gyro_y_cal /= 1000;
        this->gyro_z_cal /= 1000;
        // Init Timer
        this->loop_timer = micros();
    }

    float read()
    {
        // Get data from MPU-6050
        this->read_mpu_6050_data();

        //Subtract the offset values from the raw gyro values
        this->gyro_x -= this->gyro_x_cal;
        this->gyro_y -= this->gyro_y_cal;
        this->gyro_z -= this->gyro_z_cal;

        //Gyro angle calculations . Note 0.0000611 = 1 / (250Hz x 65.5)

        //Calculate the traveled pitch angle and add this to the angle_pitch variable
        this->angle_pitch += this->gyro_x * 0.0000611;
        //Calculate the traveled roll angle and add this to the angle_roll variable
        //0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians
        this->angle_roll += this->gyro_y * 0.0000611;

        //If the IMU has yawed transfer the roll angle to the pitch angle
        this->angle_pitch += this->angle_roll * sin(this->gyro_z * 0.000001066);
        //If the IMU has yawed transfer the pitch angle to the roll angle
        this->angle_roll -= this->angle_pitch * sin(this->gyro_z * 0.000001066);

        //Accelerometer angle calculations

        //Calculate the total accelerometer vector
        this->acc_total_vector = sqrt((this->acc_x * this->acc_x) + (this->acc_y * this->acc_y) + (this->acc_z * this->acc_z));

        //57.296 = 1 / (3.142 / 180) The Arduino asin function is in radians
        //Calculate the pitch angle
        this->angle_pitch_acc = asin((float)this->acc_y / this->acc_total_vector) * 57.296;
        //Calculate the roll angle
        this->angle_roll_acc = asin((float)this->acc_x / this->acc_total_vector) * -57.296;

        //Accelerometer calibration value for pitch
        this->angle_pitch_acc -= 0.0;
        //Accelerometer calibration value for roll
        this->angle_roll_acc -= 0.0;

        if (this->set_gyro_angles)
        {

            //If the IMU has been running
            //Correct the drift of the gyro pitch angle with the accelerometer pitch angle
            this->angle_pitch = this->angle_pitch * 0.9996 + this->angle_pitch_acc * 0.0004;
            //Correct the drift of the gyro roll angle with the accelerometer roll angle
            this->angle_roll = this->angle_roll * 0.9996 + this->angle_roll_acc * 0.0004;
        }
        else
        {
            //IMU has just started
            //Set the gyro pitch angle equal to the accelerometer pitch angle
            this->angle_pitch = this->angle_pitch_acc;
            //Set the gyro roll angle equal to the accelerometer roll angle
            this->angle_roll = this->angle_roll_acc;
            //Set the IMU started flag
            this->set_gyro_angles = true;
        }

        //To dampen the pitch and roll angles a complementary filter is used
        //Take 90% of the output pitch value and add 10% of the raw pitch value
        this->angle_pitch_output = this->angle_pitch_output * 0.9 + this->angle_pitch * 0.1;
        //Take 90% of the output roll value and add 10% of the raw roll value
        this->angle_roll_output = this->angle_roll_output * 0.9 + this->angle_roll * 0.1;
        //Wait until the loop_timer reaches 4000us (250Hz) before starting the next loop
        return this->angle_pitch_output;
    }
};
