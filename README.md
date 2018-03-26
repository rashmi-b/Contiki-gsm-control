# Contiki-gsm-control
Added a stand alone GSM remote control project when sim module is integrated with motes such as Zolertia's RE-mote. 
The GSM process is added inside border-router.c as a separate thread. It can be disabled when not required.
AT command have been abstracted and user can send comands from host such as sim_init, sim_modem_connect, sim_ipconnect, sim_udp_send, etc.
The AT command send code pulled from Zolertia's at-test-master by Antonio Lignan <alinan@zolertia.com>
This version adapted for SIM900 module integrated with Zolertia's RE-mote (Zoul platform). 
contiki/dev/sim/ contains at command set and FSM to handle commands from host
Custom host code to send command to activate gsm module can be found at host-side.c
