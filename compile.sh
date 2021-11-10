arduino-cli compile --fqbn arduino:avr:uno lockd.ino 
arduino-cli upload -p /dev/cu.usbmodem1D131 --fqbn arduino:avr:uno lockd.ino 
pnpm run start