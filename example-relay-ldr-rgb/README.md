
**ESPHomeController settings**

Given example contains ready configuration for platform which has connected

 - LDR
 - RGB strib
 - Relay
Wires are following:

LDR connected to pin A0 , (second wire is connected to ground via resistor 1k)

RGB strip signal wire connected to pin  GPIO 1 -D5

Relay signal wire connected to the GPIO 2 - D4 (Relay is based on the low level signal to switch ON) , If you have high level logic please change *isinvert*  property to false on the Relay service properties

System configuration as well contain Time service, which allows to execute small home automation, please adapt your time zones

*How it works*
When system properly configured by wiring setting. You can

 - Use internal web site to
        - Switch ON/OFF Relay
        - Switch ON/OFF RGB strip
        - Handle color and brigthness of RGB 
        - Set animation of RGB
        - See time value
        - See LDR value
    
    - Use Home Kit on mobile (check *homekit2mqtt_config.json* to MQTT connectio)
         - Switch ON/OFF Relay
        - Switch ON/OFF RGB strip
        - Handle color and brigthness of RGB 
        - See LDR value

System contains following Automation/Triggers (see *triggers.json*):

 - RGB strip will be ON on every day at 19:00 (with some green Color)
- RGB strip will be OFF at 22;00
- During ON the brigthness of RGB will be depend on lightness provided by LDR

RGB strip configuration contains *"manualtime":600*  which is means that after manual set of the state from Web or Home automation RGB will be back to previous state in 10 min (600 sec). if you don't need this, setup *"manualtime":0*

When power is ON:
Relay will be in state OFF
RGB will load previous state from the Spiff file is RGBStrip_state.json. This state is saved every time when you did manul control on the RGB

This is just as is, you can see that a lot of possibilities to setup what you need by only changing configuration files without changing binaries


Examples contains *homekit2mqtt_config.json*  this file or (! as part of content) you need to put to appropriate file of  homekit2mqqt configuration file, where you have installed you app
