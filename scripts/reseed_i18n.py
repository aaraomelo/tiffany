#!/usr/bin/env python3
"""
Re-seeda a tabela i18n_strings a partir dos JSON bundle do frontend.

Uso (no host patria):
  python3 reseed_i18n.py \
    --pt /root/patria-app-repo/src/i18n_v2/strings/pt.json \
    --en /root/patria-app-repo/src/i18n_v2/strings/en.json \
    --out /tmp/i18n_reseed.sql

  docker cp /tmp/i18n_reseed.sql postgres-prod:/tmp/i18n_reseed.sql
  docker exec postgres-prod psql -U patria -d patria -f /tmp/i18n_reseed.sql

Depois invalidar cache do controller:
  docker restart patria-api
"""

import argparse
import json
import sys


def esc(s: str) -> str:
    return s.replace("'", "''")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--pt", required=True, help="caminho do pt.json bundle")
    ap.add_argument("--en", required=True, help="caminho do en.json bundle")
    ap.add_argument("--out", required=True, help="caminho do SQL gerado")
    args = ap.parse_args()

    pt = json.load(open(args.pt))
    en = json.load(open(args.en))

    rows = []
    for lang, d in [("pt", pt), ("en", en)]:
        for k, v in d.items():
            rows.append(f"('{lang}','{esc(k)}','{esc(v)}',NOW())")

    with open(args.out, "w") as f:
        f.write("BEGIN;\n")
        f.write("TRUNCATE i18n_strings RESTART IDENTITY;\n")
        f.write("INSERT INTO i18n_strings (lang, key, value, updated_at) VALUES\n")
        f.write(",\n".join(rows))
        f.write(";\nCOMMIT;\n")

    print(f"wrote {args.out}: pt={len(pt)} en={len(en)} rows={len(rows)}")


if __name__ == "__main__":
    main()
