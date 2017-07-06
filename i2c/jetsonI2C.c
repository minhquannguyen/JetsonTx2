#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "jetsonI2C.h"

/*!
 *  Function to open i2c bus specified and initialize
 *  communication with i2c device
 *
 *  param[in]   addr    i2c device address
 *  param[in]   bus     TX2_I2C_BUS i2c bus to open
 *  param[out]  fd      File descriptor pointer
 *
 *  returns SUCCESS
 */
Status
i2c_open
(
    TxU8            addr,
    TX2_I2C_BUS     bus,
    TxS32           *fd
)
{
    TxS32 length;
    char i2cBuf[BUF_SIZE];

    length = snprintf(i2cBuf, sizeof(i2cBuf), I2C_DIR "%d", bus);
    if (length < 0)
    {
        printf("%s: Error formatting directory\n", __FUNCTION__);
        return FILE_HANDLING_ERROR;
    }

    *fd = open(i2cBuf, O_RDWR);
    if (*fd < 0)
    {
       printf("%s: Error opening %s\n", __FUNCTION__, i2cBuf);
       return FILE_OPEN_ERROR;
    }

    if (ioctl(*fd, I2C_SLAVE, addr) < 0)
    {
        printf("%s: Error with ioctl to address = 0x%x\n",
            __FUNCTION__, addr);
        return IOCTL_ERROR;
    }

    return SUCCESS;
}

/*!
 *  Function to close i2c bus specified
 *
 *  param[in]  fd   File descriptor pointer
 *
 *  returns SUCCESS
 */
Status
i2c_close
(
    TxS32 *fd
)
{
    Status status = SUCCESS;

    if (close(*fd) < 0)
    {
        printf("%s: Error closing i2c file status = %d\n",
            __FUNCTION__, status);
        status = FILE_CLOSE_ERROR;
    }

    return status;
}

/*!
 *  Function to write word value to device register through i2c bus specified
 *  using smbus. This should be equivalent to i2c_write_word().
 *
 *  param[in]  reg          Device register
 *  param[in]  writeVal     Value to write
 *  param[in]  fd           File descriptor to i2c bus
 *
 *  returns SUCCESS
 */
Status
i2c_smbus_write_word
(
    TxU8    reg,
    TxU32   writeVal,
    TxS32   fd
)
{
    Status status = SUCCESS;

    if (i2c_smbus_write_word_data(fd, reg, writeVal) < 0)
    {
        printf("%s: Error i2c writing 0x%x to address 0x%x\n",
            __FUNCTION__, writeVal, reg);
        status = I2C_WRITE_ERROR;
    }

    return status;
}

/*!
 *  Function to read word value from device register through i2c bus specified
 *  using smbus. This should be equivalent to i2c_read_word().
 *
 *  param[in]   reg     Device register
 *  param[in]   fd      File descriptor to i2c bus
 *  param[out]  result  Read value pointer
 *
 *  returns SUCCESS
 */
Status
i2c_smbus_read_word
(
    TxU8    reg,
    TxS32   fd,
    TxU32   *result
)
{
    Status status = SUCCESS;

    *result = i2c_smbus_read_word_data(fd, reg);
    if (*result < 0)
    {
        printf("%s: Error i2c reading register 0x%x\n",
            __FUNCTION__, reg);
        status = I2C_READ_ERROR;
    }

    return status;
}

/*!
 *  Function to write word value to device register through i2c bus specified
 *  using i2c write. This should be quivalent to i2c_smbus_write_word().
 *
 *  param[in]  reg          Device register
 *  param[in]  writeVal     Value to write
 *  param[in]  fd           File descriptor to i2c bus
 *
 *  returns SUCCESS
 */
Status
i2c_write_word
(
    TxU8    reg,
    TxU32   writeVal,
    TxS32   fd
)
{
    TxU8 writeBuf[BUF_SIZE];
    Status status = SUCCESS;
    TxU32 mask = 0xff;

    memset(writeBuf, 0, sizeof(writeBuf));

    /*
     * Write value to buffer in little endian format.
     * e.g. val     = 0x1234
     *      buf[1]  = 0x34
     *      buf[2]  = 0x12
     */
    writeBuf[0] = reg;
    for (TxU32 i = 0; i < BYTES_IN_WORD; i++)
    {
        writeBuf[i+1] = (writeVal & mask) >> (BYTE_SIZE * i);
        mask <<= BYTE_SIZE;
    }

    if (write(fd, writeBuf, 5) != 5)
    {
        printf("%s: Error i2c writing 0x%x to address 0x%x\n",
            __FUNCTION__, writeVal, reg);
        status = I2C_WRITE_ERROR;
    }

    return status;
}

/*!
 *  Function to read word value from device register through i2c bus specified
 *  using i2c read. This should be quivalent to i2c_smbus_read_word().
 *
 *  param[in]   reg     Device register
 *  param[in]   fd      File descriptor to i2c bus
 *  param[out]  result  Read value pointer
 *
 *  returns SUCCESS
 */
Status
i2c_read_word
(
    TxU8    reg,
    TxS32   fd,
    TxU32   *result
)
{
    TxU8 readBuf[BYTES_IN_WORD];
    Status status = SUCCESS;

    memset(readBuf, 0, sizeof(readBuf));

    if (read(fd, readBuf, BYTES_IN_WORD) != BYTES_IN_WORD)
    {
        printf("%s: Error i2c reading register 0x%x\n",
            __FUNCTION__, reg);
        status = I2C_READ_ERROR;
    }

    /*
     * Based on i2c_write_word, it is writing to file in little endian format,
     * so read back and convert from little endian.
     *
     * TODO: Test if word is read back correctly
     */
    *result = 0;
    for (TxU32 i = 0; i < BYTES_IN_WORD; i++)
    {
        *result |= (readBuf[i] << (WORD_SIZE - (BYTE_SIZE * i)));
    }

    return status;
}

/*!
 *  Function to write byte value to device register through i2c bus specified
 *  using smbus. This should be equivalent to i2c_write_byte().
 *
 *  param[in]  reg          Device register
 *  param[in]  writeVal     Value to write
 *  param[in]  fd           File descriptor to i2c bus
 *
 *  returns SUCCESS
 */
Status
i2c_smbus_write_byte
(
    TxU8    reg,
    TxU8    writeVal,
    TxS32   fd
)
{
    Status status = SUCCESS;

    if (i2c_smbus_write_byte_data(fd, reg, writeVal) < 0)
    {
        printf("%s: Error i2c writing 0x%x to address 0x%x\n",
            __FUNCTION__, writeVal, reg);
        status = I2C_WRITE_ERROR;
    }

    return status;
}

/*!
 *  Function to read byte value from device register through i2c bus specified
 *  using smbus. This should be equivalent to i2c_read_word().
 *
 *  param[in]   reg     Device register
 *  param[in]   fd      File descriptor to i2c bus
 *  param[out]  result  Read value pointer
 *
 *  returns SUCCESS
 */
Status
i2c_smbus_read_byte
(
    TxU8    reg,
    TxS32   fd,
    TxU8    *result
)
{
    Status status = SUCCESS;

    *result = i2c_smbus_read_byte_data(fd, reg);
    if (*result < 0)
    {
        printf("%s: Error i2c reading register 0x%x\n",
            __FUNCTION__, reg);
        status = I2C_READ_ERROR;
    }

    return status;
}

/*!
 *  Function to write byte value to device register through i2c bus specified
 *  using i2c write. This should be quivalent to i2c_smbus_write_word().
 *
 *  param[in]  reg          Device register
 *  param[in]  writeVal     Value to write
 *  param[in]  fd           File descriptor to i2c bus
 *
 *  returns SUCCESS
 */
Status
i2c_write_byte
(
    TxU8    reg,
    TxU8    writeVal,
    TxS32   fd
)
{
    TxU8 writeBuf[2];
    Status status = SUCCESS;
    TxU32 mask = 0xff;

    memset(writeBuf, 0, sizeof(writeBuf));

    writeBuf[0] = reg;
    writeBuf[1] = writeVal;

    if (write(fd, writeBuf, 2) != 2)
    {
        printf("%s: Error i2c writing 0x%x to address 0x%x\n",
            __FUNCTION__, writeVal, reg);
        status = I2C_WRITE_ERROR;
    }

    return status;
}

/*!
 *  Function to read byte value from device register through i2c bus specified
 *  using i2c read. This should be quivalent to i2c_smbus_read_word().
 *
 *  param[in]   reg     Device register
 *  param[in]   fd      File descriptor to i2c bus
 *  param[out]  result  Read value pointer
 *
 *  returns SUCCESS
 */
Status
i2c_read_word
(
    TxU8    reg,
    TxS32   fd,
    TxU8    *result
)
{
    TxU8 readBuf[1];
    Status status = SUCCESS;

    memset(readBuf, 0, sizeof(readBuf));

    if (read(fd, readBuf, 1) != 1)
    {
        printf("%s: Error i2c reading register 0x%x\n",
            __FUNCTION__, reg);
        status = I2C_READ_ERROR;
    }

    return status;
}

/*!
 *  Simple test to open/close i2c-1
 */
Status test1()
{
    int fd;
    Status status = SUCCESS;
    TxU8 devAddr = 0x40;

    status = i2c_open(devAddr, DEFAULT_I2C_BUS, &fd);
    if (status != SUCCESS)
    {
        printf("%s: i2c_open failed\n",
                __FUNCTION__);
        goto cleanup;
    }

    status = i2c_close(&fd);
    if (status != SUCCESS)
    {
        printf("%s: i2c_close failed\n",
                __FUNCTION__);
        goto cleanup;
    }

cleanup:
    return status;
}

/*!
 *  Simple test to open/close i2c-1 and write/read to specifc device register.
 *  TODO: Need to find correct device register
 */
Status test2(TxU32 devReg)
{
    int fd;
    Status status = SUCCESS;
    TxU8 devAddr = 0x40;
    TxU32 writeVal = 0x12345678;
    TxU32 smbusResult;
    TxU32 i2cResult;

    status = i2c_open(devAddr, DEFAULT_I2C_BUS, &fd);
    if (status != SUCCESS)
    {
        printf("%s: i2c_open failed\n",
                __FUNCTION__);
        goto end;
    }

    status = i2c_smbus_write_word(devReg, writeVal, fd);
    if (status != SUCCESS)
    {
        printf("%s: i2c_write failed\n",
                __FUNCTION__);
        goto cleanup;
    }

    status = i2c_smbus_read_word(devReg, fd, &smbusResult);
    if (status != SUCCESS)
    {
        printf("%s: i2c_read failed\n",
                __FUNCTION__);
        goto cleanup;
    }

    if (writeVal != smbusResult)
    {
        printf("%s: Test2 failed - value written and value read back is not the same!\n",
                __FUNCTION__);
        goto cleanup;
    }

    status = i2c_write_word(devReg, writeVal, fd);
    if (status != SUCCESS)
    {
        printf("%s: i2c_write failed\n",
                __FUNCTION__);
        goto cleanup;
    }

    status = i2c_read_word(devReg, fd, &i2cResult);
    if (status != SUCCESS)
    {
        printf("%s: i2c_read failed\n",
                __FUNCTION__);
        goto cleanup;
    }

    if (writeVal != i2cResult)
    {
        printf("%s: Test2 failed - value written and value read back is not the same!\n",
                __FUNCTION__);
        goto cleanup;
    }

    if (smbusResult != i2cResult)
    {
        printf("%s: Test2 failed - smbus value and i2c value is not the same!\n",
                __FUNCTION__);
        goto cleanup;
    }

    status = i2c_close(&fd);
    if (status != SUCCESS)
    {
        printf("%s: i2c_close failed\n",
                __FUNCTION__);
        goto end;
    }

cleanup:
    close(fd);
end:
    return status;
}

int main()
{
    Status status = SUCCESS;
    status = test1();
    status = test2(0xff);
    return status;
}
