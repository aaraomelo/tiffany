#!/bin/bash
# instala_canal_remoto.sh — corre NA Patria (scp + ssh). Backup nginx, sobe o loopback, reload.
set -euo pipefail
cp -a /etc/nginx/sites-available/goldenkingdom.patriatechnology.com \
  "/root/gk.nginx.bak.$(date -u +%Y%m%d%H%M)"
cp /tmp/goldenkingdom.conf /etc/nginx/sites-available/goldenkingdom.patriatechnology.com
mkdir -p /opt/canal
cp /tmp/serve_canal.mjs /tmp/canal_loopback.mjs /opt/canal/
cat > /etc/systemd/system/tiffany-canal.service <<'EOF'
[Unit]
Description=Tiffany canal WSS fan-out
After=network.target

[Service]
Type=simple
WorkingDirectory=/opt/canal
ExecStart=/usr/bin/node /opt/canal/serve_canal.mjs
Environment=TIFFANY_CANAL_WS=47314
Environment=TIFFANY_CANAL_BIND=127.0.0.1
Restart=always
RestartSec=2

[Install]
WantedBy=multi-user.target
EOF
systemctl daemon-reload
systemctl enable --now tiffany-canal
systemctl restart tiffany-canal
nginx -t
systemctl reload nginx
echo "--- canal.patriatechnology.com ---"
curl -sf https://canal.patriatechnology.com/
echo "--- goldenkingdom ---"
curl -sf -o /dev/null -w 'gk %{http_code}\n' https://goldenkingdom.patriatechnology.com/
systemctl is-active tiffany-canal
ss -ltnp | grep 47314 || true
