/****************************************************************************
*
* Copyright (c) 2015 Wi-Fi Alliance
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/

/*
 *  File: wfa_serial.c 
 *  library functions for serial port,
 *  They are common library to open, close and send receive over serial port.
 *
 *  Revision History:
 *
 *
 */



//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <termios.h>
#include <stdio.h>

#include "wfa_types.h"
#include "wfa_serial.h"
#include "wfa_debug.h"

/************ For Debug enable below *************************/ 

#define DEBUG_SERIAL
#ifdef DEBUG_SERIAL
extern unsigned short wfa_defined_debug;
#endif


int wfa_getBaudeRate(int baudrate)
{
    switch (baudrate){
    case 0:
        return -1;
    case 2400:
        return B2400;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    case 460800:
        return B460800;
    case 921600:
        return B921600;
    case 1000000:
        return B1000000;
    case 1152000:
        return B1152000;
    case 1500000:
        return B1500000;
    case 2000000:
        return B2000000;
    case 2500000:
        return B2500000;
    case 3000000:
        return B3000000;
    case 3500000:
        return B3500000;
    case 4000000:
        return B4000000;
    default:
        printf("baud not in list, default 115200");
        return B115200;
    }
}

/**
* @brief       Open serial port for communicaiton 
*
*
*
* @param    t_ifaceHandle *handle, handle of the structure 
* @param    char *device  device name, port to be used
* @param    int baudRate  buadrate for the port
*
*
* @return   WFA_SUCCESS, WFA_ERROR.
*/



int wfaOpenSerial(t_ifaceHandle *handle, char *device, int baudRate)
{
    int fd= -1;
    int baud;
#ifdef DEBUG_SERIAL
    DPRINT_INFO(WFA_OUT,"%s \n",__func__);
#endif
    if((handle == NULL) || (device == NULL))
        return WFA_ERROR; //ERROR_NULL_POINTER;

    fd = open(device, O_RDWR | O_NOCTTY ); 
    if (fd <0) {
        DPRINT_INFO(WFA_ERR,"Not able to open serial port\n");
        perror(device);
        exit(1);
    }

    handle->if_attr.serial.fd = fd;
#ifdef DEBUG_SERIAL
    DPRINT_INFO(WFA_OUT,"baud rate %d \n",baudRate);
#endif
    if(( baud =  wfa_getBaudeRate( baudRate)) == -1)
        return WFA_ERROR;

    /* save current port settings */
    if (tcgetattr(fd,&(handle->if_attr.serial.oldtio)) != 0) 
    {
       DPRINT_INFO(WFA_ERR,"error %d from tcgetattr", errno);
        return WFA_ERROR;
    }

    bzero(&(handle->if_attr.serial.newtio),sizeof(handle->if_attr.serial.newtio));
    /* Set the buad rate for the serial port */

    handle->if_attr.serial.newtio.c_cflag |= baud;
    handle->if_attr.serial.newtio.c_cflag |= (CLOCAL | CREAD);
    handle->if_attr.serial.newtio.c_cflag |= CRTSCTS;
    /*No parity 8N1 */
    handle->if_attr.serial.newtio.c_cflag |= CS8;

    /* setting flow control*/
    /* shut off xon/xoff ctrl */
    handle->if_attr.serial.newtio.c_iflag &= ~(IXON | IXOFF | IXANY); 
    handle->if_attr.serial.newtio.c_oflag = 0; /*&= ~OPOST;*/

    /* no signaling chars, no echo, */
    handle->if_attr.serial.newtio.c_cc[VTIME]    = 5;   /* inter-character timer */
    handle->if_attr.serial.newtio.c_cc[VMIN]     = 250;  /* blocking read until 250 chars received */
#ifdef DEBUG_SERIAL
    DPRINT_INFO(WFA_OUT,"handle->if_attr.serial.newtio.c_cflag =%x\n",handle->if_attr.serial.newtio.c_cflag );
#endif
    /* clean the port*/
    tcflush(fd, TCIFLUSH);
    if(tcsetattr(fd, TCSANOW, &(handle->if_attr.serial.newtio)) != 0)
    {
        DPRINT_INFO(WFA_ERR,"error %d from tcsetattr", errno);
        return WFA_ERROR;
    }
    return WFA_SUCCESS;
}



/**
* @brief       Close serial port for communicaiton 
*
*
* @param    t_ifaceHandle *handle, handle of the structure 
*
* @return   WFA_SUCCESS, WFA_ERROR.
*/

int wfaCloseSerial(t_ifaceHandle *handle)
{
#ifdef DEBUG_SERIAL
    DPRINT_INFO(WFA_OUT,"%s \n",__func__);
#endif
    if (handle == NULL)
        return WFA_ERROR;

    if (handle->if_attr.serial.fd <0) {
       return -1;
    }
    /*restore back old settings*/
    tcsetattr(handle->if_attr.serial.fd ,TCSANOW,&handle->if_attr.serial.oldtio);
    close(handle->if_attr.serial.fd );

    handle->if_attr.serial.fd = -1;
    return WFA_SUCCESS;
}


/**
* @brief       Send data over serial port 
*
*
* @param    t_ifaceHandle *handle, handle of the structure 
* @param    char *buffer  buffer to be sent
* @param    int bufferLen  length of the buffer to be sent
*
* @return   WFA_SUCCESS, WFA_ERROR.
*/

int wfaSerialSend(t_ifaceHandle *handle, char *buffer, int bufferLen)
{
#ifdef DEBUG_SERIAL
    DPRINT_INFO(WFA_OUT,"%s \n",__func__);
#endif
    if((handle == NULL) || (buffer == NULL))
        return WFA_ERROR; //ERROR_NULL_POINTER;

    if (handle->if_attr.serial.fd<0) {
       return -1;
    }

    /*send the buffer serial port*/
    if (write(handle->if_attr.serial.fd, buffer, bufferLen) < 0)
		return WFA_FAILURE;
    /* 100 micro sec per char in case of 115200 baud*/
    usleep((bufferLen + 25) *200);  
    return WFA_SUCCESS;
}


/**
* @brief       Receive data over serial port 
*
*
*
* @param    t_ifaceHandle *handle, handle of the structure 
* @param    char *buffer  buffer for recv data
* @param    int bufferLen  size of the receive data buffer
*
* @return   WFA_SUCCESS, WFA_ERROR.
*/

int wfaSerialRecv(t_ifaceHandle *handle, char *buffer, int bufferLen, int *recevLen)
{
    int numByte = 0;
    int byteLeft = 0;
    int bytes;
    unsigned int delayUs = 0;

#ifdef DEBUG_SERIAL
        DPRINT_INFO(WFA_OUT,"%s \n",__func__);
#endif

    if(handle == NULL || buffer == NULL )
    {   
        DPRINT_INFO(WFA_ERR,"handle or buffer error \n");
        return WFA_ERROR;
    }

    if (handle->if_attr.serial.fd<0) {
        DPRINT_INFO(WFA_ERR,"Serial fd error\n");
       return -1;
    }
    /* calculating delay to receive some byte, 200 
    based on baud rate but at least 1000 usec*/
    delayUs = (10000000 * 200) / handle->if_attr.serial.baudrate + 1000;
    
    
    /*send the buffer serial port*/
    byteLeft = bufferLen;
    FD_ZERO(&handle->fds);
    FD_SET(handle->if_attr.serial.fd, &handle->fds);
    if (select(handle->if_attr.serial.fd+1, &handle->fds, NULL, NULL, &handle->timeout) > 0)
    {
        numByte = read(handle->if_attr.serial.fd, buffer, bufferLen);
    }
    else
    {
        numByte = -1;
        DPRINT_INFO(WFA_ERR,"Timeout on serial read\n");
    }
    if(numByte < 0) {
        DPRINT_INFO(WFA_ERR,"serial read fail\n");
        *recevLen = numByte;
        return WFA_ERROR;
    }
    if(byteLeft >= numByte)
        byteLeft -= numByte;
    else
        byteLeft = 0;

    while(byteLeft) {
        wUSLEEP(delayUs);
        ioctl(handle->if_attr.serial.fd, FIONREAD, &bytes);
#ifdef DEBUG_SERIAL
        DPRINT_INFO(WFA_OUT,"Bytes Availble %d \n",bytes);
#endif
        if(!bytes ) /* proceed if data still in buffer */
           break;
        numByte = read(handle->if_attr.serial.fd, &buffer[numByte], byteLeft);
        if(byteLeft >= numByte)
            byteLeft -= numByte;
        else
            byteLeft = 0;
    }

    *recevLen = bufferLen - byteLeft;
#ifdef DEBUG_SERIAL
    DPRINT_INFO(WFA_OUT,"Data receive %d \n",*recevLen);
#endif
    return WFA_SUCCESS;
}


