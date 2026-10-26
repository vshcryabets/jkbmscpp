# Bluetooth Debugging for ARM/Linux Systems

## System-Level Fixes for Orange Pi Zero 3 / Armbian

### 1. Check Bluetooth Service Status
```bash
sudo systemctl status bluetooth
sudo systemctl restart bluetooth
```

### 2. Increase Bluetooth Buffers
```bash
# Check current settings
cat /proc/sys/net/core/rmem_default
cat /proc/sys/net/core/rmem_max

# Increase buffers (add to /etc/sysctl.conf)
sudo sysctl -w net.core.rmem_default=262144
sudo sysctl -w net.core.rmem_max=16777216
```

### 3. Bluetooth Configuration
Edit `/etc/bluetooth/main.conf`:
```ini
[General]
# Increase connection intervals for stability
MinConnectionInterval=6
MaxConnectionInterval=12
ConnectionLatency=0
SupervisionTimeout=300

# Enable experimental features if needed
Experimental=true
```

### 4. Power Management
Disable Bluetooth power management:
```bash
echo 'ACTION=="add", SUBSYSTEM=="usb", ATTRS{idVendor}=="*", ATTRS{idProduct}=="*", TEST=="power/control", ATTR{power/control}="on"' | sudo tee /etc/udev/rules.d/50-usb-power.rules
```

### 5. Debug Commands
```bash
# Monitor Bluetooth events
sudo btmon

# Check adapter status
hciconfig -a
hcitool dev

# Reset adapter if needed
sudo hciconfig hci0 down
sudo hciconfig hci0 up
```

## Code-Level Solutions

### 1. Retry Logic
Add retry mechanism for failed requests

### 2. Connection Quality Check
Verify RSSI and connection parameters before critical operations

### 3. Fragmentation Handling
Handle responses that might come in multiple packets on ARM systems