from hermes_tools import terminal

# Vamos simular o montador manualmente para LE_len1
# Linhas relevantes do .erg (123-130):
# :LE_len1
# LOAD 40        ; A = payload[0]
# LOAD 48        ; B = 0x00
# ADD            ; R = payload[0] + 0 = payload[0]
# STORE_IND 24   ; raw_out[0] = R
# JMP NA_ERR_CHECK

# Simular o montador:
# Passagem 0: registar rótulos
#   LE_len1: pos = ?
#   LE_len2: pos = ?
#   NA_ERR_CHECK: pos = ?

# Vamos descobrir os offsets dos rótulos
r = terminal("cd /c/Users/jssim/aarao/tiffany/can && python3 sim_montador.py")
print(r["output"])