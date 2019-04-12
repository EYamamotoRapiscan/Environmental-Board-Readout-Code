# Environmental-Board-Readout-Code
Sample code to read out environmental monitor boards, speed messages, and send to SC880 via serial

Note this requires a firmware update since the bbone puts guard characters around messages and the SC880 parses these to ensure the complete message is received uncorrupted.

Systemd info:

[Unit]
Description=Rapiscan Environmental Monitor
After=network.target

[Service]
ExecStart=/home/debian/Desktop/EmMessageController -10 1 2 3 4 5 6
Restart=always
RestartSec=0

[Install]
WantedBy=default.target


[Unit]
Description=Rapiscan SCxxx Serial Link
After=network.target

[Service]
ExecStart=/home/debian/Desktop/SCxx0CommLink 1
Restart=always
RestartSec=0

[Install]
WantedBy=default.target



[Unit]
Description=Rapiscan PLC Monitor
After=network.target

[Service]
ExecStart=/home/debian/Desktop/PLCSerialLink
Restart=always
RestartSec=0

[Install]
WantedBy=default.target
