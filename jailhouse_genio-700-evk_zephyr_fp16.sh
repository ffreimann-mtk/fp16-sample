modprobe jailhouse
jailhouse enable /usr/share/jailhouse/cells/genio-700-evk.cell
jailhouse cell create /usr/share/jailhouse/cells/genio-700-evk-zephyr.cell
jailhouse cell load zephyr /root/zephyr.bin -a 0x8000
jailhouse cell start zephyr

