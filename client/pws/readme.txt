
# set time
tzselect
ln -sf /usr/share/zoneinfo/Europe/Stockholm /etc/localtime

shutdown -r now

timedatectl


# clean pacman
pacman -Syy
pacman -Scc
pacman -Suu

find /etc -regextype posix-extended -regex ".+\.pac(new|save|orig)" 2> /dev/null



# wifi
comment #IP=dhcp

IP=static
Address=('192.168.1.45/24')
Gateway=('192.168.1.1')
DNS=('192.168.1.1')

change ESSID
change Key
