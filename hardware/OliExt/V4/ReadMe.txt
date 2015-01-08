Please have a look at picture for A20 board modification otherwise you cannot power the board.

Upgrade of OliExt V3

1 - Pin PI14 used in General and CAN so changed this, now PI14 is for General and PI10 for CAN
2 - Change Uart4 to Uart3 and add CTS/RTS signals (Uart 0, 2, 3, 4, 6, 7 are used) Using SubD9 Female and A20 as DTE
3 - Uart4 used for EnergyMeter
4 - COM1 can be use for either Uart0 or Uart2, Uart2 with CTS/RTS signals. SW1 to select used Uart 
5 - Change pin assigned for LED display (PG0-PG5, PI0, PI1)
6 - Fix issues on SIM 900 module RI, RXT and TXD are 2.8V signals, and use TXS0104 because TXB0104 is not good for UART 