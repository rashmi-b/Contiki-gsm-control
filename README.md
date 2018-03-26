# Contiki-gsm-control
Added a stand alone GSM remote control project when sim module is integrated with motes such as Zolertia's RE-mote. 
The GSM process is added inside border-router.c as a separate thread. It can be disabled when not required.
AT command have been abstracted and user can send comands from host such as sim_init, sim_modem_connect, sim_ipconnect, sim_udp_send, etc.
The AT command send code was pulled from 
