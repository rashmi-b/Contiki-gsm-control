/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/sim/simdriver.h"
#include "at-master.h"
#include "dev/adc.h"
#include "dev/gpio.h"
#include "dev/ioc.h"

#include "sys/ctimer.h"
#include "sys/process.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "dev/leds.h"
/*---------------------------------------------------------------------------*/
#define HWTEST_GPIO_INPUT            0
#define HWTEST_GPIO_OUTPUT           1
#define HWTEST_GPIO_OUTPUT_ODD       3
#define HWTEST_GPIO_OUTPUT_LIST      4
#define HWTEST_GPIO_OUTPUT_MASK      0x55
#define HWTEST_GPIO_OUTPUT_ODD_MASK  0xAA
/*---------------------------------------------------------------------------*/
typedef struct {
  char *name;
  uint8_t port;
  uint8_t pin;
} gpio_list_t;

struct response {
    uint16_t len;
    char data[128];
} response;

CCIF extern process_event_t sim_response_event;
PROCESS_NAME(gsm_process);

static struct at_cmd at_cmd_test;
static struct at_cmd at_cmd_response;
static struct ctimer sim_timer;
static struct ctimer sim_timer2;
static struct ctimer sim_timer3;

/*---------------------------------------------------------------------------*/
static void net_connect(void *val)
{
  at_send("AT+CSTT=\"bsnlnet\"\r\n",19);
}
/*---------------------------------------------------------------------------*/
static void get_ipaddr(void *val)
{
  at_send("AT+CIFSR\r\n",10);
}
/*---------------------------------------------------------------------------*/
static void ip_shut(void *val)
{
  at_send("AT+CIPSHUT\r\n",12);
  ctimer_set(&sim_timer3, CLOCK_SECOND * 25, net_connect, NULL);
}
/*---------------------------------------------------------------------------*/
static void radio_on(void *val)
{
  at_send("AT+CFUN=1\r\n",11);
}
/*---------------------------------------------------------------------------*/

static void
at_cmd_test_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  int i;
  
   response.len = len;
   memcpy(response.data, *data, len);

   for (i=0 ; i<len ; i++) {
   printf ("%c ", *data++);
   }
   printf ("\n");
   
   process_post(&gsm_process, sim_response_event, &response);

}
/*---------------------------------------------------------------------------*/
static void
at_cmd_response_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  int i;
  
   response.len = len;
   memcpy(response.data, data, len);

   for (i=0 ; i<len ; i++) {
   printf ("%c ", *data++);
   }
   printf ("\n");
   
   process_post(&gsm_process, sim_response_event, &response);

}
/*---------------------------------------------------------------------------*/
static void init(void)
{

  /* Initialize the driver, default is UART0 */
  at_init(1);

  at_send("AT\r\n", strlen("AT\r\n"));
  printf("Enter AT init\n");
  at_register(&at_cmd_test, PROCESS_CURRENT(), "OK", 2,2, at_cmd_test_callback);

}
/*---------------------------------------------------------------------------*/
static int modem_connect(void)
{
  printf("Enter Modem Connect\n");

  at_send("AT+CMGF=1\r\n",10);
  at_register(&at_cmd_test, PROCESS_CURRENT(), "OK", 2,2, at_cmd_test_callback);
  
  ctimer_set(&sim_timer2, CLOCK_SECOND * 15, ip_shut, NULL);
  at_register(&at_cmd_response, PROCESS_CURRENT(), "", 0,64, at_cmd_response_callback);
  
  at_register(&at_cmd_test, PROCESS_CURRENT(), "OK", 2,2, at_cmd_test_callback);
  return 0;
}
/*---------------------------------------------------------------------------*/
static int ip_connect(void)
{ 
  
  printf("Enter IP Connect\n");
  at_send("AT+CIICR\r\n",10);
  at_register(&at_cmd_response, PROCESS_CURRENT(), "", 0,64, at_cmd_response_callback);

  ctimer_set(&sim_timer, CLOCK_SECOND * 15, get_ipaddr, NULL);
  at_register(&at_cmd_response, PROCESS_CURRENT(), "", 0,64, at_cmd_response_callback);
  return 0;
  
}
/*---------------------------------------------------------------------------*/
static int ping(void)
{
  printf("Enter Ping\n");
  at_send("AT+CIPPING=\"ec2-13-58-32-98.us-east-2.compute.amazonaws.com\"\r\n",
                strlen("AT+CIPPING=\"ec2-13-58-32-98.us-east-2.compute.amazonaws.com\"\r\n"));
  at_register(&at_cmd_response, PROCESS_CURRENT(), "", 0,64, at_cmd_response_callback);

  return 0;

}
/*---------------------------------------------------------------------------*/
static int udp_connect(void)
{
  printf("Enter UDP connect\n");
 
   at_send("AT+CIPSTART=\"UDP\",\"ec2-13-58-32-98.us-east-2.compute.amazonaws.com\",\"4343\"\r\n",
                strlen("AT+CIPSTART=\"UDP\",\"ec2-13-58-32-98.us-east-2.compute.amazonaws.com\",\"4343\"\r\n"));
   at_send("AT+CIPSTATUS\r\n",14);
  return 0;
}
/*---------------------------------------------------------------------------*/
static int udp_send(void)
{
  printf("Enter UDP send\n");
  
   at_send("AT+CIPSEND\r\n",12);

   at_send("9,18,27\r\n",9);
   at_send("\r\n\x1a\r\n",7);
  return 0;

}
/*---------------------------------------------------------------------------*/
static int auto_connect(void)
{
  printf("Enter Auto connect\n");

  ctimer_set(&sim_timer, CLOCK_SECOND * 20, ip_shut, NULL);
  at_register(&at_cmd_response, PROCESS_CURRENT(), "", 0,64, at_cmd_response_callback);

  ip_connect();
  return 0;
}
/*---------------------------------------------------------------------------*/
static int tcp_send(void)
{
  printf("Enter TCP send\n");

   at_send("AT+CIPSEND\r\n",12);

   at_send("9,18,27\r\n",9);
   at_send("\r\n\x1a\r\n",7);
  return 0;
 
}
/*---------------------------------------------------------------------------*/
static int sim_reset(void)
{
  printf("Enter SIM reset\n");
  
  at_send("AT+CFUN=0\r\n",11);
  at_register(&at_cmd_response, PROCESS_CURRENT(), "", 0,64, at_cmd_response_callback);

  ctimer_set(&sim_timer, CLOCK_SECOND * 30, radio_on, NULL);
  at_register(&at_cmd_response, PROCESS_CURRENT(), "", 0,64, at_cmd_response_callback);
  return 0;
}
/*---------------------------------------------------------------------------*/
const struct sim_driver SIM_DRIVER = {
  init,
  modem_connect,
  ip_connect,
  ping,
  udp_connect,
  udp_send,
  auto_connect,
  tcp_send,
  sim_reset,
};
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
