# Scripts ops

Scripts utilitários pra operação do patria-nco (seed de papers, i18n, análise de bench, restauração de ckpts).

## seed_papers.js

Popula tabelas `Paper`, `PaperI18n`, `PaperFile` a partir de metadados hardcoded + PDFs/TeX em `/app/data/papers/`. Idempotente (upsert). Roda dentro do container patria-api.

```bash
docker cp scripts/seed_papers.js patria-api:/app/scripts/seed_papers.js
docker exec patria-api node /app/scripts/seed_papers.js
```

Quando subir um paper novo, editar `seedPapers[]` no topo do arquivo, depositar os arquivos `<slug>.{pdf,tex}` em `/app/data/papers/` (PT por padrão) e re-rodar.

## upsert_paper_file.js

Upsert genérico de uma versão traduzida de PDF/TeX. Útil pra subir EN depois que o seed inicial cobriu PT.

```bash
docker cp lopes_en.pdf patria-api:/tmp/lopes_en.pdf
docker exec patria-api node /app/scripts/upsert_paper_file.js \
  --slug lopes --lang en --kind pdf --src /tmp/lopes_en.pdf
```

## reseed_i18n.py

Re-seeda `i18n_strings` a partir dos JSONs bundle do frontend (`pt.json` e `en.json`). Gera SQL idempotente (TRUNCATE + INSERT). Útil quando adicionou chaves novas no bundle que ainda não estão no banco.

```bash
python3 scripts/reseed_i18n.py \
  --pt /root/patria-app-repo/src/i18n_v2/strings/pt.json \
  --en /root/patria-app-repo/src/i18n_v2/strings/en.json \
  --out /tmp/i18n_reseed.sql

docker cp /tmp/i18n_reseed.sql postgres-prod:/tmp/i18n_reseed.sql
docker exec postgres-prod psql -U patria -d patria -f /tmp/i18n_reseed.sql
docker restart patria-api  # invalida cache em memória do I18nController
```

## bench_peak_analysis.py

Analisa `bench-evolution` e identifica step do peak histórico via média móvel. Decide se vale restaurar ckpt anterior (regressão > 1% no recente vs peak).

```bash
curl -s https://nco.patriatechnology.com/api/nco/bench-evolution > /tmp/bench.json
python3 scripts/bench_peak_analysis.py /tmp/bench.json
```

## Restauração de ckpt (procedimento manual)

Quando o bench acusa regressão e tem snapshot do peak:

1. **Identifica peak** com `bench_peak_analysis.py`.
2. **Localiza snapshot** no GEX44 em `/home/claude/nco_paperp/ckpts_v4_<n>_hiper_perlayer_s1*/step<NNNN>.pt`.
3. **Backup atual** em `/root/patria-nco/ckpts/hiper_v4_<n>.pt.bak-pre-restore-<data>`.
4. **Copia snapshot pra produção**: substitui `/root/patria-nco/ckpts/hiper_v4_<n>.pt`.
5. **Ancora no GEX44** copiando o mesmo snapshot pra `ckpts_v5_<n>_hiper_perlayer_s1/latest.pt` — o cron `sync_nco_ckpts.sh` vai detectar hash igual e não sobrescrever.
6. **Restart** o NCO server: `systemctl restart patria-nco`.
7. **Smoke test** via `POST /api/nco/maxcut` com `model=hiper_<n>`.
8. **Trigger bench** pra confirmar ganho na curva: `POST /api/nco/bench/trigger` com header `X-Admin-Token`.
