import re
import sys

# Hole den Tag-Namen ohne 'v' Prefix
version = sys.argv[1].lstrip('v')

# Aktualisiere die Version in main.cpp
with open('src/main.cpp', 'r') as file:
    content = file.read()

updated_content = re.sub(r'#define FW_VERSION ".*?"', f'#define FW_VERSION "{version}"', content)

with open('src/main.cpp', 'w') as file:
    file.write(updated_content)

# Aktualisiere die Version in manifest.json
with open('web/manifest.json', 'r') as file:
    content = file.read()

updated_content = re.sub(r'"version": ".*?"', f'"version": "{version}"', content)

with open('web/manifest.json', 'w') as file:
    file.write(updated_content)

print(f"Version updated to {version}")