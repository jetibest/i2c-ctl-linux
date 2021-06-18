#!/bin/sh
cp i2c-ctl /usr/local/bin/i2c-ctl && echo "Installation successful. Uninstall using: rm -f /usr/local/bin/i2c-ctl" >&2 || echo "Permission denied? Try to run as root: sudo $0" >&2
