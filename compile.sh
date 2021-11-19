source ./CONFIG.env
arduino-cli compile --fqbn $BOARD_TYPE 4BY4.ino 
arduino-cli upload -p $BOARD_PORT --fqbn $BOARD_TYPE 4BY4.ino