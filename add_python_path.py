# add_python_path.py
import os
import sys

# *******************************************************************
# ERSETZEN SIE DIESEN PFAD DURCH DEN VON IHNEN GEFUNDENEN PFAD!
# Dieser Pfad muss exakt mit dem Ergebnis von 'python3 -m site --user-site' übereinstimmen.
PYTHON_USER_SITE = "/Users/jan/Library/Python/3.14/lib/python/site-packages"
# *******************************************************************

# Fügt den Benutzer-Site-Paketpfad zum Build-Environment hinzu.
# Dies ermöglicht es PlatformIOs internen Python-Tools (wie esptool.py),
# das extern installierte Modul 'intelhex' zu finden.

if not os.environ.get('PYTHONPATH'):
    os.environ['PYTHONPATH'] = PYTHON_USER_SITE
else:
    # Fügt den Pfad am Anfang hinzu, falls PYTHONPATH bereits existiert
    os.environ['PYTHONPATH'] = PYTHON_USER_SITE + os.pathsep + os.environ['PYTHONPATH']

print(f"INFO: PYTHONPATH wurde für den Build auf '{os.environ['PYTHONPATH']}' erweitert.")
