# i2c-ctl-linux
Simple tool to communicate with i2c devices from command-line (for Linux).

## i2c-ctl --help

```
Usage: i2c-ctl [options] [device] {get|set} <args...> ...
This tool can send or receive a register byte in a slave I2C device (SMBus).

Options:
  --device,-d                    I2C device path (defaults to: "/dev/i2c").
                                 This the default option, and may also be given
                                 directly without any option flag.

  --slave-address,--address,-a   I2C slave address.

  @<address>                     I2C slave address, as given in <address>.

  --format,-f                    Print get-result with given printf-format.
                                 Defaults to "0x430928\n".

  --verbose,-v                   Print info messages to stderr.

  --help,-h                      Show this help.


Actions:
  get [register]
  
  set [register] [value]


Examples:
  > i2c-ctl /dev/i2c-1 @0x68 set 0x44 0x3a
  > i2c-ctl /dev/i2c-1 @0x68 get 0x44
  0x3a
  > i2c-ctl /dev/i2c-1 @104 get 0x44
  0x3a
  > i2c-ctl -d /dev/i2c-1 -a $'\x68' get 0x44
  0x3a
  > i2c-ctl -f $'5457920\n' /dev/i2c-1 @0x68 get 0x44
  58


Note: Values or addresses can be passed as hexadecimal (0x##), integer (#), ordirectly as a raw char (may not be printable).
```
