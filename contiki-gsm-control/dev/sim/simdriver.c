/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Initialiation file for the sim driver selection 
 * \author
 *        
 */

#include "dev/sim/simdriver.h"

enum states{
SIM_OFF,
SIM_READY,
SIM_CONNECTED,
SIM_DATA_READY, /* Got IP address */
SIM_PINGED,
SIM_UDP_CONNECTED,
SIM_UDP_SENT,
SIM_UDP_CLOSE
}state;

enum events{
SIM_INIT,
SIM_CONNECT,
SIM_DATA_CONNECT,
SIM_PING,
SIM_UDP_CONNECT,
SIM_UDP_SEND,
SIM_CLOSE,
SIM_CURRENT_STATE,
SIM_RESET
}cmd;

/*---------------------------------------------------------------------------*/
void state_reset(void)
{
  state = SIM_OFF;
}
/*---------------------------------------------------------------------------*/
void auto_connect(void)
{
     SIM_DRIVER.auto_connect();
}
/*---------------------------------------------------------------------------*/
unsigned short sim_command_callback(unsigned short cmd)
{
int error = -1;


   if (cmd == SIM_RESET) {
		SIM_DRIVER.sim_reset();
        	state = SIM_OFF;
	} else if (cmd == SIM_CURRENT_STATE) {
          	return state;
	}
         
   switch (state) {
       
          case SIM_OFF:
                    if (cmd == SIM_INIT) {
                        SIM_DRIVER.init();
                        error = 0;
		        state = SIM_READY;
                      } 
                     break;
          case SIM_READY:
		    if (cmd == SIM_CONNECT) {
		       SIM_DRIVER.modem_connect();	
 		       error = 0;
		       state = SIM_CONNECTED;
		     } else {
		       SIM_DRIVER.init();
		       SIM_DRIVER.modem_connect();
		       state = SIM_CONNECTED;
		     } 
                   break;
          case SIM_CONNECTED:
		    if (cmd == SIM_DATA_CONNECT) {
		       SIM_DRIVER.ip_connect();
 		       error = 0;
		       state = SIM_DATA_READY;
		     } else {
		       SIM_DRIVER.modem_connect();
		       SIM_DRIVER.ip_connect();
		       state = SIM_DATA_READY;
		     }
          case SIM_DATA_READY:
		    if (cmd == SIM_PING) {
		       SIM_DRIVER.ping();
 		       error = 0;
		       state = SIM_PINGED;
		     }
          case SIM_PINGED:
		    if (cmd == SIM_UDP_CONNECT) {
		       SIM_DRIVER.udp_connect();
 		       error = 0;
		       state = SIM_UDP_CONNECTED;
		     } else if (cmd == SIM_PING) {
		       SIM_DRIVER.ping();
		       state = SIM_PINGED;
		     }
          case SIM_UDP_CONNECTED:
		    if (cmd == SIM_UDP_SEND) {
		       SIM_DRIVER.udp_send();
 		       error = 0;
		       state = SIM_UDP_SENT;
          	     } else if (cmd == SIM_PING) {
                       SIM_DRIVER.ping();
		       state = SIM_UDP_CONNECTED;
                     } else {
		       SIM_DRIVER.udp_connect();
                       state = SIM_UDP_CONNECTED;
		     } 
          case SIM_UDP_SENT:
		    if (cmd == SIM_UDP_SEND) {
		       SIM_DRIVER.udp_send();
		       error = 0;
		       state = SIM_UDP_SENT;
		     }  else if (cmd == SIM_PING) {
                       SIM_DRIVER.ping();
		       state = SIM_UDP_CONNECTED;
                     }
        
       }

return error;
}
