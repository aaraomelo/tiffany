#!/usr/bin/env python3
"""O avanço do corpus: conceitos novos entram no cristal (ordem da mesa,
14/08 noite: «vamos avançar com o corpus»).

    tools/cristal_avanca.py cristal/avanco_X.jsonl            # insere
    tools/cristal_avanca.py cristal/avanco_X.jsonl --desfaz   # remove

Regras (o contrato 𝓜 da casa, aplicado à ferramenta):
- todo registo novo tem o ESQUEMA completo do cristal, JSON canónico
  (sort_keys, sem espaços) — um registo não-canónico é RECUSADO;
- o id tem de ser NOVO (não colidir com o cristal) e único no lote;
- meta.fonte == "tiffany" obrigatório: distingue o que NASCEU aqui do
  corpus recuperado do jornal — a contagem da curadoria
  ((conceitos − tiffany) + fusões == 4286) depende disto;
- a inserção mantém a ordem canónica por id;
- --desfaz remove exatamente os ids do lote e devolve o ficheiro
  anterior BYTE A BYTE (a volta é medida por tests/cristal_avanco.js).
"""
import json
import sys

CAMPOS = ["arestas", "confianca", "contraexemplos", "descricao", "epistemico",
          "exemplos", "id", "memoria", "meta", "origem", "palavras_chave",
          "sinonimos", "tipo", "titulo"]
CRISTAL = "cristal/cristal.jsonl"


def canonico(r):
    return json.dumps(r, ensure_ascii=False, sort_keys=True, separators=(",", ":"))


def le(caminho):
    with open(caminho, encoding="utf-8") as f:
        return [l.rstrip("\n") for l in f if l.strip()]


def main():
    if len(sys.argv) < 2:
        sys.exit("uso: cristal_avanca.py <lote.jsonl> [--desfaz]")
    lote_caminho = sys.argv[1]
    desfaz = "--desfaz" in sys.argv[2:]

    lote = le(lote_caminho)
    cristal = le(CRISTAL)
    ids_cristal = set()
    for l in cristal:
        ids_cristal.add(json.loads(l)["id"])

    ids_lote = []
    for l in lote:
        r = json.loads(l)
        if sorted(r.keys()) != CAMPOS:
            sys.exit(f"RECUSADO: esquema errado em {r.get('id','?')} — "
                     f"campos {sorted(r.keys())}")
        if canonico(r) != l:
            sys.exit(f"RECUSADO: JSON não-canónico em {r['id']}")
        if r.get("meta", {}).get("fonte") != "tiffany":
            sys.exit(f"RECUSADO: meta.fonte != 'tiffany' em {r['id']} — "
                     "o que nasce aqui declara-o")
        ids_lote.append(r["id"])
    if len(set(ids_lote)) != len(ids_lote):
        sys.exit("RECUSADO: id duplicado dentro do lote")

    if desfaz:
        alvo = set(ids_lote)
        fora = [l for l in cristal if json.loads(l)["id"] not in alvo]
        removidos = len(cristal) - len(fora)
        if removidos != len(alvo):
            sys.exit(f"RECUSADO: o lote pede {len(alvo)} remoções e o "
                     f"cristal só tem {removidos} desses ids")
        with open(CRISTAL, "w", encoding="utf-8") as f:
            for l in fora:
                f.write(l + "\n")
        print(f"desfeito: {removidos} removidos, {len(fora)} ficam")
        return

    colisao = set(ids_lote) & ids_cristal
    if colisao:
        sys.exit(f"RECUSADO: id já existe no cristal: {sorted(colisao)[:3]}")
    todos = sorted(cristal + lote, key=lambda l: json.loads(l)["id"])
    with open(CRISTAL, "w", encoding="utf-8") as f:
        for l in todos:
            f.write(l + "\n")
    print(f"avançado: {len(lote)} novos, cristal com {len(todos)} conceitos")


if __name__ == "__main__":
    main()
