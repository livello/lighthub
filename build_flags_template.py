from platformio import util
Import("env", "projenv")

def setBuildFlag(buildFlag, value):
    projenv.Append(CPPDEFINES=(buildFlag, value))


def setBuildFlag(buildFlag):
    projenv.Append(CPPDEFINES=(buildFlag))

setBuildFlag("MY_CONFIG_SERVER","lazyhome.ru")
setBuildFlag("WATCH_DOG_TICKER_DISABLE")
setBuildFlag("USE_1W_PIN=12")
setBuildFlag("SD_CARD_INSERTED")
setBuildFlag("SERIAL_BAUD","115200")
setBuildFlag("Wiz5500")
setBuildFlag("DISABLE_FREERAM_PRINT")
setBuildFlag("CUSTOM_FIRMWARE_MAC","de:ad:be:ef:fe:00")
setBuildFlag("DMX_DISABLE")
setBuildFlag("MODBUS_DISABLE")
setBuildFlag("OWIRE_DISABLE")
setBuildFlag("AVR_DMXOUT_PIN","18")
setBuildFlag("LAN_INIT_DELAY","2000")
setBuildFlag("CONTROLLINO")
setBuildFlag("ESP_WIFI_AP","MYAP")
setBuildFlag("ESP_WIFI_PWD","MYPWD")
setBuildFlag("WIFI_MANAGER_DISABLE")
setBuildFlag("DHT_DISABLE")
setBuildFlag("DHCP_RETRY_INTERVAL","60000")
setBuildFlag("RESTART_LAN_ON_MQTT_ERRORS")
setBuildFlag("RESET_PIN","5")