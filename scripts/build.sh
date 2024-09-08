arduino-cli compile --fqbn m5stack:esp32:m5stack_core2 --build-property build.extra_flags="-DESP32 -DMY_SSID=\"$WIFI_SSID\" -DMY_PASSWORD=\"$WIFI_PASSWORD\" -DMY_APIKEY=\"$OPENAI_API_KEY\""
