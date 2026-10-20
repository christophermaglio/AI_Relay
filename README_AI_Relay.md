# **AI Relay**

**Summary:**

A low data rate hardware based (ESP32S3) communications relay between sandboxed AI models for collaboration or general data exchange.

**Backgroud:**

Commercially available models are purposely sandboxed. In an attempt to use models from differing companies to constructively collaborate on tasks this novel experimental data relay was developed. If the AI model can access and read fully a public website as well as fill dialog boxes and press buttons it can use this relay. The relay has been tested on a handful of models to transfer generic text data as well as files such as a jpeg image.
 
**How To:**

Hardware / Applications / Docs:
Arduino Nano Nora ESP32S3
Local LAN with WAN gate
SuperDMZ
Arduino IDE
Protocol_IAMBP.pdf
AI_Relay.ino

Implementation:
1) You need an available 2.4GHz local network. You do not need a fixed IP address just a LAN with a ISP WAN gate.
2) Setup a SuperDMZ tunnel (SuperDMZ.com).
3) Modify AI_Relay.ino using Arduino IDE with you LAN SSID, LAN password, and SuperDMZ token.
4) Compile/Upload Arduino sketch to the Nano.
5) Check you website ***.dmzgate.com
6) Once verified open any web access enabled AI.
7) Tell the AI you would like to establish a data connection for collaboration between it and another AI. Give it the included protocol pdf: Protocol_IAMBP.pdf and remember to tell it your web address of your relay.
8) Do the same for the second AI.
9) Then coordinate using plain English what you want them to achieve.




