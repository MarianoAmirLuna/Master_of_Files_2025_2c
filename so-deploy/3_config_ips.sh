#!/bin/bash

validate_ip() {
  # Regular expression pattern for IP address validation
  local ip_pattern='^([0-9]{1,3}\.){3}[0-9]{1,3}$'

  if [[ $1 =~ $ip_pattern ]]; then
    return 0  # Valid IP address
  else
    return 1  # Invalid IP address
  fi
}

# Get host IP
hostIP=$(hostname -I | cut -d' ' -f1)
echo "Host IP: $hostIP"

# Take the replace string
read -p "Enter the kernel IP: " ipKernel
read -p "Enter the memory IP: " ipMemoria
read -p "Enter the cpu IP: " ipCPU
read -p "Enter the FS IP: " ipFS

if [[ $ipKernel == "" || $ipMemoria == "" || $ipCPU == "" || $ipFS == "" ]]; then
  echo "Error: Empty IP"
  exit 0
fi

if validate_ip "$ipKernel" && validate_ip "$ipMemoria" && validate_ip "$ipCPU" && validate_ip "$ipFS"; then
  echo "All entered IP addresses are valid updating configs"
else
  echo "Error: One or more IP addresses are invalid"
  exit 0
fi

# Regexs
sKer="IP_KERNEL=[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}"
sMem="IP_MEMORIA=[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}"
sCpu="IP_CPU=[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}"
sFs="IP_FILESYSTEM=[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}"

# Find and replace IPs in .config files inside the tests folder
find ./pruebas -type f -name "*.config" | while read -r file; do
  echo "Updating file: $file"
  sed -r -i "s#$sKer#IP_KERNEL=$ipKernel#g" "$file"
  sed -r -i "s#$sMem#IP_MEMORIA=$ipMemoria#g" "$file"
  sed -r -i "s#$sCpu#IP_CPU=$ipCPU#g" "$file"
  sed -r -i "s#$sFs#IP_FILESYSTEM=$ipFS#g" "$file"
done

echo "Configuration update complete."
