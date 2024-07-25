## Siemens S7 PLC message simulator

This program parses PLC Data Blocks from .AWL file and sends corresponding datagrams over UDP.  Message can be read from user specified JSON file and it can be modified on the fly via GUI or Runtime Configuration Interface.

### Building:

```shell
qmake && make
```

### Usage:

```
Usage: ./debug/plcsimul [options]

Siemens S7 PLC Message Simulator.
 
Default values in braces.

Options:
  -h, --help                       Displays help on commandline options.
  --help-all                       Displays help including Qt specific options.
  -v, --version                    Displays version information.
  --file-ros <file>                User Setting Json for ROS to Video Server
  --file-crane <file>              User Setting Json for Crane to Video Server
  --file-yard <file>               User Setting Json for Yard to Video Server
  -c, --cli                        Start in command line mode
  -j, --json <ros|crane|yard>      Print user settings template
  --full-logging                   Enable full logging
  --logfile <string>               File for logs (plcsimul.log)
  --ip-videoserver <ip>            IP of Video Server (127.0.0.1)
  --port-videoserver-ros <port>    ROS PLC Port on Video Server (2050)
  --port-videoserver-crane <port>  Crane PLC Port on Video Server (2051)
  --port-videoserver-yard <port>   Yard PLC Port on Video Server (2052)
  --ip-ros <ip>                    IP of simulated ROS PLC (127.0.0.1)
  --ip-crane <ip>                  IP of simulated Crane PLC (127.0.0.1)
  --ip-yard <ip>                   IP of simulated Yard PLC (127.0.0.1)
  --port-ros <port>                Port of simulated ROS PLC (2000)
  --port-crane <port>              Port of simulated Crane PLC (2001)
  --port-yard <port>               Port of simulated Yard PLC (2002)
  --interval-out <interval in ms>  Interval of sending messages (50 ms)
  --db-out-ros <.AWL file>         Datablock PLC TO VIDEOSERVER
                                   (:/DB311_ROS_PLC_to_AV_ROS_V2_1.db)
  --db-in-ros <.AWL file>          Datablock VIDEOSERVER TO PLC
                                   (:/DB344_AV_to_ROS_PLC_ROS_V2_1.db)
  --db-out-crane <.AWL file>       Datablock CRANE TO VIDEOSERVER
                                   (:/DB312_ROS_PLC_to_AV_Crane_V2_1.db)
  --db-in-crane <.AWL file>        Datablock VIDEOSERVER TO CRANE
                                   (:/DB345_AV_to_ROS_PLC_Crane_V2_1.db)
  --db-out-yard <.AWL file>        Datablock YARD TO VIDEOSERVER
                                   (:/DB313_YARD_PLC_to_AV_Yard_V2_1.db)
  --db-in-yard <.AWL file>         Datablock VIDEOSERVER TO YARD
                                   (:/DB346_AV_to_YARD_PLC_Yard_V2_1.db)
  --msgcounter-ros <string>        Counter tag in db-out-ros (WATCHDOG)
  --msgcounter-crane <string>      Counter tag in db-out-crane (WATCHDOG)
  --msgcounter-yard <string>       Counter tag in db-out-yard (WATCHDOG)
  --config-server-ros <address>    Address for ros runtime configuration
                                   interface (127.0.0.1:12000)
  --config-server-crane <address>  Address for crane runtime configuration
                                   interface (127.0.0.1:12001)
  --config-server-yard <address>   Address for yard runtime configuration
                                   interface (127.0.0.1:12002)


```

### Settings File

User specified settings are given to the program in JSON. Given JSON  can be just a subset of all values.  

Example of settings file:

```json
{
   "HEADER": {
      "MESSAGE_ID": 1,
      "FIRST_INDEX": 1,
      "LAST_INDEX": 1,
   },
   "CONTROL_PLACES[1]": {
      "TYPE": 1,
      "ID": 13,
      "SELECTION_TYPE": 0,
      "SELECTION_ID": 1,
      "OPERATOR_ID": 6,
      "AV_FUNCTION_PARAMETER": 0,
      "GENERAL_DATA.ACTIVE_CONTROLS": false,
      "GENERAL_DATA.CROSSHAIR_VISIBLE": false,
      "MONITOR_REQUESTS[1].SCREEN_1_VIEW_ID": 1,
      "MONITOR_REQUESTS[1].SCREEN_2_VIEW_ID": 13,
      "MONITOR_REQUESTS[1].SCREEN_3_VIEW_ID": 7,
      "MONITOR_REQUESTS[1].SCREEN_4_VIEW_ID": 2,
      "CONTROLLERS[1].CONTROLLER_VALUE_1": 0,
      "CONTROLLERS[1].CONTROLLER_VALUE_2": 0,
      "CONTROLLERS[1].BUTTONS[1]": false,
      "CONTROLLERS[1].BUTTONS[2]": true,
      "CONTROLLERS[2].CONTROLLER_VALUE_1": 254,
      "CONTROLLERS[2].CONTROLLER_VALUE_2": 0,
      "CONTROLLERS[2].BUTTONS[1]": true,
      "CONTROLLERS[2].BUTTONS[2]": false,
   }
}

```

Each simulation (ros, crane, yard) uses their own user settings file.

### Getting JSON template for User Settings.

Command line parameter ```-j <ros|crane|yard>``` prints out the JSON template for given outgoing data block. On Linux user can capture that print out.

```bash
./plcsimul -j <ros|crane|yard> > name_of_the_file.json
```

### Logging

This program implements two logging modes, partial logging and full logging.
Partial logging only writes log message when value changes.
Full logging logs all incoming messages in their full form.

Logs are only written in to a file and program uses partial logging by default.

Example of log messages:

```
14:21:18.266 :: HEADER : MSG_Counter : 0 -> 238
14:21:18.267 :: HEADER : AUDIO_SERVER_OK : false -> true
14:21:18.267 :: HEADER : notused_6_1 : false -> true
14:21:18.267 :: HEADER : notused_6_2 : false -> true
14:21:18.268 :: HEADER : notused_6_4 : false -> true
14:21:18.446 :: HEADER : MSG_Counter : 238 -> 0
```

Format:

```
Time Stamp :: Struct Name : Value Name : Old Value -> New Value
```



### Runtime Configuration Interface

All three outgoing plcsimulations (ros, crane, yard) offer a runtime configuration HTTP API. Address of this API can be set with `--config-server-*` command line parameter. Giving an empty address ("") with `--config-server-*` flag disables API. 

```
HTTP API User Guide:

Apply user settings from new file in runtime:
Make a HTTP POST request to HTTP API Address with message body consisting of "file=/path/to/user_settings_json".
```

#### Good to notice:

When using relative paths, paths should be relative to the working directory where plcsimul was run.
Existing values are not automatically cleared when applying user settings in runtime.



### Notes on reading data block to internal data structure

Data block files use structures as a way to organize data. These files can have nested structures.
When handling data block files, the program "flattens" these structures to the point where there is no
more nested structures. Names of the structures are added to the name of value.

Example:

```
From:
structure_name
├── value0
├── value1
└── sub_structure_name
│   ├── sub_value0
│   ├── sub_value1
│   └── sub_value2
└──value2

To:
Structure Name
├── value0
├── value1
├── sub_structure_name.sub_value0
├── sub_structure_name.sub_value1
├── sub_structure_name.sub_value2
└──value2
```

If data block contains array of substructures, substructure names are appended with index indicator `[n]`.

Example:

```
From:
structure_name
├── value0
├── value1
└── sub_structure_name	//first array
│   ├── sub_value0
│   ├── sub_value1
│   └── sub_value2
└── sub_structure_name	//second array
│   ├── sub_value0
│   ├── sub_value1
│   └── sub_value2
└──value2

To:
Structure Name
├── value0
├── value1
├── sub_structure_name[1].sub_value0
├── sub_structure_name[1].sub_value1
├── sub_structure_name[1].sub_value2
├── sub_structure_name[2].sub_value0
├── sub_structure_name[2].sub_value1
├── sub_structure_name[2].sub_value2
└──value2
```

