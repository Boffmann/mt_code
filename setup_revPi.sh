sudo su
apt update

echo "Installing OpenSplice dependencies..."
sleep 3
apt install -y flex bison perl gawk cmake vim

echo "Downloading OpenSplice Source code..."
sleep 3
wget https://github.com/ADLINK-IST/opensplice/archive/OSPL_V6_9_190925OSS_RELEASE.zip

echo "Unzipping OpenSplice Sourcecode..."
sleep 3
unzip OSPL_V6_9_190925OSS_RELEASE.zip -d ospl

echo "Compiling OpenSplice..."
sleep 3
cd ospl/OSPL_V6_9_190925OSS_RELEASE
source ./configure & make

echo "Installing OpenSplice..."
make install
source /home/ospl/OSPL_V6_9_190925OSS_RELEASE/install/HDE/armv7l.linux-dev/release.com
echo "source /home/ospl/OSPL_V6_9_190925OSS_RELEASE/install/HDE/armv7l.linux-dev/release.com" >> /home/pi/.bashrc

ip_addr=$(ip addr show eth0 | awk '$1 == "inet" {gsub(/\/.*$/, "", $2); print $2}')
sed "s/<NetworkInterfaceAddress>.*<\/NetworkInterfaceAddress>/<NetworkInterfaceAddress>$ip_addr<\/NetworkInterfaceAddress>/" $OSPL_HOME/etc/config/ospl.xml

sed -i '$ d' /etc/rc.local

echo "ip_addr=\$(ip addr show eth0 | awk '\$1 == \"inet\" {gsub(/\/.*$/, \"\", \$2); print \$2}')" >> /etc/rc.local
echo "sed \"s/<NetworkInterfaceAddress>.*<\\/NetworkInterfaceAddress>/<NetworkInterfaceAddress>\$ip_addr<\\/NetworkInterfaceAddress>/\" \$OSPL_HOME/etc/config/ospl.xml" >> /etc/rc.local
echo "exit 0" >> /etc/rc.local
