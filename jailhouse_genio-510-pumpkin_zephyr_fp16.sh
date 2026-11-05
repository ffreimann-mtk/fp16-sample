modprobe jailhouse
jailhouse enable /usr/share/jailhouse/cells/genio-510-pumpkin.cell
jailhouse cell create /usr/share/jailhouse/cells/genio-510-pumpkin-zephyr.cell
jailhouse cell load zephyr /root/zephyr.bin -a 0x8000
jailhouse cell start zephyr

