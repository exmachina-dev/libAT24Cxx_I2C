/* mbed simplified access to Microchip 24LCxx Serial EEPROM devices (I2C)
 * Copyright (c) 2010-2012 ygarcia, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or 
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <sstream>

#include "AT24Cxx_I2C.h"

namespace _AT24CXX_I2C {

    AT24CXX_I2C::AT24CXX_I2C(I2C *p_i2c_instance, const unsigned char p_address, const unsigned int p_frequency) {
        _i2c_instance = p_i2c_instance;

        _slaveAddress = (p_address << 1) | 0xa0; // Slave address format is: 1 0 1 0 A3 A2 A1 R/W
        _i2c_instance->frequency(p_frequency); // Set the frequency of the I2C interface
    }

    AT24CXX_I2C::~AT24CXX_I2C() {
        _i2c_instance = NULL;
    }

    bool AT24CXX_I2C::erase_memory(const short p_startAddress, const int p_count, const unsigned char p_pattern) {

        unsigned char ebuffer[AT24C_MAX_BUFFER_LENGTH];
        for (int i=0; i<p_count; i++)
            ebuffer[i] = p_pattern;
        return write(p_startAddress, ebuffer, p_count);
    }

    bool AT24CXX_I2C::write(const short p_address, const unsigned char p_byte) {

        // 1.Prepare buffer
        char i2cBuffer[3]; // Memory address + one byte of data
        // 1.1. Memory address
        short address = p_address + 1; // Index start to 1
        i2cBuffer[0] = (unsigned char)(address >> 8);
        i2cBuffer[1] = (unsigned char)((unsigned char)address & 0xff);
        // 1.2. Datas
        i2cBuffer[2] = p_byte;

        // 2. Send I2C start + I2C address + Memory Address + Datas + I2C stop
        int result = _i2c_instance->write(_slaveAddress, i2cBuffer, 3);
        wait(0.02);

        return (bool)(result == 0);
    }

    bool AT24CXX_I2C::write(const short p_address, const short p_short, const AT24CXX_I2C::Mode p_mode) {

        // 1.Prepare buffer
        char i2cBuffer[4]; // Memory address + one short (2 bytes)
        // 1.1. Memory address
        short address = p_address + 1; // Index start to 1
        i2cBuffer[0] = (unsigned char)(address >> 8);
        i2cBuffer[1] = (unsigned char)((unsigned char)address & 0xff);
        // 1.2. Datas
        if (p_mode == BigEndian) {
            i2cBuffer[2] = (unsigned char)(p_short >> 8);
            i2cBuffer[3] = (unsigned char)((unsigned char)p_short & 0xff);
        } else {
            i2cBuffer[2] = (unsigned char)((unsigned char)p_short & 0xff);
            i2cBuffer[3] = (unsigned char)(p_short >> 8);
        }

        // 2. Send I2C start + I2C address + Memory Address + Datas + I2C stop
        int result = _i2c_instance->write(_slaveAddress, i2cBuffer, 4);
        wait(0.02);

        return (bool)(result == 0);
    }

    bool AT24CXX_I2C::write(const short p_address, const int p_int, const AT24CXX_I2C::Mode p_mode) {

        // 1.Prepare buffer
        char i2cBuffer[6]; // Memory address + one integer (4 bytes)
        // 1.1. Memory address
        short address = p_address + 1; // Index start to 1
        i2cBuffer[0] = (unsigned char)(address >> 8);
        i2cBuffer[1] = (unsigned char)((unsigned char)address & 0xff);
        // 1.2. Datas
        if (p_mode == BigEndian) {
            i2cBuffer[2] = (unsigned char)(p_int >> 24);
            i2cBuffer[3] = (unsigned char)(p_int >> 16);
            i2cBuffer[4] = (unsigned char)(p_int >> 8);
            i2cBuffer[5] = (unsigned char)((unsigned char)p_int & 0xff);
        } else {
            i2cBuffer[2] = (unsigned char)((unsigned char)p_int & 0xff);
            i2cBuffer[3] = (unsigned char)(p_int >> 8);
            i2cBuffer[4] = (unsigned char)(p_int >> 16);
            i2cBuffer[5] = (unsigned char)(p_int >> 24);
        }

        // 2. Send I2C start + I2C address + Memory Address + Datas + I2C stop
        int result = _i2c_instance->write(_slaveAddress, i2cBuffer, 6);
        wait(0.02);

        return (bool)(result == 0);
    }

    bool AT24CXX_I2C::write(const short p_address, const char *p_datas, const int p_length) {

        // 1.Prepare buffer
        int length = (p_length == -1) ? strlen(p_datas) : p_length;
        short address = p_address;

        int result = 0;
        for (short b=0; b<length; b+=AT24C_PAGE_SIZE) {
            short baddress = address + b;
            short blength = (length - b) < AT24C_PAGE_SIZE ? length - b : AT24C_PAGE_SIZE;
#if AT24C_ADDRESS_SIZE == 1
            char i2cBuffer[blength + 1];

            i2cBuffer[0] = (unsigned char)((unsigned char)baddress & 0xff);
            for (short i=0; i<blength; i++)
                i2cBuffer[i + 1] = p_datas[b + i];

            result += _i2c_instance->write(_slaveAddress, i2cBuffer, blength + 1);
#elif AT24C_ADDRESS_SIZE == 2
            char i2cBuffer[blength + 2];

            i2cBuffer[0] = (unsigned char)(baddress >> 8);
            i2cBuffer[1] = (unsigned char)((unsigned char)baddress & 0xff);
            for (short i=0; i<blength; i++)
                i2cBuffer[i + 2] = p_datas[b + i];

            result += _i2c_instance->write(_slaveAddress, i2cBuffer, blength + 2);
#endif
        }

        return (bool)(result == 0);
    }

    bool AT24CXX_I2C::write(const short p_address, const unsigned char *p_datas, const int p_length) {
        return write(p_address, (const char *)p_datas, p_length);
    }

    bool AT24CXX_I2C::read(const short p_address, unsigned char * p_byte) {

        // 1.Prepare buffer
        char i2cBuffer[2];
        // 1.1. Memory address
        i2cBuffer[0] = (unsigned char)((unsigned char)p_address & 0xff);

        // 2. Send I2C start + memory address
        if (_i2c_instance->write(_slaveAddress, i2cBuffer, 2, true) == 0) {
            // 2. read data + I2C stop
            int result = _i2c_instance->read(_slaveAddress, (char *)p_byte, 1);

            return (bool)(result == 0);
        }

        return false;
    }

    bool AT24CXX_I2C::read(const short p_address, short *p_short, const AT24CXX_I2C::Mode p_mode) {

        // 1.Prepare buffer
        char i2cBuffer[2];
        // 1.1. Memory address
        i2cBuffer[0] = (unsigned char)((unsigned char)p_address >> 8);
        i2cBuffer[1] = (unsigned char)((unsigned char)p_address & 0xff);

        // 2. Send I2C start + memory address
        if (_i2c_instance->write(_slaveAddress, i2cBuffer, 2, true) == 0) {
            // 2. read data + I2C stop
            int result = _i2c_instance->read(_slaveAddress, i2cBuffer, 2);
            if (result == 0) {
                if (p_mode ==  BigEndian) {
                    *p_short = (short)(i2cBuffer[0] << 8 | i2cBuffer[1]);
                } else {
                    *p_short = (short)(i2cBuffer[1] << 8 | i2cBuffer[0]);
                }

                return true;
            }
        }

        return false;
    }

    bool AT24CXX_I2C::read(const short p_address, int *p_int, const AT24CXX_I2C::Mode p_mode) {

        // 1.Prepare buffer
        char i2cBuffer[4];
        // 1.1. Memory address
        i2cBuffer[0] = (unsigned char)(p_address >> 8);
        i2cBuffer[1] = (unsigned char)((unsigned char)p_address & 0xff);

        // 2. Send I2C start + memory address
        if (_i2c_instance->write(_slaveAddress, i2cBuffer, 2, true) == 0) {
            wait(0.02);
            // 2. read data + I2C stop
            int result = _i2c_instance->read(_slaveAddress, i2cBuffer, 4);
            if (result == 0) {
                wait(0.02);
                if (p_mode ==  BigEndian) {
                    *p_int = (int)(i2cBuffer[0] << 24 | i2cBuffer[1] << 16 | i2cBuffer[2] << 8 | i2cBuffer[3]);
                } else {
                    *p_int = (int)(i2cBuffer[3] << 24 | i2cBuffer[2] << 16 | i2cBuffer[1] << 8 | i2cBuffer[0]);
                }

                return true;
            }

            return false;
        }

        return false;
    }

    bool AT24CXX_I2C::read(const short p_address, unsigned char * p_datas, const int p_length) {

        // 1.Prepare buffer
        short address = p_address;
        int length = 0;

        if (p_length == -1) {
            length = sizeof(p_datas);
        } else {
            length = p_length;
        }

        int result = 0;
        for (short b=0; b<length; b+=AT24C_PAGE_SIZE) {
            short baddress = address + b;
            char blength = (length - b) < AT24C_PAGE_SIZE ? length - b : AT24C_PAGE_SIZE;
#if AT24C_ADDRESS_SIZE == 1
            char i2cBuffer[1];
            i2cBuffer[0] = (unsigned char)((unsigned char)baddress & 0xff);

            if (_i2c_instance->write(_slaveAddress, i2cBuffer, 1, true) == 0) {
#elif AT24C_ADDRESS_SIZE == 2
            char i2cBuffer[2];
            i2cBuffer[0] = (unsigned char)((unsigned char)baddress >> 8);
            i2cBuffer[1] = (unsigned char)((unsigned char)baddress & 0xff);

            if (_i2c_instance->write(_slaveAddress, i2cBuffer, 2, true) == 0) {
#endif

                unsigned char buffer[blength];

                result += _i2c_instance->read(_slaveAddress, (char *)buffer, blength);

                if (result == 0) {
                    memcpy(p_datas + b, &buffer, blength);
                }
            }
        }

        return (bool)(result == 0);

        return false;
    }

} // End of namespace _24CXX_I2C
